#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CONN 10
#define MAX_FILENAME_LEN 256

void *handle_connection(void *sock_fd);

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address parameters
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, MAX_CONN) == -1)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept connections and handle them in separate threads
    while (1)
    {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) == -1)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_connection, (void *)&client_fd);
    }

    return 0;
}

void *handle_connection(void *sock_fd)
{
    int client_fd = *((int *)sock_fd);
    char filename[MAX_FILENAME_LEN];
    ssize_t bytes_received;

    // Receive filename
    if ((bytes_received = recv(client_fd, filename, MAX_FILENAME_LEN, 0)) == -1)
    {
        perror("Receive failed");
        close(client_fd);
        pthread_exit(NULL);
    }
    filename[bytes_received] = '\0';
    printf("Received file: %s\n", filename);

    // Open file
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("File open failed");
        close(client_fd);
        pthread_exit(NULL);
    }

    // Receive file data and write to file
    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = recv(client_fd, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, bytes_read, file);
    }

    // Close file and connection
    fclose(file);
    close(client_fd);
    printf("File received successfully: %s\n", filename);

    pthread_exit(NULL);
}
