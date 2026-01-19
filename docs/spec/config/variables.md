# Variables

Configuration is provided via environment variables.

- `PORT`
  - TCP port string passed to `getaddrinfo`.
  - Default: `8080`.
- `CONTENT_PATH`
  - Absolute or relative path to the HTML file served at `/`.
  - Default: `/main.html`.
- `IO_TIMEOUT_SECONDS`
  - Positive integer for socket send/receive timeouts.
  - Default: `5`.
- `ASYNC_WORKERS`
  - Positive integer for worker thread count.
  - Default: `4`.
- `ASYNC_QUEUE_SIZE`
  - Positive integer for the async dispatch queue capacity.
  - Default: `128`.
