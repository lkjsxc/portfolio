#define _POSIX_C_SOURCE 200809L
#include "http_request.h"

#include <string.h>

int http_parse_request_line(const char* req, HttpRequestLine* out) {
    const char* line_end = strstr(req, "\r\n");
    if (!line_end) {
        return 0;
    }

    const char* sp1 = memchr(req, ' ', (size_t)(line_end - req));
    if (!sp1) {
        return 0;
    }

    out->method = req;
    out->method_len = (size_t)(sp1 - req);

    const char* target_start = sp1 + 1;
    const char* sp2 = memchr(target_start, ' ', (size_t)(line_end - target_start));
    if (!sp2) {
        return 0;
    }

    out->target = target_start;
    out->target_len = (size_t)(sp2 - target_start);
    return out->target_len > 0;
}

int http_method_is(const HttpRequestLine* req, const char* expected) {
    size_t expected_len = strlen(expected);
    return req->method_len == expected_len &&
           memcmp(req->method, expected, expected_len) == 0;
}

int http_path_equals(const HttpRequestLine* req, const char* path) {
    if (req->target_len == 0 || req->target[0] != '/') {
        return 0;
    }

    const char* q = memchr(req->target, '?', req->target_len);
    size_t path_len = q ? (size_t)(q - req->target) : req->target_len;
    size_t expected_len = strlen(path);
    return path_len == expected_len && memcmp(req->target, path, expected_len) == 0;
}