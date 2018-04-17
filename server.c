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

/** Constant */
#define ARGUMENT 3
#define PORT_INDEX 1
#define MAX_RESERVED_PORT 1024

int main(int argc, char *argv[]) {
    int socket_descriptor, port_number;
    char receive_buffer[1000], send_buffer[2000];
    struct sockaddr_in server_address, client_address;
    socklen_t client;

    /* Initialisation */
    // Check that argument to command line is sufficient
    if (argc != ARGUMENT) {
        fprintf(stderr, "./server [port number] [path to web root]\n");
        exit(1);
    }

    // Extract arguments from command line and make sure everything is valid
    port_number = atoi(argv[PORT_INDEX]);
    if (port_number < MAX_RESERVED_PORT) {
        fprintf(stderr, "Port number is reserved, please choose ports above
            1024\n");
        exit(1);
    }

    /* Create Socket */
    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    // Check for error, socket_descriptor will be -1 if there is ERROR
    if (socket_descriptor < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Bind Socket */
     // Create server address that will be used to listen incoming connection
     memset(&server_address, '0', sizeof(server_address));
     server_address.sin_family = AF_INET;
     server_address.sin_addr.s_addr = htonl(INADDR_ANY);
     server_address.sin_port = htons(port_number);

     // Bind the socket with address (IP and port number)
     if (bind(socket_descriptor, (struct sockaddr *) &server_address,
        sizeof(server_address)) < 0) {
            perror("ERROR on binding process");
            exit(1);
     }

    return 0;
}
