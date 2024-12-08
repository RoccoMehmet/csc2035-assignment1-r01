#include <fcntl.h>          /* For O_* constants */
#include <sys/stat.h>       /* For mode constants */
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shobject_name.h"
#include "sem_jobqueue.h"

// Define constants
#define SEM_NEW_FAIL -1
#define MUTEX_SEM_SUCCESS 1
#define FULL_SEM_SUCCESS 2
#define EMPTY_SEM_SUCCESS 4
#define ALL_SEM_SUCCESS (MUTEX_SEM_SUCCESS | FULL_SEM_SUCCESS | EMPTY_SEM_SUCCESS)

// Semaphore labels
#define sem_mutex_label "sjq.mutex"
#define sem_full_label "sjq.full"
#define sem_empty_label "sjq.empty"

// Helper function to create semaphores
static int sem_new(sem_t** sem, const char* sem_label, int init_value, int success) {
    char sem_name[MAX_NAME_SIZE];
    shobject_name(sem_label, sem_name);
    sem_t* new_sem = sem_open(sem_name, O_CREAT | O_EXCL, S_IRWXU, init_value);

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
    if (sem) {
        sem_close(sem);
        shobject_name(sem_label, sem_name);
        sem_unlink(sem_name);
    }
}

// Create a new sem_jobqueue with associated semaphores
sem_jobqueue_t* sem_jobqueue_new(proc_t* proc) {
    sem_jobqueue_t* sjq = (sem_jobqueue_t*)malloc(sizeof(sem_jobqueue_t));
    if (!sjq) return NULL;

    sjq->ijq = ipc_jobqueue_new(proc);
    if (!sjq->ijq) {
        free(sjq);
        return NULL;
    }

    int r = sem_new(&sjq->mutex, sem_mutex_label, 1, MUTEX_SEM_SUCCESS);
    r |= sem_new(&sjq->full, sem_full_label, 0, FULL_SEM_SUCCESS);
    r |= sem_new(&sjq->empty, sem_empty_label, ipc_jobqueue_space(sjq->ijq), EMPTY_SEM_SUCCESS);

    if (r == ALL_SEM_SUCCESS) {
        printf("sem_jobqueue_new: Successfully created semaphores\n");
        return sjq;
    }

    // Clean up on failure
    if (r & FULL_SEM_SUCCESS) sem_delete(sjq->full, sem_full_label);
    if (r & EMPTY_SEM_SUCCESS) sem_delete(sjq->empty, sem_empty_label);
    sem_delete(sjq->mutex, sem_mutex_label);
    ipc_jobqueue_delete(sjq->ijq);
    free(sjq);

    printf("sem_jobqueue_new: Semaphore creation failed\n");
    return NULL;
}

// Dequeue a job from the queue
job_t* sem_jobqueue_dequeue(sem_jobqueue_t* sjq, job_t* dst) {
    if (!sjq) return NULL;

    printf("sem_jobqueue_dequeue: Waiting for 'full' semaphore...\n");
    if (sem_wait(sjq->full) == -1) {
        perror("sem_wait full failed");
        return NULL;
    }
    printf("sem_jobqueue_dequeue: Acquired 'full' semaphore\n");

    printf("sem_jobqueue_dequeue: Waiting for 'mutex' semaphore...\n");
    if (sem_wait(sjq->mutex) == -1) {
        sem_post(sjq->full); // Unlock full semaphore if mutex failed
        perror("sem_wait mutex failed");
        return NULL;
    }
    printf("sem_jobqueue_dequeue: Acquired 'mutex' semaphore\n");

    // Critical work: simulate processing
    do_critical_work(sjq->ijq->proc);

    // Dequeue the job
    job_t* job = ipc_jobqueue_dequeue(sjq->ijq, dst);

    // Unlock mutex and signal the empty semaphore (space available)
    printf("sem_jobqueue_dequeue: Releasing 'mutex' semaphore\n");
    sem_post(sjq->mutex);
    printf("sem_jobqueue_dequeue: Released 'mutex' semaphore\n");

    printf("sem_jobqueue_dequeue: Signaling 'empty' semaphore\n");
    sem_post(sjq->empty);

    return job;
}

