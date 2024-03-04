#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int connect_to_server(const char *host, int port)
{
    printf("Connecting to server...\n");
    struct hostent *he;
    struct sockaddr_in server_addr;

    if ((he = gethostbyname(host)) == NULL)
    {
        herror("gethostbyname");
        return -1;
    }
    else
    {
        printf("Host found.\n");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, he->h_addr_list[0], he->h_length);
    memset(&(server_addr.sin_zero), '\0', 8);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("Socket creation failed");
        return -1;
    }
    else
    {
        printf("Socket created successfully.\n");
    }

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Connection failed");
        return -1;
    }
    else
    {
        printf("Connected successfully.\n");
    }

    return socket_fd;
}

// Function to send HTTP request and receive response
void send_request_and_save(int socket_fd, const char *request)
{
    if (send(socket_fd, request, strlen(request), 0) == -1)
    {
        perror("Send failed");
        exit(1);
    }

    FILE *file = fopen("index2.html", "a"); // Open in append mode
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Read response and write to file
    while ((bytes_received = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0)) > 0)
    {
        printf("received %d bytes\n", bytes_received);
        buffer[bytes_received] = '\0';
        fprintf(file, "%s", buffer);
    }

    if (bytes_received < 0)
    {
        perror("Receive failed");
    }
    else
    {
        printf("Response received successfully.\n");
    }

    fclose(file);
}

// Main function to fetch index.html using GET
int main()
{
    const char *host = "www.google.com"; // Replace with the desired web server
    int port = 80;                       // HTTP port
    const char *path = "/";              // Path to the resource

    char get_request[1000];
    sprintf(get_request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, host);

    int socket_fd = connect_to_server(host, port);
    if (socket_fd == -1)
    {
        exit(1);
    }
    else
    {
        printf("Connected to server\n");
    }
    send_request_and_save(socket_fd, get_request);

    // Close the socket
    close(socket_fd);

    return 0;
}
