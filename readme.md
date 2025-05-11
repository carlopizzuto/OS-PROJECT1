# CS 4348 - Project 1

### Carlo Pizzuto

### Files

- **logger.c:** Logs commands into a given .txt file

- **encryptor.c:** Encrypts / Decrypts a given message using a Vigenère cipher.

- **driver.c:** Driver program that allows for interaction with the logger and encryptor

### Usage

1. First, compile all of the programs using the commands:

```bash
gcc -o driver driver.c
gcc -o logger logger.c
gcc -o encrypter encrypter.c (note that it is encrypter, not encryptor)
```

2. Then, run the driver program using the command:

```bash
./driver <logfile>
```

### Example

```bash
./driver log.txt
```


