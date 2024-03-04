#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 80
#define BUFFER_SIZE 1024

void send_http_request(int client_socket, const char *host, const char *path, const char *method)
{
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "%s %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", method, path, host);

    ssize_t bytes_sent = send(client_socket, request, strlen(request), 0);
    if (bytes_sent < 0)
    {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Sent %ld bytes\n", bytes_sent);
    }
}

void receive_http_response(int client_socket, const char *filename)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    int headers_done = 0;               // Flag to indicate when headers are done
    FILE *file = fopen(filename, "wb"); // Open file for writing in binary mode
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("File '%s' opened successfully\n", filename);
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

void getRequest(int client_socket, const char *host, const char *path, const char *filename)
{
    send_http_request(client_socket, host, path, "GET");
    receive_http_response(client_socket, filename);
}

void postRequest(int client_socket, const char *host, const char *path, const char *data, const char *filename)
{
    send_http_request(client_socket, host, path, "POST");
    send(client_socket, data, strlen(data), 0);
    receive_http_response(client_socket, filename);
}

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    const char *host = "www.example.com";
    const char *path_get = "/index.html";
    const char *path_post = "/submit.php";
    const char *post_data = "key1=value1&key2=value2"; // Example POST data
    const char *filename_get = "response_get.html";
    const char *filename_post = "response_post.html";

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

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Error creating client socket");
        exit(EXIT_FAILURE);
    }
    char *server_address = inet_ntoa(*(struct in_addr *)he->h_addr);
    printf("Server address: %s\n", inet_ntoa(*(struct in_addr *)he->h_addr));
    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_address);
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Connected to server\n");
    }

    // getRequest(client_socket, host, path_get, filename_get);

    postRequest(client_socket, host, path_post, post_data, filename_post);

    if (close(client_socket) == -1)
    {
        perror("Error closing socket");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket closed\n");
    }

    return 0;
}
