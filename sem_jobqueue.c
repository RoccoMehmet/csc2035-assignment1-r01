/*
 * Replace the following string of 0s with your student number
 * 000000000
 */
#include <fcntl.h>          /* For O_* constants */
#include <sys/stat.h>       /* For mode constants */
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shobject_name.h"
#include "sem_jobqueue.h"

// Define missing constants
#define SEM_NEW_FAIL -1
#define MUTEX_SEM_SUCCESS 0
#define FULL_SEM_SUCCESS 1
#define EMPTY_SEM_SUCCESS 2
#define ALL_SEM_SUCCESS (MUTEX_SEM_SUCCESS | FULL_SEM_SUCCESS | EMPTY_SEM_SUCCESS)

// Define semaphore labels
#define sem_mutex_label "mutex_sem"
#define sem_full_label "full_sem"
#define sem_empty_label "empty_sem"

// Helper function to create new semaphores
static int sem_new(sem_t** sem, const char* sem_label, int init_value, int success) {
    char sem_name[MAX_NAME_SIZE];
    shobject_name(sem_label, sem_name);
    sem_t* new_sem = sem_open(sem_name, O_CREAT, S_IRWXU, init_value);

    if (new_sem == SEM_FAILED) {
        perror("Semaphore creation failed");
        return SEM_NEW_FAIL;
    }

    *sem = new_sem;
    return success;
}

// Helper function to delete semaphores
static void sem_delete(sem_t* sem, const char* sem_label) {
    char sem_name[MAX_NAME_SIZE];
    sem_close(sem);
    shobject_name(sem_label, sem_name);
    sem_unlink(sem_name);
}

// sem_jobqueue_new function to create a new sem_jobqueue_t and semaphores
sem_jobqueue_t* sem_jobqueue_new(proc_t* proc) {
    sem_jobqueue_t* sjq = (sem_jobqueue_t*)malloc(sizeof(sem_jobqueue_t));
    if (!sjq) return NULL;

    sjq->ijq = ipc_jobqueue_new(proc);  // Initializes the queue (delays all but init process)
    if (!sjq->ijq) {
        free(sjq);
        return NULL;
    }

    int r = sem_new(&sjq->mutex, sem_mutex_label, 1, MUTEX_SEM_SUCCESS);
    if (r != MUTEX_SEM_SUCCESS) {
        ipc_jobqueue_delete(sjq->ijq);
        free(sjq);
        return NULL;
    }

    sem_wait(sjq->mutex); // Lock mutex before initializing full/empty semaphores

    r |= sem_new(&sjq->full, sem_full_label, 0, FULL_SEM_SUCCESS)
         | sem_new(&sjq->empty, sem_empty_label, ipc_jobqueue_space(sjq->ijq), EMPTY_SEM_SUCCESS);

    if (r == ALL_SEM_SUCCESS) {
        sem_post(sjq->mutex);  // Release mutex if everything succeeded
        return sjq;
    }

    // Clean up semaphores if any part of the setup failed
    if (r & FULL_SEM_SUCCESS) sem_delete(sjq->full, sem_full_label);
    if (r & EMPTY_SEM_SUCCESS) sem_delete(sjq->empty, sem_empty_label);

    sem_post(sjq->mutex);
    sem_delete(sjq->mutex, sem_mutex_label);
    ipc_jobqueue_delete(sjq->ijq);
    free(sjq);

    return NULL;
}

// sem_jobqueue_dequeue - Dequeues the highest priority job
job_t* sem_jobqueue_dequeue(sem_jobqueue_t* sjq, job_t* dst) {
    if (!sjq) return NULL;

    if (sem_wait(sjq->full) == -1) return NULL;
    if (sem_wait(sjq->mutex) == -1) return NULL;

    do_critical_work((proc_t*)sjq);
    job_t* job = ipc_jobqueue_dequeue(sjq->ijq, dst);

    sem_post(sjq->mutex);
    sem_post(sjq->empty);

    return job;
}

// sem_jobqueue_enqueue - Enqueues a job into the queue
void sem_jobqueue_enqueue(sem_jobqueue_t* sjq, job_t* job) {
    if (!sjq || !job) return;

    if (sem_wait(sjq->empty) == -1) return;
    if (sem_wait(sjq->mutex) == -1) return;

    do_critical_work((proc_t*)sjq);
    ipc_jobqueue_enqueue(sjq->ijq, job);

    sem_post(sjq->mutex);
    sem_post(sjq->full);
}

// sem_jobqueue_is_empty - Checks if the queue is empty
bool sem_jobqueue_is_empty(sem_jobqueue_t* sjq) {
    if (!sjq) return true;

    if (sem_wait(sjq->mutex) == -1) return true;
    do_critical_work((proc_t*)sjq);
    bool is_empty = ipc_jobqueue_is_empty(sjq->ijq);
    sem_post(sjq->mutex);

    return is_empty;
}

// sem_jobqueue_is_full - Checks if the queue is full
bool sem_jobqueue_is_full(sem_jobqueue_t* sjq) {
    if (!sjq) return true;

    if (sem_wait(sjq->mutex) == -1) return true;
    do_critical_work((proc_t*)sjq);
    bool is_full = ipc_jobqueue_is_full(sjq->ijq);
    sem_post(sjq->mutex);

    return is_full;
}

// sem_jobqueue_peek - Peeks at the highest priority job in the queue
job_t* sem_jobqueue_peek(sem_jobqueue_t* sjq, job_t* dst) {
    if (!sjq) return NULL;

    if (sem_wait(sjq->mutex) == -1) return NULL;
    do_critical_work((proc_t*)sjq);
    job_t* job = ipc_jobqueue_peek(sjq->ijq, dst);
    sem_post(sjq->mutex);

    return job;
}

// sem_jobqueue_size - Gets the size of the queue
int sem_jobqueue_size(sem_jobqueue_t* sjq) {
    if (!sjq) return 0;

    if (sem_wait(sjq->mutex) == -1) return 0;
    do_critical_work((proc_t*)sjq);
    int size = ipc_jobqueue_size(sjq->ijq);
    sem_post(sjq->mutex);

    return size;
}

// sem_jobqueue_space - Gets the space in the queue
int sem_jobqueue_space(sem_jobqueue_t* sjq) {
    if (!sjq) return 0;

    if (sem_wait(sjq->mutex) == -1) return 0;
    do_critical_work((proc_t*)sjq);
    int space = ipc_jobqueue_space(sjq->ijq);
    sem_post(sjq->mutex);

    return space;
}

// sem_jobqueue_delete - Deletes the sem_jobqueue and releases resources
void sem_jobqueue_delete(sem_jobqueue_t* sjq) {
    if (!sjq) return;

    sem_delete(sjq->mutex, sem_mutex_label);
    sem_delete(sjq->full, sem_full_label);
    sem_delete(sjq->empty, sem_empty_label);
    ipc_jobqueue_delete(sjq->ijq);
    free(sjq);
}
