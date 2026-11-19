#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

// table entery
typedef struct {
    int pfn;
    int valid;
} pte_t;

// TLB entry
typedef struct {
    int pfn;
    int vpn;
    int pid;
    int valid;
    uint32_t timestamp;
} tlb_entry_t;

// saved registers of the process
typedef struct{
    uint32_t r1;
    uint32_t r2;
} pcb_t;

uint32_t* memory;
pte_t* page_tables[4];
tlb_entry_t tlb[8];
pcb_t processes[4];

int current_pid = 0;
int defined = FALSE;
int off_bits;
int pfn_bits;
int vpn_bits;
uint32_t timestamp = 0;


// Output file
FILE* output_file;

// TLB replacement strategy (FIFO or LRU)
char* strategy;

char** tokenize_input(char* input) {
    char** tokens = NULL;
    char* token = strtok(input, " ");
    int num_tokens = 0;

    while (token != NULL) {
        num_tokens++;
        tokens = realloc(tokens, num_tokens * sizeof(char*));
        tokens[num_tokens - 1] = malloc(strlen(token) + 1);
        strcpy(tokens[num_tokens - 1], token);
        token = strtok(NULL, " ");
    }

    num_tokens++;
    tokens = realloc(tokens, num_tokens * sizeof(char*));
    tokens[num_tokens - 1] = NULL;

    return tokens;
}

// Pointer return to register r1 or r2 of current process based on the name provided
// NULL if name is invalid
uint32_t* get_register(char* name){
    if(strcmp(name, "r1") == 0){
        return &processes[current_pid].r1;
    }
    if(strcmp(name, "r2") == 0){
        return &processes[current_pid].r2;
    }
    return NULL;
}

// Pick a TLB slot: the first invalid one, otherwise the one with the oldest timestamp
int pick_tlb_slot(){
    for(int i = 0; i<8; i++){
        if (tlb[i].valid == FALSE){
            return i;
        }
    }
    int oldest = 0;
    for(int i = 1; i<8; i++){
        if(tlb[i].timestamp < tlb[oldest].timestamp){
            oldest = i;
        }
    }
    return oldest;
}

// Insert a mapping into the TLB, reusing the existing entry if this VPN is already there
void tlb_insert(int vpn, int pfn){
    int slot = -1;

    for(int i = 0; i<8; i++){
        if(tlb[i].valid == TRUE && tlb[i].pid == current_pid && tlb[i].vpn == vpn){
            slot = i;
        }
    }
    if(slot == -1){
        slot = pick_tlb_slot();
    }
    tlb[slot].vpn = vpn;
    tlb[slot].pfn = pfn;
    tlb[slot].pid = current_pid;
    tlb[slot].valid = TRUE;
    tlb[slot].timestamp = timestamp;
}
// Translate virtual address to physical address
// -1 if page not mapped
int translate(int va){
    int shift = off_bits;
    int offset_mask = (1 << off_bits) - 1;
    int vpn = va >> shift;
    int offset = va & offset_mask;
    int pfn = -1;

    // Check TLB for translation
    for(int i = 0; i<8; i++){
        if(tlb[i].valid == TRUE && tlb[i].pid == current_pid && tlb[i].vpn == vpn){
            pfn = tlb[i].pfn;
            if(strcmp(strategy, "LRU") == 0){
                tlb[i].timestamp = timestamp;
            }
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d hit in TLB entry %d. PFN is %d\n", current_pid, vpn, i, pfn);
            break;
        }
    }

    // If not found in TLB, check page table
    if(pfn == -1){
        fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d caused a TLB miss\n", current_pid, vpn);
        if(page_tables[current_pid][vpn].valid == FALSE){
            fprintf(output_file, "Current PID: %d. Translating. Translation for VPN %d not found in page table\n", current_pid, vpn);
            return -1;
        }
        pfn = page_tables[current_pid][vpn].pfn;
        fprintf(output_file, "Current PID: %d. Translating. Successfully mapped VPN %d to PFN %d\n", current_pid, vpn, pfn);
        tlb_insert(vpn, pfn);
    }

    // Calculate physical address
    return (pfn << shift) | offset;
}



