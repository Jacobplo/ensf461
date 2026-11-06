#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

/**
 * Page table entry
 */
typedef struct pte_s {
  int pfn;
  bool valid;
} pte_s;

typedef struct pcb_s {
  int r1;
  int r2;
  pte_s *page_table;
  bool defined;
} pcb_s;

pcb_s processes[4];
int current_pid = 0;

// Output file
FILE *output_file;

// TLB replacement strategy (FIFO or LRU)
char *strategy;

char **tokenize_input(char *input) {
  char **tokens = NULL;
  char *token = strtok(input, " ");
  int num_tokens = 0;

  while (token != NULL) {
    num_tokens++;
    tokens = realloc(tokens, num_tokens * sizeof(char *));
    tokens[num_tokens - 1] = malloc(strlen(token) + 1);
    strcpy(tokens[num_tokens - 1], token);
    token = strtok(NULL, " ");
  }

  num_tokens++;
  tokens = realloc(tokens, num_tokens * sizeof(char *));
  tokens[num_tokens - 1] = NULL;

  return tokens;
}

int main(int argc, char *argv[]) {
  const char usage[] =
      "Usage: memsym.out <strategy> <input trace> <output trace>\n";
  char *input_trace;
  char *output_trace;
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
  FILE *input_file = fopen(input_trace, "r");
  output_file = fopen(output_trace, "w");


  while (!feof(input_file)) {
    // Read input file line by line
    char *rez = fgets(buffer, sizeof(buffer), input_file);
    if (!rez) {
      fprintf(stderr, "Reached end of trace. Exiting...\n");
      return -1;
    } else {
      // Remove endline character
      buffer[strlen(buffer) - 1] = '\0';
    }
    char **tokens = tokenize_input(buffer);

    // TODO: Implement your memory simulator
   
    // Ignore comments
    if (tokens[0][0] == '%') continue;

    if (strcmp(tokens[0], "define") == 0) {
      if (processes[current_pid].defined) {
        fprintf(output_file, "Current PID: %d. Error: multiple calls to define in the same trace\n", current_pid);
      }
      else {
        int off_bits = atoi(tokens[1]);
        int pfn_bits = atoi(tokens[2]);
        int vpn_bits = atoi(tokens[3]);

        fprintf(output_file, "Current PID: %d. Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n", current_pid, off_bits, pfn_bits, vpn_bits);

        processes[current_pid].page_table = malloc(pow(2, vpn_bits) * sizeof(pte_s));
        processes[current_pid].page_table->valid = false; 
        processes[current_pid].defined = true;
      } 
    }

    // Deallocate tokens
    for (int i = 0; tokens[i] != NULL; i++)
      free(tokens[i]);
    free(tokens);
  }

  // Close input and output files
  fclose(input_file);
  fclose(output_file);

  return 0;
}
