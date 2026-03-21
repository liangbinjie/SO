#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

long* llenar_matriz_mmap(long n, long m, size_t size) {
    // Usamos mmap para asignar memoria
    long *matriz = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                        MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    
    if (matriz == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        return NULL;
    }

    // Llenamos la matriz con valores aleatorios entre 1 y 5
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

    size_t size_nm = n * m * sizeof(long);
    size_t size_mn = m * n * sizeof(long);
    size_t size_result = m * m * sizeof(long);

    // Punteros para las matrices
    long *matrizNM = NULL;
    long *matrizMN = NULL;
    long *matrizResultante = NULL;

    srand(time(NULL)); 
    
    for (int ciclo = 0; ciclo < 100; ciclo++) { // 100 ciclos
        // Llenar matrices usando mmap
        matrizNM = llenar_matriz_mmap(n, m, size_nm);
        matrizMN = llenar_matriz_mmap(m, n, size_mn);
        
        // Asignar memoria para la matriz resultante usando mmap
        matrizResultante = mmap(NULL, size_result, PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        
        if (matrizNM == NULL || matrizMN == NULL || matrizResultante == MAP_FAILED) {
            fprintf(stderr, "Error allocating memory for matrices using mmap\n");
            if (matrizNM != NULL && matrizNM != MAP_FAILED) munmap(matrizNM, size_nm);
            if (matrizMN != NULL && matrizMN != MAP_FAILED) munmap(matrizMN, size_mn);
            if (matrizResultante != NULL && matrizResultante != MAP_FAILED) munmap(matrizResultante, size_result);
            return 1;
        }

        for (long i = 0; i < m * m; i++) {
            matrizResultante[i] = 0;
        }

        gettimeofday(&start, NULL);
        
        for (long i = 0; i < m; i++) {
            for (long j = 0; j < m; j++) {
                long sum = 0;
                for (long k = 0; k < n; k++) {
                    sum += matrizMN[i * n + k] * matrizNM[k * m + j];
                }
                matrizResultante[i * m + j] = sum;
            }
        }

        gettimeofday(&end, NULL);
        double tiempo = (double)(end.tv_sec - start.tv_sec) + 
                       (double)(end.tv_usec - start.tv_usec) / 1000000.0;
        tiempos[ciclo] = tiempo;
        printf("Tiempo para ciclo %d: %f segundos\n", ciclo + 1, tiempo);
        

        munmap(matrizNM, size_nm);
        munmap(matrizMN, size_mn);
        munmap(matrizResultante, size_result);
    }

    // tiempo promedio
    double suma = 0.0;
    for (int i = 0; i < 100; i++) {
        suma += tiempos[i];
    }
    double promedio = suma / 100.0;
    printf("Tiempo promedio: %f segundos\n", promedio);

    // desviación estándar
    double desviacion = 0.0;
    for (int i = 0; i < 100; i++) {
        desviacion += pow(tiempos[i] - promedio, 2);
    }
    desviacion = sqrt(desviacion / 100.0);
    printf("Desviación estándar: %f segundos\n", desviacion);

    return 0;
}