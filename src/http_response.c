#define _POSIX_C_SOURCE 200809L
#include "http_response.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static void http_date(char out[static 64]) {
    time_t now = time(NULL);
    struct tm tm_utc;
    if (gmtime_r(&now, &tm_utc) == NULL) {
        out[0] = '\0';
        return;
    }

    if (strftime(out, 64, "%a, %d %b %Y %H:%M:%S GMT", &tm_utc) == 0) {
        out[0] = '\0';
    }
}

static int send_all(int fd, const void* buf, size_t len) {
    const unsigned char* p = (const unsigned char*)buf;
    size_t sent = 0;

    while (sent < len) {
        size_t remaining = len - sent;
        size_t chunk = remaining > (size_t)SSIZE_MAX ? (size_t)SSIZE_MAX : remaining;

#ifdef MSG_NOSIGNAL
        ssize_t n = send(fd, p + sent, chunk, MSG_NOSIGNAL);
#else
        ssize_t n = send(fd, p + sent, chunk, 0);
#endif
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            return -1;
        }
        sent += (size_t)n;
    }

    return 0;
}

void http_send_response(int client_socket, const HttpResponseSpec* response) {
    char date[64];
    http_date(date);

    char header[512];
    int header_len;
    if (date[0] != '\0') {
        header_len = snprintf(header,
                              sizeof(header),
                              "HTTP/1.1 %d %s\r\n"
                              "Date: %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              response->status,
                              response->reason,
                              date,
                              response->content_type,
                              response->body_len);
    } else {
        header_len = snprintf(header,
                              sizeof(header),
                              "HTTP/1.1 %d %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              response->status,
                              response->reason,
                              response->content_type,
                              response->body_len);
    }

    if (header_len < 0 || (size_t)header_len >= sizeof(header)) {
        return;
    }

    if (send_all(client_socket, header, (size_t)header_len) != 0) {
        return;
    }
    if (response->send_body && response->body_len > 0) {
        (void)send_all(client_socket, response->body, response->body_len);
    }
}