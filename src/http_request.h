#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stddef.h>

typedef struct {
    const char* method;
    size_t method_len;
    const char* target;
    size_t target_len;
} HttpRequestLine;

int http_parse_request_line(const char* req, HttpRequestLine* out);
int http_method_is(const HttpRequestLine* req, const char* expected);
int http_path_equals(const HttpRequestLine* req, const char* path);

#endif