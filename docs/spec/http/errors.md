# Errors

## Malformed Request
- Status: `400 Bad Request`
- Body: `Bad Request\n`
- The body is always sent because the method may be unknown.

## Method Not Allowed
- Status: `405 Method Not Allowed`
- Body: `Method Not Allowed\n`
- The body is always sent regardless of method.

## Transport Errors
- I/O errors during response transmit may close the connection without retry.
