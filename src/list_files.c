#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Lista de palabras clave importantes
const char *keywords[] = {
    "password",
    "username",
    "port",
    "host"
};
const int num_keywords = sizeof(keywords) / sizeof(keywords[0]);

// Función para verificar si una línea contiene alguna palabra clave
int contains_keyword(const char *line) {
    for (int i = 0; i < num_keywords; i++) {
        if (strstr(line, keywords[i])) {
            return 1;
        }
    }
    return 0;
}

// Función para listar archivos de configuración en un directorio específico sin búsqueda recursiva
void list_config_files(const char *dir_path, const char *file_extension) {
    struct dirent *entry;
    DIR *dp = opendir(dir_path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    printf("Listando archivos en el directorio: %s\n", dir_path);
    while ((entry = readdir(dp))) {
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat path_stat;
        stat(full_path, &path_stat);

        if (S_ISREG(path_stat.st_mode)) {
            if (strstr(entry->d_name, file_extension)) {
                printf("Archivo de configuración encontrado: %s\n", full_path);
            }
        }
    }

    closedir(dp);
}

// Función para leer y extraer datos de un archivo de configuración
void read_config_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file) {
        char line[256];
        printf("Contenido de %s:\n", file_path);
        while (fgets(line, sizeof(line), file)) {
            // Mostrar solo las líneas que contienen palabras clave importantes
            if (contains_keyword(line)) {
                printf("%s", line);
            }
        }
        fclose(file);
    } else {
        perror("fopen");
    }
}

// Nueva función para realizar la búsqueda recursiva y leer datos de configuración
void search_and_read_config_files(const char *dir_path, const char *file_extension) {
    struct dirent *entry;
    DIR *dp = opendir(dir_path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    printf("Buscando archivos en el directorio: %s\n", dir_path);
    while ((entry = readdir(dp))) {
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat path_stat;
        stat(full_path, &path_stat);

        if (S_ISREG(path_stat.st_mode)) {
            if (strstr(entry->d_name, file_extension)) {
                printf("Archivo de configuración encontrado: %s\n", full_path);
                read_config_file(full_path);
            }
        } else if (S_ISDIR(path_stat.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Búsqueda recursiva en subdirectorios
            search_and_read_config_files(full_path, file_extension);
        }
    }

    closedir(dp);
}