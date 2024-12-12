#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

// Define maximum number of jobs
#define MAX_JOBS 100

// Job structure
typedef struct {
    pid_t pid;      // Process ID
    int status;     // Job status (e.g., running, finished)
    char cmd[256];  // Command associated with the job
} Job;

// Function to write job log to a file
int write_job_log(const char *log_filename, Job *job, int job_count) {
    FILE *file = fopen(log_filename, "a");
    if (!file) {
        perror("Failed to open log file");
        return -1;
    }

    // Write all jobs to the file
    for (int i = 0; i < job_count; i++) {
        fprintf(file, "PID: %d, Status: %d, Command: %s\n", job[i].pid, job[i].status, job[i].cmd);
    }

    fclose(file);
    return 0;
}

// Function to read job log from a file
int read_job_log(const char *log_filename, Job *job, int job_count) {
    FILE *file = fopen(log_filename, "r");
    if (!file) {
        perror("Failed to open log file");
        return -1;
    }

    // Read all jobs from the file
    for (int i = 0; i < job_count; i++) {
        if (fscanf(file, "PID: %d, Status: %d, Command: %255[^\n]\n", &job[i].pid, &job[i].status, job[i].cmd) != 3) {
            perror("Error reading job log");
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}

// Function to delete a job log file
int delete_job_log(const char *log_filename) {
    if (remove(log_filename) != 0) {
        perror("Failed to delete log file");
        return -1;
    }
    return 0;
}

// Function to initialize a job
void init_job(Job *job, pid_t pid, int status, const char *cmd) {
    job->pid = pid;
    job->status = status;
    strncpy(job->cmd, cmd, sizeof(job->cmd) - 1);
    job->cmd[sizeof(job->cmd) - 1] = '\0';  // Ensure null termination
}

// Main function for testing purposes
int main() {
    // Sample log file and job data
    const char *log_filename = "./out/joblog_0.txt";
    Job jobs[MAX_JOBS];

    // Initialize some jobs
    init_job(&jobs[0], 12345, 1, "Job1 command");
    init_job(&jobs[1], 12346, 2, "Job2 command");

    // Write jobs to log file
    if (write_job_log(log_filename, jobs, 2) == -1) {
        return 1;
    }

    // Read jobs from log file
    Job read_jobs[MAX_JOBS];
    if (read_job_log(log_filename, read_jobs, 2) == -1) {
        return 1;
    }

    // Print out the read jobs for verification
    for (int i = 0; i < 2; i++) {
        printf("Read job: PID = %d, Status = %d, Command = %s\n", read_jobs[i].pid, read_jobs[i].status, read_jobs[i].cmd);
    }

    // Delete the log file
    if (delete_job_log(log_filename) == -1) {
        return 1;
    }

    return 0;
}
