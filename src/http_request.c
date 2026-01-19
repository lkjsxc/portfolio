#define _POSIX_C_SOURCE 200809L
#include "http_request.h"

#include <string.h>

static int token_equals(const char* token, size_t token_len, const char* expected) {
    size_t expected_len = strlen(expected);
    return token_len == expected_len && memcmp(token, expected, expected_len) == 0;
}

static size_t request_path_len(const HttpRequestLine* req) {
    if (req->target_len == 0 || req->target[0] != '/') {
        return 0;
    }

    const char* q = memchr(req->target, '?', req->target_len);
    return q ? (size_t)(q - req->target) : req->target_len;
}

HttpParseResult http_parse_request_line(const char* req) {
    HttpParseResult result = {0};
    const char* line_end = strstr(req, "\r\n");
    if (!line_end) {
        return result;
    }

    const char* sp1 = memchr(req, ' ', (size_t)(line_end - req));
    if (!sp1) {
        return result;
    }

    result.line.method = req;
    result.line.method_len = (size_t)(sp1 - req);

    const char* target_start = sp1 + 1;
    const char* sp2 = memchr(target_start, ' ', (size_t)(line_end - target_start));
    if (!sp2) {
        return result;
    }

    result.line.target = target_start;
    result.line.target_len = (size_t)(sp2 - target_start);
    result.ok = result.line.target_len > 0;
    return result;
}

HttpMethod http_method_from_line(const HttpRequestLine* req) {
    if (token_equals(req->method, req->method_len, "GET")) {
        return HTTP_METHOD_GET;
    }
    if (token_equals(req->method, req->method_len, "HEAD")) {
        return HTTP_METHOD_HEAD;
    }
    return HTTP_METHOD_UNKNOWN;
}

HttpRoute http_route_from_line(const HttpRequestLine* req) {
    size_t path_len = request_path_len(req);
    if (path_len == 0) {
        return HTTP_ROUTE_UNKNOWN;
    }
    if (token_equals(req->target, path_len, "/")) {
        return HTTP_ROUTE_ROOT;
    }
    return HTTP_ROUTE_UNKNOWN;
}
