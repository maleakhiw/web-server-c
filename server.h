/**
 * HTTP Server (GET) - Header File
 * Author: Maleakhi Agung Wijaya (maleakhiw)
 * Student ID: 784091
 * Date: 15/04/2018
 */

 #ifndef SERVER_H
 #define SERVER_H

/** Library */
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

/*****************************************************************************/

/** Constant */
#define ARGUMENT 3
#define PORT_INDEX 1
#define WEB_ROOT_PATH_INDEX 2
#define BACKLOG 10
#define BUFFER_LENGTH 8000
#define HEADER_PATTERN_START 4
#define FINISH_READING_FLAG 2

const char STATUS_OK[] = "HTTP/1.0 200 OK\r\n";
const char STATUS_NOT_FOUND[] = "HTTP/1.0 404 Not Found\r\n";
const char STATUS_BAD_REQUEST[] = "HTTP/1.0 400 Bad Request\r\n";
const char MIME_HTML[] = "Content-Type: text/html\r\n";
const char MIME_JPG[] = "Content-Type: image/jpeg\r\n";
const char MIME_CSS[] = "Content-Type: text/css\r\n";
const char MIME_JS[] = "Content-Type: application/javascript\r\n";
const char CRLF[] = "\r\n";
const char BODY_NOT_FOUND[] =
"<!DOCTYPE html>\r\n<html><head><title>404 Not Found</title></head>\r\n\
<body><center><h1>404 Not Found</h1></center></body></html>\r\n";

/** Function Declaration */
void initialise_arguments(int argc, char *argv[], int *port_number);
int setup_server(struct sockaddr_in *server_address, int port_number);
char *get_relative_path(char *receive_buffer, int socket_descriptor,
   int connection_descriptor, int *is_free);
char *get_content_type(char *relative_path);
int sendall(int s, char *buf, int *len);
void *process_request(void *connection_descriptor_pointer);
void handle_multithread(int connection_descriptor);
int receive_all(char *receive_buffer, int *receive_buffer_size,
   int connection_descriptor);
void send_http_success(char send_buffer[], char *content_type,
   int connection_descriptor);
void send_http_failure(char send_buffer[], char *content_type,
   int connection_descriptor);

#endif
