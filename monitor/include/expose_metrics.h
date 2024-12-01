/**
 * @file expose_metrics.h
 * @brief Programa para leer el uso de CPU y memoria y exponerlos como métricas de Prometheus.
 */

#include "metrics.h"
// #include "read_cpu_usage.h"
#include <errno.h>
#include <prom.h>
#include <promhttp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Para sleep

/**
 * @brief Tamaño del buffer utilizado para leer archivos del sistema.
 */
#define BUFFER_SIZE 256

/**
 * @brief Actualiza la métrica de uso de CPU.
 */
void update_cpu_gauge();

/**
 * @brief Actualiza la métrica de uso de memoria.
 */
void update_memory_gauge();

/**
 * @brief Actualiza la métrica de uso de memoria (segunda versión).
 */
void update_memory_gauge2();

/**
 * @brief Actualiza la métrica de I/O de disco.
 */
void update_disk_io_gauge();

/**
 * @brief Actualiza la métrica de estadísticas de red.
 */
void update_network_gauge();

/**
 * @brief Actualiza la métrica de conteo de procesos.
 */
void update_process_count_gauge();

/**
 * @brief Actualiza la métrica de cambios de contexto.
 */
void update_context_switches_gauge();

/**
 * @brief Función del hilo para exponer las métricas vía HTTP en el puerto 8000.
 * @param arg Argumento no utilizado.
 * @return NULL
 */
void* expose_metrics(void* arg);

/**
 * @brief Inicializa el mutex y las métricas.
 * @return EXIT_SUCCESS si la inicialización es exitosa, de lo contrario EXIT_FAILURE.
 */
int init_metrics();

/**
 * @brief Destruye el mutex.
 */
void destroy_mutex();