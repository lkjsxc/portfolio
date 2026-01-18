#ifndef CONTENT_STORE_H
#define CONTENT_STORE_H

#include <stddef.h>

typedef struct {
    char* data;
    size_t length;
} Content;

void content_load(Content* out, const char* path);
void content_free(Content* content);

#endif