# Overview

## Scope
Single-process HTTP server that serves one static HTML file at `/` and a health endpoint at `/healthz`.

## Behavior Summary
- Listens on a configured TCP port (default: 8080).
- Supports IPv4 and IPv6.
- Handles only `GET` and `HEAD` requests.
- Routes:
  - `/` -> serves the configured HTML content.
  - `/healthz` -> serves a plain-text health response.
- Any other path returns `404`.
- Any unsupported method returns `405`.
- Malformed request lines return `400`.

## Conflict Consolidation
- The runtime port is controlled by `PORT` (default 8080), while the container exposes 8080 in the Dockerfile. When `PORT` is set to a non-default value, ensure the container runtime maps that port explicitly.
- The content path is controlled by `CONTENT_PATH` (default `/main.html`). The Docker image copies `main.html` to `/main.html` and does not provide any other content files.
