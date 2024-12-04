/*
* Replace the following string of 0s with your student number
 * 000000000
 */
#include "ipc_jobqueue.h"
#include "proc.h"  // For the do_critical_work function

/*
 * DO NOT EDIT the ipc_jobqueue_new function.
 */
ipc_jobqueue_t* ipc_jobqueue_new(proc_t* proc) {
    // Create a new ipc object and allocate a pri_jobqueue in shared memory
    ipc_jobqueue_t* ijq = ipc_new(proc, "ipc_jobq", sizeof(pri_jobqueue_t));

    if (!ijq)
        return NULL;

    // Initialize the pri_jobqueue if this is the init process
    if (proc->is_init)
        pri_jobqueue_init((pri_jobqueue_t*) ijq->addr);

    return ijq;
}

/*
 * ipc_jobqueue_dequeue(ipc_jobqueue_t* ijq, job_t* dst)
 *
 * This is a wrapper for pri_jobqueue_dequeue.
 * Adds the critical work delay as specified.
 */
job_t* ipc_jobqueue_dequeue(ipc_jobqueue_t* ijq, job_t* dst) {
    if (!ijq) return NULL;

    // Simulate critical work
    do_critical_work((proc_t*)ijq);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_dequeue((pri_jobqueue_t*)ijq->addr, dst);
}

/*
 * ipc_jobqueue_enqueue(ipc_jobqueue_t* ijq, job_t* job)
 *
 * This is a wrapper for pri_jobqueue_enqueue.
 * Adds the critical work delay as specified.
 */
void ipc_jobqueue_enqueue(ipc_jobqueue_t* ijq, job_t* job) {
    if (!ijq || !job) return;

    // Simulate critical work
    do_critical_work((proc_t*)ijq);

    // Call the corresponding function in pri_jobqueue
    pri_jobqueue_enqueue((pri_jobqueue_t*)ijq->addr, job);
}

/*
 * ipc_jobqueue_is_empty(ipc_jobqueue_t* ijq)
 *
 * This is a wrapper for pri_jobqueue_is_empty.
 * Adds the critical work delay as specified.
 */
bool ipc_jobqueue_is_empty(ipc_jobqueue_t* ijq) {
    if (!ijq) return true;

    // Simulate critical work
    do_critical_work((proc_t*)ijq);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_is_empty((pri_jobqueue_t*)ijq->addr);
}

/*
 * ipc_jobqueue_is_full(ipc_jobqueue_t* ijq)
 *
 * This is a wrapper for pri_jobqueue_is_full.
 * Adds the critical work delay as specified.
 */
bool ipc_jobqueue_is_full(ipc_jobqueue_t* ijq) {
    if (!ijq) return false;

    // Simulate critical work
    do_critical_work((proc_t*)ijq);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_is_full((pri_jobqueue_t*)ijq->addr);
}

/*
 * ipc_jobqueue_peek(ipc_jobqueue_t* ijq, job_t* dst)
 *
 * This is a wrapper for pri_jobqueue_peek.
 * Adds the critical work delay as specified.
 */
job_t* ipc_jobqueue_peek(ipc_jobqueue_t* ijq, job_t* dst) {
    if (!ijq) return NULL;

    // Simulate critical work
    do_critical_work((proc_t*)ijq);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_peek((pri_jobqueue_t*)ijq->addr, dst);
}

/*
 * ipc_jobqueue_size(ipc_jobqueue_t* ijq)
 *
 * This is a wrapper for pri_jobqueue_size.
 * Adds the critical work delay as specified.
 */
int ipc_jobqueue_size(ipc_jobqueue_t* ijq) {
    if (!ijq) return 0;

    // Simulate critical work
    do_critical_work((proc_t*)ijq);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_size((pri_jobqueue_t*)ijq->addr);
}

/*
 * ipc_jobqueue_space(ipc_jobqueue_t* ijq)
 *
 * This is a wrapper for pri_jobqueue_space.
 * Adds the critical work delay as specified.
 */
int ipc_jobqueue_space(ipc_jobqueue_t* ijq) {
    if (!ijq) return 0;

    // Simulate critical work
    do_critical_work((proc_t*)ijq);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_space((pri_jobqueue_t*)ijq->addr);
}

/*
 * ipc_jobqueue_delete(ipc_jobqueue_t* ijq)
 *
 * Deletes an ipc_jobqueue, deallocating resources associated with the queue.
 */
void ipc_jobqueue_delete(ipc_jobqueue_t* ijq) {
    if (!ijq) return;

    // Clean up the ipc object and associated shared memory resources
    ipc_delete(ijq);
}
