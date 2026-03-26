# Multiplicaciones de matrices - Lab de Hilos Procesos e IPC

En este laboratorio se solicitó crear un programa que se generará dos matrices, de tamaños `NxM` y `MxN`, con celdas conteniendo una generación aleatoria de números del 1 al 5. 

Para luego, multiplicar ambas matrices y que quede una matriz resultante de tamaño `MxM`.

Este proceso de generación de matrices aleatorias y luego generar una multiplicación se debe repetir por 100 veces, cada ciclo se debe tomar el tiempo de procesado utilizando `gettimeofday`.

Para esta primera parte, se hizo lo siguiente:

Se inicializaron punteros apuntando a 3 matrices con los respectivos tamaños discutidos anteriormente usando Malloc. Malloc es una función de C que sirve para alocar memoria, o mejor dicho, reservar una cantidad específica de memoria en el heap al momento de ejecución (Aleksandar Haber PhD, 2024).

Las primeras matrices se llenan con la siguiente función:
```c
void llenar_matriz(long *matriz, long n, long m) {
    for (long i = 0; i < n; i++) {
        for (long j = 0; j < m; j++) {
            matriz[i * m + j] = rand() % 5 + 1;
        }
    }
}
```
Esta función tiene una complejidad de `O(n*m)`, pues tiene que llenar `n * m` espacios por cada matriz.

Luego, podemos empezar con la multiplicación de ambas matrices. Para esto hay que multiplicar la `matrizMN` por la `matrizNM` para que quede resultante una `matriz MxM`.

Esta multiplicación se realiza de la siguiente manera:
```c
for (long fila = 0; fila < m; fila++) {
    for (long col = 0; col < m; col++) {
        matrizMM[fila * m + col] = 0;
        for (long k = 0; k < n; k++) {
            matrizMM[fila * m + col] += matrizMN[fila * n + k] * matrizNM[k * m + col];
        }
    }
}
```
Donde cada celda se calcula sumando las multiplicaciones de cada elemento de la fila y columna en la que se encuentra. Es decir, si se quiere obtener la celda de la fila=2, columna=3, se va a multiplicar elemento k de la fila 2 por el elemento k de la columna 3, para luego sumar esos productos.

Esto tiene una complejidad de `O(m^2 * n)`, donde `m` es la cantidad de `M` que nosotros ingresamos, se debe realizar dos pasadas, por eso el elevado a la dos, y multiplicamos por `n` elementos que tiene la fila o columna.

Dicho ejercicio, se obtuvieron los siguientes resultados después de 100 pasados:
### Imagen 1
Se obtuvo un promedio de 3.139240 segundos con una desviación estándar de 0.064637 segundos

## Parte Dos
Para la parte dos, se establece que hay que calcular cada celda utilizando hilos. Para esto, C nos proporciona `pthread`. 

Para usar hilos, `pthread` necesita recibir como argumentos el hilo, una función, y el argumento, donde el argumento es básicamente los datos con los que vamos a trabajar. 

Como el hilo sólo recibe un argumento, tendremos que hacer un struct para pasar los datos necesarios al thread.

```c
typedef struct {
    long start_row; // fila inicial del bloque de filas a multiplicar
    long end_row;   // fila final del bloque de filas a multiplicar
    long m;          // tamaño de la matriz resultante (m x m)
    long n;          // tamaño de la matriz intermedia (m x n)
    long *matrizMN;  // matriz MN (m x n)
    long *matrizNM;  // matriz NM (n x m)
    long *matrizResultante; // matriz resultante (m x m)
} ThreadData;       // el struct se llama ThreadData y se usará para pasar los datos a cada hilo
```

Por lo anterior, se debe crear una función que produzca las celdas resultantes:
```c
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
```
Esta función recibe el struct para acceder a los campos requeridos.

En este "approach" se obtiene la cantidad de núcleos que tiene nuestro procesador, para luego con esa cantidad, crear 2 hilos por núcleo.

Con eso en mente, se va a procesar `M/hilos` filas por hilo.

El último hilo va a procesar las filas sobrantes.

```c
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
```

Este ejercicio tuvo los siguientes resultados:

### Imagen 2

Una duración promedio de 0.764123 segundos por ciclo, 0.040118 segundos de desviación estándar.

## IPC (Comunicación entre Procesos)
Los pipes permiten que dos o más procesos se comuniquen al crear un canal unidireccional o bidireccional entre ellos. Los pipes son básicamente un tubo que permite una comunicación virtual donde los datos se pueden transferir en una sola dirección o en ambas. El sistema lo maneja como una fila FIFO. Entre sus ventajas está su simplicidad y flexibilidad, ya que son una forma directa y simple de implementar para que los procesos se comuniquen (TutorialsPoint, s. f.; GeeksforGeeks, 2025).

Otras ventajas son la eficiencia, pues permite la transmisión de datos a gran velocidad, siendo un método confiable, ya que si un dato presenta un error, este no podrá ser enviado (GeeksforGeeks, 2025).

