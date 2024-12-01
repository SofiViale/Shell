#ifndef COMMANDS_H
#define COMMANDS_H

#include <sys/types.h>

/**
 * @brief Ejecuta un comando con pipes.
 *
 * @param input El comando a ejecutar.
 */
void execute_command_with_pipes(char* input);

/**
 * @brief Cambia el directorio actual.
 *
 * @param path El directorio al que se desea cambiar. Si es NULL, muestra el directorio actual.
 *             Si es "-", cambia al último directorio de trabajo.
 */
void change_directory(char* path);

/**
 * @brief Limpia la pantalla.
 */
void clear_screen();

/**
 * @brief Muestra un comentario o el valor de una variable de entorno.
 *
 * @param comment El comentario a mostrar. Si comienza con '$', se interpreta como una variable de entorno.
 */
void echo_comment(char* comment);

/**
 * @brief Cierra la shell.
 */
void quit_shell();

/**
 * @brief Ejecuta un comando.
 *
 * @param input El comando a ejecutar.
 */
void execute_command(char* input);

/**
 * @brief Manejador de señales.
 *
 * @param sig La señal recibida.
 */
void signal_handler(int sig);

/**
 * @brief Maneja la redirección de entrada y salida.
 *
 * @param input El comando con redirección.
 * @param in_fd El descriptor de archivo de entrada.
 * @param out_fd El descriptor de archivo de salida.
 */
void handle_redirection(char* input, int* in_fd, int* out_fd);

/**
 * @brief Cambia el estado de un trabajo.
 *
 * @param pid El PID del trabajo.
 * @param status El estado del trabajo.
 */
extern pid_t foreground_pid;

#endif // COMMANDS_H