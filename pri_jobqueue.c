/*
 * Replace the following string of 0s with your student number
 * 000000000
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pri_jobqueue.h"

/*
 * pri_jobqueue_new()
 *
 * Dynamically allocates and initializes a priority job queue.
 */
pri_jobqueue_t* pri_jobqueue_new() {
    pri_jobqueue_t* pjq = (pri_jobqueue_t*)malloc(sizeof(pri_jobqueue_t));
    if (!pjq) return NULL;

    pri_jobqueue_init(pjq);
    return pjq;
}

/*
 * pri_jobqueue_init(pri_jobqueue_t* pjq)
 *
 * Initializes the queue: sets size to 0, and initializes all jobs.
 */
void pri_jobqueue_init(pri_jobqueue_t* pjq) {
    if (!pjq) return;

    pjq->buf_size = JOB_BUFFER_SIZE;
    pjq->size = 0;

    // Initialize all jobs in the buffer
    for (int i = 0; i < pjq->buf_size; i++) {
        job_init(&pjq->jobs[i]);
    }
}

/*
 * pri_jobqueue_dequeue(pri_jobqueue_t* pjq, job_t* dst)
 *
 * Removes the highest priority job from the queue.
 * The job is copied into dst, or dynamically allocated if dst is NULL.
 */
job_t* pri_jobqueue_dequeue(pri_jobqueue_t* pjq, job_t* dst) {
    if (!pjq || pri_jobqueue_is_empty(pjq)) return NULL;

    int highest_priority_index = -1;
    unsigned int highest_priority = ~0U;  // Start with max unsigned value.

    // Find the highest priority job
    for (int i = 0; i < pjq->buf_size; i++) {
        if (pjq->jobs[i].priority > 0 && pjq->jobs[i].priority < highest_priority) {
            highest_priority = pjq->jobs[i].priority;
            highest_priority_index = i;
        }
    }

    if (highest_priority_index == -1) return NULL;

    job_t* highest_priority_job = &pjq->jobs[highest_priority_index];
    if (!dst) {
        dst = job_new(highest_priority_job->pid, highest_priority_job->id,
                      highest_priority_job->priority, highest_priority_job->label);
        if (!dst) return NULL;  // Allocation failed.
    } else {
        job_copy(highest_priority_job, dst);  // Copy the job to dst.
    }

    // Mark the slot as empty
    job_init(highest_priority_job);
    pjq->size--;  // Decrement the queue size.

    return dst;
}

/*
 * pri_jobqueue_enqueue(pri_jobqueue_t* pjq, job_t* job)
 *
 * Adds a new job to the queue.
 */
void pri_jobqueue_enqueue(pri_jobqueue_t* pjq, job_t* job) {
    if (!pjq || !job || pri_jobqueue_is_full(pjq) || job->priority == 0) return;

    // Find the first empty slot
    for (int i = 0; i < pjq->buf_size; i++) {
        if (pjq->jobs[i].priority == 0) {  // Empty slot (priority 0).
            job_copy(job, &pjq->jobs[i]);  // Copy the job into the slot.
            pjq->size++;  // Increment the queue size.
            return;
        }
    }
}

/*
 * pri_jobqueue_is_empty(pri_jobqueue_t* pjq)
 *
 * Returns true if the queue is empty, false otherwise.
 */
bool pri_jobqueue_is_empty(pri_jobqueue_t* pjq) {
    return (!pjq || pjq->size == 0);
}

/*
 * pri_jobqueue_is_full(pri_jobqueue_t* pjq)
 *
 * Returns true if the queue is full, false otherwise.
 */
bool pri_jobqueue_is_full(pri_jobqueue_t* pjq) {
    return (pjq && pjq->size >= pjq->buf_size);
}

/*
 * pri_jobqueue_peek(pri_jobqueue_t* pjq, job_t* dst)
 *
 * Returns the highest priority job without removing it.
 */
job_t* pri_jobqueue_peek(pri_jobqueue_t* pjq, job_t* dst) {
    if (!pjq || pri_jobqueue_is_empty(pjq)) return NULL;

    int highest_priority_index = -1;
    unsigned int highest_priority = ~0U;

    // Find the highest priority job
    for (int i = 0; i < pjq->buf_size; i++) {
        if (pjq->jobs[i].priority > 0 && pjq->jobs[i].priority < highest_priority) {
            highest_priority = pjq->jobs[i].priority;
            highest_priority_index = i;
        }
    }

    if (highest_priority_index == -1) return NULL;

    job_t* highest_priority_job = &pjq->jobs[highest_priority_index];
    if (!dst) {
        return job_new(highest_priority_job->pid, highest_priority_job->id,
                       highest_priority_job->priority, highest_priority_job->label);
    } else {
        job_copy(highest_priority_job, dst);
        return dst;
    }
}

/*
 * pri_jobqueue_size(pri_jobqueue_t* pjq)
 *
 * Returns the current size of the queue (number of valid jobs).
 */
int pri_jobqueue_size(pri_jobqueue_t* pjq) {
    return pjq ? pjq->size : 0;
}

/*
 * pri_jobqueue_space(pri_jobqueue_t* pjq)
 *
 * Returns the available space in the queue.
 */
int pri_jobqueue_space(pri_jobqueue_t* pjq) {
    return pjq ? (pjq->buf_size - pjq->size) : 0;
}

/*
 * pri_jobqueue_delete(pri_jobqueue_t* pjq)
 *
 * Frees the dynamically allocated memory for the job queue.
 */
void pri_jobqueue_delete(pri_jobqueue_t* pjq) {
    if (pjq) {
        free(pjq);
    }
}
