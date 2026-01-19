#define _POSIX_C_SOURCE 200809L
#include "app_config.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>

static const char* env_or_default(const char* name, const char* fallback) {
    const char* value = getenv(name);
    if (!value || value[0] == '\0') {
        return fallback;
    }
    return value;
}

static int parse_positive_int(const char* value, int fallback) {
    if (!value || value[0] == '\0') {
        return fallback;
    }

    errno = 0;
    char* end = NULL;
    long parsed = strtol(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0') {
        return fallback;
    }
    if (parsed <= 0 || parsed > INT_MAX) {
        return fallback;
    }
    return (int)parsed;
}

AppConfig config_load(void) {
    AppConfig config;
    config.content_path = env_or_default("CONTENT_PATH", DEFAULT_CONTENT_PATH);
    config.port = env_or_default("PORT", DEFAULT_SERVER_PORT);
    config.io_timeout_seconds = parse_positive_int(getenv("IO_TIMEOUT_SECONDS"),
                                                   DEFAULT_IO_TIMEOUT_SECONDS);
    config.async_workers = parse_positive_int(getenv("ASYNC_WORKERS"),
                                              DEFAULT_ASYNC_WORKERS);
    config.async_queue_size = parse_positive_int(getenv("ASYNC_QUEUE_SIZE"),
                                                 DEFAULT_ASYNC_QUEUE_SIZE);
    return config;
}
