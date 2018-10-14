#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define DEFAULT_TIME 10
#define THREAD_NUM 10
#define PRODUCT 2

void* threadpool_thread(void* threadpool);

void* threadpool_adjust(void* threadpool);

int threadpool_free(threadpool_t* pool);

int is_thread_alive(pthread_t tid);

threadpool_t* threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size) {
    threadpool_t* pool = NULL;
    int i;
    do{
        if( (pool =  (threadpool_t*) malloc(sizeof(threadpool_t))) == NULL ) {
            printf("malloc threadpool failed\n");
            break;
        }

        pool->min_thr_num = min_thr_num;
        pool->max_thr_num = max_thr_num;
        pool->live_thr_num = min_thr_num;
        pool->busy_thr_num = 0;
        pool->wait_exit_thr_num = 0;

        pool->queue_header = 0;
        pool->queue_tailer = 0;
        pool->queue_size = 0;
        pool->queue_max_size = queue_max_size;

        pool->shutdown = 0;

        if( (pool->threads = (pthread_t*) malloc(sizeof(pthread_t) * max_thr_num)) == NULL ) {
            printf("malloc threads failed\n");
            break;
        }

        if( (pool->task_queue = (threadpool_task_t*) malloc(sizeof(threadpool_task_t) * queue_max_size)) == NULL ) {
            printf("malloc task_queue failed\n");
            break;
        }

        if(pthread_mutex_init(&pool->lock, NULL) != 0 ||
           pthread_mutex_init(&pool->counter, NULL) != 0 ||
           pthread_cond_init(&pool->queue_not_full, NULL) != 0 ||
           pthread_cond_init(&pool->queue_not_empty, NULL) != 0) {
               printf("init mutex or condition failed\n");
               break;
           }
        
        for(i = 0; i < pool->min_thr_num; i++) {
            pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*) pool);
            printf("start process 0x%x...\n",(unsigned int) pool->threads[i]);
        }
        pthread_create(&pool->adjust_tid, NULL, threadpool_adjust, (void*) pool);

        return pool;

    } while(0);

    threadpool_free(pool);

    return NULL;
}

int threadpool_add(threadpool_t* pool, void* (*func)(void*), void* argv) {
    pthread_mutex_lock(&(pool->lock));

    while( pool->queue_size == pool->queue_max_size && !pool->shutdown ) {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));
    }

    if(pool->shutdown) {
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }

    if(pool->task_queue[pool->queue_tailer].argv != NULL) {
        free(pool->task_queue[pool->queue_tailer].argv);
        pool->task_queue[pool->queue_tailer].argv = NULL;
    }

    pool->task_queue[pool->queue_tailer].func = func;
    pool->task_queue[pool->queue_tailer].argv = argv;
    pool->queue_tailer = (pool->queue_tailer + 1) % pool->queue_max_size;
    pool->queue_size++;

    pthread_cond_signal(&(pool->queue_not_empty));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

int threadpool_destroy(threadpool_t* pool) {
    int i;
    if(pool == NULL) {
        return -1;
    }

    pool->shutdown = 1;

    pthread_join(pool->adjust_tid, NULL);

    for(i = 0; i < pool->live_thr_num; i++) {
        pthread_cond_broadcast(&(pool->queue_not_empty));
    }

    for(i = 0; i < pool->max_thr_num; i++) {
        if(is_thread_alive(pool->threads[i])) {
            pthread_join(pool->threads[i], NULL);
        }
    }

    threadpool_free(pool);
    
    return 0;
}

int threadpool_all_num(threadpool_t* pool) {
    int num = -1;
    pthread_mutex_lock(&(pool->lock));
    num = pool->live_thr_num;
    pthread_mutex_unlock(&(pool->lock));
    return num;
}

int threadpool_busy_num(threadpool_t* pool) {
    int num = -1;
    pthread_mutex_lock(&(pool->counter));
    num = pool->busy_thr_num;
    pthread_mutex_unlock(&(pool->counter));
    return num;
}


