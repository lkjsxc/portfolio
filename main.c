#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_CONTENT_PATH "/main.html"
#define DEFAULT_SERVER_PORT "8080"
#define REQUEST_MAX_BYTES 8192
#define DEFAULT_IO_TIMEOUT_SECONDS 5

typedef struct {
    char* data;
    size_t length;
} ContentProvider;

typedef struct {
    int fds[2];
    nfds_t fds_len;
} HttpServer;

typedef struct {
    const char* content_path;
    const char* port;
    int io_timeout_seconds;
} AppConfig;

static ContentProvider AppContent;
static HttpServer AppServer;
static AppConfig AppSettings;

static void die_perror(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

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

static void config_init() {
    AppSettings.content_path = env_or_default("CONTENT_PATH", DEFAULT_CONTENT_PATH);
    AppSettings.port = env_or_default("PORT", DEFAULT_SERVER_PORT);
    AppSettings.io_timeout_seconds = parse_positive_int(getenv("IO_TIMEOUT_SECONDS"),
                                                        DEFAULT_IO_TIMEOUT_SECONDS);
}

static void set_socket_timeouts(int fd) {
    struct timeval tv;
    tv.tv_sec = AppSettings.io_timeout_seconds;
    tv.tv_usec = 0;

    (void)setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    (void)setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

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
        size_t chunk = remaining;
        if (chunk > (size_t)SSIZE_MAX) {
            chunk = (size_t)SSIZE_MAX;
        }

#ifdef MSG_NOSIGNAL
        ssize_t n = send(fd, p + sent, chunk, MSG_NOSIGNAL);
#else
        ssize_t n = send(fd, p + sent, chunk, 0);
#endif
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EPIPE || errno == ECONNRESET) {
                return -1;
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

static void send_response(int client_socket,
                          int status,
                          const char* reason,
                          const char* content_type,
                          const void* body,
                          size_t body_len,
                          int send_body) {
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
                              status,
                              reason,
                              date,
                              content_type,
                              body_len);
    } else {
        header_len = snprintf(header,
                              sizeof(header),
                              "HTTP/1.1 %d %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              status,
                              reason,
                              content_type,
                              body_len);
    }

    if (header_len < 0 || (size_t)header_len >= sizeof(header)) {
        return;
    }

    if (send_all(client_socket, header, (size_t)header_len) != 0) {
        return;
    }
    if (send_body && body_len > 0) {
        (void)send_all(client_socket, body, body_len);
    }
}

static void content_init() {
    int fd = open(AppSettings.content_path, O_RDONLY);
    if (fd < 0) {
        die_perror("Failed to open content file");
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        die_perror("Failed to stat content file");
    }
    if (st.st_size < 0) {
        fprintf(stderr, "Invalid content file size\n");
        exit(EXIT_FAILURE);
    }

    size_t size = (size_t)st.st_size;
    AppContent.data = malloc(size);
    if (!AppContent.data && size != 0) {
        die_perror("Allocation failed");
    }
    AppContent.length = size;

    size_t got = 0;
    while (got < size) {
        ssize_t n = read(fd, AppContent.data + got, size - got);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            die_perror("Read failed");
        }
        if (n == 0) {
            break;
        }
        got += (size_t)n;
    }
    close(fd);

    if (got != size) {
        fprintf(stderr,
                "Short read for %s (expected %zu, got %zu)\n",
                AppSettings.content_path,
                size,
                got);
        exit(EXIT_FAILURE);
    }

    fprintf(stderr,
            "[Content] Loaded %zu bytes from %s\n",
            AppContent.length,
            AppSettings.content_path);
}

static int parse_request_line(const char* req,
                              const char** method,
                              size_t* method_len,
                              const char** target,
                              size_t* target_len) {
    const char* line_end = strstr(req, "\r\n");
    if (!line_end) {
        return 0;
    }

    const char* sp1 = memchr(req, ' ', (size_t)(line_end - req));
    if (!sp1) {
        return 0;
    }

    *method = req;
    *method_len = (size_t)(sp1 - req);

    const char* target_start = sp1 + 1;
    const char* sp2 = memchr(target_start, ' ', (size_t)(line_end - target_start));
    if (!sp2) {
        return 0;
    }

    *target = target_start;
    *target_len = (size_t)(sp2 - target_start);
    return *target_len > 0;
}

