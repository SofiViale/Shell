#include "../include/metrics.h"

#define MEMINFO_PATH "/proc/meminfo"
#define STAT_PATH "/proc/stat"
#define DISKSTATS_PATH "/proc/diskstats"
#define NETDEV_PATH "/proc/net/dev"
#define BUFFER_SIZE 256
#define CPU_FIELDS 8

double get_memory_usage()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long total_mem = 0, free_mem = 0;

    // Abrir el archivo /proc/meminfo
    fp = fopen(MEMINFO_PATH, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " MEMINFO_PATH);
        return -1.0;
    }

    // Leer los valores de memoria total y disponible
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemTotal: %llu kB", &total_mem) == 1)
        {
            continue; // MemTotal encontrado
        }
        if (sscanf(buffer, "MemAvailable: %llu kB", &free_mem) == 1)
        {
            break; // MemAvailable encontrado, podemos dejar de leer
        }
    }

    fclose(fp);

    // Verificar si se encontraron ambos valores
    if (total_mem == 0 || free_mem == 0)
    {
        fprintf(stderr, "Error al leer la información de memoria desde " MEMINFO_PATH "\n");
        return -1.0;
    }

    // Calcular el porcentaje de uso de memoria
    double used_mem = total_mem - free_mem;
    double mem_usage_percent = (used_mem / total_mem) * 100.0;

    return mem_usage_percent;
}

double get_cpu_usage()
{
    static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0, prev_iowait = 0,
                              prev_irq = 0, prev_softirq = 0, prev_steal = 0;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    unsigned long long totald, idled;
    double cpu_usage_percent;

    // Abrir el archivo /proc/stat
    FILE* fp = fopen(STAT_PATH, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " STAT_PATH);
        return -1.0;
    }

    char buffer[BUFFER_SIZE * 4];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error al leer " STAT_PATH);
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Analizar los valores de tiempo de CPU
    int ret = sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait,
                     &irq, &softirq, &steal);
    if (ret < CPU_FIELDS)
    {
        fprintf(stderr, "Error al parsear " STAT_PATH "\n");
        return -1.0;
    }

    // Calcular las diferencias entre las lecturas actuales y anteriores
    unsigned long long prev_idle_total = prev_idle + prev_iowait;
    unsigned long long idle_total = idle + iowait;

    unsigned long long prev_non_idle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;
    unsigned long long non_idle = user + nice + system + irq + softirq + steal;

    unsigned long long prev_total = prev_idle_total + prev_non_idle;
    unsigned long long total = idle_total + non_idle;

    totald = total - prev_total;
    idled = idle_total - prev_idle_total;

    if (totald == 0)
    {
        fprintf(stderr, "Totald es cero, no se puede calcular el uso de CPU!\n");
        return -1.0;
    }

    // Calcular el porcentaje de uso de CPU
    cpu_usage_percent = ((double)(totald - idled) / totald) * 100.0;

    // Actualizar los valores anteriores para la siguiente lectura
    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;
    prev_iowait = iowait;
    prev_irq = irq;
    prev_softirq = softirq;
    prev_steal = steal;

    return cpu_usage_percent;
}

void get_memory_usage2(double* total_mem, double* used_mem, double* free_mem)
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long mem_total = 0, mem_free = 0, mem_available = 0;

    fp = fopen(MEMINFO_PATH, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " MEMINFO_PATH);
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        sscanf(buffer, "MemTotal: %llu kB", &mem_total);
        sscanf(buffer, "MemFree: %llu kB", &mem_free);
        sscanf(buffer, "MemAvailable: %llu kB", &mem_available);
    }

    fclose(fp);

    *total_mem = (double)mem_total / 1024.0;
    *free_mem = (double)mem_free / 1024.0;
    *used_mem = *total_mem - *free_mem;
}

void get_disk_io_stats(unsigned long long* reads, unsigned long long* writes)
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    *reads = 0;
    *writes = 0;

    fp = fopen(DISKSTATS_PATH, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " DISKSTATS_PATH);
        return;
    }

    // Leer las estadísticas de disco
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        unsigned int major, minor;
        char device[32];
        unsigned long long read_sectors, write_sectors;

        // Parsear la línea para obtener las estadísticas de disco
        if (sscanf(buffer, "%u %u %s %*u %*u %llu %*u %*u %*u %llu", &major, &minor, device, &read_sectors,
                   &write_sectors) == 5)
        {
            *reads += read_sectors;
            *writes += write_sectors;
        }
    }

    fclose(fp);
}

void get_network_stats(unsigned long long* rx_bytes, unsigned long long* tx_bytes)
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    *rx_bytes = 0;
    *tx_bytes = 0;

    fp = fopen(NETDEV_PATH, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " NETDEV_PATH);
        return;
    }

    // Saltar las dos primeras líneas de encabezado
    fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        char interface[BUFFER_SIZE];
        unsigned long long rx_packets, rx_errs, rx_drop, rx_fifo, rx_frame, rx_compressed, rx_multicast;
        unsigned long long tx_packets, tx_errs, tx_drop, tx_fifo, tx_colls, tx_carrier, tx_compressed;

        sscanf(buffer, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", interface,
               rx_bytes, &rx_packets, &rx_errs, &rx_drop, &rx_fifo, &rx_frame, &rx_compressed, &rx_multicast, tx_bytes,
               &tx_packets, &tx_errs, &tx_drop, &tx_fifo, &tx_colls, &tx_carrier, &tx_compressed);
    }

    fclose(fp);
}

int get_process_count()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    int process_count = 0;

    fp = fopen(STAT_PATH, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " STAT_PATH);
        return -1;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "procs_running %d", &process_count) == 1)
        {
            break;
        }
    }

    fclose(fp);
    return process_count;
}

unsigned long long get_context_switches()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long context_switches = 0;

    fp = fopen(STAT_PATH, "r");
    if (fp == NULL)
    {
        perror("Error al abrir " STAT_PATH);
        return 0;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "ctxt %llu", &context_switches) == 1)
        {
            break;
        }
    }

    fclose(fp);
    return context_switches;
}
