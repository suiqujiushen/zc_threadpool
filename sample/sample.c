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
	zc_threadpool_t *pool = zc_threadpool_create(3);
	zc_threadpool_add_task(pool, func1, NULL);
	zc_threadpool_add_task(pool, func2, NULL);
	sleep(1);
	zc_threadpool_destroy(pool);

	return 0;
}
