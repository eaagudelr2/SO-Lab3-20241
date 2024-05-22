#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>

// Estructura para pasar argumentos a cada hilo
struct ThreadArgs {
    int start;
    int end;
    double* X;
    double* Y;
    double a;
    double exec_time;
    int thread_id;
};

// Función ejecutada por cada hilo
void* saxpy_thread(void* args) {
    struct ThreadArgs* t_args = (struct ThreadArgs*)args;

    struct timeval t_start, t_end;
    gettimeofday(&t_start, NULL);

    // Realizar SAXPY para la porción de datos asignada al hilo
    for (int i = t_args->start; i < t_args->end; i++) {
        t_args->Y[i] = t_args->Y[i] + t_args->a * t_args->X[i];
    }

    gettimeofday(&t_end, NULL);
    t_args->exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
    t_args->exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms

    return NULL;
}

void run_saxpy(int num_threads, int p, double* X, double* Y, double a) {
    pthread_t threads[num_threads];
    struct ThreadArgs thread_args[num_threads];

    int chunk_size = p / num_threads;
    int remainder = p % num_threads;
    int start = 0;

    for (int i = 0; i < num_threads; i++) {
        int extra = (i < remainder) ? 1 : 0;
        int end = start + chunk_size + extra;
        thread_args[i].start = start;
        thread_args[i].end = end;
        thread_args[i].X = X;
        thread_args[i].Y = Y;
        thread_args[i].a = a;
        thread_args[i].thread_id = i;

        pthread_create(&threads[i], NULL, saxpy_thread, (void*)&thread_args[i]);
        start = end;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char* argv[]) {
    int p = 50000000;
    double* X;
    double a;
    double* Y;
    struct timeval t_start, t_end;
    double exec_time;
    int thread_counts[] = {2, 4, 8, 16};

    X = (double*) malloc(sizeof(double) * p);
    Y = (double*) malloc(sizeof(double) * p);
    a = (double) rand() / RAND_MAX;

    for(int i = 0; i < p; i++){
        X[i] = (double)rand() / RAND_MAX;
        Y[i] = (double)rand() / RAND_MAX;
    }

    FILE* results_file = fopen("execution_times.txt", "w");
    if (results_file == NULL) {
        fprintf(stderr, "Error al abrir el archivo de resultados.\n");
        return 1;
    }

    for (int i = 0; i < 4; i++) {
        int num_threads = thread_counts[i];
        gettimeofday(&t_start, NULL);
        run_saxpy(num_threads, p, X, Y, a);
        gettimeofday(&t_end, NULL);

        exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
        exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms

        printf("Execution time with %d threads: %f ms\n", num_threads, exec_time);
        fprintf(results_file, "%d %f\n", num_threads, exec_time);
    }

    fclose(results_file);

    printf("Last 3 values of Y: %f, %f, %f\n", Y[p-3], Y[p-2], Y[p-1]);

    free(X);
    free(Y);

    return 0;
}
