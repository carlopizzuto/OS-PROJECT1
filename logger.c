#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <log_file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *file_path = argv[1];
    FILE *logfile = fopen(file_path, "a");

    if (logfile == NULL) {
        fprintf(stderr, "Failed to open log file: %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_SIZE];
    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(line, "QUIT\n") == 0) {
            break;
        }

        // Remove the newline character if present
        line[strcspn(line, "\n")] = '\0';

        time_t now = time(NULL);
        struct tm *local_time = localtime(&now);
        char timestamp[20];

        char action[20], message[MAX_LINE_SIZE];
        
        // Parse the action and message
        if (sscanf(line, "%19s %[^\n]", action, message) != 2) {
            fprintf(stderr, "Invalid log format\n");
            continue;
        }
        
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M", local_time);

        fprintf(logfile, "%s [%s] %s\n", timestamp, action, message);
        fflush(logfile);

        // Print the line
        printf("%s %s\n", timestamp, line);
    }

    fclose(logfile);
    return 0;
}