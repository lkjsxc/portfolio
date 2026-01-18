# Functional Pipeline

## Stages
1. Read bytes (effectful).
2. Parse the request line into a structured view.
3. Classify method and route using pure functions.
4. Build an `HttpResponseSpec` (pure).
5. Send the response and close the connection (effectful).

## Core Invariants
- Parsing never mutates input; it returns a value or failure.
- Classification is total: unknown input maps to explicit `UNKNOWN` variants.
- Response construction is deterministic based on method, route, and content.
