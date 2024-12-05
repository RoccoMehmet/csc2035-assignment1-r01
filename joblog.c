/*
 * Replace the following string of 0s with your student number
 * 000000000
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "joblog.h"

/*
 * DO NOT EDIT the new_log_name function. It is a private helper
 * function provided for you to create a log name from a process
 * descriptor for use when reading, writing, and deleting a log file.
 */
static char* new_log_name(proc_t* proc) {
    static char* joblog_name_fmt = "%s/%.31s%07d.txt";
    if (!proc)
        return NULL;

    char* log_name;
    asprintf(&log_name, joblog_name_fmt, JOBLOG_PATH, proc->type_label, proc->id);

    return log_name;
}

/*
 * DO NOT EDIT the joblog_init function that sets up the log directory
 * if it does not already exist.
 */
int joblog_init(proc_t* proc) {
    if (!proc) {
        errno = EINVAL;
        return -1;
    }

    int r = 0;
    if (proc->is_init) {
        struct stat sb;
        if (stat(JOBLOG_PATH, &sb) != 0) {
            errno = 0;
            r = mkdir(JOBLOG_PATH, 0777);
        } else if (!S_ISDIR(sb.st_mode)) {
            unlink(JOBLOG_PATH);
            errno = 0;
            r = mkdir(JOBLOG_PATH, 0777);
        }
    }

    joblog_delete(proc); // In case log exists for proc
    return r;
}

/*
 * Reads a specific job entry from the log file.
 */
job_t* joblog_read(proc_t* proc, int entry_num, job_t* job) {
    if (!proc || entry_num < 0) {
        errno = EINVAL;
        return NULL;
    }

    char* log_name = new_log_name(proc);
    if (!log_name) {
        errno = ENOMEM;
        return NULL;
    }

    FILE* file = fopen(log_name, "r");
    free(log_name); // Clean up the dynamically allocated string
    if (!file) {
        return NULL; // File couldn't be opened
    }

    char line[JOB_STR_SIZE];
    int current_entry = 0;
    while (fgets(line, sizeof(line), file)) {
        if (current_entry == entry_num) {
            fclose(file);
            return str_to_job(line, job);
        }
        current_entry++;
    }

    fclose(file);
    errno = ENOENT;  // Entry not found
    return NULL;
}

/*
 * Appends a job entry to the log file.
 */
void joblog_write(proc_t* proc, job_t* job) {
    if (!proc || !job) {
        errno = EINVAL;
        return;
    }

    char* log_name = new_log_name(proc);
    if (!log_name) {
        errno = ENOMEM;
        return;
    }

    FILE* file = fopen(log_name, "a");
    free(log_name);  // Clean up the dynamically allocated string
    if (!file) {
        errno = ENOENT;  // Couldn't open file for appending
        return;
    }

    char job_str[JOB_STR_SIZE];
    if (job_to_str(job, job_str)) {
        fprintf(file, "%s\n", job_str);  // Append the job string
    }

    fclose(file);
}

/*
 * Deletes the log file for the given process.
 */
void joblog_delete(proc_t* proc) {
    if (!proc) {
        errno = EINVAL;
        return;
    }

    char* log_name = new_log_name(proc);
    if (!log_name) {
        errno = ENOMEM;
        return;
    }

    unlink(log_name);  // Remove the file. Errors are ignored as specified.
    free(log_name);    // Clean up the dynamically allocated string
}
