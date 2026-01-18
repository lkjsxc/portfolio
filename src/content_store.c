#define _POSIX_C_SOURCE 200809L
#include "content_store.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static void die_perror(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void content_load(Content* out, const char* path) {
    int fd = open(path, O_RDONLY);
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
    out->data = malloc(size);
    if (!out->data && size != 0) {
        die_perror("Allocation failed");
    }
    out->length = size;

    size_t got = 0;
    while (got < size) {
        ssize_t n = read(fd, out->data + got, size - got);
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
                path,
                size,
                got);
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "[Content] Loaded %zu bytes from %s\n", out->length, path);
}

void content_free(Content* content) {
    free(content->data);
    content->data = NULL;
    content->length = 0;
}