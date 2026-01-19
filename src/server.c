#define _POSIX_C_SOURCE 200809L
#include "server.h"

#include "async_pool.h"
#include "request_handler.h"

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    int fds[2];
    nfds_t fds_len;
} HttpServer;

typedef struct {
    const AppConfig* config;
    const Content* content;
} WorkerContext;

static void die_perror(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static HttpServer server_init(const AppConfig* config, int* out_has_v4, int* out_has_v6) {
    HttpServer server;
    server.fds_len = 0;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* res = NULL;
    int gai = getaddrinfo(NULL, config->port, &hints, &res);
    if (gai != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
        exit(EXIT_FAILURE);
    }

    int have_v4 = 0;
    int have_v6 = 0;
    for (struct addrinfo* ai = res; ai != NULL; ai = ai->ai_next) {
        if (server.fds_len >= 2) {
            break;
        }
        if (ai->ai_family == AF_INET && have_v4) {
            continue;
        }
        if (ai->ai_family == AF_INET6 && have_v6) {
            continue;
        }
        if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6) {
            continue;
        }

        int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) {
            continue;
        }

        int one = 1;
        (void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        if (ai->ai_family == AF_INET6) {
            (void)setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one));
        }

        if (bind(fd, ai->ai_addr, ai->ai_addrlen) != 0) {
            close(fd);
            continue;
        }

        if (listen(fd, SOMAXCONN) != 0) {
            close(fd);
            continue;
        }

        server.fds[server.fds_len++] = fd;
        if (ai->ai_family == AF_INET) {
            have_v4 = 1;
        }
        if (ai->ai_family == AF_INET6) {
            have_v6 = 1;
        }
    }

    freeaddrinfo(res);

    if (server.fds_len == 0) {
        die_perror("Failed to bind any listen socket");
    }

    fprintf(stderr,
            "[Server] Listening on port %s (%s%s)\n",
            config->port,
            have_v6 ? "IPv6" : "",
            (have_v6 && have_v4) ? "+IPv4" : (have_v4 ? "IPv4" : ""));

    if (out_has_v4) {
        *out_has_v4 = have_v4;
    }
    if (out_has_v6) {
        *out_has_v6 = have_v6;
    }

    return server;
}

static void handle_client_task(int client_fd, void* ctx) {
    const WorkerContext* worker = (const WorkerContext*)ctx;
    handle_client(client_fd, worker->config, worker->content);
    close(client_fd);
}

static void server_loop(const HttpServer* server, AsyncPool* pool) {
    struct pollfd pfds[2];
    for (nfds_t i = 0; i < server->fds_len; i++) {
        pfds[i].fd = server->fds[i];
        pfds[i].events = POLLIN;
        pfds[i].revents = 0;
    }

    while (1) {
        int n = poll(pfds, server->fds_len, -1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            die_perror("poll failed");
        }

        for (nfds_t i = 0; i < server->fds_len; i++) {
            if ((pfds[i].revents & POLLIN) == 0) {
                continue;
            }

            struct sockaddr_storage client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client = accept(pfds[i].fd, (struct sockaddr*)&client_addr, &client_len);
            if (client < 0) {
                if (errno == EINTR) {
                    continue;
                }
                perror("accept");
                continue;
            }

            async_pool_submit(pool, client);
        }
    }
}

void server_run(const AppConfig* config, const Content* content) {
    int have_v4 = 0;
    int have_v6 = 0;
    HttpServer server = server_init(config, &have_v4, &have_v6);
    WorkerContext worker = {
        .config = config,
        .content = content,
    };
    AsyncPool pool = async_pool_start((size_t)config->async_workers,
                                      (size_t)config->async_queue_size,
                                      handle_client_task,
                                      &worker);
    (void)have_v4;
    (void)have_v6;
    server_loop(&server, &pool);
    async_pool_stop(&pool);
}
