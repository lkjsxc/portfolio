# Request Line

## Parsing Rules
- Only the request line is parsed.
- The request target must start with `/`.
- Query strings are ignored for routing (only the path segment is used).

## Methods
- `GET` and `HEAD` are accepted.
- Any other method results in `405 Method Not Allowed`.
