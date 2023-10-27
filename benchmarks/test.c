#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"

// test timer
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
    while (1)
    {
        //printf("WAIT");
    }
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
    printf("thread0: %lu\n", thread1);

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
    printf("thread1: %lu\n", thread2);

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

// infinite mutex lock loop
#if 1

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
