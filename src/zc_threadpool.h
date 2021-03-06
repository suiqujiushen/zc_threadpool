#ifndef __ZC_THREADPOOL_H
#define __ZC_THREADPOOL_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	NOSTOP = 0,
	IMMEDIATELY,
	GRACEFULLY
} shutdown_status_t;

typedef struct _task {
	void (*_func)(void *arg);
	void *_arg;
	struct _task *next;
} task_t;

typedef struct _zc_queue {
	task_t *_front;
	task_t *_rear;
} queue_t;

typedef struct _zc_threadpool {
	int _thread_num;
	pthread_t *_threads;
	queue_t *_task_queue;
	pthread_mutex_t _lock;
	pthread_cond_t _cond;
	shutdown_status_t _shutdown;
} zc_threadpool_t;

zc_threadpool_t * 
zc_threadpool_create(int thread_num);

int 
zc_threadpool_add_task(zc_threadpool_t *pool, 
	void (*func)(void *arg), void *arg);

void 
zc_threadpool_destroy(zc_threadpool_t *pool, shutdown_status_t shutdown);

#ifdef __cplusplus
}
#endif

#endif	// __ZC_THREADPOOL_H
