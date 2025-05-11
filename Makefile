# Makefile for CS 4348 Project 1

# Compiler
CC = gcc

# Targets
all: driver logger encrypter

driver: driver.c
	$(CC) -o driver driver.c

logger: logger.c
	$(CC) -o logger logger.c

encrypter: encrypter.c
	$(CC) -o encrypter encrypter.c

clean:
	rm -f driver logger encrypter


