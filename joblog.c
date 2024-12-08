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
#include "joblog.h"

// Define the maximum number of log entries
#define MAX_LOG_ENTRIES 1024

// Function to initialize the joblog file
int joblog_init(const char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error opening joblog file");
        return -1;
    }
    return fd;
}

// Function to write a joblog entry
int joblog_write(int fd, joblog_entry *entry) {
    if (fd == -1 || entry == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Prepare the log entry as a formatted string
    char log_buffer[256];
    int written = snprintf(log_buffer, sizeof(log_buffer),
                           "pid:%07d,id:%05d,pri:%05d,label:%s\n",
                           entry->pid, entry->id, entry->pri, entry->label);

    if (written < 0) {
        return -1; // Formatting error
    }

    // Write the log entry to the file
    ssize_t bytes_written = write(fd, log_buffer, written);
    if (bytes_written != written) {
        return -1; // Write error
    }

    return 0; // Success
}

// Function to read joblog entries
int joblog_read(int fd, joblog_entry *entries, int max_entries) {
    if (fd == -1 || entries == NULL || max_entries <= 0) {
        errno = EINVAL;
        return -1;
    }

    // Read the file into a buffer
    char buffer[256];
    int entry_count = 0;
    while (entry_count < max_entries) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            break; // EOF or error
        }

        buffer[bytes_read] = '\0'; // Null-terminate the string

        // Parse the log entry
        joblog_entry *entry = &entries[entry_count];
        if (sscanf(buffer, "pid:%d,id:%d,pri:%d,label:%255s",
                   &entry->pid, &entry->id, &entry->pri, entry->label) == 4) {
            entry_count++;
        }
    }

    return entry_count; // Return the number of entries read
}

// Function to delete the joblog file
int joblog_delete(const char *filename) {
    if (unlink(filename) == -1) {
        perror("Error deleting joblog file");
        return -1;
    }
    return 0; // Success
}
