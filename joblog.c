#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "joblog.h"
#include "job.h"
#include "proc.h"
#include <sys/stat.h>
/*
 * joblog_init(proc_t* proc)
 * 
 * Initialise a joblog for the given process to write jobs to.
 * Log initialisation does the following:
 * 1. if proc is the init process (its is_init field is true), create the
 *    parent directory (./out) for logs if it does not already exist.
 * 2. delete any existing log for the given process.
 *
 * Parameters:
 * proc - the non-NULL descriptor of a process that will log jobs
 *
 * Return:
 * On success: 0
 * On failure: -1, and errno is set as specified in Errors.
 */
int joblog_init(proc_t* proc) {
    if (proc == NULL) {
        errno = EINVAL;
        return -1;
    }

    // If the process is the init process, create the log directory
    if (proc->is_init) {
        if (mkdir("./out", 0777) == -1 && errno != EEXIST) {
            return -1;
        }
    }

    // Generate the log filename and remove it if it exists
    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->id);
    unlink(log_file);

    return 0;
}

/*
 * joblog_read(proc_t* proc, int entry_num, job_t* job)
 *
 * Read the specified entry from the given process' log.
 * The entry_num specifies the log entry to read.
 *
 * Parameters:
 * proc - the non-null descriptor of the process whose log to read
 * entry_num - the non-negative number of the entry in the log to read
 * job - a pointer to a job to convert the string entry to. If job is NULL,
 *      the function dynamically allocates one.
 *
 * Return:
 * On success: a job that is the entry in the given process' log specified
 *      by the given entry_num. If the job is dynamically allocated, it is
 *      the responsibility of the user to free the memory allocated.
 * On failure: NULL, and failures are handled as specified in the Notes section
 */
job_t* joblog_read(proc_t* proc, int entry_num, job_t* job) {
    if (proc == NULL || entry_num < 0) {
        return NULL;
    }

    // Generate the log file name based on the process ID
    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->id);

    // Open the log file
    FILE *log = fopen(log_file, "r");
    if (log == NULL) {
        return NULL;
    }

    char line[JOB_STR_SIZE];
    int current_entry = 0;

    // Read the log file line by line
    while (fgets(line, sizeof(line), log) != NULL) {
        if (current_entry == entry_num) {
            // If the job parameter is NULL, dynamically allocate memory
            if (job == NULL) {
                job = (job_t*)malloc(sizeof(job_t));
            }

            // Temporary variables to hold parsed values
            int pid, job_id, priority;
            char label[32];

            // Parse the line using the format defined in JOB_STR_FMT
            if (sscanf(line, JOB_STR_FMT, &pid, &job_id, &priority, label) == 4) {
                // Assign the parsed values to the job struct
                job->id = job_id;
                job->priority = priority;  // Ensure priority is an unsigned int
                strncpy(job->label, label, sizeof(job->label) - 1);
                job->label[sizeof(job->label) - 1] = '\0';  // Ensure null termination
            }

            fclose(log);
            return job;
        }
        current_entry++;
    }

    fclose(log);
    return NULL;
}

/*
 * joblog_write(proc_t* proc, job_t* job)
 *
 * Write an entry for the given job to the given process' log.
 * The entry is written as a line in the following format:
 *      pid:<proc_id>,id:<job_id>,pri:<priority>:label:<label>
 *
 * Parameters:
 * proc - the non-null descriptor of a process that will log jobs
 * job - the non-null pointer to a job descriptor
 */
void joblog_write(proc_t* proc, job_t* job) {
    if (proc == NULL || job == NULL) {
        return;
    }

    // Generate the log file name
    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->id);

    // Open the log file in append mode
    FILE *log = fopen(log_file, "a");
    if (log == NULL) {
        return;
    }

    // Write the job entry to the log file
    fprintf(log, JOB_STR_FMT, proc->id, job->id, job->priority, job->label);

    fclose(log);
}

/*
 * joblog_delete(proc_t* proc)
 *
 * Delete the given process' log.
 *
 * Parameters:
 * proc - the non-null descriptor of the process whose log to delete
 */
void joblog_delete(proc_t* proc) {
    if (proc == NULL) {
        return;
    }

    // Generate the log file name
    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->id);

    // Delete the log file
    unlink(log_file);
}
