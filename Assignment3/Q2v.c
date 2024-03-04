#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

// Function to create a TCP socket
int create_tcp_socket()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

// Function to connect to a server
int connect_to_server(const char *host, int port)
{
    struct hostent *he;
    struct sockaddr_in server_addr;

    if ((he = gethostbyname(host)) == NULL)
    {
        herror("gethostbyname");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, he->h_addr_list[0], he->h_length);
    memset(&(server_addr.sin_zero), '\0', 8);

    int socket_fd = create_tcp_socket();
    if (socket_fd == -1)
    {
        perror("Socket creation failed");
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Connection failed");
        return -1;
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

    FILE *file = fopen("index.html", "a"); // Open in append mode
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
        buffer[bytes_received] = '\0';
        fprintf(file, "%s", buffer); // Write to file
        // printf("%s", buffer);        // Print received data
    }

    if (bytes_received < 0)
    {
        perror("Receive failed");
    }

    fclose(file);
}

// // Function to send HTTP request and receive response
// void send_request_and_save(int socket_fd, const char *request)
// {
//     if (send(socket_fd, request, strlen(request), 0) == -1)
//     {
//         perror("Send failed");
//         exit(1);
//     }

//     FILE *file = fopen("index.html", "w");
//     if (file == NULL)
//     {
//         perror("Error opening file");
//         exit(1);
//     }

//     char buffer[BUFFER_SIZE];
//     int bytes_received;

//     while ((bytes_received = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0)) > 0)
//     {
//         buffer[bytes_received] = '\0';
//         fprintf(file, "%s", buffer);
//         printf("%s", buffer); // Print received data

//         // Check for redirection
//         if (strstr(buffer, "HTTP/1.1 301") != NULL || strstr(buffer, "HTTP/1.1 302") != NULL)
//         {
//             printf("Redirected. Following redirect...\n");

//             // Find location header
//             char *location_start = strstr(buffer, "Location:");
//             if (location_start != NULL)
//             {
//                 location_start += strlen("Location:");
//                 while (*location_start == ' ')
//                 {
//                     location_start++;
//                 }
//                 char *location_end = strchr(location_start, '\r');
//                 if (location_end != NULL)
//                 {
//                     *location_end = '\0';

//                     // Close the current file and socket
//                     fclose(file);
//                     close(socket_fd);

//                     // Parse the redirect location
//                     char redirect_host[100], redirect_path[100];
//                     if (sscanf(location_start, "http://%99[^/]/%99s", redirect_host, redirect_path) == 2)
//                     {
//                         printf("Redirect host: %s, Redirect path: %s\n", redirect_host, redirect_path);

//                         // Connect to the redirect location
//                         int redirect_socket_fd = connect_to_server(redirect_host, 80);
//                         if (redirect_socket_fd != -1)
//                         {
//                             // Send new request for the redirected path
//                             send_request_and_save(redirect_socket_fd,
//                                                   (char[]){"GET /"});
//                             break;
//                         }
//                         else
//                         {
//                             perror("Failed to connect to redirect host");
//                             exit(1);
//                         }
//                     }
//                     else
//                     {
//                         printf("Failed to parse redirect location: %s\n", location_start);
//                     }
//                 }
//                 else
//                 {
//                     printf("Invalid redirect location\n");
//                 }
//             }
//             else
//             {
//                 printf("No 'Location' header found in redirect response\n");
//             }
//         }
//     }

//     if (bytes_received < 0)
//     {
//         perror("Receive failed");
//     }

//     fclose(file);
// }

// Main function to fetch index.html using GET
int main()
{
    const char *host = "www.google.com"; // Replace with the desired web server
    int port = 80;                   // HTTP port
    const char *path = "/";          // Path to the resource

    // Create and connect to the server
    printf("Connecting to server...\n");
    int socket_fd = connect_to_server(host, port);
    if (socket_fd == -1)
    {
        exit(1);
    }
    printf("Connected successfully.\n");

    // Fetch index.html file using GET request and save it
    printf("Fetching index.html file...\n");
    char get_request[100];
    sprintf(get_request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, host);
    send_request_and_save(socket_fd, get_request);
    printf("Index.html file fetched and saved.\n");

    // Close the socket
    close(socket_fd);

    return 0;
}
