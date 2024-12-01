#include "../include/monitor.h"
#include <cjson/cJSON.h>
#include <errno.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief Tamaño del buffer utilizado para la entrada del usuario.
 */
#define INPUT_SIZE 10

/**
 * @brief Tamaño del buffer utilizado para leer y verificar la salida.
 */
#define BUFFER_SIZE 1024

/**
 * @brief Intervalo predeterminado en segundos para la recolección de métricas.
 */
#define DEFAULT_INTERVAL 1

/**
 * @brief Número de métricas soportadas.
 */
#define METRICS_COUNT 6

pid_t monitor_pid = -1;

void create_config_file(const char* metrics[], int num_metrics, int interval)
{
    cJSON* jobj = cJSON_CreateObject();
    cJSON* jmetrics = cJSON_CreateObject();

    for (int i = 0; i < num_metrics; i++)
    {
        cJSON_AddBoolToObject(jmetrics, metrics[i], cJSON_True);
    }

    cJSON_AddItemToObject(jobj, "metrics", jmetrics);
    cJSON_AddNumberToObject(jobj, "interval", interval);

    // Obtener la ruta del archivo config.json desde la variable de entorno PROJECT_ROOT
    const char* project_root = getenv("PROJECT_ROOT");
    if (project_root == NULL)
    {
        fprintf(stderr, "Error: La variable de entorno PROJECT_ROOT no está definida.\n");
        exit(EXIT_FAILURE);
    }
    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), "%s/config.json", project_root);

    FILE* fp = fopen(config_path, "w");
    if (fp != NULL)
    {
        fputs(cJSON_Print(jobj), fp);
        fclose(fp);
        printf("Archivo %s creado/actualizado.\n", config_path);
    }
    else
    {
        perror("fopen");
    }

    cJSON_Delete(jobj);
}

void update_config_file()
{
    char input[INPUT_SIZE];
    int interval;
    cJSON* config = cJSON_CreateObject();
    cJSON* metrics = cJSON_CreateObject();

    printf("Enter 't' or 'f' for the following metrics:\n");

    printf("CPU: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf("Invalid input. Setting CPU to true by default.\n");
        cJSON_AddBoolToObject(metrics, "cpu", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "cpu", input[0] == 't');
    }

    printf("Memory: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf("Invalid input. Setting Memory to true by default.\n");
        cJSON_AddBoolToObject(metrics, "memory", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "memory", input[0] == 't');
    }

    printf("Disk IO: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf("Invalid input. Setting Disk IO to true by default.\n");
        cJSON_AddBoolToObject(metrics, "disk_io", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "disk_io", input[0] == 't');
    }
    printf("Process Count: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf("Invalid input. Setting Process Count to true by default.\n");
        cJSON_AddBoolToObject(metrics, "process_count", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "process_count", input[0] == 't');
    }

    printf("Context Switches: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf("Invalid input. Setting Context Switches to true by default.\n");
        cJSON_AddBoolToObject(metrics, "context_switches", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "context_switches", input[0] == 't');
    }
    printf("Network Stats: ");
    if (fgets(input, sizeof(input), stdin) == NULL || (input[0] != 't' && input[0] != 'f'))
    {
        printf("Invalid input. Setting Network Stats to true by default.\n");
        cJSON_AddBoolToObject(metrics, "network_stats", 1);
    }
    else
    {
        cJSON_AddBoolToObject(metrics, "network_stats", input[0] == 't');
    }

    cJSON_AddItemToObject(config, "metrics", metrics);

    printf("Enter sleep time (in seconds): ");
    if (fgets(input, sizeof(input), stdin) == NULL || (interval = atoi(input)) <= 0)
    {
        printf("Invalid input. Setting sleep time to %d second by default.\n", DEFAULT_INTERVAL);
        interval = DEFAULT_INTERVAL;
    }
    cJSON_AddNumberToObject(config, "interval", interval);

    char* config_string = cJSON_Print(config);
    // Obtener la ruta del archivo config.json desde la variable de entorno PROJECT_ROOT
    const char* project_root = getenv("PROJECT_ROOT");
    if (project_root == NULL)
    {
        fprintf(stderr, "Error: La variable de entorno PROJECT_ROOT no está definida.\n");
        exit(EXIT_FAILURE);
    }
    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), "%s/config.json", project_root);

    FILE* file = fopen(config_path, "w");
    if (file == NULL)
    {
        perror("fopen");
        cJSON_Delete(config);
        free(config_string);
        return;
    }
    fprintf(file, "%s", config_string);
    fclose(file);

    cJSON_Delete(config);
    free(config_string);

    printf("Configuration updated successfully.\n");

    if (monitor_pid != -1)
    {
        kill(monitor_pid, SIGUSR1);
        printf("Señal SIGUSR1 enviada al monitor para recargar la configuración.\n");
    }
}

