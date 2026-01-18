# Timeouts and Limits

## Socket Timeouts
- Send and receive timeouts are both set to `IO_TIMEOUT_SECONDS`.

## Request Size Limit
- The server reads up to 8192 bytes per request.
- Bytes beyond this limit are ignored.

## Parsing Limitations
- Only the request line is parsed; headers are ignored.
