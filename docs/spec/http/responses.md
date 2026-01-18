# Routes and Responses

## `/`
- Status: `200 OK`
- `Content-Type: text/html; charset=UTF-8`
- `Content-Length` equals the size of the configured HTML file.
- `GET` returns the body; `HEAD` returns headers only.

## `/healthz`
- Status: `200 OK`
- `Content-Type: text/plain; charset=UTF-8`
- Body: `ok\n` for `GET`; headers-only for `HEAD`.

## Any Other Path
- Status: `404 Not Found`
- Body: `Not Found\n`
