#include "common.h"
#include "matrix_utils.h"
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#define TEMP_FILE_PATTERN "sparse_temp_%d.txt"

void worker_process(const Matrix *matrix, int start_row, int end_row) {
    int count = count_non_zeros_in_range(matrix, start_row, end_row);

    if (count <= 254) {
        exit(count);
    } else {
        // Write to temp file if count is too large for exit status
        char filename[256];
        sprintf(filename, TEMP_FILE_PATTERN, getpid());
        FILE *file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%d", count);
            fclose(file);
        }
        exit(255);
    }
}

int main(int argc, char *argv[]) {
    ProgramArgs args;
    if (!parse_args(argc, argv, &args)) {
        fprintf(stderr, "Invalid arguments\n");
        return EXIT_FAILURE;
    }

    Matrix *matrix = load_matrix_from_file(args.filename, args.rows, args.cols);
    if (!matrix) {
        return EXIT_FAILURE;
    }

    // Calculate rows per process
    int rows_per_process = args.rows / args.workers;
    int extra_rows = args.rows % args.workers;

    pid_t pids[args.workers];
    int start_row = 0;

    for (int i = 0; i < args.workers; i++) {
        int end_row = start_row + rows_per_process - 1;
        if (i < extra_rows) {
            end_row++;
        }

        pids[i] = fork();
        if (pids[i] == 0) {
            // Child process
            worker_process(matrix, start_row, end_row);
            exit(EXIT_SUCCESS); // Should never reach here
        } else if (pids[i] < 0) {
            perror("fork failed");
            free_matrix(matrix);
            return EXIT_FAILURE;
        }

        start_row = end_row + 1;
    }

    // Parent process - collect results
    int total_non_zeros = 0;
    for (int i = 0; i < args.workers; i++) {
        int status;
        waitpid(pids[i], &status, 0);

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == 255) {
                // Read from temp file
                char filename[256];
                sprintf(filename, TEMP_FILE_PATTERN, pids[i]);
                FILE *file = fopen(filename, "r");
                if (file) {
                    int count;
                    fscanf(file, "%d", &count);
                    fclose(file);
                    remove(filename);
                    total_non_zeros += count;
                }
            } else {
                total_non_zeros += exit_code;
            }
        }
    }

    int total_elements = args.rows * args.cols;
    int zero_count = total_elements - total_non_zeros;
    int actual_percentage = (zero_count * 100) / total_elements;

    printf("The matrix in the file %s has a total of %d zeros (%d%%). ", 
           args.filename, zero_count, actual_percentage);
    if (actual_percentage >= args.percentage) {
        printf("Therefore, it IS considered sparse.\n");
    } else {
        printf("Therefore, it is NOT considered sparse.\n");
    }

    free_matrix(matrix);
    return EXIT_SUCCESS;
}

bool parse_args(int argc, char *argv[], ProgramArgs *args) {
    memset(args, 0, sizeof(ProgramArgs));

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            args->rows = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            args->cols = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            strncpy(args->filename, argv[++i], 255);
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            args->workers = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            args->percentage = atoi(argv[++i]);
        }
    }

    // Validate arguments
    if (args->rows <= 0 || args->cols <= 0 || args->workers <= 0 || 
        args->percentage < 0 || args->percentage > 100 || 
        strlen(args->filename) == 0) {
        return false;
    }

    return true;
}