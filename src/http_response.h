#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <stddef.h>

typedef struct {
    int status;
    const char* reason;
    const char* content_type;
    const void* body;
    size_t body_len;
    int send_body;
} HttpResponseSpec;

void http_send_response(int client_socket, const HttpResponseSpec* response);

#endif