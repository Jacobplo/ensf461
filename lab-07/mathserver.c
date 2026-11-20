#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

typedef enum op_type_t {
    SET,
    ADD,
    SUB,
    MUL,
    DIV,
    PRI,
    PIA,
    FIB,
    NONE,
} op_type_t;

typedef struct op_node_t {
    op_type_t type;
    int operand;
    struct op_node_t *next;
} op_node_t;

typedef struct ctx_t {
    int id;
    pthread_t thread;
    int val;
    op_node_t *head;
    op_node_t *tail;
} ctx_t;

#define NUM_CTX 16

static ctx_t contexts[NUM_CTX];
static FILE *output_file;

void init_contexts(void);

op_type_t parse_op_type(const char *str);

void enqueue(ctx_t *context, op_type_t type, int operand);

op_node_t dequeue(ctx_t *context);

void *ctx_worker(void *arg);

void append_log(const char *str);

bool is_prime(int num);

int fib(int n);

int main(int argc, char* argv[]) {
    const char usage[] = "Usage: mathserver.out <input trace> <output trace>\n";
    char* input_trace;
    char* output_trace;
    char buffer[1024];

    init_contexts();

    // Parse command line arguments
    if (argc != 3) {
        printf("%s", usage);
        return 1;
    }

    input_trace = argv[1];
    output_trace = argv[2];

    // Open input and output files
    FILE *input_file = fopen(input_trace, "r");

    while (!feof(input_file)) {
        // Read input file line by line
        char *rez = fgets(buffer, sizeof(buffer), input_file);
        if (!rez)
            break;
        else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }

        // Get operation type
        char *token = strtok(rez, " ");
        op_type_t op = parse_op_type(token);

        // Get context
        token = strtok(NULL, " ");
        size_t ctx_idx = atoi(token);

        // Get operand, if applicable
        size_t operand;
        if (op < PRI) {
            token = strtok(NULL, " ");
            operand = atoi(token);
        }

        enqueue(&contexts[ctx_idx], op, operand);
    }
    fclose(input_file);

    output_file = fopen(output_trace, "w");

    // Dispatch threads.
    for (size_t i = 0; i < NUM_CTX; i++) {
        pthread_create(&contexts[i].thread, NULL, ctx_worker, &contexts[i].id);
    }

    // Wait for thread completion.
    for (size_t i = 0; i < NUM_CTX; i++) {
        pthread_join(contexts[i].thread, NULL);
    }

    fclose(output_file);

    return 0;
}

void init_contexts(void) {
    for (size_t i = 0; i < NUM_CTX; i++) {
        contexts[i].id = i;
        contexts[i].val = 0;
        contexts[i].head = NULL;
        contexts[i].tail = NULL;
    } 
}

op_type_t parse_op_type(const char *str) {
    if (strncmp(str, "set", 3) == 0) {
        return SET;       
    }
    else if (strncmp(str, "add", 3) == 0) {
        return ADD;
    }
    else if (strncmp(str, "sub", 3) == 0) {
        return SUB;
    }
    else if (strncmp(str, "mul", 3) == 0) {
        return MUL;
    }
    else if (strncmp(str, "div", 3) == 0) {
        return DIV;
    }
    else if (strncmp(str, "pri", 3) == 0) {
        return PRI;
    }
    else if (strncmp(str, "pia", 3) == 0) {
        return PIA;
    }
    else if (strncmp(str, "fib", 3) == 0) {
        return FIB;
    }

    return NONE;
}

void enqueue(ctx_t *context, op_type_t type, int operand) {
    op_node_t *prev_tail = context->tail;

    context->tail = malloc(sizeof(op_node_t));
    context->tail->type = type;
    context->tail->operand = operand;
    context->tail->next = NULL;

    if (prev_tail != NULL) {
        prev_tail->next = context->tail;
    }

    if (context->head == NULL) {
        context->head = context->tail;
    }
}

op_node_t dequeue(ctx_t *context) {
    if (context->head == NULL) {
        op_node_t ret = { .type = NONE };
        return ret;
    }

    op_node_t *next = context->head->next;
    
    op_node_t ret = *context->head;
    
    free(context->head);

    context->head = next;

    if (context->head == NULL) {
        context->tail = NULL;
    }

    return ret;
}

void *ctx_worker(void *arg) {
    int id = *(int *)arg;

    ctx_t *ctx = &contexts[id];

    char log_buf[1024] = { 0 };
    int logs_buffered = 0;
    char log_builder[128];

    while (ctx->head != NULL) {
        op_node_t op = dequeue(ctx);

        int num;
        switch (op.type) {
            case SET:
                ctx->val = op.operand;
                snprintf(log_builder, 128, "ctx %02d: set to value %d\n", id, op.operand);
                break;

            case ADD:
                ctx->val += op.operand;
                snprintf(log_builder, 128, "ctx %02d: add %d (result: %d)\n", id, op.operand, ctx->val);
                break;

            case SUB:
                ctx->val -= op.operand;
                snprintf(log_builder, 128, "ctx %02d: sub %d (result: %d)\n", id, op.operand, ctx->val);
                break;

            case MUL:
                ctx->val *= op.operand;
                snprintf(log_builder, 128, "ctx %02d: mul %d (result: %d)\n", id, op.operand, ctx->val);
                break;

            case DIV:
                ctx->val /= op.operand;
                snprintf(log_builder, 128, "ctx %02d: div %d (result: %d)\n", id, op.operand, ctx->val);
                break;

            case PRI:
                break;

            case PIA:
                break;

            case FIB:
                num = fib(ctx->val); 
                snprintf(log_builder, 128, "ctx %02d: fib (result: %d)\n", id, num);
                break;

            default:
            case NONE:
                break;
        }

        logs_buffered++;
        strcat(log_buf, log_builder);

        if (logs_buffered >= 10) {
            append_log(log_buf);
            log_buf[0] = 0;
            logs_buffered = 0;
        }
    }

    if (logs_buffered > 0) {
        append_log(log_buf);
    }

    return NULL;
}

void append_log(const char *str) {
    static pthread_mutex_t m;
    static bool initialized = false;
    
    if (!initialized) {
        pthread_mutex_init(&m, NULL);
        initialized = true;
    }

    pthread_mutex_lock(&m);

    fputs(str, output_file);

    pthread_mutex_unlock(&m);
}

bool is_prime(int num) {
    for (int i = 2; i < num; i++) {
        if (num % i == 0) {
            return false;
        }
    }

    return true;
}

int fib(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;

    return fib(n - 1) + fib(n - 2);
}
