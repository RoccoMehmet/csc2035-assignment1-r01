#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define JOB_STR_SIZE 128
#define MAX_NAME_SIZE 64

typedef struct {
    pid_t pid;
    int id;
    int priority;
    char label[MAX_NAME_SIZE];
} job_t;

typedef struct {
    pid_t pid;
    int is_init;
} proc_t;

// Helper function to get the log file name for a process
char* get_log_filename(proc_t* proc) {
    static char log_fname[128];
    snprintf(log_fname, sizeof(log_fname), "./out/joblog_%d.txt", proc->pid);
    return log_fname;
}

// Job log write function
void joblog_write(proc_t* proc, job_t* job) {
    if (proc == NULL || job == NULL) {
        return;  // Don't attempt to write if arguments are NULL
    }

    // Open the log file for appending
    FILE *log_file = fopen(get_log_filename(proc), "a");
    if (log_file == NULL) {
        perror("Error opening log file for writing");
        return;  // If file can't be opened, do nothing
    }

    // Write the job entry to the log file
    fprintf(log_file, "pid:%07d,id:%05d,pri:%05d,label:%-*s\n",
            proc->pid, job->id, job->priority, MAX_NAME_SIZE - 1, job->label);
    fclose(log_file);
}

// Job log read function
job_t* joblog_read(proc_t* proc, int entry_num, job_t* job) {
    if (proc == NULL || entry_num < 0) {
        return NULL;  // Invalid arguments, return NULL
    }

    // Open the log file
    FILE *log_file = fopen(get_log_filename(proc), "r");
    if (log_file == NULL) {
        perror("Error opening log file for reading");
        return NULL;  // If file can't be opened, return NULL
    }

    // Seek to the correct entry (entry_num)
    int line_num = 0;
    char line[JOB_STR_SIZE];
    while (fgets(line, sizeof(line), log_file)) {
        if (line_num == entry_num) {
            // Parse the job entry (format: pid:<pid>,id:<id>,pri:<priority>,label:<label>)
            sscanf(line, "pid:%d,id:%d,pri:%d,label:%s", 
                   &job->pid, &job->id, &job->priority, job->label);

            fclose(log_file);
            return job;  // Successfully read the job entry
        }
        line_num++;
    }

    fclose(log_file);
    return NULL;  // No entry found
}

// Job log initialization
int joblog_init(proc_t* proc) {
    if (proc == NULL) {
        return -1;  // Invalid argument
    }

    // Create the directory if the process is the init process
    if (proc->is_init) {
        if (mkdir("./out", 0777) == -1 && errno != EEXIST) {
            perror("Error creating log directory");
            return -1;
        }
    }

    // Delete existing log (if any)
    char *log_filename = get_log_filename(proc);
    if (unlink(log_filename) == -1 && errno != ENOENT) {
        perror("Error deleting log file");
        return -1;
    }

    return 0;  // Successful initialization
}

// Job log deletion
void joblog_delete(proc_t* proc) {
    if (proc == NULL) {
        return;  // No operation if proc is NULL
    }

    // Construct the log filename
    char *log_filename = get_log_filename(proc);
    
    // Delete the log file
    if (unlink(log_filename) == 0) {
        printf("Log file %s deleted successfully.\n", log_filename);
    } else {
        perror("Error deleting log file");
    }
}

// Job log read bounds (check if entry_num is out of bounds)
job_t* joblog_read_bounds(proc_t* proc, int entry_num, job_t* job) {
    if (proc == NULL || entry_num < 0) {
        return NULL;  // Invalid arguments, return NULL
    }

    // Open the log file
    FILE *log_file = fopen(get_log_filename(proc), "r");
    if (log_file == NULL) {
        perror("Error opening log file");
        return NULL;
    }

    // Seek to the correct entry (entry_num)
    int line_num = 0;
    char line[JOB_STR_SIZE];
    while (fgets(line, sizeof(line), log_file)) {
        if (line_num == entry_num) {
            // Parse the job entry
            sscanf(line, "pid:%d,id:%d,pri:%d,label:%s", 
                   &job->pid, &job->id, &job->priority, job->label);

            fclose(log_file);
            return job;
        }
        line_num++;
    }

    fclose(log_file);
    return NULL;  // No entry found or out-of-bounds
}

// Job log initialization (with directory creation)
int joblog_init_with_directory() {
    if (mkdir("./out", 0777) == -1 && errno != EEXIST) {
        perror("Error creating log directory");
        return -1;
    }
    return 0;  // Success
}

// Main program to test joblog functionality
int main() {
    proc_t proc = { .pid = getpid(), .is_init = 1 };  // Set process as init
    joblog_init(&proc);  // Initialize job log system

    // Create and write a job entry
    job_t job1 = { .id = 1, .priority = 5, .label = "Job_1" };
    joblog_write(&proc, &job1);

    // Read back the job entry
    job_t read_job;
    joblog_read(&proc, 0, &read_job);
    printf("Read Job - PID: %d, ID: %d, Priority: %d, Label: %s\n", 
            read_job.pid, read_job.id, read_job.priority, read_job.label);

    // Delete the job log
    joblog_delete(&proc);

    return 0;
}
