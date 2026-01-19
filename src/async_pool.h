#ifndef ASYNC_POOL_H
#define ASYNC_POOL_H

#include <pthread.h>
#include <stddef.h>

typedef void (*AsyncTaskFn)(int client_fd, void* ctx);

typedef struct {
    int* queue;
    size_t capacity;
    size_t count;
    size_t head;
    size_t tail;
    int shutting_down;
    AsyncTaskFn task;
    void* ctx;
    pthread_mutex_t mutex;
    pthread_cond_t has_items;
    pthread_cond_t has_space;
    pthread_t* threads;
    size_t thread_count;
} AsyncPool;

AsyncPool async_pool_start(size_t worker_count,
                           size_t queue_size,
                           AsyncTaskFn task,
                           void* ctx);
void async_pool_submit(AsyncPool* pool, int client_fd);
void async_pool_stop(AsyncPool* pool);

#endif
