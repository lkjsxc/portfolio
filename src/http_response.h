#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <stddef.h>

void http_send_response(int client_socket,
                        int status,
                        const char* reason,
                        const char* content_type,
                        const void* body,
                        size_t body_len,
                        int send_body);

#endif