static int method_is(const char* method, size_t method_len, const char* expected) {
    size_t expected_len = strlen(expected);
    return method_len == expected_len && memcmp(method, expected, expected_len) == 0;
}

static int path_equals(const char* target, size_t target_len, const char* path) {
    if (target_len == 0 || target[0] != '/') {
        return 0;
    }

    const char* q = memchr(target, '?', target_len);
    size_t path_len = q ? (size_t)(q - target) : target_len;
    size_t expected_len = strlen(path);
    return path_len == expected_len && memcmp(target, path, expected_len) == 0;
}

static void handle_client(int client_socket) {
    set_socket_timeouts(client_socket);

    char req[REQUEST_MAX_BYTES + 1];
    size_t used = 0;
    req[0] = '\0';

    while (used < REQUEST_MAX_BYTES) {
        ssize_t n = recv(client_socket, req + used, REQUEST_MAX_BYTES - used, 0);
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
        req[used] = '\0';
        if (strstr(req, "\r\n\r\n") != NULL) {
            break;
        }
    }

    const char* method = NULL;
    const char* target = NULL;
    size_t method_len = 0;
    size_t target_len = 0;

    if (!parse_request_line(req, &method, &method_len, &target, &target_len)) {
        static const char body[] = "Bad Request\n";
        send_response(client_socket,
                      400,
                      "Bad Request",
                      "text/plain; charset=UTF-8",
                      body,
                      sizeof(body) - 1,
                      1);
        return;
    }

    int is_get = method_is(method, method_len, "GET");
    int is_head = method_is(method, method_len, "HEAD");
    if (!is_get && !is_head) {
        static const char body[] = "Method Not Allowed\n";
        send_response(client_socket,
                      405,
                      "Method Not Allowed",
                      "text/plain; charset=UTF-8",
                      body,
                      sizeof(body) - 1,
                      1);
        return;
    }

    int send_body = is_get ? 1 : 0;
    if (path_equals(target, target_len, "/")) {
        send_response(client_socket,
                      200,
                      "OK",
                      "text/html; charset=UTF-8",
                      AppContent.data,
                      AppContent.length,
                      send_body);
        return;
    }

    if (path_equals(target, target_len, "/healthz")) {
        static const char body[] = "ok\n";
        send_response(client_socket,
                      200,
                      "OK",
                      "text/plain; charset=UTF-8",
                      body,
                      sizeof(body) - 1,
                      send_body);
        return;
    }

    static const char body[] = "Not Found\n";
    send_response(client_socket,
                  404,
                  "Not Found",
                  "text/plain; charset=UTF-8",
                  body,
                  sizeof(body) - 1,
                  send_body);
}

static void server_init() {
    AppServer.fds_len = 0;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* res = NULL;
    int gai = getaddrinfo(NULL, AppSettings.port, &hints, &res);
    if (gai != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
        exit(EXIT_FAILURE);
    }

    int have_v4 = 0;
    int have_v6 = 0;
    for (struct addrinfo* ai = res; ai != NULL; ai = ai->ai_next) {
        if (AppServer.fds_len >= 2) {
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

        AppServer.fds[AppServer.fds_len++] = fd;
        if (ai->ai_family == AF_INET) {
            have_v4 = 1;
        }
        if (ai->ai_family == AF_INET6) {
            have_v6 = 1;
        }
    }

    freeaddrinfo(res);

    if (AppServer.fds_len == 0) {
        die_perror("Failed to bind any listen socket");
    }

        fprintf(stderr, "[Server] Listening on port %s (%s%s)\n",
            AppSettings.port,
            have_v6 ? "IPv6" : "",
            (have_v6 && have_v4) ? "+IPv4" : (have_v4 ? "IPv4" : ""));
}

static void server_start() {
    struct pollfd pfds[2];
    for (nfds_t i = 0; i < AppServer.fds_len; i++) {
        pfds[i].fd = AppServer.fds[i];
        pfds[i].events = POLLIN;
        pfds[i].revents = 0;
    }

    while (1) {
        int n = poll(pfds, AppServer.fds_len, -1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            die_perror("poll failed");
        }

        for (nfds_t i = 0; i < AppServer.fds_len; i++) {
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

            handle_client(client);
            close(client);
        }
    }
}

int main() {
    (void)signal(SIGPIPE, SIG_IGN);
    config_init();
    content_init();
    server_init();
    server_start();
}
