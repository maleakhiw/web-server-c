/*
 * Author: Maleakhi Agung Wijaya
 * Student ID: 784091
 * Date: 17/04/2018
 */

 /* Library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

/** TODO: Bug List */
// - postman still persistent connection (being prioritised)

/*****************************************************************************/

/* Constant */
#define ARGUMENT 3
#define PORT_INDEX 1
#define WEB_ROOT_PATH_INDEX 2

#define BACKLOG 10
#define BUFFER_LENGTH 8000

const char STATUS_OK[] = "HTTP/1.0 200 OK\r\n";
const char STATUS_NOT_FOUND[] = "HTTP/1.0 404 Not Found\r\n";
const char STATUS_BAD_REQUEST[] = "HTTP/1.0 400 Bad Request\r\n";

const char MIME_HTML[] = "Content-Type: text/html\r\n";
const char MIME_JPG[] = "Content-Type: image/jpeg\r\n";
const char MIME_CSS[] = "Content-Type: text/css\r\n";
const char MIME_JS[] = "Content-Type: application/javascript\r\n";

const char CRLF[] = "\r\n";

/* Declaration */
char *get_relative_path(char *receive_buffer, int socket_descriptor, int new_socket_descriptor, int *is_free);
int setup_server(struct sockaddr_in *server_address, int port_number);
char *get_content_type(char *relative_path);
int sendall(int s, char *buf, int *len);

/*****************************************************************************/

