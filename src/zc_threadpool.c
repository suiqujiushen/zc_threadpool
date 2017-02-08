#include "zc_threadpool.h"

static void *thread_func(void *arg);

static task_t *queue_pop(queue_t *q);
static int queue_empty(queue_t *q);
static int queue_push(queue_t *q, task_t *task);

zc_threadpool_t *
zc_threadpool_create(int thread_num)
{
	if (thread_num <= 0) return NULL;
	zc_threadpool_t *pool = (zc_threadpool_t *)calloc(1, sizeof(zc_threadpool_t));
	if (!pool) return NULL;
	
	pthread_mutex_init(&pool->_lock, NULL);
	pthread_cond_init(&pool->_cond, NULL);
	queue_t *queue = (queue_t *)calloc(1, sizeof(queue_t));
	if (!queue) {
		free(pool);
		return NULL;
	}
	pool->_task_queue = queue;
	
	pool->_thread_num = thread_num;
	pool->_threads = (pthread_t *)calloc(pool->_thread_num, sizeof(pthread_t));
	if (!pool->_threads) {
        free(pool->_task_queue);
		free(pool);
		return NULL;
	}
	int i;
	for (i = 0; i < pool->_thread_num; ++i) {
		if (0 != pthread_create(&pool->_threads[i], NULL, thread_func, pool)) {
			printf("pthread_create err\n");
			// kill the existing threads and free resources
			zc_threadpool_destroy(pool, IMMEDIATELY);
			return NULL;
		}
	}
	pool->_shutdown = NOSTOP;

	return pool;
}

int 
zc_threadpool_add_task(zc_threadpool_t *pool,
	void (*func)(void *arg), void *arg)
{
	if (!pool || !func)
		return -1;
	task_t *task = (task_t *)malloc(sizeof(task_t));
	task->_func = func;
	task->_arg = arg;
	pthread_mutex_lock(&pool->_lock);
	queue_push(pool->_task_queue, task);
	pthread_mutex_unlock(&pool->_lock);
	pthread_cond_signal(&pool->_cond);

	return 0;
}

void 
zc_threadpool_destroy(zc_threadpool_t *pool, shutdown_status_t shutdown)
{
	if (pool) {
		pthread_mutex_lock(&pool->_lock);
		pool->_shutdown = (shutdown == GRACEFULLY) ? GRACEFULLY : IMMEDIATELY;
		if (pool->_shutdown == IMMEDIATELY) {
			while (!queue_empty(pool->_task_queue)) {
				task_t *task = queue_pop(pool->_task_queue);
				free(task);
			}
		}
		pthread_cond_broadcast(&pool->_cond);
		pthread_mutex_unlock(&pool->_lock);
		int i = 0;
		for (; i < pool->_thread_num; ++i) {
			pthread_join(pool->_threads[i], NULL);
		}
		free(pool->_task_queue);
		free(pool->_threads);
		pthread_mutex_destroy(&pool->_lock);
		pthread_cond_destroy(&pool->_cond);
		free(pool);
	}
}

static void *
thread_func(void *arg)
{
	zc_threadpool_t *pool = (zc_threadpool_t *)arg;
	task_t *task;
	while (1) {
		pthread_mutex_lock(&pool->_lock);
		while (queue_empty(pool->_task_queue) && pool->_shutdown == NOSTOP) {
			pthread_cond_wait(&pool->_cond, &pool->_lock);
		}
		if (pool->_shutdown == IMMEDIATELY || (pool->_shutdown == GRACEFULLY && queue_empty(pool->_task_queue))) {
			break;
		}
		task = queue_pop(pool->_task_queue);
		pthread_mutex_unlock(&pool->_lock);
		(*(task->_func))(task->_arg);
		free(task);
	}
	
	pthread_mutex_unlock(&pool->_lock);
	pthread_exit(NULL);
}

static int 
queue_empty(queue_t *q) {
	return ((q->_front == NULL) ? 1 : 0);
}

static int 
queue_push(queue_t *q, task_t *task) {
	if (!q || !task)
		return -1;
	if (queue_empty(q)) {
		q->_front = q->_rear = task;
	}
	else {
		q->_rear->next = task;
		q->_rear = q->_rear->next;
	}
	return 0;
}

static task_t *
queue_pop(queue_t *q) {
	if (queue_empty(q))
		return NULL;
	task_t *task = q->_front;
	if (q->_front->next == NULL) {
		q->_front = q->_rear = NULL;
	}
	else {
		q->_front = q->_front->next;
	}

	return task;
}
