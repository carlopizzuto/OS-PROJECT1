#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_SIZE 1024

char passkey[MAX_LINE_SIZE] = "";

void encrypt_message(const char *message, const char *key, char *encrypted_message) {
    size_t message_length = strlen(message);
    size_t key_length = strlen(key);

    printf("Message: %s, Key: %s\n", message, key);

    for (size_t i = 0; i < message_length; i++) {
        int key_index = i % key_length;
        int shift = (key[key_index] - 'A') % 26;
        char encrypted_char = ((message[i] + shift - 'A') % 26) + 'A';
        encrypted_message[i] = encrypted_char;
    }
    encrypted_message[message_length] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <key>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *key = argv[1];
    int key_length = strlen(key);

    char encrypted_message[MAX_LINE_SIZE];

    encrypt_message("ATTACKATDAWN", "LEMON", encrypted_message);
    printf("Encrypted: %s \n", encrypted_message);
}