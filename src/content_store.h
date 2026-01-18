#ifndef CONTENT_STORE_H
#define CONTENT_STORE_H

#include <stddef.h>

typedef struct {
    char* data;
    size_t length;
} Content;

Content content_load(const char* path);
void content_free(Content* content);

#endif