#include "../include/pipe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @brief Tamaño máximo de la entrada del usuario.
 */
#define MAX_INPUT 1024
/**
 * @brief Número máximo de argumentos permitidos.
 */
#define MAX_ARGS (MAX_INPUT / 2 + 1)

void execute_command_with_pipes(char* input)
{

    if (strchr(input, '|') == NULL)
    {
        // No hay pipes, no hacer nada
        return;
    }

    char* commands[MAX_ARGS];
    int num_commands = 0;
    char* command = strtok(input, "|");
    while (command != NULL)
    {
        while (*command == ' ')
            command++;
        commands[num_commands++] = command;
        command = strtok(NULL, "|");
    }

    if (num_commands <= 1)
    {
        // No hay suficientes comandos para usar pipes
        return;
    }

    int* pipefds = malloc(2 * (num_commands - 1) * sizeof(int));
    if (pipefds == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipefds + i * 2) == -1)
        {
            perror("pipe");
            free(pipefds);
            exit(EXIT_FAILURE);
        }
    }

    int i = 0;
    int status;
    pid_t pid;
    for (i = 0; i < num_commands; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            free(pipefds);
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Proceso hijo
            if (i < num_commands - 1)
            {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            if (i > 0)
            {
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            for (int j = 0; j < 2 * (num_commands - 1); j++)
            {
                close(pipefds[j]);
            }

            char* args[MAX_ARGS];
            char* token = strtok(commands[i], " ");
            int k = 0;
            while (token != NULL)
            {
                args[k++] = token;
                token = strtok(NULL, " ");
            }
            args[k] = NULL;

            if (execvp(args[0], args) == -1)
            {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
    }

    for (int i = 0; i < 2 * (num_commands - 1); i++)
    {
        close(pipefds[i]);
    }

    for (int i = 0; i < num_commands; i++)
    {
        wait(&status);
    }

    free(pipefds);
}
