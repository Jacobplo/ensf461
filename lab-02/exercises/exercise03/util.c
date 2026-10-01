#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int* read_next_line(FILE* fin) {
    int* ret = NULL;

    char *line = NULL;
    size_t size = 0;
    ssize_t len = 0;

    // Get next line in the stream 
    if ((len = getline(&line, &size, fin)) != -1) {
        // Allocate an initial buffer of 128 integers
        ret = malloc(128 * sizeof(int));
        size_t count = 0;

        // Tokenize the line, splitting at commas
        char *token = strtok(line, ",");
        while (token != NULL) {
            // Parse elements as integers and insert into the new array
            ret[count + 1] = atoi(token);
            count++;

            // Get next token
            token = strtok(0, ",");
        };

        ret[0] = count;

        // Reallocate the buffer based on used size
        ret = realloc(ret, (count + 1) * sizeof(int));
    }

    if (line != NULL) {
        free(line);
    }

    return ret;
}


float compute_average(int* line) {
    if (line[0] == 0) return 0.0;
    
    float sum = 0;

    size_t len = line[0];
    for (size_t i = 1; i < len + 1; i++) {
        sum += line[i]; 
    }

    return sum / len;
}


float compute_stdev(int* line) {
    if (line[0] == 0) return 0.0;
    
    float mean = compute_average(line);

    float sum_of_squares = 0;

    size_t len = line[0];
    for (size_t i = 1; i < len + 1; i++) {
        sum_of_squares += powf(line[i] - mean, 2);
    }

    return sqrt(sum_of_squares / len);
}
