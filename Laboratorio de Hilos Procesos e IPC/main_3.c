/*

Adaptacion del codigo main_1.c 
Utilizando MMAP en lugar de Malloc
Este codigo comparte las matrices entre procesos
Para esto, se requiere utlizar fork

*/

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

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

    long *matrizNM = mmap(
                        NULL,                      // dirección (el SO decide)
                        n * m * sizeof(long),      // tamaño
                        PROT_READ | PROT_WRITE,    // permisos
                        MAP_SHARED | MAP_ANONYMOUS, // memoria sin archivo, compartida entre procesos
                        -1,                        // no file descriptor
                        0                          // offset
                    );
    long *matrizMN = mmap(NULL, m * n * sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    long *matrizMM = mmap(NULL, m * m * sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    srand(time(NULL));

    double tiempos[100];

    int cores = (int)sysconf(_SC_NPROCESSORS_ONLN); // obtener cantidad de núcleos disponibles
    if (cores < 1) {
        cores = 1;
    }

    long filas_por_proceso = m / cores; // cantidad de filas que cada proceso debe procesar que seria m entre cores

    for (int ciclo = 0; ciclo < 100; ciclo++) {

        gettimeofday(&start, NULL);

        llenar_matriz(matrizNM, n, m);
        llenar_matriz(matrizMN, m, n);

        if (matrizNM == MAP_FAILED || matrizMN == MAP_FAILED || matrizMM == MAP_FAILED) {
            fprintf(stderr, "Error de MMAP\n");
            return 1;
        }

        for (int p = 0; p < cores; p++) {
            pid_t pid = fork();

            if (pid == 0) {
                // por cada proceso hijo, se asigna un bloque de filas de la matriz resultante para calcular
                long inicio = p * filas_por_proceso;
                long fin;

                if (p == cores - 1) { // el último proceso se encarga de las filas restantes
                    fin = m;
                } else {
                    fin = (p + 1) * filas_por_proceso;
                }

                for (long fila = inicio; fila < fin; fila++) {
                    for (long col = 0; col < m; col++) {
                        long sum = 0;
                        for (long k = 0; k < n; k++) {
                            sum += matrizMN[fila * n + k] * matrizNM[k * m + col];
                        }
                    matrizMM[fila * m + col] = sum;
                    }   
                }
            exit(0);
            }
        }

        for (int i = 0; i < cores; i++) {
            wait(NULL);
        }

        gettimeofday(&end, NULL);

        printf("Ciclo %d: Tiempo de ejecución: %f segundos\n", ciclo + 1, (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0);
        
        tiempos[ciclo] = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    }

    munmap(matrizNM, n * m * sizeof(long));
    munmap(matrizMN, m * n * sizeof(long));
    munmap(matrizMM, m * m * sizeof(long));

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
