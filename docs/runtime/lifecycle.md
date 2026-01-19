# Lifecycle

## Startup Sequence
1. Load configuration from environment.
2. Read the configured HTML file into memory.
3. Bind listening sockets (IPv4 and/or IPv6).
4. Start the asynchronous worker pool.
5. Enter the poll loop and accept connections.

## Connection Handling
- Single-threaded accept loop with a bounded dispatch queue.
- Requests are processed concurrently by worker threads.
- One request per connection; connection always closed after a response.
- Request handling uses a functional core (parse -> classify -> respond).
- All side effects remain in socket IO and content loading.

## Shutdown
- No graceful shutdown mechanism; the process exits only on fatal error or external termination.
