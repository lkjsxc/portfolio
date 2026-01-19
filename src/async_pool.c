#define _POSIX_C_SOURCE 200809L
#include "async_pool.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void die_pthread(int err, const char* msg) {
    if (err == 0) {
        return;
    }
    errno = err;
    perror(msg);
    exit(EXIT_FAILURE);
}

static void die_message(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static void* worker_main(void* arg) {
    AsyncPool* pool = (AsyncPool*)arg;
    while (1) {
        int client_fd = -1;
        die_pthread(pthread_mutex_lock(&pool->mutex), "pthread_mutex_lock");
        while (!pool->shutting_down && pool->count == 0) {
            die_pthread(pthread_cond_wait(&pool->has_items, &pool->mutex),
                        "pthread_cond_wait");
        }
        if (pool->shutting_down && pool->count == 0) {
            die_pthread(pthread_mutex_unlock(&pool->mutex), "pthread_mutex_unlock");
            break;
        }
        client_fd = pool->queue[pool->head];
        pool->head = (pool->head + 1) % pool->capacity;
        pool->count--;
        die_pthread(pthread_cond_signal(&pool->has_space), "pthread_cond_signal");
        die_pthread(pthread_mutex_unlock(&pool->mutex), "pthread_mutex_unlock");
        pool->task(client_fd, pool->ctx);
    }
    return NULL;
}

void async_pool_start(AsyncPool* pool,
                      size_t worker_count,
                      size_t queue_size,
                      AsyncTaskFn task,
                      void* ctx) {
    if (worker_count == 0 || queue_size == 0 || task == NULL) {
        die_message("Async pool requires workers, queue size, and task");
    }

    if (!pool) {
        die_message("Async pool requires a valid destination");
    }

    memset(pool, 0, sizeof(*pool));
    pool->capacity = queue_size;
    pool->thread_count = worker_count;
    pool->task = task;
    pool->ctx = ctx;

    pool->queue = calloc(queue_size, sizeof(int));
    if (!pool->queue) {
        die_message("Failed to allocate async queue");
    }
    pool->threads = calloc(worker_count, sizeof(pthread_t));
    if (!pool->threads) {
        die_message("Failed to allocate async worker threads");
    }

    die_pthread(pthread_mutex_init(&pool->mutex, NULL), "pthread_mutex_init");
    die_pthread(pthread_cond_init(&pool->has_items, NULL), "pthread_cond_init");
    die_pthread(pthread_cond_init(&pool->has_space, NULL), "pthread_cond_init");

    for (size_t i = 0; i < worker_count; i++) {
        die_pthread(pthread_create(&pool->threads[i], NULL, worker_main, pool),
                    "pthread_create");
    }
}

void async_pool_submit(AsyncPool* pool, int client_fd) {
    if (!pool) {
        return;
    }
    die_pthread(pthread_mutex_lock(&pool->mutex), "pthread_mutex_lock");
    while (!pool->shutting_down && pool->count == pool->capacity) {
        die_pthread(pthread_cond_wait(&pool->has_space, &pool->mutex),
                    "pthread_cond_wait");
    }
    if (pool->shutting_down) {
        die_pthread(pthread_mutex_unlock(&pool->mutex), "pthread_mutex_unlock");
        return;
    }
    pool->queue[pool->tail] = client_fd;
    pool->tail = (pool->tail + 1) % pool->capacity;
    pool->count++;
    die_pthread(pthread_cond_signal(&pool->has_items), "pthread_cond_signal");
    die_pthread(pthread_mutex_unlock(&pool->mutex), "pthread_mutex_unlock");
}

void async_pool_stop(AsyncPool* pool) {
    if (!pool) {
        return;
    }
    die_pthread(pthread_mutex_lock(&pool->mutex), "pthread_mutex_lock");
    pool->shutting_down = 1;
    die_pthread(pthread_cond_broadcast(&pool->has_items), "pthread_cond_broadcast");
    die_pthread(pthread_cond_broadcast(&pool->has_space), "pthread_cond_broadcast");
    die_pthread(pthread_mutex_unlock(&pool->mutex), "pthread_mutex_unlock");

    for (size_t i = 0; i < pool->thread_count; i++) {
        die_pthread(pthread_join(pool->threads[i], NULL), "pthread_join");
    }

    die_pthread(pthread_mutex_destroy(&pool->mutex), "pthread_mutex_destroy");
    die_pthread(pthread_cond_destroy(&pool->has_items), "pthread_cond_destroy");
    die_pthread(pthread_cond_destroy(&pool->has_space), "pthread_cond_destroy");
    free(pool->threads);
    free(pool->queue);
    pool->threads = NULL;
    pool->queue = NULL;
}
