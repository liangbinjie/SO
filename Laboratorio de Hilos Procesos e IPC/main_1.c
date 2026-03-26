#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

void llenar_matriz(long *matriz, long n, long m) {
    for (long i = 0; i < n; i++) {
        for (long j = 0; j < m; j++) {
            matriz[i * m + j] = rand() % 5 + 1;
        }
    }
}

int main(int argc, char** argv) {
    struct timeval start, end;

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <N> <M>\n", argv[0]);
        return 1;
    }

    long n = atoi(argv[1]);
    long m = atoi(argv[2]);

    long *matrizNM = malloc(n * m * sizeof(long));
    long *matrizMN = malloc(m * n * sizeof(long));
    long *matrizMM = malloc(m * m * sizeof(long));

    srand(time(NULL));

    double tiempos[100];



    for (int ciclo = 0; ciclo < 100; ciclo++) {

        gettimeofday(&start, NULL);

        llenar_matriz(matrizNM, n, m);
        llenar_matriz(matrizMN, m, n);

        if (matrizNM == NULL || matrizMN == NULL || matrizMM == NULL) {
            fprintf(stderr, "Error alocando memoria\n");
            return 1;
        }

        for (long fila = 0; fila < m; fila++) {
            for (long col = 0; col < m; col++) {
                matrizMM[fila * m + col] = 0;
                for (long k = 0; k < n; k++) {
                    matrizMM[fila * m + col] += matrizMN[fila * n + k] * matrizNM[k * m + col];
                }
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
