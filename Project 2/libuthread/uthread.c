#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

#define IDLE 0
#define RUNNING 1
#define READY 2
#define EXITED 3

struct uthread_tcb {
	int state; // states are described above
    uthread_ctx_t* ctx; // context for the thread. We will set context during thread creation
    char *stack; // We will initialize this during thread creation
	int thread_id; // Thread id.
};

// queue_t threads_MAIN;
queue_t threads;

struct uthread_tcb* current_thread;

struct uthread_tcb *uthread_current(void)
{
	return current_thread;
}

void uthread_yield(void)
{
	// Get the head of the queue
    // move the selected TCB to the end of the queue
    // switch context into this TCB
    struct uthread_tcb* next_thread;
    // current_thread = threads->head;
    // next_thread = threads->head->next;


    // Take it from the start of queue, push it to end of queue
    queue_dequeue(threads, (void**)&current_thread);


	if(queue_length > 0){
		queue_dequeue(threads, (void**)&next_thread);
		if(next_thread != NULL){
			uthread_ctx_switch(current_thread->ctx, next_thread->ctx);
		} else {
			perror("Nothing to yield");
			exit(-1);
		}
	}
	queue_enqueue(threads, current_thread);


    exit(0);
}

void uthread_exit(void)
{
	free(current_thread);
	uthread_yield();
}

int uthread_create(uthread_func_t func, void *arg)
{
    // First, create a TCB for thread. Set state to ready, initialize stack, initialize context
	printf("allocating memory for thread\n");
	struct uthread_tcb* thread = malloc(sizeof(struct uthread_tcb));

    if (thread == NULL) {
        return -1;
    }

    thread->state = RUNNING;
    thread->stack = (char*)uthread_ctx_alloc_stack();
	printf("thread stack:%s\n", thread->stack);
	printf("thread operation\n");
    //
    // CHECK void *top_of_stack IS SET TO 1. CHECK IF CORRECT.
    //
	printf("uthread init start\n");
	if (uthread_ctx_init(thread->ctx, thread->stack, func, arg) == -1) { // type conversion because function returns void*
        return -1;
    }
	printf("thread inint finished\n");
    // Now that the thread is set up and is ready to run, we can push it into the queue (phase 1)
    queue_enqueue(threads, thread);
    return 0;
}

int uthread_start(uthread_func_t func, void *arg)
{
    /* IDEA:
     * Create a thread
     * switch context to this thread
     * then, this thread is run
     * in the infinite loop, call uthread_yield(), so it looks for other runnable threads
     * after the infinite loop, call uthread_exit()
     */
    // queue_t threads was declared as a global variable. Initialize it here
	printf("Creating queue\n");
    threads = queue_create();

	// Assign everything that a thread needs
    // Create an "idle" thread
	printf("Entering uthread create\n");
    uthread_create(func, arg);
	printf("After uthread create\n");
    // Infinite loop until no more threads are ready to run in the system
    while (queue_length(threads) != 0) {
        uthread_yield();
    }

    uthread_exit();
	return 0;
}

void uthread_block(void)
{
	/* TODO Phase 2/3 */
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/* TODO Phase 2/3 */
}
