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
void swap(int signum){
}

void foo(){
    while(1){
    printf("IN foo\n");
    }
}

void bar(){
        printf("IN bar");
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

    printf("DONE");


    /*struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &swap;
	sigaction (SIGPROF, &sa, NULL);

    struct itimerval timer;
    timer.it_interval.tv_usec = 0
	timer.it_interval.tv_sec = 0;

    timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 1;

    setitimer(ITIMER_PROF, &timer, NULL);*/

}

