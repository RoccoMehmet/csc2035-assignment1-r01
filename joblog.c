/*
 * Replace the following string of 0s with your student number
 * 000000000
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>  // Include for mkdir
#include "joblog.h"
#include "job.h"
#include "proc.h"

/* joblog_init function */
int joblog_init(proc_t* proc) {
    if (proc == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Create the parent directory for logs if init process
    if (proc->is_init) {
        if (mkdir("./out", 0755) == -1 && errno != EEXIST) {
            return -1;
        }
    }

    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->id);

    // Delete existing log if present
    if (remove(log_file) == -1 && errno != ENOENT) {
        return -1;
    }

    return 0;
}

/* joblog_write function */
void joblog_write(proc_t* proc, job_t* job) {
    if (proc == NULL || job == NULL) {
        return;
    }

    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->id);

    FILE *log = fopen(log_file, "a");
    if (log == NULL) {
        return;
    }

    fprintf(log, "pid:%07d,id:%05d,pri:%05u,label:%-*s\n", proc->id, job->id, job->priority, MAX_NAME_SIZE-1, job->label);
    fclose(log);
}

/* joblog_read function */
job_t* joblog_read(proc_t* proc, int entry_num, job_t* job) {
    if (proc == NULL || entry_num < 0) {
        return NULL;
    }

    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->id);

    FILE *log = fopen(log_file, "r");
    if (log == NULL) {
        return NULL;
    }

    char line[JOB_STR_SIZE];
    int current_entry = 0;
    while (fgets(line, sizeof(line), log) != NULL) {
        if (current_entry == entry_num) {
            if (job == NULL) {
                job = (job_t*)malloc(sizeof(job_t));
            }

            // Correct the format specifier
            sscanf(line, JOB_STR_FMT, &job->id, &job->priority, job->label); 
            fclose(log);
            return job;
        }
        current_entry++;
    }

    fclose(log);
    return NULL;
}

/* joblog_delete function */
void joblog_delete(proc_t* proc) {
    if (proc == NULL) {
        return;
    }

    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->id);

    if (remove(log_file) == -1) {
        // Handle error if necessary
        return;
    }
}
