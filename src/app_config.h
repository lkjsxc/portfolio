#ifndef APP_CONFIG_H
#define APP_CONFIG_H

typedef struct {
    const char* content_path;
    const char* port;
    int io_timeout_seconds;
} AppConfig;

#define DEFAULT_CONTENT_PATH "/main.html"
#define DEFAULT_SERVER_PORT "8080"
#define DEFAULT_IO_TIMEOUT_SECONDS 5

void config_load(AppConfig* out);

#endif