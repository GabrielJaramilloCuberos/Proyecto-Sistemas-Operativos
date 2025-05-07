#include "matrix_utils.h"
#include <stdio.h>
#include <stdlib.h>

Matrix* create_matrix(int rows, int cols) {
    Matrix *matrix = malloc(sizeof(Matrix));
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->data = malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix->data[i] = malloc(cols * sizeof(int));
    }
    return matrix;
}

void free_matrix(Matrix *matrix) {
    for (int i = 0; i < matrix->rows; i++) {
        free(matrix->data[i]);
    }
    free(matrix->data);
    free(matrix);
}

Matrix* load_matrix_from_file(const char *filename, int rows, int cols) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    Matrix *matrix = create_matrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (fscanf(file, "%d", &matrix->data[i][j]) != 1) {
                fprintf(stderr, "Error reading matrix data\n");
                fclose(file);
                free_matrix(matrix);
                return NULL;
            }
        }
    }
    fclose(file);
    return matrix;
}

int count_non_zeros_in_range(const Matrix *matrix, int start_row, int end_row) {
    int count = 0;
    for (int i = start_row; i <= end_row; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            if (matrix->data[i][j] != 0) {
                count++;
            }
        }
    }
    return count;
}