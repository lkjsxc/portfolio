# Compile

```sh
gcc -O2 -pthread -o main src/*.c
```

- Static linking is optional for local builds.
- Use `-pthread` to enable the async worker pool.