/* Main Function */
int main(int argc, char *argv[]) {
    int socket_descriptor, port_number, new_socket_descriptor;
    char *receive_buffer, send_buffer[BUFFER_LENGTH];
    char *web_root_path, *relative_path, *full_path, *content_type;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len;

    int n, file_found, len;
    int is_free = 0;
    FILE *file_descriptor;
    size_t bytes_read = 0;

    /* Initialisation */
    // Check that argument to command line is sufficient
    if (argc != ARGUMENT) {
        fprintf(stderr, "Please provide the correct command: ./server [port number] [path to web root]\n");
        exit(1);
    }

    // Extract arguments from command line and make sure everything is valid
    port_number = atoi(argv[PORT_INDEX]);
    web_root_path = argv[WEB_ROOT_PATH_INDEX];

    // Initialise address, buffers
    memset(&server_address, 0, sizeof(server_address));
    memset(send_buffer, 0, BUFFER_LENGTH);

    /* Setup server */
    socket_descriptor = setup_server(&server_address, port_number);

    /* Accept Client Connection */
    client_address_len = sizeof(client_address);

    // Continuously accept for every clients
    while(1) {
        // New socket descriptor will be used to send and receive later
        new_socket_descriptor = accept(socket_descriptor, (struct sockaddr *) &client_address, &client_address_len);
        // Check for ERROR
        if (new_socket_descriptor < 0) {
            perror("ERROR on accept");
            close(socket_descriptor);
            exit(1);
        }

        /* TODO: After accept, create new socket and handle the process */

        /* Receive client request and process it */
        // Initialise receive buffer to BUFFER_LENGTH, but possible to realloc
        receive_buffer = (char *) malloc(sizeof(char) * BUFFER_LENGTH);
        assert(receive_buffer != NULL);

        // Read client's request
        n = recv(new_socket_descriptor, receive_buffer, BUFFER_LENGTH-1, 0);
        receive_buffer[n] = '\0'; // Add null byte at the end
        printf("%s\n", receive_buffer);
        if (n < 0) {
            perror("ERROR receiving from socket");
            close(new_socket_descriptor);
            close(socket_descriptor);
            exit(1);
        }

        // Get the URI of the client request
        relative_path = get_relative_path(receive_buffer, socket_descriptor, new_socket_descriptor, &is_free);
        // If client request is invalid, close connection and do not continue process it
        if (relative_path == NULL) {
            free(receive_buffer);
            receive_buffer = NULL;
            continue;
        }

        // Combine to get full path
        full_path = (char *) malloc((strlen(relative_path) + strlen(web_root_path) + 1) * sizeof(char));
        assert(full_path != NULL);
        strcpy(full_path, web_root_path);
        strcat(full_path, relative_path);

        /* Send response to client */
        // Try to open the file requested by client
        file_descriptor = fopen(full_path, "r");
        // File not found
        if (file_descriptor == NULL) {
            file_found = 0;
        }
        // File exist
        else {
            file_found = 1;
        }

        // Get the content type
        content_type = get_content_type(relative_path);

        // Send header accordingly, 200 for found file 404 otherwise
        if (file_found) {
            len = sizeof(STATUS_OK) - 1;
            strcpy(send_buffer, STATUS_OK);
            n = sendall(new_socket_descriptor, send_buffer, &len); // trim the nullbyte
            if (n < 0) {
                perror("ERROR sending from socket");
                close(new_socket_descriptor);
                close(socket_descriptor);
                exit(1);
            }

            // Determine which content type to send
            if (strcmp(content_type, "html") == 0) {
                len = sizeof(MIME_HTML) - 1;
                strcpy(send_buffer, MIME_HTML);
                n = sendall(new_socket_descriptor, send_buffer, &len); // trim the nullbyte
                if (n < 0) {
                    perror("ERROR sending from socket");
                    close(new_socket_descriptor);
                    close(socket_descriptor);
                    exit(1);
                }
            }
            else if (strcmp(content_type, "css") == 0) {
                len = sizeof(MIME_CSS) - 1;
                strcpy(send_buffer, MIME_CSS);
                n = sendall(new_socket_descriptor, send_buffer, &len); // trim the nullbyte
                if (n < 0) {
                    perror("ERROR sending from socket");
                    close(new_socket_descriptor);
                    close(socket_descriptor);
                    exit(1);
                }
            }
            else if (strcmp(content_type, "js") == 0) {
                len = sizeof(MIME_JS) - 1;
                strcpy(send_buffer, MIME_JS);
                n = sendall(new_socket_descriptor, send_buffer, &len); // trim the nullbyte
                if (n < 0) {
                    perror("ERROR sending from socket");
                    close(new_socket_descriptor);
                    close(socket_descriptor);
                    exit(1);
                }
            }
            else {
                len = sizeof(MIME_JPG) - 1;
                strcpy(send_buffer, MIME_JPG);
                n = sendall(new_socket_descriptor, send_buffer, &len);   // trim the nullbyte
                if (n < 0) {
                    perror("ERROR sending from socket");
                    close(new_socket_descriptor);
                    close(socket_descriptor);
                    exit(1);
                }
            }

            len = sizeof(CRLF) - 1;
            strcpy(send_buffer, CRLF);
            n = sendall(new_socket_descriptor, send_buffer, &len); // trim the nullbyte, end of header
            if (n < 0) {
                perror("ERROR sending from socket");
                close(new_socket_descriptor);
                close(socket_descriptor);
                exit(1);
            }

            // read file in chunks
            while ((bytes_read = fread(send_buffer, sizeof(char), sizeof(send_buffer), file_descriptor))) {
                len = bytes_read;
                n = sendall(new_socket_descriptor, send_buffer, &len);
                if (n < 0) {
                    perror("ERROR sending from socket");
                    close(new_socket_descriptor);
                    close(socket_descriptor);
                    exit(1);
                }
            }

        }
        else {
            len = sizeof(STATUS_NOT_FOUND) - 1;
            strcpy(send_buffer, STATUS_NOT_FOUND);
            n = sendall(new_socket_descriptor, send_buffer, &len);
            if (n < 0) {
                perror("ERROR sending from socket");
                close(new_socket_descriptor);
                close(socket_descriptor);
                exit(1);
            }

            // Determine which content type to send
            if (strcmp(content_type, "html") == 0) {
                len = sizeof(MIME_HTML) - 1;
                strcpy(send_buffer, MIME_HTML);
                n = sendall(new_socket_descriptor, send_buffer, &len); // trim the nullbyte
                if (n < 0) {
                    perror("ERROR sending from socket");
                    close(new_socket_descriptor);
                    close(socket_descriptor);
                    exit(1);
                }
            }
            else if (strcmp(content_type, "css") == 0) {
                len = sizeof(MIME_CSS) - 1;
                strcpy(send_buffer, MIME_CSS);
                n = sendall(new_socket_descriptor, send_buffer, &len); // trim the nullbyte
                if (n < 0) {
                    perror("ERROR sending from socket");
                    close(new_socket_descriptor);
                    close(socket_descriptor);
                    exit(1);
                }
            }
            else if (strcmp(content_type, "js") == 0) {
                len = sizeof(MIME_JS) - 1;
                strcpy(send_buffer, MIME_JS);
                n = sendall(new_socket_descriptor, send_buffer, &len); // trim the nullbyte
                if (n < 0) {
                    perror("ERROR sending from socket");
                    close(new_socket_descriptor);
                    close(socket_descriptor);
                    exit(1);
                }
            }
            else {
                len = sizeof(MIME_JPG) - 1;
                strcpy(send_buffer, MIME_JPG);
                n = sendall(new_socket_descriptor, send_buffer, &len);   // trim the nullbyte
                if (n < 0) {
                    perror("ERROR sending from socket");
                    close(new_socket_descriptor);
                    close(socket_descriptor);
                    exit(1);
                }
            }

            len = sizeof(CRLF) - 1;
            strcpy(send_buffer, CRLF);
            n = sendall(new_socket_descriptor, send_buffer, &len); // trim the nullbyte, end of header
            if (n < 0) {
                perror("ERROR sending from socket");
                close(new_socket_descriptor);
                close(socket_descriptor);
                exit(1);
            }
        }

        /* Close socket and free */
        // free all malloc'd objects
        free(receive_buffer);
        receive_buffer = NULL;
        free(full_path);
        full_path = NULL;
        if (is_free) {
            free(relative_path);
            relative_path = NULL;
            is_free = 0;
        }

        close(new_socket_descriptor);
    }

    close(socket_descriptor);
    return 0;
}

