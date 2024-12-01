#include "../include/expose_metrics.h"
#include "../include/metrics.h"
#include <cjson/cJSON.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

/**
 * @brief Señal para recargar la configuración.
 */
volatile sig_atomic_t reload_config = 0;

/**
 * @brief Señal para detener el programa.
 */
volatile sig_atomic_t stop_program = 0;

/**
 * @brief Variables booleanas para controlar qué métricas se deben mostrar.
 */

/** @brief Indica si se debe mostrar la métrica de uso de CPU. */
bool show_cpu_usage = false;

/** @brief Indica si se debe mostrar la métrica de uso de memoria. */
bool show_memory_usage = false;

/** @brief Indica si se debe mostrar la métrica de I/O de disco. */
bool show_disk_io = false;

/** @brief Indica si se deben mostrar las estadísticas de red. */
bool show_network_stats = false;

/** @brief Indica si se debe mostrar el conteo de procesos. */
bool show_process_count = false;

/** @brief Indica si se deben mostrar los cambios de contexto. */
bool show_context_switches = false;

/**
 * @brief Intervalo de tiempo entre actualizaciones de métricas.
 */
int interval = 5;

/**
 * @brief Manejador de señales para recargar la configuración o detener el programa.
 *
 * @param signal Señal recibida.
 */
void handle_signal(int signal)
{
    if (signal == SIGUSR1)
    {
        reload_config = 1;
    }
    else if (signal == SIGINT)
    {
        stop_program = 1;
    }
}

/**
 * @brief Lee la configuración desde un archivo JSON.
 *
 * @param config_filename Nombre del archivo de configuración.
 */
void read_config(const char* config_filename)
{
    FILE* file = fopen(config_filename, "r");
    if (file == NULL)
    {
        perror("Error al abrir el archivo de configuración, usando métricas por defecto");
        // Usar métricas por defecto
        show_cpu_usage = true;
        show_memory_usage = true;
        show_disk_io = true;
        show_network_stats = true;
        show_process_count = true;
        show_context_switches = true;
        interval = 5;
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';

    cJSON* json = cJSON_Parse(data);
    if (json == NULL)
    {
        perror("Error al parsear el archivo JSON, usando métricas por defecto");
        free(data);
        // Usar métricas por defecto
        show_cpu_usage = true;
        show_memory_usage = true;
        show_disk_io = true;
        show_network_stats = true;
        show_process_count = true;
        show_context_switches = true;
        interval = 5;
        return;
    }

    cJSON* metrics_json = cJSON_GetObjectItemCaseSensitive(json, "metrics");
    cJSON* interval_json = cJSON_GetObjectItemCaseSensitive(json, "interval");

    if (!cJSON_IsObject(metrics_json) || !cJSON_IsNumber(interval_json))
    {
        perror("Formato de archivo JSON inválido, usando métricas por defecto");
        cJSON_Delete(json);
        free(data);
        // Usar métricas por defecto
        show_cpu_usage = true;
        show_memory_usage = true;
        show_disk_io = true;
        show_network_stats = true;
        show_process_count = true;
        show_context_switches = true;
        interval = 5;
        return;
    }

    show_cpu_usage = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "cpu"));
    show_memory_usage = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "memory"));
    show_disk_io = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "disk_io"));
    show_network_stats = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "network_stats"));
    show_process_count = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "process_count"));
    show_context_switches = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "context_switches"));

    interval = interval_json->valueint;

    cJSON_Delete(json);
    free(data);
}

/**
 * @brief Punto de entrada del programa.
 *
 * @param argc Número de argumentos.
 * @param argv Argumentos del programa.
 * @return int Código de salida.
 */
int main(int argc, char* argv[])
{
    signal(SIGUSR1, handle_signal);
    signal(SIGINT, handle_signal);

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <ruta_al_archivo_config.json>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* config_filename = argv[1];

    // Leer la configuración inicial
    read_config(config_filename);

    // Creamos un hilo para exponer las métricas vía HTTP
    pthread_t tid;
    if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error al crear el hilo del servidor HTTP\n");
        return EXIT_FAILURE;
    }

    init_metrics();

    // Bucle principal para actualizar las métricas según el intervalo especificado
    while (!stop_program)
    {
        if (reload_config)
        {
            // Volver a leer la configuración
            read_config(config_filename);
            reload_config = 0;
        }

        if (show_cpu_usage)
        {
            update_cpu_gauge();
        }
        if (show_memory_usage)
        {
            update_memory_gauge();
            update_memory_gauge2();
        }
        if (show_disk_io)
        {
            update_disk_io_gauge();
        }
        if (show_network_stats)
        {
            update_network_gauge();
        }
        if (show_process_count)
        {
            update_process_count_gauge();
        }
        if (show_context_switches)
        {
            update_context_switches_gauge();
        }

        sleep(interval);
    }

    return EXIT_SUCCESS;
}
