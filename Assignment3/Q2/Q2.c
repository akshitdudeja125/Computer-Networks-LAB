#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 80
#define BUFFER_SIZE 1024
#define FILENAME "response.html"
#define HEADER_FILENAME "header.txt"

void receive_http_response(int client_socket)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    bool headers_done = false;

    FILE *file = fopen(FILENAME, "wb");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    FILE *header_file = fopen(HEADER_FILENAME, "wb");
    if (header_file == NULL)
    {
        perror("Error opening header file");
        exit(EXIT_FAILURE);
    }

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        if (!headers_done)
        {
            char *end_of_headers = strstr(buffer, "\r\n\r\n");
            if (end_of_headers != NULL)
            {
                fwrite(buffer, 1, end_of_headers - buffer + 4, header_file);
                fwrite(end_of_headers + 4, 1, bytes_received - (end_of_headers - buffer) - 4, file);
                headers_done = true;
            }
            else
                fwrite(buffer, 1, bytes_received, header_file);
        }
        else
            fwrite(buffer, 1, bytes_received, file);
    }

    fclose(file);
}

void perform_get_request(int client_socket, const char *host, const char *path)
{
    char request[BUFFER_SIZE];

    snprintf(request, BUFFER_SIZE, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);

    ssize_t bytes_sent = send(client_socket, request, strlen(request), 0);
    if (bytes_sent < 0)
    {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }

    receive_http_response(client_socket);
}

void perform_post_request(int client_socket, const char *host, const char *path, const char *data)
{
    size_t data_length = strlen(data);

    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s", path, host, data_length, data);

    ssize_t bytes_sent = send(client_socket, request, strlen(request), 0);
    if (bytes_sent < 0)
    {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }

    receive_http_response(client_socket);
}

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    const char *host = "www.google.com";
    const char *path = "/index.html";
    // const char *post_data = "key1=value1&key2=value2";
    const char *post_data = "query=example";

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Error creating client socket");
        exit(EXIT_FAILURE);
    }

    struct hostent *server;
    server = gethostbyname(host);
    if (server == NULL)
        herror("No such Host");

    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr, server->h_addr_list[0], server->h_length);
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Perform GET request
    // printf("Performing GET request...\n");
    // perform_get_request(client_socket, host, path);

    // Perform POST request
    printf("\nPerforming POST request...\n");
    perform_post_request(client_socket, host, path, post_data);

    close(client_socket);

    return 0;
}
