#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "app_config.h"
#include "content_store.h"

#define REQUEST_MAX_BYTES 8192

void handle_client(int client_socket, const AppConfig* config, const Content* content);

#endif