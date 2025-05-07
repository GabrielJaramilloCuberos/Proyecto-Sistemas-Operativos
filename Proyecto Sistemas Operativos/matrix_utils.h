#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include "common.h"

// Structure to represent a matrix
typedef struct {
    int rows;
    int cols;
    int **data;
} Matrix;

// Matrix functions
Matrix* create_matrix(int rows, int cols);
void free_matrix(Matrix *matrix);
Matrix* load_matrix_from_file(const char *filename, int rows, int cols);
int count_non_zeros_in_range(const Matrix *matrix, int start_row, int end_row);

#endif // MATRIX_UTILS_H