#include <limits.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/commands.h"
#include "../include/monitor.h"
#include "../include/pipe.h"

/**
 * @brief Definición del color rosa para el prompt.
 */
#define COLOR_PINK "\033[1;35m"

/**
 * @brief Definición para restablecer el color del prompt.
 */
#define COLOR_RESET "\033[0m"

/**
 * @brief Tamaño máximo de la entrada del usuario.
 */
#define MAX_LINES 100

/**
 * @brief Tamaño máximo del nombre del host.
 */
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

/**
 * @brief Configura los manejadores de señales para SIGINT, SIGTSTP y SIGQUIT.
 */
void setup_signal_handlers()
{
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTSTP, &sa, NULL) == -1)
    {
        perror("sigaction SIGTSTP");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGQUIT, &sa, NULL) == -1)
    {
        perror("sigaction SIGQUIT");
        exit(EXIT_FAILURE);
    }
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
    char* input[MAX_LINES];

    if (argc > 2)
    {
        fprintf(stderr, "Uso: %s [archivo_de_comandos]\n", argv[0]);
        return 1;
    }

    setup_signal_handlers();

    if (argc == 2)
    {
        FILE* file = fopen(argv[1], "r");
        if (file == NULL)
        {
            perror("fopen");
            return 1;
        }

        char input[MAX_INPUT];
        while (fgets(input, sizeof(input), file) != NULL)
        {
            execute_command(input);
        }

        fclose(file);
        return 0;
    }
    else
    {
        char hostname[HOST_NAME_MAX];
        char cwd[PATH_MAX];
        char* username;
        char input[MAX_INPUT];

        // Obtener el nombre de usuario desde la variable de entorno USER
        username = getenv("USER");
        if (username == NULL)
        {
            perror("getenv USER");
            return 1;
        }

        // Obtener el nombre del host desde la variable de entorno HOSTNAME
        if (getenv("HOSTNAME") != NULL)
        {
            snprintf(hostname, sizeof(hostname), "%s", getenv("HOSTNAME"));
        }
        else
        {
            // Si la variable de entorno HOSTNAME no está establecida, usar gethostname
            if (gethostname(hostname, sizeof(hostname)) != 0)
            {
                perror("gethostname");
                return 1;
            }
        }

        while (1)
        {
            // Obtener el directorio actual
            if (getcwd(cwd, sizeof(cwd)) == NULL)
            {
                perror("getcwd");
                return 1;
            }

            // Formatear y mostrar el prompt
            printf(COLOR_PINK "%s@%s:%s$ " COLOR_RESET, username, hostname, cwd);

            // Leer el comando del usuario
            if (fgets(input, sizeof(input), stdin) == NULL)
            {
                perror("fgets");
                return 1;
            }
            execute_command(input);
        }

        return 0;
    }
}
