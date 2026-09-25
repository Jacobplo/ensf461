#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int* read_next_line(FILE* fin) {
    // TODO: This function reads the next line from the input file
    // The line is a comma-separated list of integers
    // Return the list of integers as an array where the first element
    // is the number of integers in the rest of the array
    // Return NULL if there are no more lines to read
    
    int* ret = NULL;

    char *line = NULL;
    size_t size = 0;
    ssize_t len = 0;

    if ((len = getline(&line, &size, fin)) != -1) {
        ret = malloc(128 * sizeof(int));
        size_t count = 0;

        char *token = strtok(line, ",");
        while (token != NULL) {
            ret[count + 1] = atoi(token);
            count++;

            token = strtok(0, ",");
        };

        ret[0] = count;

        ret = realloc(ret, (count + 1) * sizeof(int));
    }

    if (line != NULL) {
        free(line);
    }

    return ret;
}


float compute_average(int* line) {
    // TODO: Compute the average of the integers in the vector
    // Recall that the first element of the vector is the number of integers
    
    float sum = 0;

    size_t len = line[0];
    for (size_t i = 1; i < len + 1; i++) {
        sum += line[i]; 
    }

    return sum / len;
}


float compute_stdev(int* line) {
    // TODO: Compute the standard deviation of the integers in the vector
    // Recall that the first element of the vector is the number of integers
    
    float mean = compute_average(line);

    float sum_of_squares = 0;

    size_t len = line[0];
    for (size_t i = 1; i < len + 1; i++) {
        sum_of_squares += powf(line[i] - mean, 2);
    }

    return sqrt(sum_of_squares / len);
}
