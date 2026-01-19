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

static int method_allows_body(HttpMethod method) {
    return method == HTTP_METHOD_GET;
}

static HttpResponseSpec response_plain(int status,
                                       const char* reason,
                                       const char* body,
                                       size_t body_len,
                                       int send_body) {
    HttpResponseSpec response = {
        .status = status,
        .reason = reason,
        .content_type = "text/plain; charset=UTF-8",
        .body = body,
        .body_len = body_len,
        .send_body = send_body,
    };
    return response;
}

static HttpResponseSpec response_html_ok(const Content* content, int send_body) {
    HttpResponseSpec response = {
        .status = 200,
        .reason = "OK",
        .content_type = "text/html; charset=UTF-8",
        .body = content->data,
        .body_len = content->length,
        .send_body = send_body,
    };
    return response;
}

static HttpResponseSpec response_bad_request(void) {
    static const char body[] = "Bad Request\n";
    return response_plain(400, "Bad Request", body, sizeof(body) - 1, 1);
}

static HttpResponseSpec response_method_not_allowed(void) {
    static const char body[] = "Method Not Allowed\n";
    return response_plain(405, "Method Not Allowed", body, sizeof(body) - 1, 1);
}

static HttpResponseSpec response_not_found(int send_body) {
    static const char body[] = "Not Found\n";
    return response_plain(404, "Not Found", body, sizeof(body) - 1, send_body);
}

static HttpResponseSpec response_for_route(HttpRoute route,
                                           const Content* content,
                                           int send_body) {
    switch (route) {
        case HTTP_ROUTE_ROOT:
            return response_html_ok(content, send_body);
        case HTTP_ROUTE_UNKNOWN:
        default:
            return response_not_found(send_body);
    }
}

void handle_client(int client_socket, const AppConfig* config, const Content* content) {
    set_socket_timeouts(client_socket, config->io_timeout_seconds);

    char req[REQUEST_MAX_BYTES + 1];
    (void)receive_request(client_socket, req, REQUEST_MAX_BYTES);

    HttpParseResult parsed = http_parse_request_line(req);
    if (!parsed.ok) {
        HttpResponseSpec response = response_bad_request();
        http_send_response(client_socket, &response);
        return;
    }

    HttpMethod method = http_method_from_line(&parsed.line);
    if (method == HTTP_METHOD_UNKNOWN) {
        HttpResponseSpec response = response_method_not_allowed();
        http_send_response(client_socket, &response);
        return;
    }

    int send_body = method_allows_body(method);
    HttpRoute route = http_route_from_line(&parsed.line);
    HttpResponseSpec response = response_for_route(route, content, send_body);
    http_send_response(client_socket, &response);
}
