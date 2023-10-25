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
struct node *runqueue_head;
struct node *current_thread;

//global contexts to swtich between
static void* sched_stack_pointer;
static ucontext_t scheduler_context;
ucontext_t main_context;
//NEED TO ADD BENCHMARK CONTEXt

//other global declarations
static int accessedFirstTime = 0;
ucontext_t curr;
static void schedule();

//global timer for interupt (just basic ten seconds for FCFS rn)
void timer_handler(int signum) {
    setcontext(&scheduler_context);
}

struct sigaction sa;

struct itimerval timer;


void setUpTimer(){
	sa.sa_handler = &timer_handler;
	sa.sa_flags = SA_RESTART;  // Restart functions if interrupted by handler
	sigaction(SIGPROF, &sa, NULL);

	// Initial expiration
	timer.it_value.tv_sec = 1;
	timer.it_value.tv_usec = 0;

	// Periodic interval
	timer.it_interval.tv_sec = 1;
	timer.it_interval.tv_usec = 0;

	// Start the timer
}

void* thread_wrapper(void *arg) {
    thread_wrapper_arg_t *wrapper_arg = (thread_wrapper_arg_t *)arg;
    void *ret = wrapper_arg->function(wrapper_arg->arg); // Call the original function
	wrapper_arg->thread->state = TERMINATED;
	swapcontext(&curr,&scheduler_context);
    free(wrapper_arg); // Clean up dynamically allocated memory for the argument
    return ret;
}

int setUpSchedulerContext(){
	if (getcontext(&scheduler_context) < 0){
		perror("getcontext");
		exit(1);
		}

		sched_stack_pointer = (void*)malloc(STACK_SIZE);
		scheduler_context.uc_link = NULL;
		scheduler_context.uc_stack.ss_sp = sched_stack_pointer;
		scheduler_context.uc_stack.ss_size = STACK_SIZE;
		scheduler_context.uc_stack.ss_flags = 0;

		//makes context NEED TO CHANGE THE FUNCTION
		getcontext(&curr);
		makecontext(&scheduler_context,(void (*)())schedule,0);
}

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

	   if(accessedFirstTime == 0){//makes scheduler context if it has not been created yet by testing if this is first time calling create func
	   setUpTimer();
	   setUpSchedulerContext();
	   	accessedFirstTime++;
	   }
	   
	   if (getcontext(&threadd->context) < 0){
		perror("getcontext");
		exit(1);
		}
		getcontext(&main_context);
		setitimer(ITIMER_PROF, &timer, NULL);
		if(threadd->state != TERMINATED){

	   threadd->threadId = thread;

	   threadd->stack_pointer = (void *)malloc(STACK_SIZE); //allocates stack space
		if (threadd->stack_pointer == NULL){
		perror("Failed to allocate stack");
		exit(1);
		}

		thread_wrapper_arg_t *wrapper_arg = (thread_wrapper_arg_t *)malloc(sizeof(thread_wrapper_arg_t));
    	if(!wrapper_arg) {
       		perror("Failed to allocate memory for wrapper_arg");
        	return -1;
    	}
    	wrapper_arg->function = function;
    	wrapper_arg->arg = arg;
		wrapper_arg->thread = threadd;

	   threadd->context.uc_link = NULL;
	   threadd->context.uc_stack.ss_sp = threadd->stack_pointer; //sets context stack
	   threadd->context.uc_stack.ss_size = STACK_SIZE;
	   threadd->context.uc_stack.ss_flags=0;

	   printf("about to call make  context\n");

		makecontext(&threadd->context, (void (*)())thread_wrapper, 1, wrapper_arg);
	   //makecontext(&threadd->context,(void (*)())function, 1, arg); //makes context

	   printf("called make context\n");

		
	   threadd->state = READY; //sets thread state

	   //must add to run queue
	   enqueue(&runqueue_head, threadd);
	   getcontext(&curr);
	   swapcontext(&curr,&scheduler_context);
		}
	
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
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
	//- initialize data structures for this mutex

	// YOUR CODE HERE

	// Initialize the mutex as unlocked
	// 0 unlocked, 1 locked
    mutex->isLocked = 0; 

    // Initialize the blocked threads queue
    mutex->blocked_threads = NULL;

	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex)
{
        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE

		// If the mutex is locked
		while (__sync_lock_test_and_set(&mutex->isLocked, 1))
		{
			// Get the current thread
			tcb *currentThread = current_thread->data;

			// Set its state to BLOCKED
			currentThread->state = BLOCKED;

			// Add the current thread to the blocked threads list of the mutex
			enqueue(&mutex, currentThread);

			// Yield the CPU to the scheduler
			worker_yield();
		}
		return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex)
{
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE

    // Release the mutex
    __sync_lock_release(&mutex->isLocked);

    // If there are threads waiting for the mutex
    if (mutex->blocked_threads)
	{
        // Move one thread from the blocked list to the run queue
        struct node *temp = dequeue(&mutex->blocked_threads);
		struct TCB *nextThread = temp->data;
        nextThread->state = READY;
        enqueue(&runqueue_head, nextThread); 
    }
    return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex)
{
	// - de-allocate dynamic memory created in worker_mutex_init

	// Check if the mutex is locked
    if (mutex->isLocked)
	{
        perror("Attempt to destroy a locked mutex");
        return -1;
    }

    // If there are threads waiting for the mutex
    while (mutex->blocked_threads)
	{
        // Move threads from the blocked queue to the run queue
		struct node *temp = dequeue(&mutex->blocked_threads);
        struct TCB *nextThread = temp->data;
        enqueue(&runqueue_head, nextThread); 
    }

    // Deallocate the blocked threads queue
    struct node *current = mutex->blocked_threads;
	struct node *next_node;
	while (current != NULL)
	{
		next_node = current->next;
		free(current);
		current = next_node;
	}
	mutex->blocked_threads = NULL;

	return 0;
};

/* scheduler */
static void schedule() {
	if(runqueue_head == NULL){
		setcontext(&main_context);
	}else{
		struct node *temp = dequeue(&runqueue_head);
		current_thread = temp; // when swapping context make sure to set the currently running thread
		swapcontext(&curr,&temp->data->context);
	}
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

// FUNCTION TO ENQUEUE TO ANY QUEUE
void enqueue(struct node **queue_head, struct TCB *thread)
{
    struct node *t = (struct node*)malloc(sizeof(struct node));
    t->data = thread;
    t->next = NULL;

    if (*queue_head == NULL)
	{
        *queue_head = t;
    }
	else
	{
        struct node *temp = *queue_head;
        while (temp->next != NULL)
		{
            temp = temp->next;
        }
        temp->next = t;
    }
}

// METHOD TO DEQUEUE FROM ANY QUEUE
struct node* dequeue(struct node **queue_head)
{
    if (*queue_head == NULL)
	{
        return NULL;
    }
    struct node *temp = *queue_head;
    *queue_head = (*queue_head)->next;
    return temp;
}