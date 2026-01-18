# Overview

## Scope
Single-process HTTP server that serves one static HTML file at `/` and a health endpoint at `/healthz`.

## Behavior Summary
- Listens on a configured TCP port (default: 8080).
- Supports IPv4 and IPv6 (best-effort binding).
- Handles only `GET` and `HEAD` requests.
- Routes:
  - `/` -> serves the configured HTML content.
  - `/healthz` -> serves a plain-text health response.
- Any other path returns `404`.
- Any unsupported method returns `405`.
- Malformed request lines return `400`.
- Loads the HTML content into memory during startup and serves it as-is.
- Processes a single request per connection and closes the connection after responding.

## Consolidated Conflicts
- The runtime port is controlled by `PORT` (default 8080), while the container exposes 8080. When `PORT` is set to a non-default value, the container runtime must map that port explicitly.
- The content path is controlled by `CONTENT_PATH` (default `/main.html`). The container image only includes `/main.html`; overriding `CONTENT_PATH` requires mounting or baking a different file into the image.
