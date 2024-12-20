#include "sem_jobqueue.h"
#include <stdlib.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

struct sem_jobqueue {
    jobqueue_t* base_queue; // Base job queue
    sem_t mutex;            // Semaphore for mutual exclusion
    sem_t full;             // Semaphore for counting filled slots
    sem_t empty;            // Semaphore for counting empty slots
};

sem_jobqueue_t* sem_jobqueue_new(proc_t* proc) {
    if (!proc) return NULL;

    sem_jobqueue_t* sjq = malloc(sizeof(sem_jobqueue_t));
    if (!sjq) return NULL;

    sjq->base_queue = jobqueue_new(proc);
    if (!sjq->base_queue) {
        free(sjq);
        return NULL;
    }

    if (sem_init(&sjq->mutex, 1, 1) == -1 ||
        sem_init(&sjq->full, 1, 0) == -1 ||
        sem_init(&sjq->empty, 1, JOB_BUFFER_SIZE) == -1) {
        jobqueue_delete(sjq->base_queue);
        free(sjq);
        return NULL;
    }

    return sjq;
}

void sem_jobqueue_delete(sem_jobqueue_t* sjq) {
    if (!sjq) return;

    jobqueue_delete(sjq->base_queue);
    sem_destroy(&sjq->mutex);
    sem_destroy(&sjq->full);
    sem_destroy(&sjq->empty);
    free(sjq);
}

jobqueue_t* sem_jobqueue_enqueue(sem_jobqueue_t* sjq, const job_t* job) {
    if (!sjq || !job) return NULL;

    // Wait for an empty slot
    sem_wait(&sjq->empty);

    // Acquire mutex lock
    sem_wait(&sjq->mutex);

    // Enqueue the job
    jobqueue_t* result = jobqueue_enqueue(sjq->base_queue, job);

    // Release mutex lock
    sem_post(&sjq->mutex);

    // Signal a filled slot
    sem_post(&sjq->full);

    return result;
}

jobqueue_t* sem_jobqueue_dequeue(sem_jobqueue_t* sjq, job_t* job) {
    if (!sjq || !job) return NULL;

    // Wait for a filled slot
    sem_wait(&sjq->full);

    // Acquire mutex lock
    sem_wait(&sjq->mutex);

    // Dequeue the job
    jobqueue_t* result = jobqueue_dequeue(sjq->base_queue, job);

    // Release mutex lock
    sem_post(&sjq->mutex);

    // Signal an empty slot
    sem_post(&sjq->empty);

    return result;
}

bool sem_jobqueue_is_empty(sem_jobqueue_t* sjq) {
    if (!sjq) return true;

    // Check if the base queue is empty
    bool is_empty;

    sem_wait(&sjq->mutex); // Lock mutex
    is_empty = jobqueue_is_empty(sjq->base_queue);
    sem_post(&sjq->mutex); // Unlock mutex

    return is_empty;
}

bool sem_jobqueue_is_full(sem_jobqueue_t* sjq) {
    if (!sjq) return false;

    // Check if the base queue is full
    bool is_full;

    sem_wait(&sjq->mutex); // Lock mutex
    is_full = jobqueue_is_full(sjq->base_queue);
    sem_post(&sjq->mutex); // Unlock mutex

    return is_full;
}
