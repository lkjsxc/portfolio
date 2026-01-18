# Container Build

## Image Behavior
- Multi-stage build produces a static binary.
- Final image is `scratch` and includes:
  - `/main` (server binary)
  - `/main.html` (served content)
- `EXPOSE 8080` documents the default port.

## Runtime Configuration
- Override the port via `PORT`.
- Override content via `CONTENT_PATH`.
