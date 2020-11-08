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

queue_t threads;

struct uthread_tcb {
	int state; // 0 for running, 1 for ready, 2 for exited
    uthread_ctx_t ctx; // context for the thread. We will set context during thread creation
    char *stack; // We will initialize this during thread creation
};

struct uthread_tcb *uthread_current(void)
{
	/* TODO Phase 2 */
}

void uthread_yield(void)
{
	/* TODO Phase 2 */
}

void uthread_exit(void)
{
	/* TODO Phase 2 */
}

int uthread_create(uthread_func_t func, void *arg)
{
    // First, create a TCB for thread. Set state to ready, initialize stack, initialize context
    uthread_tcb thread = malloc(sizeof(uthread_tcb));

    if (thread == NULL) {
        return -1;
    }

    thread->state = 1;
    thread->stack = (char*)uthread_ctx_alloc_stack();
    //
    // CHECK void *top_of_stack IS SET TO 1. CHECK IF CORRECT.
    //
	if (uthread_ctx_init(thread->ctx, 1, func, arg) == -1) { // type conversion because function returns void*
        return -1;
    }
    // Now that the thread is set up and is ready to run, we can push it into the queue (phase 1)
    queue_enqueue(threads, thread);
    return 0;
}

int uthread_start(uthread_func_t func, void *arg)
{
    // queue_t threads was declared as a global variable. Initialize it here
    threads = queue_create();

	// Create a new thread
    // Infinite while loop until no more threads are ready to run

    if (uthread_create(func, arg) == -1) {
        return -1;
    }

    // Infinite loop until no more threads are ready to run in the system
    while (queue_length(threads) != 0) {
        // Keep going until there are no more ready threads
    }

}

void uthread_block(void)
{
	/* TODO Phase 2/3 */
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/* TODO Phase 2/3 */
}
