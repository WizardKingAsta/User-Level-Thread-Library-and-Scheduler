#include <stdio.h>
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

