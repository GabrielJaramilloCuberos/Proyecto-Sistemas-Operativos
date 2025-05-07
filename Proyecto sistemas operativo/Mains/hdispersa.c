#include "common.h"
#include "matrix_utils.h"
#include <pthread.h>

typedef struct {
    const Matrix *matrix;
    int start_row;
    int end_row;
    int non_zero_count;
} ThreadData;

void* count_non_zeros_thread(void *arg) {
    ThreadData *data = (ThreadData*)arg;
    data->non_zero_count = count_non_zeros_in_range(data->matrix, data->start_row, data->end_row);
    return (void*)(long)data->non_zero_count;
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

    // Calculate rows per thread
    int rows_per_thread = args.rows / args.workers;
    int extra_rows = args.rows % args.workers;

    pthread_t threads[args.workers];
    ThreadData thread_data[args.workers];
    int start_row = 0;

    for (int i = 0; i < args.workers; i++) {
        thread_data[i].matrix = matrix;
        thread_data[i].start_row = start_row;
        thread_data[i].end_row = start_row + rows_per_thread - 1;

        if (i < extra_rows) {
            thread_data[i].end_row++;
        }

        pthread_create(&threads[i], NULL, count_non_zeros_thread, &thread_data[i]);
        start_row = thread_data[i].end_row + 1;
    }

    // Wait for all threads to complete
    int total_non_zeros = 0;
    for (int i = 0; i < args.workers; i++) {
        void *retval;
        pthread_join(threads[i], &retval);
        total_non_zeros += (int)(long)retval;
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