Pero también tienen desventajas: poseen una capacidad limitada, ya que existe un máximo de datos que pueden transferir entre procesos. En pipes unidireccionales, un solo proceso puede enviar datos a la vez, y en pipes bidireccionales se requiere sincronización entre procesos. Además, presentan limitaciones en escalabilidad, por lo que no son adecuados para sistemas distribuidos (TutorialsPoint, s. f.; GeeksforGeeks, 2025).

Mmap es un mecanismo que permite crear un mapeo de un archivo o una región de memoria directamente en el espacio de direcciones virtual del proceso que la llama. Con esto, los datos pueden tratarse como si fueran un arreglo en memoria, sin necesidad de usar llamadas read() o write(). Es una herramienta útil para trabajar con grandes volúmenes de datos donde se requiere alto rendimiento (Arya, s. f.).

Entre sus ventajas está el acceso eficiente a los datos, ya que estos se encuentran directamente en memoria. Además, su implementación no es tan compleja y permite modificaciones concurrentes (Tencent Cloud, 2025).

Algunas desventajas incluyen que, en ciertos casos de entrada/salida con archivos mapeados en memoria, puede ser más lento que usar read y write tradicionales. También depende de arquitecturas que cuenten con una unidad de manejo de memoria (MMU) (GeeksforGeeks, 2025).

**Entonces... para este ejercicio**

Decidí utilizar MMAP, más que todo por la sencillez con la que se puede integrar a mi primer programa. También resaltar que como está especialmente hecho para trabajar con arreglos grandes de memoria, es perfecto para este ejercicio. Por lo tanto, ¿qué modificaciones se realizó al primer programa para implementar MMAP?

Como primera modificación, el llenado de la matriz en vez de utilizar malloc, se va a cambiar para usar mmap. 

```c
long *matriz = mmap(
    NULL,                      // dirección (el SO decide)
    n * m * sizeof(long),      // tamaño
    PROT_READ | PROT_WRITE,    // permisos
    MAP_SHARED | MAP_ANONYMOUS, // memoria sin archivo, compartida entre procesos
    -1,                        // no file descriptor
    0                          // offset
);
```
El llenado de la matriz se hace igual al primer programa
```c
for (long fila = inicio; fila < fin; fila++) {
    for (long col = 0; col < m; col++) {
        long sum = 0;
        for (long k = 0; k < n; k++) {
            sum += matrizMN[fila * n + k] * matrizNM[k * m + col];
        }
    matrizMM[fila * m + col] = sum;
    }   
}
```
Como se va a utilizar `fork` en este ejercicio, se va a realizar una segmentación similar al ejercicio anterior, donde cada proceso hijo va a procesar una cantidad de `m/cantidad_de_nucleos` filas. 

```c
for (int p = 0; p < cores; p++) {
    pid_t pid = fork();
    
    if (pid == 0) { // si es hijo
        // por cada proceso hijo, se asigna un bloque de filas de la matriz resultante para calcular
        long inicio = p * filas_por_proceso;
        long fin;

        if (p == cores - 1) { // el último proceso se encarga de las filas restantes
            fin = m;
        } else {
            fin = (p + 1) * filas_por_proceso;
        }

        // ... se realiza multiplicacion de matrices
    }
}
```
Por medio de MMAP con Fork se logró un tiempo levemente más rápdio que con hilos, siendo 0.690057 segundos, con una desviación estándar de 0.047865

### Imagen 3


## Conclusiones

Como se pudo ver, el primer acercamiento claramente iba a tener tiempos "largos" en comparación a la utilización de hilos, porque se está trabajando por separado y concurrentemente el trabajo.

Sin embargo, lo curioso está en que el uso de MMAP es ligeramente superior a la utilización de hilos, y posiblemente esto tenga que ver con la hora de crear los hilos o también con la forma en cómo segmenté las filas. 

Esto también agregar que como MMAP trabaja directamente sobre la memoria (y está compartida), es posible que dado a esta razón, a los procesos les cueste menos acceder a los datos y modificar los datos de la matriz resultante.


## Referencias

Aleksandar Haber PhD. (2024, 24 octubre). C Programming Tutorial: malloc() Function and Dynamic Allocation of Memory in C [Vídeo]. YouTube. https://www.youtube.com/watch?v=EsMp_pFVnus

TutorialsPoint. (s. f.). Inter process communication – Pipes.
https://www.tutorialspoint.com/inter_process_communication/inter_process_communication_pipes.htmLinks to an external site.

GeeksforGeeks. (2025). IPC technique: Pipes.
https://www.geeksforgeeks.org/operating-systems/ipc-technique-pipes/Links to an external site.

Arya, K. (2024). Unlocking performance: Real-world applications of mmap. Medium
https://medium.com/@kuldeeparyadotcom/unlocking-performance-real-world-applications-of-mmap-573663028371Links to an external site.

Tencent Cloud. (2025). Memory mapping (mmap).
https://www.tencentcloud.com/techpedia/106444Links to an external site.

GeeksforGeeks. (2025). Memory-mapped files in OS.
https://www.geeksforgeeks.org/operating-systems/memory-mapped-files-in-os/Links to an external site.

Kerrisk, M. (2026). mmap(2) — Linux manual page.
https://man7.org/linux/man-pages/man2/mmap.2.htmlLinks to an external site.