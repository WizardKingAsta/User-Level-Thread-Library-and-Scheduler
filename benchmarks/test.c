#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"
#include <time.h>
#include <sched.h>

void delay(int seconds) {
    time_t start_time = time(NULL);
    while (time(NULL) - start_time < seconds) {
        sched_yield(); // Yield the processor
    }
}

// test timer (DEBUG must be set to 1 to see print statements)
#if 0

void threadFunction0(void *arg);

int main(int argc, char **argv)
{
    pthread_t thread1;
    pthread_create(&thread1, NULL, &threadFunction0, (void*)0);
    printf("thread0: %lu\n", thread1);

    // Wait for thread to finish
    pthread_join(thread1, NULL);

    printf("\n");
    printf("exit main\n");
   
    return 0;
}

void threadFunction0(void *arg)
{
    printf("waiting\n");
    delay(3);
}
#endif

// Test pthread_exit and return value
#if 0

void* threadFunctionWithExit(void *arg);

int main(int argc, char **argv)
{
    pthread_t thread;
    int *returnValue;

    pthread_create(&thread, NULL, &threadFunctionWithExit, NULL);
    printf("Created thread: %lu\n", thread);

    // Wait for thread to finish and capture the return value
    pthread_join(thread, (void**)&returnValue);

    printf("Thread returned: %d\n", *returnValue);
    free(returnValue); // Free the dynamically allocated memory

    printf("Exiting main\n");
    return 0;
}

void* threadFunctionWithExit(void *arg)
{
    int *returnValue = malloc(sizeof(int));
    *returnValue = 42; // Some return value

    printf("Thread exiting with value: %d\n", *returnValue);
    pthread_exit(returnValue);
}
#endif

// Test for general exiting and storing return value
#if 0

void* threadFunctionReturn(void *arg);
void* threadFunctionExit(void *arg);

int main(int argc, char **argv)
{
    pthread_t thread1, thread2;
    int *returnValue1, *returnValue2;

    pthread_create(&thread1, NULL, &threadFunctionReturn, NULL);
    pthread_create(&thread2, NULL, &threadFunctionExit, NULL);

    pthread_join(thread1, (void**)&returnValue1);
    pthread_join(thread2, (void**)&returnValue2);

    if (returnValue1) 
        printf("Thread 1 returned: %d\n", *returnValue1);
    else 
        printf("Thread 1 returned NULL\n");

    if (returnValue2) 
        printf("Thread 2 exited with: %d\n", *returnValue2);
    else
        printf("Thread 2 returned NULL\n");

    free(returnValue1);
    free(returnValue2);

    printf("Exiting main\n");
    return 0;
}

void* threadFunctionReturn(void *arg)
{
    int *returnValue = malloc(sizeof(int));
    *returnValue = 123; // Some return value
    return returnValue;
}

void* threadFunctionExit(void *arg)
{
    int *returnValue = malloc(sizeof(int));
    *returnValue = 456; // Some return value
    pthread_exit(returnValue);
}
#endif



// test pthread_join
#if 0

void threadFunction0(void *arg);
void threadFunction1(void *arg);

int main(int argc, char **argv)
{
    //worker_t thread1;
    pthread_t thread1;
    pthread_create(&thread1, NULL, &threadFunction0, (void*)0);
    printf("thread0: %u\n", thread1);

    // Wait for thread to finish
    pthread_join(thread1, NULL);

    printf("exit main\n");
   
    return 0;
}

void threadFunction0(void *arg)
{
    printf("in thread 0\n");
    sleep(1);

    pthread_t thread2;
    pthread_create(&thread2, NULL, &threadFunction1, (void*)0);
    printf("thread1: %u\n", thread2);

    // Wait for thread to finish
    pthread_join(thread2, NULL);

    printf("exit thread 0\n");
}

void threadFunction1(void *arg)
{
    printf("in thread 1\n");
    sleep(1);
    printf("exit thread 1\n");

}
#endif

// test mutex
#if 1

// Define a shared resource
int sharedResource = 0;

// Define a mutex
worker_mutex_t mutex;

int threadFunction0(void *arg);
int threadFunction1(void *arg);

int main(int argc, char **argv)
{
    printf("initializing mutex\n");

    pthread_mutex_init(&mutex, NULL);

    pthread_t thread0;
    pthread_create(&thread0, NULL, &threadFunction0, (void*)0);
    printf("thread0: %lu\n", thread0);
    
    pthread_t thread1;
    pthread_create(&thread1, NULL, &threadFunction1, (void*)1);
    printf("thread1: %lu\n", thread1);
   
    printf("waiting for threads to finish\n");

    // Wait for threads to finish
    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);

    printf("destroying mutex\n");

    pthread_mutex_destroy(&mutex);

    //print_app_stats();

    printf("exiting main\n");

    return 0;
}

int threadFunction0(void *arg)
{
    int threadNum = (int)arg;

    printf("Thread %u attempt to aquire mutex\n", threadNum);

    // Try to acquire the mutex
    pthread_mutex_lock(&mutex);

    // Access the shared resource
    printf("Thread %u aquired mutex\n", threadNum);
    sharedResource++;
    delay(4); // Simulate some work

    printf("Thread %u releasing mutex\n", threadNum);

    // Release the mutex
    pthread_mutex_unlock(&mutex);

    return 0;
}

int threadFunction1(void *arg)
{
    int threadNum = (int)arg;

    printf("Thread %lu attempt to aquire mutex.\n", threadNum);

    pthread_mutex_lock(&mutex);

    // Access the shared resource
    printf("Thread %u aquired mutex\n", threadNum);
    sharedResource++;
    delay(1); // Simulate some work

    printf("Thread %u releasing mutex\n", threadNum);

    // Release the mutex
    pthread_mutex_unlock(&mutex);
    return 0;
}
#endif


// infinite mutex lock loop (loops on second lock but hits seg fault)
// for some reason having DEBUG set breaks this test
#if 0

// Define a shared resource
int sharedResource = 0;

// Define a mutex
worker_mutex_t mutex;

int testFunction(void *arg);
int testFunction2(void *arg);

int main(int argc, char **argv)
{
    pthread_mutex_init(&mutex, NULL);

    pthread_t thread1;
    pthread_create(&thread1, NULL, &testFunction, (void*)1);
    printf("thread1: %lu\n", thread1);
   
    // Wait for thread to finish
    pthread_join(thread1, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}

int testFunction(void *arg)
{
    int threadNum = (int)arg;

    // Try to acquire the mutex
    pthread_mutex_lock(&mutex);

    // Access the shared resource
    printf("Thread %lu accessing shared resource.\n", threadNum);
    sharedResource++;
    sleep(1); // Simulate some work

    pthread_t thread2;
    pthread_create(&thread2, NULL, &testFunction2, (void*)2);
    printf("thread2: %lu\n", thread2);

    pthread_join(thread2, NULL);

    // Release the mutex
    pthread_mutex_unlock(&mutex);

    return 0;
}

int testFunction2(void *arg)
{
    int threadNum = (int)arg;

    // Try to acquire the mutex
    // SHOULD GET STUCK HERE
    // context does not get set right, after mutex lock, context does not return to
    // pthread join in thread1, it returns to the line after
    pthread_mutex_lock(&mutex);

    // Access the shared resource
    printf("Thread %lu accessing shared resource.\n", threadNum);
    sharedResource++;
    sleep(1); // Simulate some work

    // Release the mutex
    pthread_mutex_unlock(&mutex);
    return 0;
}
#endif
