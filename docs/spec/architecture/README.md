# Architecture

Table of contents for architecture and functional design.

- [Functional Pipeline](pipeline.md)
- [Data Model](data-model.md)

## Summary
- Functional core: parse -> classify -> route -> response spec.
- Imperative shell: socket accept, read, write, and content loading.
- All request handling decisions are expressed as pure functions over inputs.
