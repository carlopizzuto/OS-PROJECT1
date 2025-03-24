#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_SIZE 1024

char passkey[MAX_LINE_SIZE] = "";

void encrypt_message(const char *message, const char *key, char *encrypted_message) {
    size_t message_length = strlen(message);
    size_t key_length = strlen(key);
    int space_count = 0;

    for (size_t i = 0; i < message_length; i++) {
        if (message[i] == ' ') {
            encrypted_message[i] = ' ';
            space_count++;
        } else {
            int key_index = (i - space_count) % key_length;
            int shift = (key[key_index] - 'A') % 26;
            int char_index = message[i] - 'A';
            int new_char_index = (char_index + shift) % 26;
            char encrypted_char = new_char_index + 'A';

            encrypted_message[i] = encrypted_char;
        }
    }
    encrypted_message[message_length] = '\0';
    fprintf(stdout, "RESULT %s\n", encrypted_message);
    fflush(stdout);
}

void decrypt_message(const char *message, const char *key, char *decrypted_message) {
    size_t message_length = strlen(message);
    size_t key_length = strlen(key);
    int space_count = 0;

    for (size_t i = 0; i < message_length; i++) {
        if (message[i] == ' ') {
            decrypted_message[i] = ' ';
            space_count++;
        } else {
            int key_index = (i - space_count) % key_length;
            int shift = (key[key_index] - 'A') % 26;
            int char_index = message[i] - 'A';
            int new_char_index = (char_index - shift + 26) % 26;
            char decrypted_char = new_char_index + 'A';

            decrypted_message[i] = decrypted_char;
        }
    }
    decrypted_message[message_length] = '\0';
    fprintf(stdout, "RESULT %s\n", decrypted_message);
    fflush(stdout);
}

int check_key() {
    if (strcmp(passkey, "") == 0) {
        fprintf(stdout, "ERROR Passkey not set\n");
        fflush(stdout);
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    char encrypted_message[MAX_LINE_SIZE];
    char decrypted_message[MAX_LINE_SIZE];

    char line[MAX_LINE_SIZE];
    while (fgets(line, sizeof(line), stdin)) {
        if (strncmp(line, "QUIT", 4) == 0) { // Exit condition
            break;
        }

        // Parse the command and argument
        char command[20], argument[MAX_LINE_SIZE];
        if (sscanf(line, "%19s %[^\n]", command, argument) != 2) {
            fprintf(stderr, "ERROR Invalid log format: %s", line);
            continue;
        }

        if (strcmp(command, "ENCRYPT") == 0) {
            if (check_key()) {
                encrypt_message(argument, passkey, encrypted_message);
            }
        } else if (strcmp(command, "DECRYPT") == 0) {
            if (check_key()) {
                decrypt_message(argument, passkey, decrypted_message);
            }
        } else if (strcmp(command, "PASSKEY") == 0) {
            strcpy(passkey, argument);
            fprintf(stdout, "RESULT Passkey set\n");
            fflush(stdout);
        } else {
            fprintf(stderr, "ERROR Invalid command: %s\n", command);
            continue;
        }
    }

    return 0;
}