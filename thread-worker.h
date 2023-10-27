// File:	worker_t.h

// List all group member's name: Trevor Dovan, Maanav
// username of iLab: ilab3 
// iLab Server: ilab3.cs.rutgers.edu

#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 0

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h> 
#include <ucontext.h>
#include <signal.h>

typedef uint worker_t;

typedef struct TCB
{
	/* add important states in a thread control block */
	// YOUR CODE HERE
	worker_t 	threadId;			// thread Id
	ucontext_t 	context;			// thread context
	ucontext_t 	joinContext;		// thread join context
	void*		stack_pointer;		// thread stack
	int			priority;			// thread priority
	void*		exit_value;			// thread exit value
	enum { NEW, READY, RUNNING, BLOCKED, TERMINATED } state; // thread status

} tcb; 

typedef struct Thread_wrapper
{
    void *(*function)(void*);
    void *arg;
	struct TCB *thread;

} thread_wrapper_arg_t;


/* mutex struct definition */
typedef struct worker_mutex_t
{
	/* add something here */
	// YOUR CODE HERE

	//_Atomic {LOCKED, UNLOCKED} state;
	int isLocked;
	int ownerId;
	struct node *blocked_threads;

} worker_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)
// YOUR CODE HERE

// linked list runqueue 
typedef struct node
{
	tcb *data;
	struct node *next;

} Node;

/* Function Declarations: */

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level worker threads voluntarily */
int worker_yield();

/* terminate a thread */
void worker_exit(void *value_ptr);

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr);

/* initial the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex);

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex);

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex);


/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void);

#ifdef USE_WORKERS
#define pthread_t worker_t
#define pthread_mutex_t worker_mutex_t
#define pthread_create worker_create
#define pthread_exit worker_exit
#define pthread_join worker_join
#define pthread_mutex_init worker_mutex_init
#define pthread_mutex_lock worker_mutex_lock
#define pthread_mutex_unlock worker_mutex_unlock
#define pthread_mutex_destroy worker_mutex_destroy
#endif

#endif
