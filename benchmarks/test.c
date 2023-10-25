#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"

int testFunction1(void *arg);
void testFunction2();

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */
int main(int argc, char **argv) {

	/* Implement HERE */
	int args = 4;
	worker_t testThreadId = NULL;
	if (pthread_create(&testThreadId, NULL, &testFunction1, &args) == 0)
	{
		printf("ThreadId: %d\n", testThreadId);
	}

	if (pthread_create(&testThreadId, NULL, &testFunction2, NULL) == 0)
	{
		printf("ThreadId: %d\n", testThreadId);
	}
	
	return 0;
}

int testFunction1(void *arg)
{
	int x = 2;
	int y = *((int*) arg);
	int res = x + y;
	printf("res: %d\n", res);
	printf("thread1\n");
	return res;
}

void testFunction2()
{
	printf("thread2\n");
}
