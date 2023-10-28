// File:	thread-worker.c

// List all group member's name: Trevor Dovan, Maanav
// username of iLab: ilab3 
// iLab Server: ilab3.cs.rutgers.edu

#include "thread-worker.h"
#define STACK_SIZE 1024

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;

// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE

#define DEBUG 0
#define MAX_THREADS 128

static void 		schedule();
static void 		sched_psjf();
static void 		sched_mlfq();

int 				setUpSchedulerContext();
void 				setUpTimer();
void 				timer_handler(int signum);
void* 				thread_wrapper(void *arg);
void 				enqueue(struct node **queue_head, struct TCB *thread);
struct node* 		dequeue(struct node **queue_head);

struct TCB* 		threadMap[MAX_THREADS] = {NULL};
struct TCB*			current_thread;
static int 			threadCount = 0;

struct node*		runqueue_head;

// global contexts to swtich between
static ucontext_t 	scheduler_ctx;
static ucontext_t 	current_ctx;
// ADD BENCHMARK CONTEXt

// other global declarations
static void* 		sched_stack_pointer;
static int 			accessedFirstTime = 0;
static int 			isInScheduler = 0;

static int timerCounter = 0;
void timer_handler(int signum)
{
	timerCounter++;
	if (DEBUG) printf("timer hit %d. ", timerCounter);

	if (isInScheduler)
	{
		if (DEBUG) printf("already in scheduler. returning\n");
		return;
	}

	if (DEBUG) printf("switching to sched ctx\n");
	swapcontext(&current_ctx,&scheduler_ctx);
}

void setUpTimer()
{
	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_handler;
	sigaction (SIGPROF, &sa, NULL);

	struct itimerval timer;

	//timer.it_interval.tv_usec = 10 * 1000; 
	timer.it_interval.tv_usec = 100 * 1000; 
	timer.it_interval.tv_sec = 0;

	timer.it_value.tv_usec = 1;
	timer.it_value.tv_sec = 0;

	// start the timer
	setitimer(ITIMER_PROF, &timer, NULL);
}

void* thread_wrapper(void *arg)
{
    thread_wrapper_arg_t *wrapper_arg = (thread_wrapper_arg_t *)arg;
    void *ret = wrapper_arg->function(wrapper_arg->arg); // Call the original function
	wrapper_arg->thread->state = TERMINATED;
    threadCount--;
	free(wrapper_arg); // Clean up dynamically allocated memory for the argument

	if (DEBUG) printf("thread wrapper. thread: %u terminated. switching to sched ctx\n", wrapper_arg->thread->threadId);
	setcontext(&scheduler_ctx);

	// dont think this ever gets hit
    return ret;
}

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg)
{
	// - create Thread Control Block (TCB)
	// - create and initialize the context of this worker thread
	// - allocate space of stack for this thread to run
	// - after everything is set, push this thread into run queue and 
	//   make it ready for the execution.

	// YOUR CODE HERE

	// init Thread Control Block (TCB)
	struct TCB *threadd = (struct TCB*)malloc(sizeof(struct TCB));
	
	if (getcontext(&threadd->context) < 0)
	{
		perror("getcontext");
		exit(1);
	}

	// add thread to threadMap, increment threadCount
	for (int i = 0; i < MAX_THREADS; i++)
	{
		if (threadMap[i] == NULL) {
			threadMap[i] = threadd;
			threadd->threadId = (worker_t)i;  // Use the index as the worker_t ID
			*thread = threadd->threadId;
			threadCount++;
			break;
		}
	}

	// allocate stack space
	threadd->stack_pointer = (void *)malloc(STACK_SIZE);
	if (threadd->stack_pointer == NULL)
	{
		perror("Failed to allocate stack");
		exit(1);
	}

	// wrap function
	thread_wrapper_arg_t *wrapper_arg = (thread_wrapper_arg_t *)malloc(sizeof(thread_wrapper_arg_t));
	if (!wrapper_arg)
	{
		perror("Failed to allocate memory for wrapper_arg");
		return -1;
	}

	wrapper_arg->function = function;
	wrapper_arg->arg = arg;
	wrapper_arg->thread = threadd;

	// init thread context
	threadd->context.uc_link = NULL;
	threadd->context.uc_stack.ss_sp = threadd->stack_pointer; // sets context stack
	threadd->context.uc_stack.ss_size = STACK_SIZE;
	threadd->context.uc_stack.ss_flags=0;

	makecontext(&threadd->context, (void (*)())thread_wrapper, 1, wrapper_arg);

	threadd->state = READY;

	// add to run queue
	enqueue(&runqueue_head, threadd);

	// init scheduler context and timer
	if (accessedFirstTime == 0)
	{
		accessedFirstTime++;
		setUpSchedulerContext();
		setUpTimer();

		// setup TCB for main?
		//getcontext(&main_context); // save main context during first call
	}

	// switch back to caller
    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield()
{
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE

	if (DEBUG) printf("in worker yield\n");

	if (current_thread)
	{
		// set to ready add to runqueue
		current_thread->state = READY;
		enqueue(&runqueue_head, current_thread);

		// save thread's context
		getcontext(&current_thread->context);

		// swap to scheluder
		if (DEBUG) printf("worker yield. threadId: %u yielding. switching to sched ctx\n", current_thread->threadId);
	   	swapcontext(&current_thread->context, &scheduler_ctx);
	}
	else
	{
		getcontext(&current_ctx);

		if (DEBUG) printf("worker yield. current thread null. switching to sched ctx\n");
	   	swapcontext(&current_ctx,&scheduler_ctx);
	}

	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread
	// YOUR CODE HERE
	if (current_thread)
	{
        current_thread->state = TERMINATED;
        // store the exit value somewhere accessible to worker_join
		if (value_ptr)
		{
        	current_thread->exit_value = value_ptr;
		}
		if (DEBUG) printf("worker exit. switching to sched ctx\n");
        setcontext(&scheduler_ctx);
		//free(currentThread);
	}
	else
	{
		getcontext(&current_ctx);
		if (DEBUG) printf("worker exit. switching to sched ctx\n");
	   	setcontext(&scheduler_ctx);
	}


};

/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr)
{
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
	
	// YOUR CODE HERE

	struct TCB *target_thread = threadMap[thread];

	if (target_thread == NULL)
	{
		return -1;
	}

    // Spin wait for the thread to terminate
    while (target_thread->state != TERMINATED) 
	{
        worker_yield(); // Yield the CPU to allow other threads to run
    }

    // If value_ptr is not NULL, retrieve the exit value
    if (value_ptr != NULL)
	{
        *value_ptr = target_thread->exit_value;
    }

    // Clean up the resources
    free(target_thread->stack_pointer);
    free(target_thread);

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
		// Set its state to BLOCKED
		current_thread->state = BLOCKED;

		// Add the current thread to the blocked threads list of the mutex
		enqueue(&mutex->blocked_threads, current_thread);

		getcontext(&current_thread->context);
		if (DEBUG) printf("worker mutex lock. switching to sched ctx\n");
		swapcontext(&current_thread->context, &scheduler_ctx);
	}

	// acquired mutex
	mutex->ownerId = current_thread->threadId;

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

	// Iterate over all blocked threads and move them to the run queue
    struct node *temp;
    while ((temp = dequeue(&mutex->blocked_threads)) != NULL)
    {
        struct TCB *nextThread = temp->data;
        nextThread->state = READY;
        enqueue(&runqueue_head, nextThread); 
    }

	mutex->ownerId = NULL;

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
static void schedule()
{
	if (DEBUG) printf("in schedule\n");
	// - every time a timer interrupt occurs, your worker thread library 
	//   should be contexted switched from a thread context to this 
	//   schedule() function

	// YOUR CODE HERE

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)
#ifndef MLFQ
		// Choose PSJF
		//sched_psjf();
