# Request Line

## Parsing Rules
- Only the request line is parsed.
- The request target must start with `/`.
- Query strings are ignored for routing (only the path segment is used).
- Parsing returns an explicit parse result (`ok` + request line view).

## Methods
- `GET` and `HEAD` are accepted.
- Any other method results in `405 Method Not Allowed`.
- Methods classify to `GET`, `HEAD`, or `UNKNOWN`.
