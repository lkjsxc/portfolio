FROM gcc:14 as builder

WORKDIR /app

COPY main.c .

RUN gcc -static -o main main.c

FROM scratch

COPY --from=builder /app/main /main
COPY main.html /main.html

EXPOSE 80

CMD ["/main"]
