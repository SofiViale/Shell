/**
 * @file metrics.h
 * @brief Funciones para obtener el uso de CPU y memoria desde el sistema de archivos /proc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Tamaño del buffer utilizado para leer archivos del sistema.
 */
#define BUFFER_SIZE 256

/**
 * @brief Obtiene el porcentaje de uso de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total y disponible desde /proc/meminfo y calcula
 * el porcentaje de uso de memoria.
 *
 * @return Uso de memoria como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_memory_usage();

/**
 * @brief Obtiene el porcentaje de uso de CPU desde /proc/stat.
 *
 * Lee los tiempos de CPU desde /proc/stat y calcula el porcentaje de uso de CPU
 * en un intervalo de tiempo.
 *
 * @return Uso de CPU como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_cpu_usage();

/**
 * @brief Obtiene las estadísticas de uso de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total, usada y libre desde /proc/meminfo.
 *
 * @param total_mem Puntero para almacenar la memoria total en MB.
 * @param used_mem Puntero para almacenar la memoria usada en MB.
 * @param free_mem Puntero para almacenar la memoria libre en MB.
 */
void get_memory_usage2(double* total_mem, double* used_mem, double* free_mem);

/**
 * @brief Obtiene las estadísticas de I/O de disco desde /proc/diskstats.
 *
 * Lee los valores de sectores leídos y escritos desde /proc/diskstats.
 *
 * @param reads Puntero para almacenar el número de sectores leídos.
 * @param writes Puntero para almacenar el número de sectores escritos.
 */
void get_disk_io_stats(unsigned long long* reads, unsigned long long* writes);

/**
 * @brief Obtiene las estadísticas de red desde /proc/net/dev.
 *
 * Lee los valores de bytes recibidos y transmitidos desde /proc/net/dev.
 *
 * @param rx_bytes Puntero para almacenar el número de bytes recibidos.
 * @param tx_bytes Puntero para almacenar el número de bytes transmitidos.
 */
void get_network_stats(unsigned long long* rx_bytes, unsigned long long* tx_bytes);

/**
 * @brief Obtiene el número de procesos en ejecución desde /proc/stat.
 *
 * Lee el número de procesos en ejecución desde /proc/stat.
 *
 * @return Número de procesos en ejecución, o -1 en caso de error.
 */
int get_process_count();

/**
 * @brief Obtiene el número de cambios de contexto desde /proc/stat.
 *
 * Lee el número de cambios de contexto desde /proc/stat.
 *
 * @return Número de cambios de contexto, o 0 en caso de error.
 */
unsigned long long get_context_switches();