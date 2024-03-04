#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1000007

int connect_to_server(const char *host, int port)
{
    printf("Connecting to server...\n");
    // struct hostent *he;
    struct sockaddr_in server_addr;

    // if ((he = gethostbyname(host)) == NULL)
    // {
    //     herror("gethostbyname");
    //     return -1;
    // }
    // else
    // {
    //     printf("Host found.\n");
    // }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("93.184.216.34"); // IPv4 address of www.example.com
    server_addr.sin_port = htons(port);
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

    FILE *file = fopen("indexe.html", "a"); // Open in append mode
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;
    int header_ended = 0; // Flag to track whether the header has ended

    // Read response and write to file, skipping the headers
    while ((bytes_received = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Bytes received: %d\n", bytes_received);
        // Check if the header has ended
        if (!header_ended)
        {
            char *header_end = strstr(buffer, "\r\n\r\n");
            if (header_end != NULL)
            {
                // Header found, set flag to indicate header has ended
                header_ended = 1;
                printf("Header ended\n");
                // Skip past the header and write the remaining content to file
                fwrite(header_end + 4, sizeof(char), bytes_received - (header_end - buffer) - 4, file);
            }
        }
        else
        {
            // If the header has ended, directly write the content to the file
            fwrite(buffer, sizeof(char), bytes_received, file);
        }
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
    const char *host = "www.example.com"; // Replace with the desired web server
    int port = 80;                        // HTTP port
    const char *path = "/index.html";     // Path to the resource

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