#else 
		// Choose MLFQ
		//sched_mlfq();
#endif
	
	isInScheduler = 1;
	if (runqueue_head == NULL)
	{
		if (DEBUG) printf("schedule. runqueue empty. switching back to current ctx\n");
		isInScheduler = 0;
		setcontext(&current_ctx);
	}
	else
	{
		struct node *currThreadNode = dequeue(&runqueue_head);
		current_thread = currThreadNode->data;

		if (DEBUG) printf("schedule. switching to thread ctx. threadId: %u\n", current_thread->threadId);
		isInScheduler = 0;
		setcontext(&current_thread->context);
	}

	isInScheduler = 0;
}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf()
{
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq()
{
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

int setUpSchedulerContext()
{
	// Initialize scheduler_ctx
	if (getcontext(&scheduler_ctx) < 0)
	{
		perror("getcontext");
		exit(1);
	}

	sched_stack_pointer = (void*)malloc(STACK_SIZE);
	if (sched_stack_pointer == NULL) {
        perror("Failed to allocate stack for scheduler_ctx");
        exit(1);
    }
	scheduler_ctx.uc_link = NULL;
	scheduler_ctx.uc_stack.ss_sp = sched_stack_pointer;
	scheduler_ctx.uc_stack.ss_size = STACK_SIZE;
	scheduler_ctx.uc_stack.ss_flags = 0;

	getcontext(&current_ctx);
	makecontext(&scheduler_ctx,(void (*)())schedule,0);

	// // Initialize current_ctx (is this needed?)
    // if (getcontext(&current_ctx) < 0) {
    //     perror("getcontext for current_ctx");
    //     exit(1);
    // }

    // void *current_ctx_stack_pointer = malloc(STACK_SIZE);
    // if (current_ctx_stack_pointer == NULL) {
    //     perror("Failed to allocate stack for current_ctx");
    //     exit(1);
    // }

    // current_ctx.uc_link = NULL;
    // current_ctx.uc_stack.ss_sp = current_ctx_stack_pointer;
    // current_ctx.uc_stack.ss_size = STACK_SIZE;
    // current_ctx.uc_stack.ss_flags = 0;
}

void enqueue(struct node **queue_head, struct TCB *thread)
{
	if (queue_head == NULL || thread == NULL)
	{
		perror("enqueue passed null args");
		return;
	}

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

struct node* dequeue(struct node **queue_head)
{
	if (queue_head == NULL)
	{
		return NULL;
	}
    if (*queue_head == NULL)
	{
        return NULL;
    }
    struct node *temp = *queue_head;
    *queue_head = (*queue_head)->next;
    return temp;
}