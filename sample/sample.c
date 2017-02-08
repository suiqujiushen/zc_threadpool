#include "zc_threadpool.h"

void func1(void *arg) {
	int i;
	printf("fun1 in thread %ld\n", pthread_self());
	for (i = 0; i < 10; ++i)
		printf("%d\t", i);
	printf("\n");
}

void func2(void *arg) {
	printf("fun2 in thread %ld\n", pthread_self());
	printf("Hello Linux Thread\n");
}

int main()
{
	int i;
	zc_threadpool_t *pool = zc_threadpool_create(3);
	for (i = 0; i < 10; ++i) {
		zc_threadpool_add_task(pool, func1, NULL);
		zc_threadpool_add_task(pool, func2, NULL);
	}
	zc_threadpool_destroy(pool, GRACEFULLY);
	//zc_threadpool_destroy(pool, IMMEDIATELY);

	return 0;
}
