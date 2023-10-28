# Implementation Details

This project was a collaborative effort among group members Trevor Dovan and Maanav Choudhary.

## API Functions

### worker_create
Purpose: Creates a new thread.
Implementation: Allocates a Thread Control Block (TCB), initializes the thread context, sets up a stack, and enqueues the thread into the run queue. The thread_wrapper function is used to manage thread execu tion and termination.

### worker_yield
Purpose: Voluntarily relinquishes the CPU by the calling thread.
Implementation: Changes the thread's state to READY, saves its context, and switches to the scheduler context.

### worker_exit
Purpose: Terminates the calling thread.
Implementation: De-allocates any dynamic memory created for the thread, updates the thread's state to TERMINATED, and switches to the scheduler context.

### worker_join
Purpose: Waits for a specific thread to terminate.
Implementation: Implements a spin-wait mechanism to wait for the target thread to terminate, then cleans up resources and retrieves the exit value if required.

### worker_mutex_init
Purpose: Initializes a mutex.
Implementation: Sets up the mutex structure, initializing it as unlocked and setting up a queue for blocked threads.

### worker_mutex_lock
Purpose: Acquires a mutex lock.
Implementation: Uses an atomic test-and-set operation to acquire the mutex. If the mutex is already locked, the thread is added to the blocked queue and the context is switched to the scheduler.

### worker_mutex_unlock
Purpose: Releases a mutex lock.
Implementation: Releases the mutex and moves all threads in the blocked queue to the run queue.

### worker_mutex_destroy
Purpose: Destroys a mutex.
Implementation: De-allocates any dynamic memory associated with the mutex and handles any threads waiting on the mutex.

## Scheduler Logic

### Scheduling Algorithm
The library implements a Pre-emptive Shortest Job First (PSJF) scheduling algorithm.

### Context Switching
Implemented using swapcontext to switch between thread contexts and the scheduler context.

### Timer Handling
A timer is set up to trigger context switches at regular intervals (QUANTUM).

### Thread Selection
The scheduler selects the thread with the least elapsed quantums for execution.

## Benchmark Results

### Analysis and Comparison
Unable to run benchmarks tests as scheduling algorithms were not implemented.

### Collaboration and References

## Collaboration: 
This project was a collaborative effort among group members Trevor Dovan and Maanav Choudhary.

## External Resources:
- Consulted various online forums and documentation for understanding context switching and thread management in C.
- Referred to the POSIX pthreads library documentation for API design and behavior.
- Utilized Stack Overflow for troubleshooting specific implementation issues.