int main(int argc, char* argv[]) {
    const char usage[] = "Usage: memsym.out <strategy> <input trace> <output trace>\n";
    char* input_trace;
    char* output_trace;
    char buffer[1024];

    // Parse command line arguments
    if (argc != 4) {
        printf("%s", usage);
        return 1;
    }
    strategy = argv[1];
    input_trace = argv[2];
    output_trace = argv[3];

    // Open input and output files
    FILE* input_file = fopen(input_trace, "r");
    output_file = fopen(output_trace, "w");  

    while ( !feof(input_file) ) {
        // Read input file line by line
        char *rez = fgets(buffer, sizeof(buffer), input_file);
        if ( !rez ) {
            fprintf(stderr, "Reached end of trace. Exiting...\n");
            return -1;
        } else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }
        char** tokens = tokenize_input(buffer);

        if (tokens[0] == NULL || tokens[0][0] == '%'){
            // do nothing
        }
        else if (strcmp(tokens[0], "define") == 0){
            if(defined == TRUE){
                fprintf(output_file, "Current PID: %d. Error: multiple calls to define in the same trace\n", current_pid);
                fclose(output_file);
                return 1;
            }
            off_bits = atoi(tokens[1]);
            pfn_bits = atoi(tokens[2]);
            vpn_bits = atoi(tokens[3]);

            int mem_size = 1 << (off_bits +pfn_bits);
            memory = (uint32_t*) malloc(mem_size * sizeof(uint32_t));
            for(int i = 0; i<mem_size; i++){
                memory[i] = 0;
            }

            int num_pages = 1 << vpn_bits;
            for(int i = 0; i<4; i++){
                page_tables[i] = (pte_t*) malloc(num_pages * sizeof(pte_t));
                for(int j = 0; j<num_pages; j++){
                    page_tables[i][j].pfn = 0;
                    page_tables[i][j].valid = FALSE;
                }
            }

            // TLB: 8 entries which arent valid
            for(int i = 0; i<8; i++){
                tlb[i].vpn = 0;
                tlb[i].pfn = 0;
                tlb[i].pid = 0;
                tlb[i].valid = FALSE;
                tlb[i].timestamp = 0;
            }
            
            defined = TRUE;
            fprintf(output_file, "Current PID: %d. Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n", current_pid, off_bits, pfn_bits, vpn_bits);
        }
        else if (defined == FALSE){
            fprintf(output_file, "Current PID: %d. Error: attempt to execute instruction before define\n", current_pid);
            fclose(output_file);
            return 1;
        }
        else if(strcmp(tokens[0], "ctxswitch") == 0){
            int pid = atoi(tokens[1]);
            if(pid < 0 || pid > 3){
                fprintf(output_file, "Current PID: %d. Invalid context switch to process %d\n", current_pid, pid);
                fclose(output_file);
                return 1;
            }
            current_pid = pid;
            fprintf(output_file, "Current PID: %d. Switched execution context to process: %d\n", current_pid, pid);
        }
        else if (strcmp(tokens[0], "load") == 0) {
            uint32_t* dst = get_register(tokens[1]);
            if (dst == NULL) {
                fprintf(output_file, "Current PID: %d. Error: invalid register operand %s\n", current_pid, tokens[1]);
                fclose(output_file);
                return 1;
            }
            if (tokens[2][0] == '#') {
                *dst = atoi(tokens[2] + 1);
                fprintf(output_file, "Current PID: %d. Loaded immediate %s into register %s\n", current_pid, tokens[2] + 1, tokens[1]);
            }
            else {
                int pa = translate(atoi(tokens[2]));
                if (pa == -1) {
                    fclose(output_file);
                    return 1;
                }
                *dst = memory[pa];
                fprintf(output_file, "Current PID: %d. Loaded value of location %s (%u) into register %s\n", current_pid, tokens[2], *dst, tokens[1]);
            }
        }
        else if (strcmp(tokens[0], "add") == 0) {
            uint32_t a = processes[current_pid].r1;
            uint32_t b = processes[current_pid].r2;
            processes[current_pid].r1 = a + b;
            fprintf(output_file, "Current PID: %d. Added contents of registers r1 (%u) and r2 (%u). Result: %u\n", current_pid, a, b, a + b);
        }
        else if(strcmp(tokens[0], "map") == 0){
            int vpn = atoi(tokens[1]);
            int pfn = atoi(tokens[2]);
            page_tables[current_pid][vpn].pfn = pfn;
            page_tables[current_pid][vpn].valid = TRUE;
            tlb_insert(vpn, pfn);
            fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", current_pid, vpn, pfn);
        }
        else if (strcmp(tokens[0], "unmap") == 0){
            int vpn = atoi(tokens[1]);
            page_tables[current_pid][vpn].pfn = 0;
            page_tables[current_pid][vpn].valid = FALSE;
            for(int i = 0; i<8; i++){
                if(tlb[i].valid == TRUE && tlb[i].pid == current_pid && tlb[i].vpn == vpn){
                    tlb[i].valid = FALSE;
                }
            }
            fprintf(output_file, "Current PID: %d. Unmapped virtual page number %d\n", current_pid, vpn);
        }
        else if(strcmp(tokens[0], "store") == 0){
            uint32_t value;
            if(tokens[2][0] == '#'){
                value = atoi(tokens[2]+1);
            }
            else{
                uint32_t* src = get_register(tokens[2]);
                if(src == NULL){
                    fprintf(output_file, "Current PID: %d. Error: invalid register operand %s\n", current_pid, tokens[2]);
                    fclose(output_file);
                    return 1;
                }
                value = *src;
            }
            int pa = translate(atoi(tokens[1]));
            if(pa == -1){
                fclose(output_file);
                return 1;
            }
            memory[pa] = value;
            if(tokens[2][0] == '#'){
                fprintf(output_file, "Current PID: %d. Stored immediate %s into location %s\n", current_pid, tokens[2] + 1, tokens[1]);
            }
            else{
                fprintf(output_file, "Current PID: %d. Stored value of register %s (%u) into location %s\n", current_pid, tokens[2], value, tokens[1]);
            }
        }
        else if (strcmp(tokens[0], "rinspect") == 0) {
            uint32_t* reg = get_register(tokens[1]);
            if (reg == NULL) {
                fprintf(output_file, "Current PID: %d. Error: invalid register operand %s\n", current_pid, tokens[1]);
                fclose(output_file);
                return 1;
            }
            fprintf(output_file, "Current PID: %d. Inspected register %s. Content: %u\n", current_pid, tokens[1], *reg);
        }
        else if (strcmp(tokens[0], "pinspect") == 0) {
            int vpn = atoi(tokens[1]);
            fprintf(output_file, "Current PID: %d. Inspected page table entry %d. Physical frame number: %d. Valid: %d\n", current_pid, vpn, page_tables[current_pid][vpn].pfn, page_tables[current_pid][vpn].valid);
        }
        else if (strcmp(tokens[0], "tinspect") == 0) {
            int i = atoi(tokens[1]);
            fprintf(output_file, "Current PID: %d. Inspected TLB entry %d. VPN: %d. PFN: %d. Valid: %d. PID: %d. Timestamp: %u\n", current_pid, i, tlb[i].vpn, tlb[i].pfn, tlb[i].valid, tlb[i].pid, tlb[i].timestamp);
        }
        else if (strcmp(tokens[0], "linspect") == 0) {
            int pl = atoi(tokens[1]);
            fprintf(output_file, "Current PID: %d. Inspected physical location %d. Value: %u\n", current_pid, pl, memory[pl]);
        }

        // Deallocate tokens
        for (int i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
        timestamp++;
    }

    // Close input and output files
    fclose(input_file);
    fclose(output_file);

    return 0;
}
