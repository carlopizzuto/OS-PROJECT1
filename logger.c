#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) { // Input validation
        fprintf(stderr, "Usage: %s <log_file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open the log file
    FILE *logfile = fopen(argv[1], "a");
    if (!logfile) { // File open error handling
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    // Read input from stdin
    char line[MAX_LINE_SIZE];
    while (fgets(line, sizeof(line), stdin)) {
        if (strncmp(line, "QUIT", 4) == 0) { // Exit condition
            break;
        }


        // Parse the action and message
        char action[20], message[MAX_LINE_SIZE];
        if (sscanf(line, "%19s %[^\n]", action, message) != 2) {
            fprintf(stderr, "Invalid log format: %s\n", line);
            continue;
        }
    
        // Get the current time
        time_t now = time(NULL);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M", localtime(&now));

        // Write to the log file
        fprintf(logfile, "%s [%s] %s\n", timestamp, action, message);
        fflush(logfile);
    }

    fclose(logfile);
    return 0;
}