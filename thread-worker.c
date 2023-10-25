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

#define DEBUG 1

#define QUANTUM 1000
#define t 10

static int is_initialized = 0;
static int initialized = 0;

// Global array to map worker_t to TCB pointers
tcb** tcb_array = NULL;;
int tcb_count = 0;

// RUNQUEUE
static ucontext_t uctx_main, uctx_sched;

// NEED TO ADD BENCHMARK CONTEXt
static void* sched_stack_pointer = NULL;
static int accessedFirstTime = 0;
struct node *runqueue_head = NULL;
struct node *current = NULL;

struct TCB *current_thread = NULL;

/* 
	create a new thread 
	- create Thread Control Block (TCB)
	- create and initialize the context of this worker thread
	- allocate space of stack for this thread to run after everything is set, push this thread into run queue and 
	- make it ready for the execution.
*/
int worker_create(worker_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg)
{
	// YOUR CODE HERE
    if (DEBUG) printf("[DEBUG] inside worker_create\n");

    if (!function)
    {
        perror("Invalid function pointer provided");
        return -1;
    }

	// Init library
    if (!is_initialized)
    {
        // Save the current context in uctx_main
        getcontext(&uctx_main);

        // Initialize the scheduler's context
        sched_stack_pointer = malloc(STACK_SIZE);
        if (!sched_stack_pointer)
        {
            perror("Failed to allocate memory for scheduler stack");
            return -1;
        }

        getcontext(&uctx_sched);
        uctx_sched.uc_link = NULL;
        uctx_sched.uc_stack.ss_sp = sched_stack_pointer;
        uctx_sched.uc_stack.ss_size = STACK_SIZE;
        uctx_sched.uc_stack.ss_flags = 0;
        makecontext(&uctx_sched, schedule, 0);

        is_initialized = 1;
    }

	// Allocate TCB
	tcb *pThread = (struct TCB*)malloc(sizeof(struct TCB));
	if (!pThread)
	{
		perror("Failed to allocate memory for TCB");
		return -1;
	}

	// Allocate stack space
	pThread->stack_pointer = (void *)malloc(STACK_SIZE);
	if (!pThread->stack_pointer)
	{
		perror("Failed to allocate memory for thread stack");
		free(pThread);
		return -1;
	}

	// Init context
	pThread->context.uc_link = &uctx_sched; // NULL;
	pThread->context.uc_stack.ss_sp = pThread->stack_pointer;
    //pThread->context.uc_stack.ss_sp = pThread->stack_pointer + STACK_SIZE;
	pThread->context.uc_stack.ss_size = STACK_SIZE;
	pThread->context.uc_stack.ss_flags=0;

    // Wrap function pointer and args
    wrapper_arg_t *w_arg = malloc(sizeof(wrapper_arg_t));
    if (!w_arg)
    {
        perror("Failed to allocate memory for wrapper_arg_t when creating thread");
        free(pThread->stack_pointer);
        free(pThread);
        return -1;
    }

    w_arg->function = function;
    w_arg->arg = arg;

    // Create thread context
	makecontext(&pThread->context, (void (*)(void))wrapper_function, 1, w_arg);

    // Init runqueue
	if (runqueue_head == NULL)
	{
		// IF ACCESSED FIRST TIME MAKE THREAD HEAD OF RUNQUEUE
		runqueue_head = (struct node*)malloc(sizeof(struct node));
         if (!runqueue_head)
         {
            perror("Failed to allocate memory for runqueue node");
            free(w_arg);
            free(pThread->stack_pointer);
            free(pThread);
            return -1;
        }
		runqueue_head->data = pThread;
	}

	pThread->state = READY; 

    // Add to run queue
    enqueue(&runqueue_head, pThread);

	// Store the TCB pointer in the array and increase the counter
    tcb_array = realloc(tcb_array, (tcb_count + 1) * sizeof(tcb*));
    if (!tcb_array)
    {
        perror("Failed to reallocate memory for tcb_array");
        free(runqueue_head);
        free(w_arg);
        free(pThread->stack_pointer);
        free(pThread);
        return -1;
    }
	pThread->threadId = tcb_count;
    tcb_array[tcb_count] = pThread;

	// Return the index as the worker_t value
    *thread = tcb_count; 
    tcb_count++;

    if (tcb_count == 1)
    {
        // Switch from the main context to the first thread's context
        //swapcontext(&uctx_main, &pThread->context);
        setcontext(&pThread->context);
        if (!initialized)
        {
            // setup timer to signal scheduler
            setup_timer();
            initialized = 1;
        }
    }

	return 0;
};

