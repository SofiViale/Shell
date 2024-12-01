#ifndef MONITOR_H
#define MONITOR_H

#include <sys/types.h>

/**
 * @brief Crea o actualiza el archivo config.json con las métricas y el intervalo especificado.
 *
 * @param metrics Array de cadenas que contiene los nombres de las métricas.
 * @param num_metrics Número de métricas en el array.
 * @param interval Intervalo de tiempo (en segundos) entre cada actualización de las métricas.
 */
void create_config_file(const char* metrics[], int num_metrics, int interval);

/**
 * @brief Actualiza el archivo config.json con las métricas y el intervalo especificado y envía una señal al monitor
 * para recargar la configuración.
 */
void update_config_file();

/**
 * @brief Inicia el programa de monitoreo en un proceso hijo.
 */
void start_monitor();

/**
 * @brief Detiene el programa de monitoreo enviando la señal SIGINT.
 */
void stop_monitor();

/**
 * @brief Muestra el estado actual del programa de monitoreo.
 */
void status_monitor();

/**
 * @brief PID del proceso de monitoreo.
 */
extern pid_t monitor_pid;

#endif // MONITOR_H