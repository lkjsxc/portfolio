#ifndef SERVER_H
#define SERVER_H

#include "app_config.h"
#include "content_store.h"

void server_run(const AppConfig* config, const Content* content);

#endif