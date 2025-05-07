#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

// Structure to hold command line arguments
typedef struct {
    int rows;
    int cols;
    char filename[256];
    int workers;
    int percentage;
} ProgramArgs;

// Function to parse command line arguments
bool parse_args(int argc, char *argv[], ProgramArgs *args);

#endif // COMMON_H