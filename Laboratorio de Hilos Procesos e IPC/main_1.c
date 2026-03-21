#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

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

int main(int argc, char** argv) {
    struct timeval start, end;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_1> <number_2>\n", argv[0]);
        return 1;
    }

    double tiempos[100];

    long n = atoi(argv[1]);
    long m = atoi(argv[2]);

    long *matrizNM = malloc(n * m * sizeof(long));
    long *matrizMN = malloc(m * n * sizeof(long));
    long *matrizResultante = malloc(m * m * sizeof(long));

    srand(time(NULL)); // Inicializar la semilla para números aleatorios
    
    for (int ciclo = 0; ciclo < 100; ciclo++) { // 100 ciclos
        matrizNM = llenar_matriz(n, m);
        matrizMN = llenar_matriz(m, n);

        if (matrizNM == NULL || matrizMN == NULL || matrizResultante == NULL) {
            fprintf(stderr, "Error allocating memory for matrices\n");
            return 1;
        }

        gettimeofday(&start, NULL);
        // Crear matriz resultante MxM
        for (long i = 0; i < m; i++) {
            for (long j = 0; j < m; j++) {
                matrizResultante[i * m + j] = 0;
                for (long k = 0; k < n; k++) {
                    matrizResultante[i * m + j] += matrizMN[i * n + k] * matrizNM[k * m + j];
                }
            }
        }

        gettimeofday(&end, NULL);
        double tiempo = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000.0;
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