// File:	thread-worker.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "thread-worker.h"
#define STACK_SIZE 1024

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;



// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE
//RUNQUEUE
static ucontext_t uctx_sched;
//NEED TO ADD BENCHMARK CONTEXt
static void* sched_stack_pointer;
static accessedFirstTime = 0;
struct node *head;
struct node *current;


/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {

       // - create Thread Control Block (TCB)
       // - create and initialize the context of this worker thread
       // - allocate space of stack for this thread to run
       // after everything is set, push this thread into run queue and 
       // - make it ready for the execution.

       // YOUR CODE HERE
	   struct TCB *threadd = (struct TCB*)malloc(sizeof(struct TCB)); //allocated TCB space

	   threadd->threadId = &thread;

	   threadd->stack_pointer = (void *)malloc(STACK_SIZE); //allocates stack space

	   threadd->context.uc_link = NULL;
	   threadd->context.uc_stack.ss_sp = threadd->stack_pointer; //sets context stack
	   threadd->context.uc_stack.ss_size = STACK_SIZE;
	   threadd->context.uc_stack.ss_flags=0;

	   makecontext(&threadd->context,function, 1, arg); //makes context

	   if(accessedFirstTime == 0){//makes scheduler context if it has not been created yet by testing if this is first time calling create func
		sched_stack_pointer = malloc(STACK_SIZE);
		//allocates all neccessary parts of context
		uctx_sched.uc_link = NULL;
		uctx_sched.uc_stack.ss_sp = sched_stack_pointer;
		uctx_sched.uc_stack.ss_size = STACK_SIZE;
		uctx_sched.uc_stack.ss_flags = 0;

		//makes context NEED TO CHANGE THE FUNCTION
		makecontext(&uctx_sched,schedule_psjf(),0);
		//adjusts variable to 1 to show that worker create has been accessed.
		accessedFirstTime = 1;
		//IF ACCESSED FIRST TIME MAKE THREAD HEAD OF RUNQUEUE
		head = (struct node*)malloc(sizeof(struct node));
		head->data = thread;
		current = head;
	   }else{
		enqueue(thread);
	   }

	   threadd->state = READY; //sets thread state

	   *thread = threadd;
	   //must add to run queue
	
    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread
	free(value_ptr);
	// YOUR CODE HERE
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
	
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// if (sched == PSJF)
	//		sched_psjf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// - schedule policy
#ifndef MLFQ
	// Choose PSJF
#else 
	// Choose MLFQ
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}


// Feel free to add any other functions you need

// YOUR CODE HERE

//FUNCTION TO ENQUEUE TO RUNQUEUE
void enqueue(worker_t *thread){
	struct node *t = (struct node*)malloc(sizeof(struct node));
	t->data = thread;
	current->next = t;
	current = t;
}

//METHOD TO DEQUEUE FROM RUNQUEUE
void dequeue(worker_t *thread){
	

}