// Enqueue a job into the queue
void sem_jobqueue_enqueue(sem_jobqueue_t* sjq, job_t* job) {
    if (!sjq || !job) return;

    printf("sem_jobqueue_enqueue: Waiting for 'empty' semaphore...\n");
    if (sem_wait(sjq->empty) == -1) {
        perror("sem_wait empty failed");
        return;
    }
    printf("sem_jobqueue_enqueue: Acquired 'empty' semaphore\n");

    printf("sem_jobqueue_enqueue: Waiting for 'mutex' semaphore...\n");
    if (sem_wait(sjq->mutex) == -1) {
        sem_post(sjq->empty); // Unlock empty semaphore if mutex failed
        perror("sem_wait mutex failed");
        return;
    }
    printf("sem_jobqueue_enqueue: Acquired 'mutex' semaphore\n");

    // Critical work: simulate processing
    do_critical_work(sjq->ijq->proc);

    // Enqueue the job
    ipc_jobqueue_enqueue(sjq->ijq, job);

    // Unlock mutex and signal the full semaphore (job added)
    printf("sem_jobqueue_enqueue: Releasing 'mutex' semaphore\n");
    sem_post(sjq->mutex);
    printf("sem_jobqueue_enqueue: Released 'mutex' semaphore\n");

    printf("sem_jobqueue_enqueue: Signaling 'full' semaphore\n");
    sem_post(sjq->full);
}

// Check if the queue is empty
bool sem_jobqueue_is_empty(sem_jobqueue_t* sjq) {
    if (!sjq) return true;

    // Lock the mutex for exclusive access
    if (sem_wait(sjq->mutex) == -1) return true;

    // Critical work: simulate processing
    do_critical_work(sjq->ijq->proc);

    // Check if the queue is empty
    bool is_empty = ipc_jobqueue_is_empty(sjq->ijq);

    // Unlock mutex
    sem_post(sjq->mutex);
    return is_empty;
}

// Check if the queue is full
bool sem_jobqueue_is_full(sem_jobqueue_t* sjq) {
    if (!sjq) return true;

    // Lock the mutex for exclusive access
    if (sem_wait(sjq->mutex) == -1) return true;

    // Critical work: simulate processing
    do_critical_work(sjq->ijq->proc);

    // Check if the queue is full
    bool is_full = ipc_jobqueue_is_full(sjq->ijq);

    // Unlock mutex
    sem_post(sjq->mutex);
    return is_full;
}

// Peek at the highest priority job
job_t* sem_jobqueue_peek(sem_jobqueue_t* sjq, job_t* dst) {
    if (!sjq) return NULL;

    // Lock the mutex for exclusive access
    if (sem_wait(sjq->mutex) == -1) return NULL;

    // Critical work: simulate processing
    do_critical_work(sjq->ijq->proc);

    // Peek at the job
    job_t* job = ipc_jobqueue_peek(sjq->ijq, dst);

    // Unlock mutex
    sem_post(sjq->mutex);
    return job;
}

// Get the size of the queue
int sem_jobqueue_size(sem_jobqueue_t* sjq) {
    if (!sjq) return 0;

    // Lock the mutex for exclusive access
    if (sem_wait(sjq->mutex) == -1) return 0;

    // Critical work: simulate processing
    do_critical_work(sjq->ijq->proc);

    // Get the size of the queue
    int size = ipc_jobqueue_size(sjq->ijq);

    // Unlock mutex
    sem_post(sjq->mutex);
    return size;
}

// Get the space in the queue
int sem_jobqueue_space(sem_jobqueue_t* sjq) {
    if (!sjq) return 0;

    // Lock the mutex for exclusive access
    if (sem_wait(sjq->mutex) == -1) return 0;

    // Critical work: simulate processing
    do_critical_work(sjq->ijq->proc);

    // Get the space in the queue
    int space = ipc_jobqueue_space(sjq->ijq);

    // Unlock mutex
    sem_post(sjq->mutex);
    return space;
}

// Delete the sem_jobqueue and associated resources
void sem_jobqueue_delete(sem_jobqueue_t* sjq) {
    if (!sjq) return;

    // Clean up semaphores and job queue
    sem_delete(sjq->mutex, sem_mutex_label);
    sem_delete(sjq->full, sem_full_label);
    sem_delete(sjq->empty, sem_empty_label);
    ipc_jobqueue_delete(sjq->ijq);

    // Free the memory
    free(sjq);
}
