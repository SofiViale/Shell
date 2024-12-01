#include "../include/commands.h"
#include "../include/monitor.h"
#include <ctype.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @brief Tamaño máximo de la entrada del usuario.
 */
#define USER_MAX_INPUT 1024

/**
 * @brief Número máximo de argumentos permitidos.
 */
#define MAX_ARGS (USER_MAX_INPUT / 2 + 1)

/**
 * @brief Longitud del comando "echo".
 */
#define ECHO_CMD_LEN 5

/**
 * @brief Longitud del comando "update_config".
 */
#define UPDATE_CONFIG_CMD_LEN 13
pid_t foreground_pid = -1;

void change_directory(char* path)
{
    char* oldpwd = getenv("PWD");
    char cwd[PATH_MAX];

    // Obtener el directorio actual
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        return;
    }

    // Si no se proporciona un directorio, mostrar el directorio actual
    if (path == NULL)
    {
        return;
    }

    // Si el directorio es "-", cambiar al último directorio de trabajo
    if (strcmp(path, "-") == 0)
    {
        if (oldpwd == NULL)
        {
            fprintf(stderr, "OLDPWD not set\n");
            return;
        }
        path = getenv("OLDPWD");
    }

    // Cambiar al nuevo directorio
    if (chdir(path) != 0)
    {
        perror("chdir");
        return;
    }

    if (oldpwd != NULL)
    {
        setenv("OLDPWD", oldpwd, 1);
    }

    // Obtener el nuevo directorio actual después de cambiar
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        return;
    }

    // Actualizar las variables de entorno
    setenv("PWD", cwd, 1);

    return;
}

void clear_screen()
{
    printf("\033[H\033[J");
}

void echo_comment(char* comment)
{
    char* start = comment;
    char* dollar = strchr(start, '$');

    while (dollar != NULL)
    {
        // Imprimir la parte antes del $
        fwrite(start, 1, dollar - start, stdout);

        // Encontrar el final de la variable de entorno
        char* end = dollar + 1;
        while (*end && (isalnum(*end) || *end == '_'))
        {
            end++;
        }

        // Obtener el nombre de la variable de entorno
        char var_name[256];
        strncpy(var_name, dollar + 1, end - dollar - 1);
        var_name[end - dollar - 1] = '\0';

        // Obtener el valor de la variable de entorno
        char* env_var = getenv(var_name);
        if (env_var != NULL)
        {
            printf("%s", env_var);
        }

        // Continuar buscando después de la variable de entorno
        start = end;
        dollar = strchr(start, '$');
    }

    // Imprimir la parte restante de la cadena
    printf("%s\n", start);
}

void quit_shell()
{
    exit(0);
}

void signal_handler(int sig)
{
    if (foreground_pid > 0)
    {
        kill(foreground_pid, sig);
    }
}

void handle_redirection(char* input, int* in_fd, int* out_fd)
{
    char* in_file = NULL;
    char* out_file = NULL;
    char* token;
    char* rest = input;

    while ((token = strtok_r(rest, " ", &rest)))
    {
        if (strcmp(token, "<") == 0)
        {
            in_file = strtok_r(rest, " ", &rest);
        }
        else if (strcmp(token, ">") == 0)
        {
            out_file = strtok_r(rest, " ", &rest);
        }
    }

    if (in_file)
    {
        *in_fd = open(in_file, O_RDONLY);
        if (*in_fd == -1)
        {
            perror("open input file");
            exit(EXIT_FAILURE);
        }
    }

    if (out_file)
    {
        *out_fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (*out_fd == -1)
        {
            perror("open output file");
            exit(EXIT_FAILURE);
        }
    }
}

void execute_command(char* input)
{
    char* args[MAX_ARGS];
    char* token;
    static int job_id = 1;
    int background = 0;
    int in_fd = -1, out_fd = -1;

    // Eliminar el salto de línea al final del comando
    input[strcspn(input, "\n")] = 0;

    // Comprobar si el comando debe ejecutarse en segundo plano
    if (input[strlen(input) - 1] == '&')
    {
        background = 1;
        input[strlen(input) - 1] = '\0'; // Eliminar el '&' del comando
    }

    if (strcmp(input, "quit") == 0)
    {
        quit_shell();
        return;
    }

    // Comprobar si el comando es interno y ejecutarlo
    if (input == NULL)
    {
        return;
    }
    else if (strncmp(input, "cd", 2) == 0)
    {
        change_directory(input + 3);
        return;
    }
    else if (strcmp(input, "clr") == 0)
    {
        clear_screen();
        return;
    }

    if (strchr(input, '|') != NULL)
    {
        execute_command_with_pipes(input);
        return;
    }

    if (strchr(input, '>') != NULL || strchr(input, '<') != NULL)
    {
        handle_redirection(input, &in_fd, &out_fd);
    }

    // Manejar comandos de monitoreo
    if (strcmp(input, "start_monitor") == 0)
    {
        start_monitor();
        return;
    }
    else if (strcmp(input, "stop_monitor") == 0)
    {
        stop_monitor();
        return;
    }
    else if (strcmp(input, "status_monitor") == 0)
    {
        status_monitor();
        return;
    }
    else if (strncmp(input, "update_config", UPDATE_CONFIG_CMD_LEN) == 0)
    {
        const char* metrics[] = {"cpu_usage",     "memory_usage",  "disk_io",
                                 "network_stats", "process_count", "context_switches"};
        update_config_file(metrics, 6);
        return;
    }

    // Tokenizar la entrada del usuario
    int i = 0;
    char* input_copy = strdup(input);
    token = strtok(input_copy, " ");
    while (token != NULL)
    {
        if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0)
        {
            token = strtok(NULL, " ");
            continue;
        }
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    pid_t pid = fork();

    if (pid == -1)
    {
        // Error al crear el proceso hijo
        perror("fork");
    }
    else if (pid == 0)
    {
        if (in_fd != -1)
        {
            if (dup2(in_fd, STDIN_FILENO) == -1)
            {
                perror("dup2 input");
                exit(EXIT_FAILURE);
            }
            close(in_fd);
        }

        if (out_fd != -1)
        {
            if (dup2(out_fd, STDOUT_FILENO) == -1)
            {
                perror("dup2 output");
                exit(EXIT_FAILURE);
            }
            close(out_fd);
        }

        if (strncmp(input, "echo", ECHO_CMD_LEN - 1) == 0)
        {
            // Manejar el comando echo con redirección
            char* echo_args = input + ECHO_CMD_LEN;
            if (out_fd != -1)
            {
                dprintf(out_fd, "%s\n", echo_args);
                close(out_fd);
                exit(0);
            }
            else
            {
                echo_comment(echo_args);
                exit(0);
            }
        }

        // Proceso hijo
        if (execvp(args[0], args) == -1)
        {
            // Error al ejecutar el programa
            perror("execvp");
        }
        exit(EXIT_FAILURE);
    }
    else
    {
        // Proceso padre
        if (background)
        {
            // Proceso en segundo plano
            printf("[%d] %d\n", job_id++, pid);
        }
        else
        {
            // Proceso en primer plano
            foreground_pid = pid;
            int status;
            waitpid(pid, &status, WUNTRACED);
            if (WIFSTOPPED(status))
            {
                printf("[%d]+ Proceso con pid: %d detenido                %s\n", job_id++, pid, input);
            }
            foreground_pid = -1;
        }
    }
}