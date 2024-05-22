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
    int p;
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

    // Imprimir el número de hilo y la cantidad de procesos realizados
    printf("Thread %d processed %d elements\n", t_args->thread_id, t_args->end - t_args->start);

    // Abrir archivo para escribir los datos procesados por este hilo
    char filename[20];
    sprintf(filename, "data_thread_%d.txt", t_args->start); // Nombre del archivo basado en el índice inicial del hilo
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error al abrir el archivo.\n");
        exit(1);
    }

    // Escribir los datos procesados por este hilo en el archivo
    for (int i = t_args->start; i < t_args->end; i++) {
        fprintf(file, "%.6f\n", t_args->Y[i]);
    }

    // Cerrar el archivo
    fclose(file);

    return NULL;
}

int main(int argc, char* argv[]){
    // Variables para el cálculo SAXPY y la medición de tiempo
    int p = 10000000;
    int num_threads;
    double* X;
    double a;
    double* Y;
    struct timeval t_start, t_end;
    double exec_time;

    // Obtener el número de hilos del usuario
    printf("Ingrese el número de hilos: ");
    scanf("%d", &num_threads);

    // Validar el número de hilos
    if (num_threads <= 0) {
        fprintf(stderr, "Número de hilos inválido. Debe ser un entero positivo.\n");
        return 1;
    }

    // Inicializar las variables X, Y y a
    X = (double*) malloc(sizeof(double) * p);
    Y = (double*) malloc(sizeof(double) * p);
    a = (double) rand() / RAND_MAX;

    for(int i = 0; i < p; i++){
        X[i] = (double)rand() / RAND_MAX;
        Y[i] = (double)rand() / RAND_MAX;
    }

    // Crear estructuras de datos para almacenar hilos
    pthread_t threads[num_threads];
    struct ThreadArgs thread_args[num_threads];

    // Dividir el trabajo entre los hilos de manera proporcional al número total de elementos y el número de hilos
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
        thread_args[i].p = p;
        thread_args[i].thread_id = i;

        // Crear hilo
        pthread_create(&threads[i], NULL, saxpy_thread, (void*)&thread_args[i]);

        // Actualizar el punto de inicio para el próximo hilo
        start = end;
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Calcular el tiempo de ejecución total
    gettimeofday(&t_start, NULL);
    gettimeofday(&t_end, NULL);
    exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
    exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms

    // Imprimir resultados
    printf("Execution time: %f ms \n", exec_time);
    printf("Last 3 values of Y: %f, %f, %f \n", Y[p-3], Y[p-2], Y[p-1]);
    printf("Number of threads: %d\n", num_threads);

    // Liberar memoria
    free(X);
    free(Y);

    return 0;
}
