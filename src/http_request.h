#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stddef.h>

typedef struct {
    const char* method;
    size_t method_len;
    const char* target;
    size_t target_len;
} HttpRequestLine;

typedef enum {
    HTTP_METHOD_UNKNOWN = 0,
    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD
} HttpMethod;

typedef enum {
    HTTP_ROUTE_UNKNOWN = 0,
    HTTP_ROUTE_ROOT,
    HTTP_ROUTE_HEALTHZ
} HttpRoute;

typedef struct {
    int ok;
    HttpRequestLine line;
} HttpParseResult;

HttpParseResult http_parse_request_line(const char* req);
HttpMethod http_method_from_line(const HttpRequestLine* req);
HttpRoute http_route_from_line(const HttpRequestLine* req);

#endif