/* 
	give CPU possession to other user-level worker threads voluntarily
	- change worker thread's state from Running to Ready
	- save context of this thread to its thread control block
	- switch from thread context to scheduler context
*/
int worker_yield()
{
	// YOUR CODE HERE
	
	// // Save the current context
    // if (getcontext(&current_thread->context) == -1) {
    //     perror("getcontext");
    //     return -1;
    // }

    // // Change the thread's state to READY
    // current_thread->state = READY;

    // // Enqueue the current thread back to the run queue
    // enqueue(runqueue_head, current_thread);

    // // Switch to the scheduler
    // setcontext(&uctx_sched);

    return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr)
{
	// Assuming you have a global variable for the current thread
    tcb *current = current_thread;

    if (!current)
    {
        perror("No current thread to exit");
        return;
    }

    // Save the return value if value_ptr is not NULL
    if (value_ptr)
    {
        //*(void **)value_ptr = current->return_value;
    }

    // Deallocate any dynamic memory associated with the thread
    free(current->stack_pointer);
    // If you have other dynamic memory allocations for the thread, free them here

    // Update the thread's state
    current->state = TERMINATED;

    // Remove the thread from the runqueue or any other scheduling data structures
    // This will depend on your implementation of the runqueue
    dequeue_thread(runqueue_head, current->threadId);

    // Free the TCB itself
    free(current);
};


/* 
	Wait for thread termination
	- wait for a specific thread to terminate
	- de-allocate any dynamic memory created by the joining thread
*/
int worker_join(worker_t thread, void **value_ptr)
{
	// YOUR CODE HERE
	
	// tcb *thread_tcb = (tcb *)thread; // Convert worker_t to tcb pointer
    // while (thread_tcb->state != TERMINATED) {
    //     worker_yield(); // Yield CPU until the specified thread terminates
    // }
    // // If value_ptr is not NULL, retrieve the return value of the exiting thread
    // if (value_ptr) {
    //     //*value_ptr = thread_tcb->returnValue; // Assuming you have a returnValue field in your TCB
    // }
    // // Clean up resources associated with the terminated thread
    // free(thread_tcb->stack_pointer);
    // free(thread_tcb);
    return 0;
};

/*
	initialize the mutex lock
	- initialize data structures for this mutex
*/
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
	// YOUR CODE HERE

	// mutex->is_locked = 0; // 0 means unlocked, 1 means locked
    // mutex->holder = NULL; // No thread holds the mutex initially
    return 0;
};

/* 
	aquire the mutex lock
	- use the built-in test-and-set atomic function to test the mutex
	- if the mutex is acquired successfully, enter the critical section
	- if acquiring mutex fails, push current thread into block list and context switch to the scheduler thread
*/
int worker_mutex_lock(worker_mutex_t *mutex)
{
	// YOUR CODE HERE

	// while (__sync_lock_test_and_set(&mutex->is_locked, 1))
	// {
    //     // If the mutex is already locked, block the calling thread
    //     // and switch to the scheduler
    //     worker_yield();
    // }
    // mutex->holder = current_thread; // Current thread now holds the mutex
    return 0;
};

/*
	release the mutex lock
	- release mutex and make it available again. 
	- put threads in block list to run queue 
	so that they could compete for mutex later.
*/
int worker_mutex_unlock(worker_mutex_t *mutex)
{
	// YOUR CODE HERE

	// if (mutex->holder != current_thread)
	// {
    //     // The current thread doesn't hold the mutex, so it can't unlock it
    //     return -1;
    // }
    // mutex->holder = NULL; // No thread holds the mutex now
    // __sync_lock_release(&mutex->is_locked);
    return 0;
};


/*
	destroy the mutex
	- de-allocate dynamic memory created in worker_mutex_init
*/
int worker_mutex_destroy(worker_mutex_t *mutex)
{
	if (mutex->is_locked)
	{
        // Mutex is still locked, can't destroy
        return -1;
    }
    // Free any other resources if necessary (e.g., any associated queues or structures)
    return 0;
};

/*
	scheduler
	- every time a timer interrupt occurs, your worker thread library 
	should be contexted switched from a thread context to this 
	schedule() function

	- invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	if (sched == PSJF)
		sched_psjf();
	else if (sched == MLFQ)
		sched_mlfq();	
*/
static void schedule()
{
	// YOUR CODE HERE
    if (DEBUG) printf("[DEBUG] inside schedulef\n");

#ifndef MLFQ
	sched_psjf();
#else 
	sched_mlfq();
#endif
}

