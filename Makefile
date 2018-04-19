# Author: Maleakhi Agung Wijaya (maleakhiw)
# Student ID: 784091
# Makefile for COMP30023 Assignment 1

# Constant
CC = gcc
CFLAGS = -lpthread

# Default if user type 'make'
default: server

server: server.c
	$(CC) -Wall -o server server.c $(CFLAGS)

# To clean the executable and object file
clean:
	-rm -f server
