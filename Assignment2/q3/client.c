#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <dirent.h>
#define SERVER_IP "10.10.88.233"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filePath>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // char *filePath = "sample.txt";
    char *filePath = argv[1];
    FILE *file = fopen(filePath, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // extract fileName from filePath which is text after last /
    char *fileName = strrchr(filePath, '/');
    if (fileName == NULL)
    {
        fileName = filePath;
    }
    else
    {
        fileName++;
    }

    // Send fileName to server
    send(client_socket, fileName, strlen(fileName), 0);
    printf("%s %d\n", fileName, strlen(fileName));
    // sleep for 2 seconds
    sleep(2);
    // Send file data to server
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    {
        printf("%s\n", buffer);
        printf("%d\n", bytes_read);
        send(client_socket, buffer, bytes_read, 0);
    }
    if (bytes_read < 0)
    {
        perror("Error reading from file");
    }

    fclose(file);
    close(client_socket);

    printf("File '%s' sent successfully to server\n", filePath);

    return 0;
}
