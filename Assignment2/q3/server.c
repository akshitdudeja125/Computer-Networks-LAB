#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <dirent.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024
#define IP "10.10.88.233"
#define RECEIVED_FILES_FOLDER "received_files"

void *handle_client(void *arg)
{
    int client_socket = *((int *)arg);
    free(arg); // Free memory allocated for the argument

    FILE *file;
    ssize_t bytes_received;

    DIR *dir;

    if ((dir = opendir(RECEIVED_FILES_FOLDER)) == NULL)
    {
        // mkdir(RECEIVED_FILES_FOLDER, 0777);
        if (mkdir(RECEIVED_FILES_FOLDER, 0777) == -1)
        {
            perror("Failed to open directory");
            close(client_socket);
            pthread_exit(NULL);
        }
    }
    else
    {
        closedir(dir);
    }

    char filename[BUFFER_SIZE];

    bytes_received = recv(client_socket, filename, BUFFER_SIZE, 0);
    if (bytes_received < 0)
    {
        perror("Error receiving filename");
        close(client_socket);
        return NULL;
    }

    filename[bytes_received] = '\0';

    char received_filename[BUFFER_SIZE];
    snprintf(received_filename, BUFFER_SIZE, "%s/%s", RECEIVED_FILES_FOLDER, filename);
    // Open file for writing
    file = fopen(received_filename, "wb");
    if (file == NULL)
    {
        perror("Error opening file");
        close(client_socket);
        return NULL;
    }

    // Receive file data and write to file
    char buffer[BUFFER_SIZE];
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        if (fwrite(buffer, 1, bytes_received, file) != bytes_received)
        {
            perror("Error writing to file");
            close(client_socket);
            fclose(file);
            pthread_exit(NULL);
        }
    }
    if (bytes_received < 0)
    {
        perror("Error receiving file data");
        close(client_socket);
        fclose(file);
        pthread_exit(NULL);
    }

    printf("File received: %s\n", received_filename);

    fclose(file);
    close(client_socket);
    return NULL;
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Bind to address and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, IP, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        // Accept connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0)
        {
            perror("Error accepting connection");
            continue;
        }

        printf("Connection accepted\n");

        // Create thread to handle client request
        pthread_t tid;
        int *client_socket_ptr = (int *)malloc(sizeof(int));
        if (client_socket_ptr == NULL)
        {
            perror("Error allocating memory");
            close(client_socket);
            continue;
        }
        *client_socket_ptr = client_socket;
        if (pthread_create(&tid, NULL, handle_client, (void *)client_socket_ptr) != 0)
        {
            perror("Error creating thread");
            close(client_socket);
            free(client_socket_ptr);
            continue;
        }

        // Detach thread
        pthread_detach(tid);
    }

    close(server_socket);
    return 0;
}
