# Portfolio HTTP Server

Single-process HTTP server that serves one static HTML file at `/`.

## Highlights
- Serves a configured HTML file from memory.
- Handles `GET` and `HEAD` requests.
- Routes: `/` only.
- Defaults: `PORT=8080`, `CONTENT_PATH=/main.html`.

## Quick start (local)
1. Compile:

   ```sh
   gcc -O2 -pthread -o main src/*.c
   ```

2. Run (use a relative content path for local runs):

   ```sh
   CONTENT_PATH=main.html ./main
   ```

## Configuration
- `PORT`: TCP port to bind (default `8080`).
- `CONTENT_PATH`: Path to the HTML file (default `/main.html`).
- `IO_TIMEOUT_SECONDS`: Socket send/receive timeout in seconds (default `5`).
- `ASYNC_WORKERS`: Worker thread count for async processing (default `4`).
- `ASYNC_QUEUE_SIZE`: Dispatch queue capacity for async processing (default `128`).

## Docs
- Full documentation index: [docs/README.md](docs/README.md)
- Build and run (local): [docs/build/local/README.md](docs/build/local/README.md)
- Container notes: [docs/build/container/README.md](docs/build/container/README.md)
- Specification overview: [docs/spec/overview.md](docs/spec/overview.md)

## Content
- Default HTML asset: [main.html](main.html)

## License
See [LICENSE](LICENSE).
