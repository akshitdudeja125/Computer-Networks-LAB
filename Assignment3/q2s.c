#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 80
#define BUFFER_SIZE 1024
#define FILENAME "response3.html" // Name of the file to save the response

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
    int headers_done = 0; // Flag to indicate when headers are done

    FILE *file = fopen(FILENAME, "wb"); // Open file for writing in binary mode
    if (file == NULL)
    {
        perror("Error opening file");
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
                // Skip the headers and set the flag
                fwrite(end_of_headers + 4, 1, bytes_received - (end_of_headers - buffer) - 4, file);
                headers_done = 1;
            }
        }
        else
        {
            fwrite(buffer, 1, bytes_received, file); // Write content to file
        }
    }

    fclose(file); // Close the file
}

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    const char *host = "www.example.com";
    const char *path = "/index.html";

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Error creating client socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("93.184.216.34"); // IPv4 address of www.example.com
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Send HTTP GET request
    send_http_request(client_socket, host, path);

    // Receive and save HTTP response to file
    receive_http_response(client_socket);

    // Close the socket
    close(client_socket);

    return 0;
}