#include "unity.h"
#include "../include/commands.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>

/**
 * @brief Tamaño del buffer utilizado para leer y verificar la salida.
 */
#define BUFFER_SIZE 256

/**
 * @brief Nombre del archivo temporal utilizado para redirigir la salida.
 */
#define TEMP_FILE "temp_output.txt"

/**
 * @brief Permisos utilizados para crear el archivo temporal.
 */
#define PERMISSIONS 0644
// Variables globales para las pruebas
extern pid_t foreground_pid;

// Funciones de configuración y limpieza
void setUp(void) {
    // Configuración antes de cada prueba
    foreground_pid = -1;
}

void tearDown(void) {
    // Limpieza después de cada prueba
}

// Prueba para change_directory
void test_change_directory(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        TEST_FAIL_MESSAGE("Failed to get current working directory");
    }
    char* home = getenv("HOME");

    change_directory(home);
    char new_cwd[PATH_MAX];
    if (getcwd(new_cwd, sizeof(new_cwd)) == NULL) {
        perror("getcwd");
        TEST_FAIL_MESSAGE("Failed to get new working directory");
    }

    TEST_ASSERT_EQUAL_STRING(home, new_cwd);

    change_directory(cwd);
    if (getcwd(new_cwd, sizeof(new_cwd)) == NULL) {
        perror("getcwd");
        TEST_FAIL_MESSAGE("Failed to get restored working directory");
    }

    TEST_ASSERT_EQUAL_STRING(cwd, new_cwd);
}

// Prueba para echo_comment
void test_echo_comment(void) {
    // Redirigir stdout a un archivo temporal
    int stdout_fd = dup(STDOUT_FILENO);
    int temp_fd = open(TEMP_FILE, O_WRONLY | O_CREAT | O_TRUNC, PERMISSIONS);
    if (temp_fd == -1) {
        perror("open");
        TEST_FAIL_MESSAGE("Failed to open temporary file");
    }
    if (dup2(temp_fd, STDOUT_FILENO) == -1) {
        perror("dup2");
        TEST_FAIL_MESSAGE("Failed to redirect stdout");
    }

    echo_comment("Hello, World!");
    fflush(stdout);

    // Restaurar stdout
    if (dup2(stdout_fd, STDOUT_FILENO) == -1) {
        perror("dup2");
        TEST_FAIL_MESSAGE("Failed to restore stdout");
    }
    close(stdout_fd);
    close(temp_fd);

    // Leer el archivo temporal y verificar el contenido
    FILE* temp_file = fopen(TEMP_FILE, "r");
    if (temp_file == NULL) {
        perror("fopen");
        TEST_FAIL_MESSAGE("Failed to open temporary file for reading");
    }
    char buffer[BUFFER_SIZE];
    if (fgets(buffer, sizeof(buffer), temp_file) == NULL) {
        perror("fgets");
        TEST_FAIL_MESSAGE("Failed to read from temporary file");
    }
    fclose(temp_file);

    TEST_ASSERT_EQUAL_STRING("Hello, World!\n", buffer);

    // Eliminar el archivo temporal
    remove(TEMP_FILE);
}

// Función principal para ejecutar las pruebas
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_echo_comment);
    RUN_TEST(test_change_directory);

    return UNITY_END();
}
