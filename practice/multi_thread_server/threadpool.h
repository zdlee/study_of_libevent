#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <pthread.h>

typedef struct {
    void* (*func)(void*);
    void* argv;
} threadpool_task_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_mutex_t counter;
    pthread_cond_t queue_not_full;
    pthread_cond_t queue_not_empty;

    pthread_t* threads;
    pthread_t adjust_tid;
    threadpool_task_t* task_queue;

    int min_thr_num;
    int max_thr_num;
    int live_thr_num;
    int busy_thr_num;
    int wait_exit_thr_num;

    int queue_header;
    int queue_tailer;
    int queue_size;
    int queue_max_size;

    int shutdown;
} threadpool_t;

threadpool_t* threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size);

int threadpool_add(threadpool_t* pool, void* (*func)(void*), void* argv);

int threadpool_destroy(threadpool_t* pool);

int threadpool_all_num(threadpool_t* pool);

int threadpool_busy_num(threadpool_t* pool);

#endif