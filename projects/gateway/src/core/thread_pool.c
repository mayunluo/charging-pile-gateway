#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "thread_pool.h"

typedef struct task {
    void (*func)(void *);
    void *arg;
    struct task *next;
} task_t;

struct thread_pool {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    task_t *tasks;
    pthread_t *threads;
    int thread_count;
    int stop;
};

static void *worker(void *arg) {
    struct thread_pool *pool = (struct thread_pool *)arg;
    while (1) {
        pthread_mutex_lock(&pool->lock);
        while (!pool->tasks && !pool->stop) pthread_cond_wait(&pool->cond, &pool->lock);
        if (pool->stop && !pool->tasks) { pthread_mutex_unlock(&pool->lock); break; }
        task_t *t = pool->tasks;
        pool->tasks = t->next;
        pthread_mutex_unlock(&pool->lock);

        t->func(t->arg);
        free(t);
    }
    return NULL;
}

thread_pool_t *thread_pool_create(int nthreads) {
    struct thread_pool *p = calloc(1, sizeof(*p));
    if (!p) return NULL;
    p->threads = calloc(nthreads, sizeof(pthread_t));
    p->thread_count = nthreads;
    pthread_mutex_init(&p->lock, NULL);
    pthread_cond_init(&p->cond, NULL);
    for (int i = 0; i < nthreads; ++i) pthread_create(&p->threads[i], NULL, worker, p);
    return p;
}

void thread_pool_submit(thread_pool_t *p, void (*func)(void*), void *arg) {
    task_t *t = malloc(sizeof(*t));
    t->func = func; t->arg = arg; t->next = NULL;
    pthread_mutex_lock(&p->lock);
    t->next = p->tasks; p->tasks = t;
    pthread_cond_signal(&p->cond);
    pthread_mutex_unlock(&p->lock);
}

void thread_pool_destroy(thread_pool_t *p) {
    pthread_mutex_lock(&p->lock);
    p->stop = 1;
    pthread_cond_broadcast(&p->cond);
    pthread_mutex_unlock(&p->lock);
    for (int i = 0; i < p->thread_count; ++i) pthread_join(p->threads[i], NULL);
    free(p->threads);
    // free remaining tasks
    while (p->tasks) { task_t *n = p->tasks->next; free(p->tasks); p->tasks = n; }
    free(p);
}
