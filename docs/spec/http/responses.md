# Routes and Responses

## Response Spec
Responses are built as an immutable spec:
- `status`, `reason`, `content_type`, `body`, `body_len`, `send_body`.
- `send_body` is true for `GET` and false for `HEAD`.

## `/`
- Status: `200 OK`
- `Content-Type: text/html; charset=UTF-8`
- `Content-Length` equals the size of the configured HTML file.
- `GET` returns the body; `HEAD` returns headers only.

## Any Other Path
- Status: `404 Not Found`
- Body: `Not Found\n`