void start_monitor()
{
    const char* metrics[] = {"cpu_usage",     "memory_usage",  "disk_io",
                             "network_stats", "process_count", "context_switches"};
    create_config_file(metrics, METRICS_COUNT, DEFAULT_INTERVAL);
    if (monitor_pid == -1)
    {
        monitor_pid = fork();
        if (monitor_pid == 0)
        {
            // Obtener la ruta del ejecutable metrics desde la variable de entorno PROJECT_ROOT
            const char* project_root = getenv("PROJECT_ROOT");
            if (project_root == NULL)
            {
                fprintf(stderr, "Error: La variable de entorno PROJECT_ROOT no está definida.\n");
                exit(EXIT_FAILURE);
            }
            char metrics_path[PATH_MAX];
            snprintf(metrics_path, sizeof(metrics_path), "%s/monitor/metrics", project_root);
            char config_path[PATH_MAX];
            snprintf(config_path, sizeof(config_path), "%s/config.json", project_root);

            // Proceso hijo: ejecutar el programa de monitoreo
            execl(metrics_path, "metrics", config_path, NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        }
        else if (monitor_pid < 0)
        {
            perror("fork");
        }
        else
        {
            printf("Monitor iniciado con PID %d\n", monitor_pid);
        }
    }
    else
    {
        printf("El monitor ya está en ejecución con PID %d\n", monitor_pid);
    }
}

void stop_monitor()
{
    if (monitor_pid != -1)
    {
        kill(monitor_pid, SIGINT);
        printf("Señal SIGINT enviada al monitor para detenerlo.\n");
        monitor_pid = -1;
    }
    else
    {
        printf("El monitor no está en ejecución.\n");
    }
}

void status_monitor()
{
    if (monitor_pid != -1)
    {
        printf("El monitor está en ejecución con PID %d\n", monitor_pid);
    }
    else
    {
        printf("El monitor no está en ejecución.\n");
    }
}

void handle_signal(int sig)
{
    if (sig == SIGUSR1)
    {
        printf("Recargando configuración...\n");
        // Obtener la ruta del archivo config.json desde la variable de entorno PROJECT_ROOT
        const char* project_root = getenv("PROJECT_ROOT");
        if (project_root == NULL)
        {
            fprintf(stderr, "Error: La variable de entorno PROJECT_ROOT no está definida.\n");
            exit(EXIT_FAILURE);
        }
        char config_path[PATH_MAX];
        snprintf(config_path, sizeof(config_path), "%s/config.json", project_root);

        FILE* fp = fopen(config_path, "r");
        if (fp != NULL)
        {
            char buffer[BUFFER_SIZE];
            size_t bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, fp);
            if (bytes_read > 0)
            {
                buffer[bytes_read] = '\0'; // Asegurarse de que el buffer esté terminado en nulo

                cJSON* jobj = cJSON_Parse(buffer);
                cJSON* jmetrics = cJSON_GetObjectItem(jobj, "metrics");
                cJSON* jinterval = cJSON_GetObjectItem(jobj, "interval");

                if (jmetrics && jinterval)
                {
                    printf("Métricas: %s\n", cJSON_Print(jmetrics));
                    printf("Intervalo: %d segundos\n", jinterval->valueint);
                }

                cJSON_Delete(jobj);
            }
            else
            {
                perror("fread");
            }
            fclose(fp);
        }
        else
        {
            perror("fopen");
        }
    }
    else if (sig == SIGINT)
    {
        printf("Monitor detenido.\n");
        exit(0);
    }
}
