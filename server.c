/**
 * Author: Maleakhi Agung Wijaya
 * Student ID: 784091
 * Date: 17/04/2018
 */

 /** Library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

/** Define */
#define ARGUMENT 3

int main(int argc, char *argv[]) {
    int socket_descriptor;

    // Check that argument to command line is sufficient
    if (argc != ARGUMENT) {
        fprintf(stderr, "./server [port number] [path to web root]");
        exit(1);
    }

    // Create TCP socket
    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    // Check for error, socket_descriptor will be -1 if there is ERROR
    if (socket_descriptor < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    return 0;
}
