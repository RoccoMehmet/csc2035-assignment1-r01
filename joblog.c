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

    // Handle special case for pid == 0
    if (proc->pid == 0) {
        snprintf(log_fname, sizeof(log_fname), "./out/joblog_init.txt");
    } else {
        snprintf(log_fname, sizeof(log_fname), "./out/joblog_%d.txt", proc->pid);
    }

    return log_fname;
}

// Job log write function
void joblog_write(proc_t* proc, job_t* job) {
    if (proc == NULL || job == NULL) {
        return;
    }

    // Ensure the output directory exists before writing
    if (mkdir("./out", 0777) == -1 && errno != EEXIST) {
        perror("Error creating log directory");
        return;
    }

    FILE *log_file = fopen(get_log_filename(proc), "a");
    if (log_file == NULL) {
        perror("Error opening log file for writing");
        return;
    }

    fprintf(log_file, "pid:%07d,id:%05d,pri:%05d,label:%-*s\n",
            proc->pid, job->id, job->priority, MAX_NAME_SIZE - 1, job->label);
    fclose(log_file);
}

// Job log read function
job_t* joblog_read(proc_t* proc, int entry_num, job_t* job) {
    if (proc == NULL || entry_num < 0 || job == NULL) {
        return NULL;
    }

    FILE *log_file = fopen(get_log_filename(proc), "r");
    if (log_file == NULL) {
        perror("Error opening log file for reading");
        return NULL;
    }

    int line_num = 0;
    char line[JOB_STR_SIZE];
    while (fgets(line, sizeof(line), log_file)) {
        if (line_num == entry_num) {
            sscanf(line, "pid:%d,id:%d,pri:%d,label:%s", 
                   &job->pid, &job->id, &job->priority, job->label);
            fclose(log_file);
            return job;
        }
        line_num++;
    }

    fclose(log_file);
    return NULL;
}

// Job log initialization
int joblog_init(proc_t* proc) {
    if (proc == NULL) {
        return -1;
    }

    // Create the log directory if it does not exist
    if (mkdir("./out", 0777) == -1 && errno != EEXIST) {
        perror("Error creating log directory");
        return -1;
    }

    char *log_filename = get_log_filename(proc);
    // Delete any existing log file
    if (unlink(log_filename) == -1 && errno != ENOENT) {
        perror("Error deleting log file");
        return -1;
    }

    return 0;
}

// Job log deletion
void joblog_delete(proc_t* proc) {
    if (proc == NULL) {
        return;
    }

    char *log_filename = get_log_filename(proc);
    if (unlink(log_filename) == 0) {
        printf("Log file %s deleted successfully.\n", log_filename);
    } else {
        perror("Error deleting log file");
    }
}
