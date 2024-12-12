/*
 * Replace the following string of 0s with your student number
 * 230278000
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pri_jobqueue.h"

pri_jobqueue_t* pri_jobqueue_new() {
    pri_jobqueue_t* pjq = (pri_jobqueue_t*)malloc(sizeof(pri_jobqueue_t));
    if (!pjq) return NULL;

    pri_jobqueue_init(pjq);
    return pjq;
}

void pri_jobqueue_init(pri_jobqueue_t* pjq) {
    if (!pjq) return;

    pjq->buf_size = JOB_BUFFER_SIZE;
    pjq->size = 0;

    for (int i = 0; i < pjq->buf_size; i++) {
        job_init(&pjq->jobs[i]);
    }
}

job_t* pri_jobqueue_dequeue(pri_jobqueue_t* pjq, job_t* dst) {
    if (!pjq || pri_jobqueue_is_empty(pjq)) return NULL;

    int highest_priority_index = -1;
    unsigned int highest_priority = ~0U;

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
        if (!dst) return NULL;
    } else {
        job_copy(highest_priority_job, dst);
    }

    job_init(highest_priority_job);
    pjq->size--; 
    return dst;
}

void pri_jobqueue_enqueue(pri_jobqueue_t* pjq, job_t* job) {
    if (!pjq || !job || pri_jobqueue_is_full(pjq) || job->priority == 0) return;

    for (int i = 0; i < pjq->buf_size; i++) {
        if (pjq->jobs[i].priority == 0) {
            job_copy(job, &pjq->jobs[i]); 
            pjq->size++;
            return;
        }
    }
}

bool pri_jobqueue_is_empty(pri_jobqueue_t* pjq) {
    if (!pjq) return true; 
    return (pjq->size == 0);
}

bool pri_jobqueue_is_full(pri_jobqueue_t* pjq) {
    if (!pjq) return false; 
    return (pjq->size == pjq->buf_size);
}

job_t* pri_jobqueue_peek(pri_jobqueue_t* pjq, job_t* dst) {
    if (!pjq || pri_jobqueue_is_empty(pjq)) return NULL;

    int highest_priority_index = -1;
    unsigned int highest_priority = ~0U;

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

int pri_jobqueue_size(pri_jobqueue_t* pjq) {
    if (!pjq) return 0; 
    return pjq->size;
}

int pri_jobqueue_space(pri_jobqueue_t* pjq) {
    if (!pjq) return 0;
    return (pjq->buf_size - pjq->size);
}

void pri_jobqueue_delete(pri_jobqueue_t* pjq) {
    if (pjq) {
        free(pjq);
    }
}
