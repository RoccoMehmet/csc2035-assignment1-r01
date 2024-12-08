/*
 * Replace the following string of 0s with your student number
 * 000000000
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LOG_ENTRIES 11
#define BUFFER_SIZE 1024

// Structure for job log entries
typedef struct {
    int pid;
    int id;
    int pri;
    char label[128];
} joblog_entry;

// Function to write to the job log
int joblog_write(int fd, joblog_entry *entry) {
    // Check if the entry is valid
    if (entry == NULL) {
        errno = EINVAL;  // Invalid argument error
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int bytes_written = snprintf(buffer, sizeof(buffer), 
        "pid:%07d,id:%05d,pri:%05d,label:%s\n", 
        entry->pid, entry->id, entry->pri, entry->label);
    
    // Write the formatted entry to the log file
    ssize_t written = write(fd, buffer, bytes_written);
    if (written == -1) {
        return -1;  // Writing failed, set errno automatically
    }

    return 0;  // Success
}

// Function to read from the job log
int joblog_read(int fd, joblog_entry *entries, int max_entries) {
    if (fd < 0 || entries == NULL || max_entries <= 0) {
        errno = EINVAL;  // Invalid argument error
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int entry_count = 0;
    
    while (entry_count < max_entries) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            return -1;  // Error occurred, set errno automatically
        }
        
        if (bytes_read == 0) {
            break;  // End of file reached
        }
        
        // Parse the buffer and extract job log entries
        if (sscanf(buffer, "pid:%d,id:%d,pri:%d,label:%127s", 
                   &entries[entry_count].pid, &entries[entry_count].id, 
                   &entries[entry_count].pri, entries[entry_count].label) == 4) {
            entry_count++;
        }

        if (bytes_read < sizeof(buffer)) {
            break;  // Stop if fewer bytes were read than expected
        }
    }

    return entry_count;
}

// Function to delete the job log file
int joblog_delete(const char *filename) {
    if (unlink(filename) == -1) {
        return -1;  // Deletion failed, set errno automatically
    }

    return 0;  // Success
}

// Function to initialize the joblog (open or create the log file)
int joblog_init(const char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        return -1;  // File opening failed, set errno automatically
    }
    return fd;
}

// Helper function to reset errno to the initial state
void reset_errno() {
    errno = 0;  // Reset errno
}

// Example of a test function for writing to the log
void test_joblog_write_cpid0() {
    int fd = joblog_init("joblog.txt");
    if (fd == -1) {
        perror("Failed to initialize joblog");
        return;
    }

    joblog_entry entry = {0, 1, 1, "*******************************"};
    if (joblog_write(fd, &entry) == -1) {
        perror("Failed to write joblog");
        close(fd);
        return;
    }

    close(fd);
}

// Example of a test function for reading from the log
void test_joblog_read_cpid0() {
    int fd = joblog_init("joblog.txt");
    if (fd == -1) {
        perror("Failed to initialize joblog");
        return;
    }

    joblog_entry entries[MAX_LOG_ENTRIES];
    int entry_count = joblog_read(fd, entries, MAX_LOG_ENTRIES);
    if (entry_count != MAX_LOG_ENTRIES) {
        fprintf(stderr, "Expected %d entries, got %d\n", MAX_LOG_ENTRIES, entry_count);
    }

    close(fd);
}

// Main function for testing joblog operations
int main() {
    test_joblog_write_cpid0();
    test_joblog_read_cpid0();
    
    return 0;
}
