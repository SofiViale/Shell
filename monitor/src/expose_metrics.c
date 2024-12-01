#include "../include/expose_metrics.h"

#define HTTP_PORT 8000
#define SLEEP_DURATION 1

/** Mutex para sincronización de hilos */
pthread_mutex_t lock;

/** Métrica de Prometheus para el uso de CPU */
static prom_gauge_t* cpu_usage_metric;

/** Métrica de Prometheus para el uso de memoria */
static prom_gauge_t* memory_usage_metric;

// Declaraciones de métricas
static prom_gauge_t* memory_total_metric;
static prom_gauge_t* memory_used_metric;
static prom_gauge_t* memory_free_metric;
static prom_gauge_t* process_count_metric;
static prom_gauge_t* context_switches_metric;
static prom_gauge_t* disk_read_metric;
static prom_gauge_t* disk_write_metric;
static prom_gauge_t* network_rx_metric;
static prom_gauge_t* network_tx_metric;

void update_cpu_gauge()
{
    double usage = get_cpu_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(cpu_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de CPU\n");
    }
}

void update_memory_gauge()
{
    double usage = get_memory_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de memoria\n");
    }
}

void* expose_metrics(void* arg)
{
    (void)arg; // Argumento no utilizado

    // Aseguramos que el manejador HTTP esté adjunto al registro por defecto
    promhttp_set_active_collector_registry(NULL);

    // Iniciamos el servidor HTTP en el puerto definido
    struct MHD_Daemon* daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, HTTP_PORT, NULL, NULL);
    if (daemon == NULL)
    {
        fprintf(stderr, "Error al iniciar el servidor HTTP\n");
        return NULL;
    }

    // Mantenemos el servidor en ejecución
    while (1)
    {
        sleep(SLEEP_DURATION);
    }

    // Nunca debería llegar aquí
    MHD_stop_daemon(daemon);
    return NULL;
}

/** Funciones para actualizar las métricas */
void update_memory_gauge2()
{
    double total_mem, used_mem, free_mem;
    get_memory_usage2(&total_mem, &used_mem, &free_mem);

    if (total_mem >= 0 && used_mem >= 0 && free_mem >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_total_metric, total_mem, NULL);
        prom_gauge_set(memory_used_metric, used_mem, NULL);
        prom_gauge_set(memory_free_metric, free_mem, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de memoria\n");
    }
}

void update_disk_io_gauge()
{
    unsigned long long reads, writes;
    get_disk_io_stats(&reads, &writes);

    pthread_mutex_lock(&lock);
    prom_gauge_set(disk_read_metric, reads, NULL);
    prom_gauge_set(disk_write_metric, writes, NULL);
    pthread_mutex_unlock(&lock);
}

void update_network_gauge()
{
    unsigned long long rx_bytes, tx_bytes;
    get_network_stats(&rx_bytes, &tx_bytes);

    pthread_mutex_lock(&lock);
    prom_gauge_set(network_rx_metric, rx_bytes, NULL);
    prom_gauge_set(network_tx_metric, tx_bytes, NULL);
    pthread_mutex_unlock(&lock);
}

void update_process_count_gauge()
{
    int process_count = get_process_count();
    if (process_count >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(process_count_metric, process_count, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el conteo de procesos\n");
    }
}

void update_context_switches_gauge()
{
    unsigned long long context_switches = get_context_switches();
    if (context_switches >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(context_switches_metric, context_switches, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener los cambios de contexto\n");
    }
}

int init_metrics()
{
    // Inicializamos el mutex
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "Error al inicializar el mutex\n");
        return EXIT_FAILURE;
    }

    // Inicializamos el registro de coleccionistas de Prometheus
    if (prom_collector_registry_default_init() != 0)
    {
        fprintf(stderr, "Error al inicializar el registro de Prometheus\n");
        return EXIT_FAILURE;
    }

    // Creamos la métrica para el uso de CPU
    cpu_usage_metric = prom_gauge_new("cpu_usage_percentage", "Porcentaje de uso de CPU", 0, NULL);
    if (cpu_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de CPU\n");
        return EXIT_FAILURE;
    }

    // Creamos la métrica para el uso de memoria
    memory_usage_metric = prom_gauge_new("memory_usage_percentage", "Porcentaje de uso de memoria", 0, NULL);
    if (memory_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de memoria\n");
        return EXIT_FAILURE;
    }

    // Creamos las métricas adicionales
    memory_total_metric = prom_gauge_new("memory_total", "Total Memory", 0, NULL);
    if (memory_total_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de memoria total\n");
        return EXIT_FAILURE;
    }

    memory_used_metric = prom_gauge_new("memory_used", "Used Memory", 0, NULL);
    if (memory_used_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de memoria usada\n");
        return EXIT_FAILURE;
    }

    memory_free_metric = prom_gauge_new("memory_free", "Free Memory", 0, NULL);
    if (memory_free_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de memoria libre\n");
        return EXIT_FAILURE;
    }

    process_count_metric = prom_gauge_new("process_count", "Process Count", 0, NULL);
    if (process_count_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de conteo de procesos\n");
        return EXIT_FAILURE;
    }

    context_switches_metric = prom_gauge_new("context_switches", "Context Switches", 0, NULL);
    if (context_switches_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de cambios de contexto\n");
        return EXIT_FAILURE;
    }

    disk_read_metric = prom_gauge_new("disk_read", "Disk Read", 0, NULL);
    if (disk_read_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de lectura de disco\n");
        return EXIT_FAILURE;
    }

    disk_write_metric = prom_gauge_new("disk_write", "Disk Write", 0, NULL);
    if (disk_write_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de escritura de disco\n");
        return EXIT_FAILURE;
    }

    network_rx_metric = prom_gauge_new("network_rx", "Network RX", 0, NULL);
    if (network_rx_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de recepción de red\n");
        return EXIT_FAILURE;
    }

    network_tx_metric = prom_gauge_new("network_tx", "Network TX", 0, NULL);
    if (network_tx_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de transmisión de red\n");
        return EXIT_FAILURE;
    }

    // Registramos las métricas en el registro por defecto
    prom_collector_registry_must_register_metric(cpu_usage_metric);
    prom_collector_registry_must_register_metric(memory_usage_metric);
    prom_collector_registry_must_register_metric(memory_total_metric);
    prom_collector_registry_must_register_metric(memory_used_metric);
    prom_collector_registry_must_register_metric(memory_free_metric);
    prom_collector_registry_must_register_metric(process_count_metric);
    prom_collector_registry_must_register_metric(context_switches_metric);
    prom_collector_registry_must_register_metric(disk_read_metric);
    prom_collector_registry_must_register_metric(disk_write_metric);
    prom_collector_registry_must_register_metric(network_rx_metric);
    prom_collector_registry_must_register_metric(network_tx_metric);

    return EXIT_SUCCESS;
}

void destroy_mutex()
{
    pthread_mutex_destroy(&lock);
}
