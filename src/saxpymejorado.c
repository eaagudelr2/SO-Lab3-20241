#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>

#define VECTOR_SIZE 10000000
#define MAX_THREADS 16

// Estructura para pasar argumentos a los hilos
typedef struct {
    double* X;
    double* Y;
    double a;
    int start_index;
    int end_index;
} ThreadArgs;

// Función de operación SAXPY
void* saxpy(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for (int i = args->start_index; i < args->end_index; i++) {
        args->Y[i] = args->Y[i] + args->a * args->X[i];
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Variables para obtener los parámetros de la línea de comandos
    unsigned int seed = 1;
    int p = VECTOR_SIZE;
    int n_threads = 2;
    int max_iters = 1000;

    // Obtener valores de entrada
    int opt;
    while ((opt = getopt(argc, argv, ":p:s:n:i:")) != -1) {
        switch(opt) {
            case 'p':
                p = atoi(optarg);
                assert(p > 0 && p <= VECTOR_SIZE);
                break;
            case 's':
                seed = atoi(optarg);
                break;
            case 'n':
                n_threads = atoi(optarg);
                assert(n_threads > 0 && n_threads <= MAX_THREADS);
                break;
            case 'i':
                max_iters = atoi(optarg);
                break;
            case ':':
                printf("La opción -%c necesita un valor\n", optopt);
                break;
            case '?':
                fprintf(stderr, "Uso: %s [-p <tamaño del vector>] [-s <semilla>] [-n <número de hilos>] [-i <máximo de iteraciones>]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Inicializar datos
    srand(seed);
    double* X = (double*)malloc(sizeof(double) * p);
    double* Y = (double*)malloc(sizeof(double) * p);
    double* Y_avgs = (double*)malloc(sizeof(double) * max_iters);
    double a = (double)rand() / RAND_MAX;

    // Inicializar vectores X e Y
    for (int i = 0; i < p; i++) {
        X[i] = (double)rand() / RAND_MAX;
        Y[i] = (double)rand() / RAND_MAX;
    }

    // Crear hilos
    pthread_t threads[MAX_THREADS];
    ThreadArgs args[MAX_THREADS];
    int section_size = p / n_threads;
    for (int i = 0; i < n_threads; i++) {
        args[i].X = X;
        args[i].Y = Y;
        args[i].a = a;
        args[i].start_index = i * section_size;
        args[i].end_index = (i == n_threads - 1) ? p : (i + 1) * section_size;
        pthread_create(&threads[i], NULL, saxpy, (void*)&args[i]);
    }

    printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n", p, seed, n_threads, max_iters);

    // Tomar tiempo inicial
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Esperar la finalización de los hilos
    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Tomar tiempo final
    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0;  // sec to ms
    elapsed_time += (end.tv_usec - start.tv_usec) / 1000.0; // us to ms
    printf("Tiempo de ejecución: %.2f ms\n", elapsed_time);

    // Imprimir resultados
    printf("Últimos 3 valores de Y: %.6f, %.6f, %.6f\n", Y[p - 3], Y[p - 2], Y[p - 1]);

    // Liberar memoria
    free(X);
    free(Y);
    free(Y_avgs);

    return 0;
}
