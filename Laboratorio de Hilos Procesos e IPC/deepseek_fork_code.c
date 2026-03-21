// This code was generate using DeepSeek, giving as reference main_1.c code

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
    long start_row;
    long end_row;
    long n, m;
    long *matrizMN;
    long *matrizNM;
    long *matrizResultante;
} WorkUnit;

long* llenar_matriz_mmap(long n, long m, size_t size) {
    long *matriz = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                        MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    
    if (matriz == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        return NULL;
    }

    for (long i = 0; i < n; i++) {
        for (long j = 0; j < m; j++) {
            matriz[i * m + j] = rand() % 5 + 1;
        }
    }
    return matriz;
}

void multiply_row_range(WorkUnit *wu) {
    for (long i = wu->start_row; i < wu->end_row; i++) {
        for (long j = 0; j < wu->m; j++) {
            long sum = 0;
            for (long k = 0; k < wu->n; k++) {
                sum += wu->matrizMN[i * wu->n + k] * wu->matrizNM[k * wu->m + j];
            }
            wu->matrizResultante[i * wu->m + j] = sum;
        }
    }
}

int main(int argc, char** argv) {
    struct timeval start, end;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <n> <m>\n", argv[0]);
        fprintf(stderr, "Where: n = rows of first matrix, m = columns of first matrix/rows of second matrix\n");
        return 1;
    }

    double tiempos[100];

    long n = atoi(argv[1]);
    long m = atoi(argv[2]);

    // Get number of CPU cores for optimal process count
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    long max_processes = num_cores * 2;  // Use 2x number of cores for I/O waiting
    
    printf("System has %ld CPU cores, using %ld processes\n", num_cores, max_processes);
    
    if (m > 1000) {
        printf("Warning: Large matrix (%ldx%ld) may take significant time\n", m, m);
    }

    size_t size_nm = n * m * sizeof(long);
    size_t size_mn = m * n * sizeof(long);
    size_t size_result = m * m * sizeof(long);

    long *matrizNM = NULL;
    long *matrizMN = NULL;
    long *matrizResultante = NULL;

    srand(time(NULL)); 
    
    for (int ciclo = 0; ciclo < 100; ciclo++) {
        printf("Cycle %d/100\n", ciclo + 1);
        fflush(stdout);
        
        // Llenar matrices usando mmap
        matrizNM = llenar_matriz_mmap(n, m, size_nm);
        matrizMN = llenar_matriz_mmap(m, n, size_mn);
        
        // Asignar memoria para la matriz resultante usando mmap
        matrizResultante = mmap(NULL, size_result, PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_SHARED, -1, 0);
        
        if (matrizNM == NULL || matrizMN == NULL || matrizResultante == MAP_FAILED) {
            fprintf(stderr, "Error allocating memory for matrices using mmap\n");
            if (matrizNM != NULL && matrizNM != MAP_FAILED) munmap(matrizNM, size_nm);
            if (matrizMN != NULL && matrizMN != MAP_FAILED) munmap(matrizMN, size_mn);
            if (matrizResultante != NULL && matrizResultante != MAP_FAILED) munmap(matrizResultante, size_result);
            return 1;
        }

        // Initialize result matrix to 0
        for (long i = 0; i < m * m; i++) {
            matrizResultante[i] = 0;
        }

        gettimeofday(&start, NULL);
        
        // Calculate work distribution
        long rows_per_process = (m + max_processes - 1) / max_processes;
        long actual_processes = (m + rows_per_process - 1) / rows_per_process;
        
        pid_t pids[actual_processes];
        WorkUnit work_units[actual_processes];
        
        for (long p = 0; p < actual_processes; p++) {
            work_units[p].start_row = p * rows_per_process;
            work_units[p].end_row = (p + 1) * rows_per_process;
            if (work_units[p].end_row > m) work_units[p].end_row = m;
            work_units[p].n = n;
            work_units[p].m = m;
            work_units[p].matrizMN = matrizMN;
            work_units[p].matrizNM = matrizNM;
            work_units[p].matrizResultante = matrizResultante;
            
            pid_t p_id = fork();
            
            if (p_id == 0) {
                // Child process
                multiply_row_range(&work_units[p]);
                exit(0);
            } else if (p_id > 0) {
                pids[p] = p_id;
            } else {
                fprintf(stderr, "Fork failed at cycle %d\n", ciclo + 1);
                perror("fork");
                // Clean up existing child processes
                for (long k = 0; k < p; k++) {
                    if (pids[k] > 0) {
                        kill(pids[k], SIGTERM);
                        waitpid(pids[k], NULL, 0);
                    }
                }
                munmap(matrizNM, size_nm);
                munmap(matrizMN, size_mn);
                munmap(matrizResultante, size_result);
                return 1;
            }
        }
        
        // Wait for all child processes
        for (long p = 0; p < actual_processes; p++) {
            int status;
            waitpid(pids[p], &status, 0);
        }

        gettimeofday(&end, NULL);
        double tiempo = (double)(end.tv_sec - start.tv_sec) + 
                       (double)(end.tv_usec - start.tv_usec) / 1000000.0;
        tiempos[ciclo] = tiempo;
        printf("  Tiempo para ciclo %d: %f segundos (usando %ld procesos)\n", 
               ciclo + 1, tiempo, actual_processes);
        
        // Optional verification for first cycle
        if (ciclo == 0 && m <= 10) {
            printf("Result matrix sample (first 5x5):\n");
            for (long i = 0; i < 5 && i < m; i++) {
                for (long j = 0; j < 5 && j < m; j++) {
                    printf("%ld ", matrizResultante[i * m + j]);
                }
                printf("\n");
            }
        }

        munmap(matrizNM, size_nm);
        munmap(matrizMN, size_mn);
        munmap(matrizResultante, size_result);
    }

    // Calculate statistics
    double suma = 0.0;
    for (int i = 0; i < 100; i++) {
        suma += tiempos[i];
    }
    double promedio = suma / 100.0;
    printf("\n=== RESULTADOS ===\n");
    printf("Tiempo promedio: %f segundos\n", promedio);

    double desviacion = 0.0;
    for (int i = 0; i < 100; i++) {
        desviacion += pow(tiempos[i] - promedio, 2);
    }
    desviacion = sqrt(desviacion / 100.0);
    printf("Desviación estándar: %f segundos\n", desviacion);
    
    // Calculate best and worst times
    double min_time = tiempos[0], max_time = tiempos[0];
    for (int i = 1; i < 100; i++) {
        if (tiempos[i] < min_time) min_time = tiempos[i];
        if (tiempos[i] > max_time) max_time = tiempos[i];
    }
    printf("Mejor tiempo: %f segundos\n", min_time);
    printf("Peor tiempo: %f segundos\n", max_time);

    return 0;
}