/*****************************************************************************/

/*
 * Used to setup server, starting from creating socket, address, bind, and listen
 * @param *server_address: address of the server_address (pointer)
 * @param port_number: port number
 * @return socket_descriptor: file descriptor for the socket created
 */
int setup_server(struct sockaddr_in *server_address, int port_number) {
    int socket_descriptor;

    // Create socket
    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    // Check for error, socket_descriptor will be -1 if there is ERROR
    if (socket_descriptor < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // Bind socket
    // Create server address that will be used to listen incoming connection
    (*server_address).sin_family = AF_INET;
    (*server_address).sin_addr.s_addr = htonl(INADDR_ANY);
    (*server_address).sin_port = htons(port_number);

    // Bind the socket with address (IP and port number)
    if (bind(socket_descriptor, (struct sockaddr *) server_address, sizeof(*server_address)) < 0) {
        perror("ERROR on binding process");
        close(socket_descriptor);
        exit(1);
    }

    // Listen socket
    listen(socket_descriptor, BACKLOG);

    return socket_descriptor;
}

/*
 * Get relative path from the receiver buffer
 * @param receive_buffer: buffer containing the client request
 * @param socket_descriptor: used for closing
 * @param new_socket_descriptor: used to send message if client do something wrong
 * @param is_free: used to check whether token should be free
 * @return relative_path: relative path of the file that user requested
 */
char *get_relative_path(char *receive_buffer, int socket_descriptor, int new_socket_descriptor, int *is_free) {
    char *token;
    char *default_token;
    char send_buffer[BUFFER_LENGTH];
    int n, len;

    // Safety precaution for persistent browser
    if (strlen(receive_buffer) == 0) {
        close(new_socket_descriptor);
        return NULL;
    }

    // Get the URI (second token), first token is not URI
    token = strtok(receive_buffer, " ");
    // Safety precaution and making sure client is requesting GET
    if (strcmp(token, "GET") != 0 || token == NULL) {
        len = sizeof(STATUS_BAD_REQUEST) - 1;
        strcpy(send_buffer, STATUS_BAD_REQUEST);
        n = sendall(new_socket_descriptor, send_buffer, &len); // trim nullbyte
        if (n < 0) {
            perror("ERROR sending from socket");
            close(new_socket_descriptor);
            close(socket_descriptor);
            exit(1);
        }
        close(new_socket_descriptor);
        return NULL;
    }

    // Make sure that the first token is GET, if it is not then it's not valid request
    token = strtok(NULL, " ");
    assert(token);
    // Safety precaution and making sure client is providing valid path
    if (token[0] != '/' || token == NULL) {
        len = sizeof(STATUS_BAD_REQUEST) - 1;
        strcpy(send_buffer, STATUS_BAD_REQUEST);
        n = sendall(new_socket_descriptor, send_buffer, &len); // trim nullbyte
        if (n < 0) {
            perror("ERROR sending from socket");
            close(socket_descriptor);
            close(new_socket_descriptor);
            exit(1);
        }
        return NULL;
    }

    // By default / will return index.html
    if (token[strlen(token)-1] == '/') {
        default_token = (char *) malloc((strlen(token) + strlen("index.html") + 1) * sizeof(char));
        assert(default_token != NULL);
        strcpy(default_token, token);
        strcat(default_token, "index.html");
        *is_free = 1; // later default_token will be freed

        return default_token;
    }

    return token;
}

/*
 * Get the content type to be used in HTTP response header
 * @param relative_path: relative path from client's request
 * @return token: content type
 */
char *get_content_type(char *relative_path) {
    char *token;

    token = strtok(relative_path, ".");
    assert(token != NULL);

    token = strtok(NULL, " ");
    // Process to avoid segfault, no folder found (default content type = html)
    if (token == NULL) {
        return "html";
    }

    return token;
}

/**
 * Used to handle partial send(). Adapted from Beejs guide
 * @param s: socket identifier
 * @param buf: buffer containing the data
 * @param len: pointer to an int containing number of bytes in buffer
 * @return -1 on error, 0 on success
 */
int sendall(int s, char *buf, int *len) {
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}
