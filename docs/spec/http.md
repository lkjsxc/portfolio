# HTTP Contract

## Request Line
- Only the request line is parsed.
- The request target must start with `/`.
- Query strings are ignored for routing (only the path segment is used).

## Methods
- `GET` and `HEAD` are accepted.
- Any other method results in `405 Method Not Allowed`.

## Routes and Responses
- `/`
  - `200 OK`
  - `Content-Type: text/html; charset=UTF-8`
  - `Content-Length` equals the size of the configured HTML file.
  - `GET` returns the body; `HEAD` returns headers only.
- `/healthz`
  - `200 OK`
  - `Content-Type: text/plain; charset=UTF-8`
  - Body: `ok\n` for `GET`; headers-only for `HEAD`.
- Any other path
  - `404 Not Found`
  - Body: `Not Found\n`

## Error Handling
- Malformed request line -> `400 Bad Request` with body `Bad Request\n`.
- I/O errors during response transmit may close the connection without retry.
