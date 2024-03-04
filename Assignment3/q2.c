#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#define PORT 80
#define BUFFER_SIZE 1024
#define FILENAME "responsea.html"
#define HEADER_FILENAME "headers.txt"

void send_http_request(int client_socket, const char *host, const char *path)
{
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);

    ssize_t bytes_sent = send(client_socket, request, strlen(request), 0);
    if (bytes_sent < 0)
    {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }
}

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

    FILE *header_file = fopen(HEADER_FILENAME, "w");
    if (header_file == NULL)
    {
        perror("Error opening header file");
        exit(EXIT_FAILURE);
    }

    // Receive and write the response to the file
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        if (!headers_done)
        {
            // Search for the double line break indicating end of headers
            char *end_of_headers = strstr(buffer, "\r\n\r\n");
            if (end_of_headers != NULL)
            {
                // Write the headers to the header file
                fwrite(buffer, 1, end_of_headers - buffer + 4, header_file);
                headers_done = true;

                fwrite(end_of_headers + 4, 1, bytes_received - (end_of_headers - buffer) - 4, file);
            }
        }
        else
        {
            fwrite(buffer, 1, bytes_received, file); // Write content to file
        }
    }

    fclose(file);
}

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    const char *host = "www.google.com";
    const char *path = "/index.html";

    struct hostent *he;
    if ((he = gethostbyname(host)) == NULL)
    {
        herror("gethostbyname");
        return -1;
    }
    else
    {
        printf("Host found.\n");
    }

    // Print server address
    printf("Server address: %s\n", inet_ntoa(*(struct in_addr *)he->h_addr_list[0]));
    char *server_address = inet_ntoa(*(struct in_addr *)he->h_addr_list[0]);
    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Error creating client socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_address);
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    send_http_request(client_socket, host, path);

    receive_http_response(client_socket);

    close(client_socket);

    return 0;
}