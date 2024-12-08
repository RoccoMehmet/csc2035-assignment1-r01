/*
 * Replace the following string of 0s with your student number
 * 000000000
 */
#include "joblog.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

// Function to initialize the joblog for the given process
int joblog_init(proc_t* proc) {
    if (proc == NULL) {
        errno = EINVAL;
        return -1; // Invalid argument if proc is NULL
    }

    if (proc->is_init) {
        // If the process is the init process, create the parent directory if not exists
        if (mkdir("./out", 0777) == -1 && errno != EEXIST) {
            return -1; // Failed to create directory
        }
    }

    // Construct the log file name based on proc (pseudo code for file path)
    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->pid);

    // Remove any existing log file for the process
    if (unlink(log_file) == -1 && errno != ENOENT) {
        return -1; // Failed to unlink existing file
    }

    return 0; // Success
}

// Function to write a job to the log
void joblog_write(proc_t* proc, job_t* job) {
    if (proc == NULL || job == NULL) {
        return; // If either is NULL, do nothing
    }

    // Construct the log file name based on proc (pseudo code for file path)
    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->pid);

    // Open the log file in append mode
    int fd = open(log_file, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        return; // If the file cannot be opened, do nothing
    }

    // Write the log entry using the JOB_STR_FMT from job.h
    // Assuming job_t has fields like id, pid, priority, label as per the format
    char log_buffer[JOB_STR_SIZE];
    snprintf(log_buffer, sizeof(log_buffer), "pid:%07d,id:%05d,pri:%05d,label:%-*s\n",
             job->pid, job->id, job->priority, MAX_NAME_SIZE - 1, job->label);

    // Write to the file
    if (write(fd, log_buffer, strlen(log_buffer)) == -1) {
        close(fd);
        return; // If writing fails, do nothing
    }

    close(fd); // Close the file
}

// Function to read a job from the log
job_t* joblog_read(proc_t* proc, int entry_num, job_t* job) {
    if (proc == NULL || entry_num < 0) {
        return NULL; // Invalid arguments
    }

    // Construct the log file name based on proc (pseudo code for file path)
    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->pid);

    // Open the log file for reading
    int fd = open(log_file, O_RDONLY);
    if (fd == -1) {
        return NULL; // Failed to open log file
    }

    // Read the log entry (entry_num specifies which line to read)
    char log_buffer[JOB_STR_SIZE];
    off_t offset = lseek(fd, entry_num * JOB_STR_SIZE, SEEK_SET);
    if (offset == -1) {
        close(fd);
        return NULL; // Failed to seek to the entry position
    }

    ssize_t bytes_read = read(fd, log_buffer, sizeof(log_buffer) - 1);
    if (bytes_read <= 0) {
        close(fd);
        return NULL; // Failed to read or reached EOF
    }

    // Null-terminate the string
    log_buffer[bytes_read] = '\0';

    // Parse the log entry and convert it to a job_t struct
    if (job == NULL) {
        job = malloc(sizeof(job_t)); // Dynamically allocate a job if NULL
    }

    if (sscanf(log_buffer, "pid:%d,id:%d,pri:%d,label:%255s",
               &job->pid, &job->id, &job->priority, job->label) != 4) {
        close(fd);
        return NULL; // Failed to parse the log entry
    }

    close(fd); // Close the file
    return job; // Return the parsed job
}

// Function to delete the joblog for the given process
void joblog_delete(proc_t* proc) {
    if (proc == NULL) {
        return; // If proc is NULL, do nothing
    }

    // Construct the log file name based on proc (pseudo code for file path)
    char log_file[256];
    snprintf(log_file, sizeof(log_file), "./out/joblog_%d.log", proc->pid);

    // Delete the log file
    if (unlink(log_file) == -1) {
        return; // Ignore errors while deleting the file
    }
}
