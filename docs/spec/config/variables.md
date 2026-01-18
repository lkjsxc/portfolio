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
