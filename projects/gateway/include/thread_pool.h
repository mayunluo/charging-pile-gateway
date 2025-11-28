#ifndef THREAD_POOL_H
#define THREAD_POOL_H
typedef struct thread_pool thread_pool_t;
thread_pool_t *thread_pool_create(int nthreads);
void thread_pool_submit(thread_pool_t *p, void (*func)(void*), void *arg);
void thread_pool_destroy(thread_pool_t *p);
#endif
