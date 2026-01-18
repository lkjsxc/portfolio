# Limits

## Request Size Limit
- The server reads up to 8192 bytes per request.
- Bytes beyond this limit are ignored.

## Parsing Scope
- Only the request line is parsed; headers are ignored.
- Parsing is bounded by the request buffer and stops at `\r\n\r\n`.
