#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_HISTORY 100
#define MAX_STR_LEN 1024

void log_message(int logpipe_fd, const char *action, const char *message) {
    dprintf(logpipe_fd, "%s %s\n", action, message);
}

int print_history(char **history, int history_index, int logpipe_fd) {
    if (history_index == 0) {
        log_message(logpipe_fd, "ERROR", "no history entries");
        printf("ERROR: There are no history entries yet.\n");
        return -1;
    } else {
        log_message(logpipe_fd, "HIST", "logging history");
        printf("History:\n");

        for (int i = 0; i < history_index; i++) {
            printf("  [%d] %s\n", i + 1, history[i]);
        }
    }

    fflush(stdout);
    return 0;
}

int check_input(char *input, int is_password) {
    if (strlen(input) == 0) {
        return -1;
    }
    for (int i = 0; input[i] != '\0'; i++) {
        if (!isalpha(input[i])) {
            return -1;
        }
        if (is_password && input[i] == ' ') {
            return -1;
        }
    }
    return 0;
}

void to_uppercase(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (islower(str[i])) {
            str[i] = toupper(str[i]);
        }
    }
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

    // Log the start of the driver
    log_message(logpipe[1], "START  ", "driver started");

    // Initialize history
    char *history[MAX_HISTORY];
    int history_index = 0;

    // ----------------------------- MAIN LOOP -----------------------------
    while (1) {
        // Print the command menu & prompt
        printf("-------------------------------------------------------\n");
        printf("Commands: password | encrypt | decrypt | history | quit\n");
        printf("Enter command: ");
        fflush(stdout);

        // Read the command from stdin
        char cmd[64], password[64];
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            perror("Getline failed");
            exit(EXIT_FAILURE); 
        }
        cmd[strcspn(cmd, "\n")] = '\0';

        // =========== PASSWORD ===========
        if (strcmp(cmd, "password") == 0) {
            log_message(logpipe[1], "CMD", "password");

            printf("*******************************************************\n");
            printf("You have the option to:\n");
            printf("1. Set a new password\n");
            printf("2. Choose one from the history\n");
            printf("Enter your choice: ");
            fflush(stdout);

            int flag = 0;

            // Read the choice from stdin
            char choice[20];
            if (fgets(choice, sizeof(choice), stdin) == NULL) { 
                perror("Getline failed");
                exit(EXIT_FAILURE); 
            }
            choice[strcspn(choice, "\n")] = '\0';
            fflush(stdin);

            // ***** Input a new password *****
            if (strcmp(choice, "1") == 0) {
                log_message(logpipe[1], "PWD", "set from input");
                printf("Enter new password: ");
                fflush(stdout);

                // Read the password from stdin
                if (fgets(password, sizeof(password), stdin) == NULL) { 
                    perror("Getline failed");
                    exit(EXIT_FAILURE); 
                }
                password[strcspn(password, "\n")] = '\0';
                
                // Check if the password is valid
                if (check_input(password, 1) != 0) {
                    log_message(logpipe[1], "ERROR", "invalid password");
                    printf("ERROR: Invalid password\n");
                    fflush(stdout);
                    continue;
                } else {
                    flag = 1;
                }
            } 
            // ***** Choose a password from the history *****
            else if (strcmp(choice, "2") == 0) {
                log_message(logpipe[1], "PWD", "choose from history");
                
                if (print_history(history, history_index, logpipe[1]) == -1) {
                    // There are no history entries, so continue to the next iteration
                    continue;
                }
                
                printf("Enter the index of the password you want to use: ");
                fflush(stdout);

                // Read the index from stdin
                char index[20];
                if (fgets(index, sizeof(index), stdin) == NULL) {
                    perror("Getline failed");
                    exit(EXIT_FAILURE);
                }
                index[strcspn(index, "\n")] = '\0';

                // Check if the index is valid
                int index_num = atoi(index);
                if (index_num < 1 || index_num > history_index) {
                    log_message(logpipe[1], "ERROR", "invalid history index");
                    printf("ERROR: Invalid history index\n");
                    fflush(stdout);
                    continue;
                } else {
                    // Copy the password from the history
                    strcpy(password, history[index_num-1]);

                    // Check if the password is valid
                    if (check_input(password, 1) != 0) {
                        log_message(logpipe[1], "ERROR", "invalid password");
                        printf("ERROR: Invalid password\n");
                        fflush(stdout);
                        continue;
                    } else {
                        flag = 1;
                    }
                }
            }
            // ***** Invalid choice *****
            else {
                printf("ERROR: Invalid choice\n");
                log_message(logpipe[1], "ERROR", "invalid password choice");
                fflush(stdout);
                continue;
            }
            if (flag == 1) {
                // Set the password in the encrypter (PASSKEY)
                to_uppercase(password);
                dprintf(encpipe_in[1], "PASSKEY %s\n", password);

                 // Read the response from the encrypter
                char response[MAX_STR_LEN];
                ssize_t n = read(encpipe_out[0], response, sizeof(response)-1);
                if (n > 0) {
                    response[n] = '\0';
                    printf("[encryptor] %s", response);
                    fflush(stdout);

                    // Parse the response
                    char command[20], result[MAX_STR_LEN];
                    if (sscanf(response, "%19s %[^\n]", command, result) != 2) {
                        log_message(logpipe[1], "ERROR", "invalid encryptor response");
                        printf("ERROR in encryptor response: %s", response);
                        continue;
                    }

                    // If the response is not an error
                    if (strcmp(command, "RESULT") == 0) {
                        // add the password to the history
                        history[history_index] = strdup(result);
                        history_index++;
                    } else if (strcmp(command, "ERROR") == 0) {
                        log_message(logpipe[1], "ERROR", result);
                        printf("ERROR: %s\n", result);
                    }
                }
            }
        } 
        // ============= ENCRYPT ==============
        else if (strcmp(cmd, "encrypt") == 0) {
            log_message(logpipe[1], "CMD", "encrypt");

            printf("You have the option to:\n");
            printf("1. Input a new message to encrypt\n");
            printf("2. Choose one from the history\n");
            printf("Enter your choice: ");
            fflush(stdout);
            
            // Read the choice from stdin
            char choice[20], message[MAX_STR_LEN];
            if (fgets(choice, sizeof(choice), stdin) == NULL) { 
                perror("Getline failed");
                exit(EXIT_FAILURE); 
            }
            choice[strcspn(choice, "\n")] = '\0';

            int flag = 0;

            // ***** Input a new message *****
            if (strcmp(choice, "1") == 0) {
                log_message(logpipe[1], "PWD", "set from input");
                printf("Enter new message: ");
                fflush(stdout);

                // Read the message from stdin
                if (fgets(message, sizeof(message), stdin) == NULL) { 
                    perror("Getline failed");
                    exit(EXIT_FAILURE); 
                }
                message[strcspn(message, "\n")] = '\0';
                
                // Check if the message is valid
                if (check_input(message, 0) != 0) {
                    log_message(logpipe[1], "ERROR", "invalid message");
                    printf("ERROR: Invalid message\n");
                    fflush(stdout);
                    continue;
                } else {
                    flag = 1;
                }
            } 
            // ***** Choose a message from the history *****
            else if (strcmp(choice, "2") == 0) {
                log_message(logpipe[1], "PWD", "choose from history");
                
                if (print_history(history, history_index, logpipe[1]) == -1) {
                    // There are no history entries, so continue to the next iteration
                    continue;
                }
                
                printf("Enter the index of the message you want to use: ");
                fflush(stdout);

                // Read the index from stdin
                char index[20];
                if (fgets(index, sizeof(index), stdin) == NULL) {
                    perror("Getline failed");
                    exit(EXIT_FAILURE);
                }
                index[strcspn(index, "\n")] = '\0';

                // Check if the index is valid
                int index_num = atoi(index);
                if (index_num < 1 || index_num > history_index) {
                    log_message(logpipe[1], "ERROR", "invalid history index");
                    printf("ERROR: Invalid history index\n");
                    fflush(stdout);
                    continue;
                } else {
                    // Copy the message from the history
                    strcpy(message, history[index_num-1]);

                    // Check if the message is valid
                    if (check_input(message, 0) != 0) {
                        log_message(logpipe[1], "ERROR", "invalid message");
                        printf("ERROR: Invalid message\n");
                        fflush(stdout);
                        continue;
                    } else {
                        flag = 1;
                    }
                }
            }
            // ***** Invalid choice *****
            else {
                printf("ERROR: Invalid choice\n");
                log_message(logpipe[1], "ERROR", "invalid password choice");
                fflush(stdout);
                continue;
            }
            if (flag == 1) {
                // Set the password in the encrypter (PASSKEY)
                to_uppercase(password);
                dprintf(encpipe_in[1], "PASSKEY %s\n", password);

                 // Read the response from the encrypter
                char response[MAX_STR_LEN];
                ssize_t n = read(encpipe_out[0], response, sizeof(response)-1);
                if (n > 0) {
                    response[n] = '\0';
                    printf("[encryptor] %s", response);
                    fflush(stdout);

                    // Parse the response
                    char command[20], result[MAX_STR_LEN];
                    if (sscanf(response, "%19s %[^\n]", command, result) != 2) {
                        log_message(logpipe[1], "ERROR", "invalid encryptor response");
                        printf("ERROR in encryptor response: %s", response);
                        continue;
                    }

                    // If the response is not an error
                    if (strcmp(command, "RESULT") == 0) {
                        // add the password to the history
                        history[history_index] = strdup(result);
                        history_index++;
                    } else if (strcmp(command, "ERROR") == 0) {
                        log_message(logpipe[1], "ERROR", result);
                        printf("ERROR: %s\n", result);
                    }
                }
            }
        }
        // ============= DECRYPT ==============
        else if (strcmp(cmd, "decrypt") == 0) {
            printf("Enter message: ");
            fflush(stdout);
            
            
        }
        // ============= HISTORY ==============
        else if (strcmp(cmd, "history") == 0) {
            log_message(logpipe[1], "CMD", "history");
            if (print_history(history, history_index, logpipe[1]) == 0) {
                fflush(stdout);
                continue;
            }
        }
        // ============= QUIT ==============
        else if (strcmp(cmd, "quit") == 0) {
            dprintf(logpipe[1], "QUIT\n");
            dprintf(encpipe_in[1], "QUIT\n");

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
        // ============= ERROR =============
        else {
            char error_msg[64];
            sprintf(error_msg, "Invalid command: %s", cmd);

            printf("ERROR: %s\n", error_msg);
            log_message(logpipe[1], "ERROR", error_msg);
        }
    }   
    return 0;
}