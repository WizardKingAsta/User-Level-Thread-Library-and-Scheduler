#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include "../thread-worker.h"

worker_t thread1;
worker_t thread2;
worker_t thread3;

void swap(int signum) {}

void foo() {
    printf("IN foo\n");
    printf("foo yielding...\n");
    worker_yield();
    printf("foo resumed...\n");
    worker_exit(NULL); // Exit the thread and pass a null value.
}

void bar() {
    printf("IN bar\n");
    printf("bar yielding...\n");
    worker_yield();
    printf("bar resumed...\n");
    worker_exit(NULL); // Exit the thread and pass a null value.
}

void baz() {
    int c = 1;
    while (c < 10) {
        printf("%d\n", c);
        c++;
        if (c == 5) {
            printf("baz yielding...\n");
            worker_yield();
            printf("baz resumed...\n");
        }
    }
    worker_exit(NULL); // Exit the thread and pass a null value.
}

int main() {
    if (worker_create(&thread1, NULL, foo, NULL) != 0) {
        perror("Failed to create worker thread");
        return 1;
    }

    if (worker_create(&thread2, NULL, bar, NULL) != 0) {
        perror("Failed to create worker thread");
        return 1;
    }

    if (worker_create(&thread3, NULL, baz, NULL) != 0) {
        perror("Failed to create worker thread");
        return 1;
    }

    void *ret_val1, *ret_val2, *ret_val3;
    
    // Test worker_join() function
    if (worker_join(thread1, &ret_val1) != 0) {
        perror("Failed to join thread1");
    }
    
    if (worker_join(thread2, &ret_val2) != 0) {
        perror("Failed to join thread2");
    }
    
    if (worker_join(thread3, &ret_val3) != 0) {
        perror("Failed to join thread3");
    }

    printf("DONE\n");

    return 0;
}


/*#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include "../thread-worker.h"

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */
/*
worker_t thread1;
worker_t  thread2;
worker_t  thread3;
void swap(int signum){
}

void foo(){
    //while(1){
    printf("IN foo\n");
    //}
    
}

void bar(){
        printf("IN bar\n");
}

void baz(){
    int c = 1;
    while(c < 10){
        printf("%d",c);
        c++;
    }
}

int main(){
    if(worker_create(&thread1,NULL,foo,NULL) != 0){
		perror("Failed to create worker thread");
		return 1;
	}

    if(worker_create(&thread2,NULL,bar,NULL) != 0){
		perror("Failed to create worker thread");
		return 1;
	}

    if(worker_create(&thread3,NULL,baz,NULL) != 0){
		perror("Failed to create worker thread");
		return 1;
	}

    printf("DONE");



}

*/