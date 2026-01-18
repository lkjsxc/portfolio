#define _POSIX_C_SOURCE 200809L
#include "app_config.h"
#include "content_store.h"
#include "server.h"

#include <signal.h>

int main() {
    (void)signal(SIGPIPE, SIG_IGN);

    AppConfig config;
    Content content;

    config_load(&config);
    content_load(&content, config.content_path);
    server_run(&config, &content);
    content_free(&content);
    return 0;
}