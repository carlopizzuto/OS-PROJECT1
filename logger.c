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

    const char *log_file = argv[1];
    FILE *file = fopen(log_file, "a");

    if (file == NULL) {
        fprintf(stderr, "Failed to open log file: %s\n", log_file);
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_SIZE];
    while (fgets(line, sizeof(line), stdin)) {
        // Remove the newline character if present
        line[strcspn(line, "\n")] = '\0';

        time_t now = time(NULL);
        struct tm *local_time = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);

        fprintf(file, "%s %s\n", timestamp, line);
        fflush(file);

        // Print the line
        printf("%s %s\n", timestamp, line);
    }

    fclose(file);
    return 0;
}