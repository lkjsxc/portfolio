# Image Layout

- Multi-stage build produces a static binary.
- Final image is `scratch` and includes:
  - `/main` (server binary)
  - `/main.html` (served content)
- `EXPOSE 8080` documents the default port.