void* threadpool_thread(void* threadpool) {
    threadpool_t* pool = (threadpool_t*) threadpool;
    threadpool_task_t task;

    while(1) {
        pthread_mutex_lock(&(pool->lock));

        while( pool->queue_size == 0 && !pool->shutdown ) {
            printf("thread 0x%x is waiting...\n", (int) pthread_self());
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));
            printf("OK\n");

            if(pool->wait_exit_thr_num > 0) {
                pool->wait_exit_thr_num--;

                if(pool->live_thr_num > pool->min_thr_num) {
                    pool->live_thr_num--;
                    pthread_mutex_unlock(&(pool->lock));
                    printf("thread 0x%x is exiting...\n", (int) pthread_self());
                    pthread_exit(NULL);
                }
            }
        }

        if(pool->shutdown) {
            pthread_mutex_unlock(&(pool->lock));
            printf("thread 0x%x is exiting...\n", (int) pthread_self());
            pthread_exit(NULL);
        }

        task.func = pool->task_queue[pool->queue_header].func;
        task.argv = pool->task_queue[pool->queue_header].argv;

        pool->queue_header = (pool->queue_header + 1) % pool->queue_max_size;
        pool->queue_size--;

        pthread_cond_broadcast(&(pool->queue_not_full));
        pthread_mutex_unlock(&(pool->lock));

        printf("thread 0x%x start working\n", (unsigned int)pthread_self());
        pthread_mutex_lock(&(pool->counter));
        pool->busy_thr_num++;
        pthread_mutex_unlock(&(pool->counter));

        (*(task.func))(task.argv);

        printf("thread 0x%x stop working\n", (unsigned int)pthread_self());
        pthread_mutex_lock(&(pool->counter));
        pool->busy_thr_num--;
        pthread_mutex_unlock(&(pool->counter));       
    }
    pthread_exit(NULL);
}



void* threadpool_adjust(void* threadpool) {
    threadpool_t* pool = (threadpool_t*) threadpool;
    int i, add;
    
    while(!pool->shutdown) {
        sleep(DEFAULT_TIME);

        pthread_mutex_lock(&(pool->lock));
        int queue_size = pool->queue_size;
        int live_thr_num = pool->live_thr_num;
        pthread_mutex_unlock(&(pool->lock));

        pthread_mutex_lock(&(pool->counter));
        int busy_thr_num = pool->busy_thr_num;
        pthread_mutex_unlock(&(pool->counter));

        if(queue_size > busy_thr_num * PRODUCT && live_thr_num < pool->max_thr_num) {
            pthread_mutex_lock(&(pool->lock));

            for(i = 0, add = 0; i < pool->max_thr_num && add < THREAD_NUM 
                && pool->live_thr_num < pool->max_thr_num; i++) {
                if(!is_thread_alive(pool->threads[i])) {
                    pthread_create(&(pool->threads[i]), NULL, threadpool_thread, pool);
                    add++;
                    pool->live_thr_num++;
                }
            }

            live_thr_num = pool->live_thr_num;
            pthread_mutex_unlock(&(pool->lock));
        }

        if(busy_thr_num * PRODUCT < live_thr_num && live_thr_num > pool->min_thr_num) {
            pthread_mutex_lock(&(pool->lock));
            pool->wait_exit_thr_num = THREAD_NUM;
            pthread_mutex_unlock(&(pool->lock));

            for(i = 0; i < THREAD_NUM; i++) {
                pthread_cond_signal(&(pool->queue_not_empty));
            }
        }
    }

    return NULL;
}

int threadpool_free(threadpool_t* threadpool) {
    threadpool_t* pool = (threadpool_t*) threadpool;
    if(pool == NULL) {
        return -1;
    }

    if(pool->threads) {
        free(pool->threads);
    }

    if(pool->task_queue) {
        free(pool->task_queue);
    }

    pthread_mutex_lock(&(pool->lock));
    pthread_mutex_destroy(&(pool->lock));
    pthread_mutex_lock(&(pool->counter));
    pthread_mutex_destroy(&(pool->counter));
    pthread_cond_destroy(&(pool->queue_not_empty));
    pthread_cond_destroy(&(pool->queue_not_full));

    free(pool);

    return 0;
}

int is_thread_alive(pthread_t tid){
    int kill_rc = pthread_kill(tid, 0);     
    if (kill_rc == ESRCH) {
        return 0;
    }
    return 1;
}