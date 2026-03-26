// Se modifica el programa main_1
// Para que ahora la multiplicacion de matrices se realice
// usando hilos. Por cada celda de la matriz
// resultante se crea un pthread que se encarga de calcular su valor

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// se debe crear un struct para pasar los datos a los hilos
typedef struct {
    long start_row; // fila inicial del bloque de filas a multiplicar
    long end_row;   // fila final del bloque de filas a multiplicar
    long m;          // tamaño de la matriz resultante (m x m)
    long n;          // tamaño de la matriz intermedia (m x n)
    long *matrizMN;  // matriz MN (m x n)
    long *matrizNM;  // matriz NM (n x m)
    long *matrizResultante; // matriz resultante (m x m)
} ThreadData;       // el struct se llama ThreadData y se usará para pasar los datos a cada hilo

void llenar_matriz(long *matriz, long n, long m) {
    for (long i = 0; i < n; i++) {
        for (long j = 0; j < m; j++) {
            matriz[i * m + j] = rand() % 5 + 1;
        }
    }
}

void* multiplicar(void* arg) {
    ThreadData *data = (ThreadData*)arg;
    
    for (long fila = data->start_row; fila < data->end_row; fila++) {
        for (long col = 0; col < data->m; col++) {
            long sum = 0;
            for (long k = 0; k < data->n; k++) {
                sum += data->matrizMN[fila * data->n + k] * data->matrizNM[k * data->m + col];
            }
            data->matrizResultante[fila * data->m + col] = sum;
        }
    }

    return NULL;

}


int main(int argc, char** argv) {
    struct timeval start, end;

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <N> <M>\n", argv[0]);
        return 1;
    }

    int cores = (int)sysconf(_SC_NPROCESSORS_ONLN); // obtener cantidad de núcleos disponibles
    if (cores < 1) {
        cores = 1;
    }

    long n = atol(argv[1]);
    long m = atol(argv[2]);
    if (n <= 0 || m <= 0) {
        fprintf(stderr, "N y M deben ser mayores a 0\n");
        return 1;
    }

    long num_threads = (long)cores * 2; // cantidad de hilos a usar, se puede ajustar según el tamaño de la matriz
    if (num_threads > m) {
        num_threads = m;
    }
    if (num_threads < 1) {
        num_threads = 1;
    }

    printf("Cores disponibles: %d\n", cores);
    printf("Se usarán %ld hilos\n", num_threads);

    long *matrizNM = malloc(n * m * sizeof(long));
    long *matrizMN = malloc(m * n * sizeof(long));
    long *matrizMM = malloc(m * m * sizeof(long));

    if (matrizNM == NULL || matrizMN == NULL || matrizMM == NULL) {
        fprintf(stderr, "Error alocando memoria\n");
        free(matrizNM);
        free(matrizMN);
        free(matrizMM);
        return 1;
    }

    srand(time(NULL));

    double tiempos[100];

    for (int ciclo = 0; ciclo < 100; ciclo++) {

        gettimeofday(&start, NULL);

        llenar_matriz(matrizNM, n, m);
        llenar_matriz(matrizMN, m, n);

        pthread_t threads[num_threads]; // se define cantidad de hilos a usar
        ThreadData thread_data[num_threads]; // se define un arreglo de ThreadData para pasar los datos a cada hilo

        long filas_base = m / num_threads; // cantidad de filas que cada hilo debe procesar
        long sobrantes = m % num_threads; // filas que sobran después de dividir entre los hilos, se asignarán a los primeros 'sobrantes' hilos
        long fila_actual = 0;

        for (long i = 0; i < num_threads; i++) {
            long extra = (i < sobrantes) ? 1 : 0;
            thread_data[i].start_row = fila_actual;
            thread_data[i].end_row = fila_actual + filas_base + extra;
            thread_data[i].m = m;
            thread_data[i].n = n;
            thread_data[i].matrizMN = matrizMN;
            thread_data[i].matrizNM = matrizNM;
            thread_data[i].matrizResultante = matrizMM;

            if (pthread_create(&threads[i], NULL, multiplicar, &thread_data[i]) != 0) {
                fprintf(stderr, "Error creando hilo %ld\n", i);
                return 1;
            }

            fila_actual = thread_data[i].end_row;
        }

        for (long i = 0; i < num_threads; i++) { // se espera a que todos los hilos terminen para juntar los resultados
            if (pthread_join(threads[i], NULL) != 0) {
                fprintf(stderr, "Error esperando hilo %ld\n", i);
                return 1;
            }
        }

        gettimeofday(&end, NULL);

        printf("Ciclo %d: Tiempo de ejecución: %f segundos\n", ciclo + 1, (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0);
        
        tiempos[ciclo] = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    }

    free(matrizNM);
    free(matrizMN);
    free(matrizMM);


    // Tiempo promedio
    double suma = 0.0;
    for (int i = 0; i < 100; i++)
        suma += tiempos[i];
    printf("Tiempo promedio: %f segundos\n", suma / 100.0);

    // Desviación estándar
    double promedio = suma / 100.0;
    double suma_desviacion = 0.0;
    for (int i = 0; i < 100; i++)
        suma_desviacion += pow(tiempos[i] - promedio, 2);
    printf("Desviación estándar: %f segundos\n", sqrt(suma_desviacion / 100.0));

}
