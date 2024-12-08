#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shobject_name.h"
#include "sem_jobqueue.h"

#define SEM_NEW_FAIL -1
#define MUTEX_SEM_SUCCESS 1
#define FULL_SEM_SUCCESS 2
#define EMPTY_SEM_SUCCESS 4
#define ALL_SEM_SUCCESS (MUTEX_SEM_SUCCESS | FULL_SEM_SUCCESS | EMPTY_SEM_SUCCESS)

#define sem_mutex_label "sjq.mutex"
#define sem_full_label "sjq.full"
#define sem_empty_label "sjq.empty"

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

static void sem_delete(sem_t* sem, const char* sem_label) {
    char sem_name[MAX_NAME_SIZE];
    if (sem) {
        sem_close(sem);
        shobject_name(sem_label, sem_name);
        sem_unlink(sem_name);
    }
}

sem_jobqueue_t* sem_jobqueue_new(proc_t* proc) {
    sem_jobqueue_t* sjq = (sem_jobqueue_t*)malloc(sizeof(sem_jobqueue_t));
    if (!sjq) return NULL;

    sjq->ijq = ipc_jobqueue_new(proc);
    if (!sjq->ijq) {
        free(sjq);
        return NULL;
    }

    int r = sem_new(&sjq->mutex, sem_mutex_label, 1, MUTEX_SEM_SUCCESS);
    if (r != MUTEX_SEM_SUCCESS) {
        printf("Failed to create mutex semaphore\n");
        return NULL;
    }

    r = sem_new(&sjq->full, sem_full_label, 0, FULL_SEM_SUCCESS);
    if (r != FULL_SEM_SUCCESS) {
        printf("Failed to create full semaphore\n");
        sem_delete(sjq->mutex, sem_mutex_label);
        ipc_jobqueue_delete(sjq->ijq);
        free(sjq);
        return NULL;
    }

    r = sem_new(&sjq->empty, sem_empty_label, ipc_jobqueue_space(sjq->ijq), EMPTY_SEM_SUCCESS);
    if (r != EMPTY_SEM_SUCCESS) {
        printf("Failed to create empty semaphore\n");
        sem_delete(sjq->full, sem_full_label);
        sem_delete(sjq->mutex, sem_mutex_label);
        ipc_jobqueue_delete(sjq->ijq);
        free(sjq);
        return NULL;
    }

    return sjq;
}

job_t* sem_jobqueue_dequeue(sem_jobqueue_t* sjq, job_t* dst) {
    if (!sjq) return NULL;

    printf("Waiting on full semaphore...\n");
    int result = sem_wait(sjq->full);
    if (result == -1) {
        perror("sem_wait on full failed");
        return NULL;
    }
    printf("Full semaphore acquired.\n");

    printf("Waiting on mutex semaphore...\n");
    result = sem_wait(sjq->mutex);
    if (result == -1) {
        sem_post(sjq->full); // Release full semaphore
        perror("sem_wait on mutex failed");
        return NULL;
    }
    printf("Mutex semaphore acquired.\n");

    printf("Critical section entered\n");
    do_critical_work(sjq->ijq->proc);

    job_t* job = ipc_jobqueue_dequeue(sjq->ijq, dst);
    if (job == NULL) {
        printf("Dequeue failed: Queue is empty.\n");
    }

    sem_post(sjq->mutex);
    sem_post(sjq->empty); // Signal that space is available in the queue
    printf("Exiting critical section\n");

    return job;
}

void sem_jobqueue_enqueue(sem_jobqueue_t* sjq, job_t* job) {
    if (!sjq || !job) return;

    int result = sem_wait(sjq->empty);
    if (result == -1) {
        perror("sem_wait on empty failed");
        return;
    }
    printf("Empty semaphore acquired.\n");

    printf("Waiting on mutex semaphore...\n");
    result = sem_wait(sjq->mutex);
    if (result == -1) {
        sem_post(sjq->empty); // Release empty semaphore
        perror("sem_wait on mutex failed");
        return;
    }
    printf("Mutex semaphore acquired.\n");

    printf("Critical section entered\n");
    do_critical_work(sjq->ijq->proc);
    ipc_jobqueue_enqueue(sjq->ijq, job);

    sem_post(sjq->mutex);
    sem_post(sjq->full); // Signal that the queue is full
    printf("Exiting critical section\n");
}

void sem_jobqueue_delete(sem_jobqueue_t* sjq) {
    if (sjq) {
        sem_delete(sjq->empty, sem_empty_label);
        sem_delete(sjq->full, sem_full_label);
        sem_delete(sjq->mutex, sem_mutex_label);
        ipc_jobqueue_delete(sjq->ijq);
        free(sjq);
    }
}
