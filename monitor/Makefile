TARGET = metrics

CC = gcc

SRC_DIR = src
INCLUDE_DIR = include
PROMETHEUS_DIR = prometheus-client-c
BUILD_DIR_PROM = $(PROMETHEUS_DIR)/prom/build
BUILD_DIR_PROMHTTP = $(PROMETHEUS_DIR)/promhttp/build
PROMETHEUS_LIB_DIR = /usr/local/lib
MICROHTTPD_INCLUDE_DIR = /usr/include

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/expose_metrics.c $(SRC_DIR)/metrics.c

CFLAGS = -I$(PROMETHEUS_DIR) -I$(MICROHTTPD_INCLUDE_DIR) -I$(INCLUDE_DIR) -I/usr/include/cjson
LDFLAGS = -L$(PROMETHEUS_LIB_DIR) -lprom -pthread -lpromhttp -lcjson

export LD_LIBRARY_PATH := $(PROMETHEUS_LIB_DIR):$(LD_LIBRARY_PATH)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)
	rm -rf $(PROMETHEUS_DIR)