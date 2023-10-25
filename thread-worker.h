// File:	worker_t.h

// List all group member's name:
// username of iLab:
// iLab Server:

#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

// added
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>  // For ITIMER_REAL and struct itimerval
#include <limits.h>    // For INT_MAX
//added

typedef uint worker_t;

typedef struct TCB
{
	/* add important states in a thread control block */

	// YOUR CODE HERE

	uint threadId;			// thread Id

	ucontext_t context;		// thread context
	void* stack_pointer;	// thread stack

	int burst_time; 		// Estimated execution time
	int elapsed; 			// Elapsed time counter
	int response_time; 		// Elapsed time counter

	int priority;			// thread priority

	enum {
		NEW,
		READY,
		RUNNING,
		BLOCKED,
		TERMINATED
	} state; 				// thread status

} tcb; 

/* mutex struct definition */
typedef struct worker_mutex_t
{
	/* add something here */
	//_Atomic {LOCKED, UNLOCKED} state;
	int ownerId;
	// YOUR CODE HERE

	int is_locked;
	worker_t holder;
	
} worker_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE

//create linked list runqueue 
typedef struct node
{
	struct TCB *data;
    struct node *next;
} node_t;

typedef struct
{
    void *(*function)(void *);
    void *arg;
} wrapper_arg_t;


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
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex);

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex);

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex);

static void schedule();
static void sched_psjf();
static void sched_mlfq();

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

/*********** Helper Functions ***********/

tcb* getThread(worker_t thread);

void enqueue(node_t **head, tcb *new_thread);
tcb* dequeue(node_t **head);
tcb* dequeue_thread(node_t *head, worker_t thread_id);
void wrapper_function(void *wrapper_arg);
void update_global_statistics(tcb *current_thread);

