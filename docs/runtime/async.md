# Asynchronous Processing

## Model
The server accepts connections on the main thread and dispatches each client socket to a
worker pool. Each worker handles request parsing and response writing synchronously, but
multiple connections can be processed concurrently across workers.

## Queueing
- Accepted sockets are enqueued in a bounded ring buffer.
- When the queue is full, the accept loop waits until a worker drains space.
- This provides backpressure without dropping connections.

## Worker Behavior
- One request per connection.
- Request handling reuses the same functional pipeline (parse -> classify -> respond).
- Connections always close after a response.

## Configuration
See `docs/spec/config/variables.md` for `ASYNC_WORKERS` and `ASYNC_QUEUE_SIZE`.
