#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

/*

Básicamente a diferencia del primer codigo main_1
En este se crean hilos para cada bloque de filas de la matriz resultante

Por ejemplo, si se recibe una matriz de 6x7 para dar un ejemplo sencillo de entender
y se decide usar 4 hilos, cada hilo se encargará de multiplicar un bloque de filas de la matriz resultante.
- Hilo 1: Multiplica las filas 0 y 1 de la matriz resultante.
- Hilo 2: Multiplica las filas 2 y 3 de la matriz resultante.
- Hilo 3: Multiplica la fila 4 de la matriz resultante.
- Hilo 4: Multiplica la fila 5 de la matriz resultante.

Esta distribucion permite que cada hilo trabaje en paralelo en su bloque de filas, lo que puede mejorar el 
rendimiento al aprovechar múltiples núcleos de CPU. Además, se asegura de que no haya conflictos entre los hilos, 
ya que cada uno trabaja en un bloque de filas distinto de la matriz resultante.

*/


// struct para pasar datos a los hilos
typedef struct {
    long start_row;
    long end_row;
    long m;          
    long n;          
    long *matrizMN;  
    long *matrizNM; 
    long *matrizResultante; 
} ThreadData;

long* llenar_matriz(long n, long m) {
    long *matriz = malloc(n * m * sizeof(long));
    if (matriz == NULL) {
        return NULL;
    }

    for (long i = 0; i < n; i++) {
        for (long j = 0; j < m; j++) {
            matriz[i * m + j] = rand() % 5 + 1;
        }
    }
    return matriz;
}

// Hilo para multiplicar un bloque de filas de la matriz resultante
void* multiplicar(void* arg) {
    ThreadData *data = (ThreadData*)arg;
    
    for (long i = data->start_row; i < data->end_row; i++) {
        for (long j = 0; j < data->m; j++) {
            long sum = 0;
            for (long k = 0; k < data->n; k++) {
                sum += data->matrizMN[i * data->n + k] * data->matrizNM[k * data->m + j];
            }
            data->matrizResultante[i * data->m + j] = sum;
        }
    }
    
    return NULL;
}

int main(int argc, char** argv) {
    struct timeval start, end;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_1> <number_2>\n", argv[0]);
        return 1;
    }

    long n = atoi(argv[1]);
    long m = atoi(argv[2]);
    
    // Se determina el número de hilos a usar, basado en el número de núcleos disponibles
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN); // cuantos nucleos tiene el CPU
    if (num_threads > m) num_threads = m; // No usar más hilos que filas
    
    long *matrizNM = NULL;
    long *matrizMN = NULL;
    long *matrizResultante = NULL;

    double tiempos[100];

    srand(time(NULL)); 
    
    for (int ciclo = 0; ciclo < 100; ciclo++) { // 100 ciclos
        matrizNM = llenar_matriz(n, m);
        matrizMN = llenar_matriz(m, n);
        matrizResultante = malloc(m * m * sizeof(long));

        if (matrizNM == NULL || matrizMN == NULL || matrizResultante == NULL) {
            fprintf(stderr, "Error allocating memory for matrices\n");
            return 1;
        }

        gettimeofday(&start, NULL);
        
        // creacion de threads
        pthread_t threads[num_threads];
        ThreadData thread_data[num_threads];
        
        long rows_per_thread = m / num_threads;
        long remaining_rows = m % num_threads;
        long current_row = 0;
        
        for (int t = 0; t < num_threads; t++) {
            long rows_for_this_thread = rows_per_thread + (t < remaining_rows ? 1 : 0);
            
            thread_data[t].start_row = current_row;
            thread_data[t].end_row = current_row + rows_for_this_thread;
            thread_data[t].m = m;
            thread_data[t].n = n;
            thread_data[t].matrizMN = matrizMN;
            thread_data[t].matrizNM = matrizNM;
            thread_data[t].matrizResultante = matrizResultante;
            
            pthread_create(&threads[t], NULL, multiplicar, &thread_data[t]);
            
            current_row += rows_for_this_thread;
        }
        
        // esperar a que lo hilos terminen para juntarlos
        for (int t = 0; t < num_threads; t++) {
            pthread_join(threads[t], NULL);
        }
        
        gettimeofday(&end, NULL);
        double tiempo = (double)(end.tv_sec - start.tv_sec) + 
                       (double)(end.tv_usec - start.tv_usec) / 1000000.0;
        tiempos[ciclo] = tiempo;
        printf("Tiempo para ciclo %d: %f segundos\n", ciclo + 1, tiempo);
    }
    
    free(matrizNM);
    free(matrizMN);
    free(matrizResultante);

    // Calcular el tiempo promedio    
    double suma = 0.0;
    for (int i = 0; i < 100; i++) {
        suma += tiempos[i];
    }
    double promedio = suma / 100.0;
    printf("Tiempo promedio: %f segundos\n", promedio);

    // Calcular la desviación estándar
    double desviacion = 0.0;
    for (int i = 0; i < 100; i++) {
        desviacion += pow(tiempos[i] - promedio, 2);
    }
    desviacion = sqrt(desviacion / 100.0);
    printf("Desviación estándar: %f segundos\n", desviacion);

    return 0;
}