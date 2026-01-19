#ifndef APP_CONFIG_H
#define APP_CONFIG_H

typedef struct {
    const char* content_path;
    const char* port;
    int io_timeout_seconds;
    int async_workers;
    int async_queue_size;
} AppConfig;

#define DEFAULT_CONTENT_PATH "/main.html"
#define DEFAULT_SERVER_PORT "8080"
#define DEFAULT_IO_TIMEOUT_SECONDS 5
#define DEFAULT_ASYNC_WORKERS 4
#define DEFAULT_ASYNC_QUEUE_SIZE 128

AppConfig config_load(void);

#endif
