# Data Model

## Request Line
```
HttpRequestLine {
  method: &str,
  method_len: usize,
  target: &str,
  target_len: usize
}
```

## Method Classification
```
HttpMethod = GET | HEAD | UNKNOWN
```

## Route Classification
```
HttpRoute = ROOT | UNKNOWN
```

## Parse Result (Result-like)
```
HttpParseResult = { ok: bool, line: HttpRequestLine }
```

## Response Spec
```
HttpResponseSpec {
  status: i32,
  reason: &str,
  content_type: &str,
  body: *const u8,
  body_len: usize,
  send_body: bool
}
```

## Notes
- `UNKNOWN` captures any unsupported or invalid input.
- `send_body` is computed from the method (GET => true, HEAD => false).
- Error responses for malformed lines bypass method classification.
