FROM gcc:14 as builder

WORKDIR /app

COPY src/ src/

RUN gcc -static -O2 -o main src/*.c

FROM scratch

COPY --from=builder /app/main /main
COPY main.html /main.html

EXPOSE 8080

CMD ["/main"]
