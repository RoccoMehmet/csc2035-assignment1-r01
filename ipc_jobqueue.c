#include "ipc_jobqueue.h"
#include "proc.h"  // For the do_critical_work function

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

job_t* ipc_jobqueue_dequeue(ipc_jobqueue_t* ijq, job_t* dst) {
    if (!ijq) return NULL;

    // Simulate critical work
    do_critical_work(ijq->proc);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_dequeue((pri_jobqueue_t*)ijq->addr, dst);
}

void ipc_jobqueue_enqueue(ipc_jobqueue_t* ijq, job_t* job) {
    if (!ijq || !job) return;

    // Simulate critical work
    do_critical_work(ijq->proc);

    // Call the corresponding function in pri_jobqueue
    pri_jobqueue_enqueue((pri_jobqueue_t*)ijq->addr, job);
}

bool ipc_jobqueue_is_empty(ipc_jobqueue_t* ijq) {
    if (!ijq) return true; // Return true if the queue is NULL

    // Simulate critical work
    do_critical_work(ijq->proc);

    // Check if the underlying pri_jobqueue is empty
    return pri_jobqueue_is_empty((pri_jobqueue_t*)ijq->addr);
}

bool ipc_jobqueue_is_full(ipc_jobqueue_t* ijq) {
    if (!ijq) return false;

    // Simulate critical work
    do_critical_work(ijq->proc);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_is_full((pri_jobqueue_t*)ijq->addr);
}

job_t* ipc_jobqueue_peek(ipc_jobqueue_t* ijq, job_t* dst) {
    if (!ijq) return NULL;

    // Simulate critical work
    do_critical_work(ijq->proc);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_peek((pri_jobqueue_t*)ijq->addr, dst);
}

int ipc_jobqueue_size(ipc_jobqueue_t* ijq) {
    if (!ijq) return 0;

    // Simulate critical work
    do_critical_work(ijq->proc);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_size((pri_jobqueue_t*)ijq->addr);
}

int ipc_jobqueue_space(ipc_jobqueue_t* ijq) {
    if (!ijq) return 0;

    // Simulate critical work
    do_critical_work(ijq->proc);

    // Call the corresponding function in pri_jobqueue
    return pri_jobqueue_space((pri_jobqueue_t*)ijq->addr);
}

void ipc_jobqueue_delete(ipc_jobqueue_t* ijq) {
    if (!ijq) return;

    // Clean up the ipc object and associated shared memory resources
    ipc_delete(ijq);
}
