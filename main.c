#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define FILENAME "main.html"
#define SERVER_PORT 80
#define LISTEN_BACKLOG 3
#define BUFFER_SIZE 1024

typedef struct {
    char* data;
    size_t length;
} ContentProvider;

typedef struct {
    int fd;
    struct sockaddr_in address;
} HttpServer;

static ContentProvider AppContent;
static HttpServer AppServer;

static void content_init() {
    FILE* file = fopen(FILENAME, "r");
    if (!file) {
        perror("Failed to open content file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    rewind(file);

    char* raw_html = malloc(fsize + 1);
    if (!raw_html) {
        perror("Allocation failed");
        exit(EXIT_FAILURE);
    }

    if (fread(raw_html, 1, fsize, file) != (size_t)fsize) {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }
    fclose(file);
    raw_html[fsize] = '\0';

    size_t header_est = 256;
    AppContent.data = malloc(fsize + header_est);
    if (!AppContent.data) {
        perror("Response allocation failed");
        exit(EXIT_FAILURE);
    }

    int header_len = sprintf(AppContent.data,
                             "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/html\r\n"
                             "Content-Length: %ld\r\n"
                             "\r\n",
                             fsize);

    memcpy(AppContent.data + header_len, raw_html, fsize);
    AppContent.length = header_len + fsize;

    free(raw_html);
    printf("[Content] Loaded %zu bytes from %s\n", AppContent.length, FILENAME);
}

static void content_serve(int client_socket) {
    send(client_socket, AppContent.data, AppContent.length, 0);
}

static void server_init() {
    int opt = 1;

    if ((AppServer.fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(AppServer.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    AppServer.address.sin_family = AF_INET;
    AppServer.address.sin_addr.s_addr = INADDR_ANY;
    AppServer.address.sin_port = htons(SERVER_PORT);

    if (bind(AppServer.fd, (struct sockaddr*)&AppServer.address, sizeof(AppServer.address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(AppServer.fd, LISTEN_BACKLOG) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[Server] Listening on port %d\n", SERVER_PORT);
}

static void server_start() {
    int addrlen = sizeof(AppServer.address);
    int new_socket;

    while (1) {
        if ((new_socket = accept(AppServer.fd,
                                 (struct sockaddr*)&AppServer.address,
                                 (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        char buffer[BUFFER_SIZE];
        read(new_socket, buffer, BUFFER_SIZE);

        content_serve(new_socket);
        close(new_socket);
    }
}

int main() {
    content_init();
    server_init();
    server_start();
}