/*
	Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm
	- your own implementation of PSJF
	(feel free to modify arguments and return types)
*/
static void sched_psjf()
{
	// YOUR CODE HERE
    if (DEBUG) printf("[DEBUG] inside sched_psjf\n");

    if (runqueue_head == NULL) {
        return NULL; // No threads to schedule.
    }

    tcb* next_thread = dequeue(&runqueue_head);

    // run next thread
    if (next_thread)
    {
        if (DEBUG) printf("[DEBUG] setting next thread context\n");
        current_thread = next_thread;
        next_thread->state = RUNNING;
        setcontext(&(next_thread->context));
    }
    else
    {
        // No more threads to run. Decide what to do here.
    }
}

#define NUM_LEVELS 4
struct node *runqueues[NUM_LEVELS];

/*
	Preemptive MLFQ scheduling algorithm
	- your own implementation of MLFQ
	(feel free to modify arguments and return types)
*/
static void sched_mlfq()
{
	// YOUR CODE HERE

	// struct TCB *next_thread = NULL;
    // for (int i = 0; i < NUM_LEVELS; i++) {
    //     if (runqueues[i]) {
    //         next_thread = dequeue_from(&runqueues[i]);
    //         break;
    //     }
    // }

    // if (next_thread) {
    //     next_thread->state = RUNNING;
    //     current_thread = next_thread;
    //     setcontext(&next_thread->context);
    // }
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void)
{
       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}

// Feel free to add any other functions you need

// YOUR CODE HERE

/*********** Helper Functions ***********/

tcb* getThread(worker_t thread)
{
    if (thread < tcb_count)
	{
        return tcb_array[thread];
    }
    return NULL;
}

//FUNCTION TO ENQUEUE TO RUNQUEUE
void enqueue(node_t **head, tcb *new_thread)
{
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (!new_node)
    {
        perror("Failed to allocate memory for new node");
        return;
    }
    new_node->data = new_thread;
    new_node->next = NULL;

    if (*head == NULL || (*head)->data->priority > new_thread->priority)
    {
        new_node->next = *head;
        *head = new_node;
    } 
    else
    {
        node_t *temp = *head;
        while (temp->next && temp->next->data->priority <= new_thread->priority)
        {
            temp = temp->next;
        }
        new_node->next = temp->next;
        temp->next = new_node;
    }
}


//METHOD TO DEQUEUE FROM RUNQUEUE
tcb* dequeue(node_t **head)
{
    if (*head == NULL)
    {
        return NULL;
    }

    node_t *temp = *head;
    tcb *thread_to_return = temp->data;
    *head = (*head)->next;
    free(temp);
    return thread_to_return;
}

tcb* dequeue_thread(node_t *head, worker_t thread_id)
{
    if (head == NULL)
    {
        return NULL;
    }

    node_t *temp = head;
    node_t *prev = NULL;

    while (temp)
    {
        if (temp->data->threadId == thread_id)
        {
            if (prev)
            {
                prev->next = temp->next;
            } else
            {
                head = temp->next;
            }
            tcb *thread_to_return = temp->data;
            free(temp);
            return thread_to_return;
        }
        prev = temp;
        temp = temp->next;
    }
    return NULL;
}

void update_global_statistics(tcb *current_thread)
{
	tot_cntx_switches++;
    avg_turn_time += current_thread->elapsed;
    avg_resp_time += current_thread->response_time;
}

struct itimerval timer;
struct sigaction sa;

void timer_handler(int signum)
{
    if (DEBUG) printf("[DEBUG] timer envoked\n");
    if (DEBUG) printf("[DEBUG] swapping to sched context\n");
    setcontext(&uctx_sched);
    // Switch to the scheduler's context

    // Reset the timer
    struct itimerval timer;
    timer.it_value.tv_sec = 0;           // Interval in seconds
    timer.it_value.tv_usec = t * 1000;  // t ms
    timer.it_interval.tv_sec = 0;       // Not using periodic timer here
    timer.it_interval.tv_usec = t * 1000;  // t ms
    setitimer(ITIMER_REAL, &timer, NULL);   
}

void setup_timer()
{
    struct itimerval timer;
    struct sigaction sa;

    // Set up the signal handler
    sa.sa_handler = timer_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    // Configure the timer to expire after t ms
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = t * 1000;  // t ms
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = t * 1000;  // t ms

    // Start the timer
    setitimer(ITIMER_REAL, &timer, NULL);
}

void wrapper_function(void *wrapper_arg)
{
    if (!wrapper_arg)
    {
        perror("Wrapper function received a NULL argument");
        //return;
    }

    wrapper_arg_t *arg = (wrapper_arg_t *)wrapper_arg;
    void *ret_val = NULL;
  
    if (DEBUG) printf("[DEBUG] running a thread for the first time\n");
    ret_val = arg->function(arg->arg);
 
    worker_exit(ret_val);
    free(wrapper_arg);
}

