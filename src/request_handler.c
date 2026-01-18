#define _POSIX_C_SOURCE 200809L
#include "request_handler.h"

#include "http_request.h"
#include "http_response.h"

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static void set_socket_timeouts(int fd, int seconds) {
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    (void)setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    (void)setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

static size_t receive_request(int client_socket, char* buffer, size_t cap) {
    size_t used = 0;
    buffer[0] = '\0';

    while (used < cap) {
        ssize_t n = recv(client_socket, buffer + used, cap - used, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (n == 0) {
            break;
        }
        used += (size_t)n;
        buffer[used] = '\0';
        if (strstr(buffer, "\r\n\r\n") != NULL) {
            break;
        }
    }

    return used;
}

void handle_client(int client_socket, const AppConfig* config, const Content* content) {
    set_socket_timeouts(client_socket, config->io_timeout_seconds);

    char req[REQUEST_MAX_BYTES + 1];
    (void)receive_request(client_socket, req, REQUEST_MAX_BYTES);

    HttpRequestLine line;
    if (!http_parse_request_line(req, &line)) {
        static const char body[] = "Bad Request\n";
        http_send_response(client_socket,
                           400,
                           "Bad Request",
                           "text/plain; charset=UTF-8",
                           body,
                           sizeof(body) - 1,
                           1);
        return;
    }

    int is_get = http_method_is(&line, "GET");
    int is_head = http_method_is(&line, "HEAD");
    if (!is_get && !is_head) {
        static const char body[] = "Method Not Allowed\n";
        http_send_response(client_socket,
                           405,
                           "Method Not Allowed",
                           "text/plain; charset=UTF-8",
                           body,
                           sizeof(body) - 1,
                           1);
        return;
    }

    int send_body = is_get ? 1 : 0;
    if (http_path_equals(&line, "/")) {
        http_send_response(client_socket,
                           200,
                           "OK",
                           "text/html; charset=UTF-8",
                           content->data,
                           content->length,
                           send_body);
        return;
    }

    if (http_path_equals(&line, "/healthz")) {
        static const char body[] = "ok\n";
        http_send_response(client_socket,
                           200,
                           "OK",
                           "text/plain; charset=UTF-8",
                           body,
                           sizeof(body) - 1,
                           send_body);
        return;
    }

    static const char body[] = "Not Found\n";
    http_send_response(client_socket,
                       404,
                       "Not Found",
                       "text/plain; charset=UTF-8",
                       body,
                       sizeof(body) - 1,
                       send_body);
}