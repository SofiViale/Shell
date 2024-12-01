#include "../include/monitor.h"
#include "unity.h"
#include <linux/limits.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/**
 * @brief Configuración inicial antes de cada prueba.
 */
void setUp(void)
{
    // No se necesita configuración inicial para estas pruebas
    // Buscar la ruta del archivo CMakeLists.txt
    char current_path[PATH_MAX];
    if (getcwd(current_path, sizeof(current_path)) == NULL)
    {
        perror("getcwd");
        TEST_FAIL_MESSAGE("Failed to get current working directory");
    }

    char cmake_path[PATH_MAX + 16]; // Aumentar el tamaño del buffer para evitar truncación
    snprintf(cmake_path, sizeof(cmake_path), "%s/CMakeLists.txt", current_path);

    while (access(cmake_path, F_OK) != 0)
    {
        // Subir un nivel en el directorio
        char *last_slash = strrchr(current_path, '/');
        if (last_slash == NULL)
        {
            TEST_FAIL_MESSAGE("CMakeLists.txt not found in any parent directory");
        }
        *last_slash = '\0';
        snprintf(cmake_path, sizeof(cmake_path), "%s/CMakeLists.txt", current_path);
    }

    setenv("PROJECT_ROOT", current_path, 1);
}

/**
 * @brief Limpieza después de cada prueba.
 */
void tearDown(void)
{
    // Detener el monitor si está en ejecución
    stop_monitor();
}

/**
 * @brief Verifica si un proceso está en ejecución.
 *
 * @param pid PID del proceso a verificar.
 * @return int 1 si el proceso está en ejecución, 0 en caso contrario.
 */
int is_process_running(pid_t pid)
{
    return kill(pid, 0) == 0;
}

/**
 * @brief Prueba para iniciar el monitor.
 */
void test_start_monitor(void)
{
    printf("Iniciando monitor...\n");
    fflush(stdout);

    start_monitor();

    // Verificar si el proceso está en ejecución
    if (!is_process_running(monitor_pid))
    {
        perror("kill");
        TEST_FAIL_MESSAGE("El proceso del monitor no está en ejecución");
    }

    printf("Monitor iniciado con PID: %d\n", monitor_pid);
    fflush(stdout);

    // Detener el monitor para limpiar
    printf("Deteniendo monitor...\n");
    fflush(stdout);
    stop_monitor();
}

/**
 * @brief Prueba para verificar el estado del monitor.
 */
void test_status_monitor(void)
{
    printf("Iniciando monitor...\n");
    fflush(stdout);

    start_monitor();

    // Verificar si el proceso está en ejecución
    if (!is_process_running(monitor_pid))
    {
        perror("kill");
        TEST_FAIL_MESSAGE("El proceso del monitor no está en ejecución");
    }

    printf("Monitor iniciado con PID: %d\n", monitor_pid);
    fflush(stdout);

    // Capturar la salida de status_monitor
    fflush(stdout);
    int saved_stdout = dup(fileno(stdout));
    FILE* output_file = freopen("test_output.txt", "w+", stdout);
    if (output_file == NULL)
    {
        perror("freopen");
        TEST_FAIL_MESSAGE("No se pudo redirigir stdout");
    }

    status_monitor();

    fflush(stdout);
    dup2(saved_stdout, fileno(stdout));
    close(saved_stdout);

    // Leer el archivo de salida
    output_file = fopen("test_output.txt", "r");
    if (output_file == NULL)
    {
        perror("fopen");
        TEST_FAIL_MESSAGE("No se pudo abrir el archivo de salida");
    }
    char output[256];
    if (fgets(output, sizeof(output), output_file) == NULL)
    {
        perror("fgets");
        TEST_FAIL_MESSAGE("No se pudo leer el archivo de salida");
    }
    fclose(output_file);

    // Eliminar códigos de color ANSI de la salida
    char* ansi_removed_output = output;
    char* output_ptr = output;
    while (*ansi_removed_output)
    {
        if (*ansi_removed_output == '\x1B')
        {
            while (*ansi_removed_output && *ansi_removed_output != 'm')
            {
                ansi_removed_output++;
            }
            if (*ansi_removed_output)
            {
                ansi_removed_output++;
            }
        }
        else
        {
            *output_ptr++ = *ansi_removed_output++;
        }
    }
    *output_ptr = '\0';

    char expected_output[256];
    snprintf(expected_output, sizeof(expected_output), "El monitor está en ejecución con PID %d\n", monitor_pid);
    TEST_ASSERT_EQUAL_STRING(expected_output, output);

    // Detener el monitor
    printf("Deteniendo monitor...\n");
    fflush(stdout);
    stop_monitor();
}

/**
 * @brief Función principal para ejecutar las pruebas.
 *
 * @return int Código de salida.
 */
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_start_monitor);
    RUN_TEST(test_status_monitor);
    return UNITY_END();
}
