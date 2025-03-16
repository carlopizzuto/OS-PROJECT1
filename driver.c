#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>

#define MAX_HISTORY 100
#define MAX_STR_LEN 1024

void log_message(int logpipe_fd, const char *action, const char *message) {
    dprintf(logpipe_fd, "%s %s\n", action, message);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./driver <log_file_path>\n");
        exit(EXIT_FAILURE);
    }
    const char *log_file_path = argv[1];

    // -------------------- PIPES --------------------
    // Logger pipe
    int logpipe[2]; 
    if (pipe(logpipe) == -1) {
        perror("Logger pipe - logpipe");
        exit(EXIT_FAILURE);
    }
    // Encrypter pipes
    int encpipe_in[2], encpipe_out[2]; 
    if (pipe(encpipe_in) == -1) {
        perror("Encrypter input pipe - encpipe_in");
        exit(EXIT_FAILURE);
    }
    if (pipe(encpipe_out) == -1) {
        perror("Encrypter output pipe - encpipe_out");
        exit(EXIT_FAILURE);
    }

    // -------------------- FORKS --------------------
    // ======= LOGGER =======
    pid_t logger_pid = fork();
    if (logger_pid < 0) {
        // Fork error
        perror("Logger fork - logger_pid");
        exit(EXIT_FAILURE);
    } else if (logger_pid == 0) {
        // CHILD - logger process
        // Redirect stdin to the logpipe
        dup2(logpipe[0], STDIN_FILENO); 

        // Close unused pipes
        close(logpipe[1]); 
        close(encpipe_in[0]); close(encpipe_in[1]); 
        close(encpipe_out[0]); close(encpipe_out[1]);

        execl("./logger", "logger", log_file_path, (char*)NULL);
        perror("Execl logger failed");
        exit(EXIT_FAILURE);
    }
    // PARENT - close logger pipe
    close(logpipe[0]);

    // ======= ENCRYPTER =======
    pid_t encrypter_pid = fork();
    if (encrypter_pid < 0) {
        // Fork error
        perror("Encrypter fork - encrypter_pid");
        exit(EXIT_FAILURE);
    } else if (encrypter_pid == 0) {
        // CHILD - encrypter process
        // Redirect encrypter input pipe
        dup2(encpipe_in[0], STDIN_FILENO);
        // Redirect encrypter output pipe
        dup2(encpipe_out[1], STDOUT_FILENO);

        // Close unused pipes
        close(logpipe[0]); close(logpipe[1]);
        close(encpipe_in[1]); 
        close(encpipe_out[0]);
        
        execl("./encrypter", "encrypter", (char*)NULL);
        perror("Execl encrypter failed");
        exit(EXIT_FAILURE);
    }
    // PARENT - close encrypter pipes
    close(encpipe_in[0]);
    close(encpipe_out[1]);

    log_message(logpipe[1], "START  ", "Driver started");

    char *history[MAX_HISTORY];
    int history_index = 0;

    while (1) {
        printf("\n-------------------------------------------------------\n");
        printf("Commands: password | encrypt | decrypt | history | quit\n");
        printf("Enter command: ");
        fflush(stdout);

        char cmd[64];

        // Read input from stdin
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            perror("Getline failed");
            exit(EXIT_FAILURE); 
        }
        cmd[strcspn(cmd, "\n")] = '\0';

        if (strcmp(cmd, "password") == 0) {
            printf("Enter password: ");
            fflush(stdout);

            char password[64];
            if (fgets(password, sizeof(password), stdin) == NULL) { 
                perror("Getline failed");
                exit(EXIT_FAILURE); 
            }
            password[strcspn(password, "\n")] = '\0';

            // Log the password
            log_message(logpipe[1], "PASSWORD", password);
        } else if (strcmp(cmd, "quit") == 0) {
            dprintf(logpipe[1], "QUIT\n");
            dprintf(encpipe_out[1], "QUIT\n");

            // Close the logpipe
            close(logpipe[1]);

            // Close the pipes
            close(logpipe[1]);
            close(encpipe_in[1]);
            close(encpipe_out[0]);

            // Wait for children to finish
            waitpid(logger_pid, NULL, 0);
            waitpid(encrypter_pid, NULL, 0);

            // Free history
            for (int i = 0; i < history_index; i++) {
                free(history[i]);
            }

            printf("GOODBYE\n");
            break;
        }
        else {
            char error_msg[64];
            sprintf(error_msg, "Invalid command: %s", cmd);

            printf("ERROR: %s\n", error_msg);
            log_message(logpipe[1], "ERROR", error_msg);
        }
    }   
    return 0;
}