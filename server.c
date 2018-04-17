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
#define WEB_ROOT_PATH_INDEX 2
#define MAX_RESERVED_PORT 1024
#define BACKLOG 10

int main(int argc, char *argv[]) {
    int socket_descriptor, port_number, new_socket_descriptor;
    char receive_buffer[256], send_buffer[256];
    char *web_root_path;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len;

    /* Initialisation */
    // Check that argument to command line is sufficient
    if (argc != ARGUMENT) {
        fprintf(stderr, "./server [port number] [path to web root]\n");
        exit(1);
    }

    // Extract arguments from command line and make sure everything is valid
    port_number = atoi(argv[PORT_INDEX]);
    web_root_path = argv[WEB_ROOT_PATH_INDEX];
    if (port_number < MAX_RESERVED_PORT) {
        fprintf(stderr, "Port number is reserved, please choose ports above 1024\n");
        exit(1);
    }

    // Initialise address, buffers
    memset(&server_address, '0', sizeof(server_address));
    memset(receive_buffer, '0', 256);
    memset(send_buffer, '0', 256);

    /* Create Socket */
    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    // Check for error, socket_descriptor will be -1 if there is ERROR
    if (socket_descriptor < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Bind Socket */
    // Create server address that will be used to listen incoming connection
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port_number);

    // Bind the socket with address (IP and port number)
    if (bind(socket_descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("ERROR on binding process");
        exit(1);
    }

    /* Listen on Socket */
    listen(socket_descriptor, BACKLOG);

    /* Accept Client Connection */
    client_address_len = sizeof(client_address);

    // New socket descriptor will be used to send and receive later
    new_socket_descriptor = accept(socket_descriptor, (struct sockaddr *) &client_address, &client_address_len);

    // Check for ERROR
    if (new_socket_descriptor < 0) {
        perror("ERROR on accept");
        exit(1);
    }

    printf("Success accepting %d\n", new_socket_descriptor);

    return 0;
}
