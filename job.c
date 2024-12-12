/*
 * Replace the following string of 0s with your student number
 * 230278000
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "job.h"

job_t* job_new(pid_t pid, unsigned int id, unsigned int priority, const char* label) {
    return job_set((job_t*) malloc(sizeof(job_t)), pid, id, priority, label);
}

job_t* job_copy(job_t* src, job_t* dst) {
    if (!src) return NULL;
    if (strnlen(src->label, MAX_NAME_SIZE) != MAX_NAME_SIZE - 1) {
        return NULL;
    }
    if (dst == src) {
        return dst;  
    }
    if (!dst) {
        dst = (job_t*) malloc(sizeof(job_t));
        if (!dst) return NULL; 
    }
    dst->pid = src->pid;
    dst->id = src->id;
    dst->priority = src->priority;
    strncpy(dst->label, src->label, MAX_NAME_SIZE - 1);
    dst->label[MAX_NAME_SIZE - 1] = '\0';
    return dst;
}

void job_init(job_t* job) {
    if (!job) return;
    job->pid = 0;
    job->id = 0;
    job->priority = 0;
    strncpy(job->label, PAD_STRING, MAX_NAME_SIZE - 1);
    job->label[MAX_NAME_SIZE - 1] = '\0';
}

bool job_is_equal(job_t* j1, job_t* j2) {
    if (j1 == j2) return true;
    if (!j1 || !j2) return false;

    return (j1->pid == j2->pid) &&
           (j1->id == j2->id) &&
           (j1->priority == j2->priority) &&
           (strncmp(j1->label, j2->label, MAX_NAME_SIZE - 1) == 0);
}

job_t* job_set(job_t* job, pid_t pid, unsigned int id, unsigned int priority, const char* label) {
    if (!job) return NULL;

    job->pid = pid;
    job->id = id;
    job->priority = priority;

    if (!label || label[0] == '\0') {
        strncpy(job->label, PAD_STRING, MAX_NAME_SIZE - 1);
        job->label[MAX_NAME_SIZE - 1] = '\0';
    } else {
        size_t len = strnlen(label, MAX_NAME_SIZE - 1);
        strncpy(job->label, label, len);
        memset(job->label + len, '*', MAX_NAME_SIZE - 1 - len);
        job->label[MAX_NAME_SIZE - 1] = '\0'; 
    }
    return job;
}

char* job_to_str(job_t* job, char* str) {
    if (!job || strnlen(job->label, MAX_NAME_SIZE) != MAX_NAME_SIZE - 1) {
        return NULL;
    }
    if (!str) {
        str = (char*) malloc(JOB_STR_SIZE);
        if (!str) return NULL;
    }
    int ret = snprintf(str, JOB_STR_SIZE, JOB_STR_FMT, job->pid, job->id, job->priority, job->label);
    if (ret < 0 || ret >= JOB_STR_SIZE) {
        if (!str) free(str);
        return NULL;
    }
    return str;
}

job_t* str_to_job(char* str, job_t* job) {
    if (!str || strnlen(str, JOB_STR_SIZE + 1) != JOB_STR_SIZE - 1) return NULL;
    pid_t pid;
    unsigned int id, priority;
    char label[MAX_NAME_SIZE];
    if (sscanf(str, JOB_STR_FMT, &pid, &id, &priority, label) != 4) {
        return NULL;
    }
    if (!job) {
        job = (job_t*) malloc(sizeof(job_t));
        if (!job) return NULL;
    }
    job->pid = pid;
    job->id = id;
    job->priority = priority;
    strncpy(job->label, label, MAX_NAME_SIZE - 1);
    job->label[MAX_NAME_SIZE - 1] = '\0';
    return job;
}

void job_delete(job_t* job) {
    if (job) free(job);
}
