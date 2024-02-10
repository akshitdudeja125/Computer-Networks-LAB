#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define IP "10.10.88.233"

#define OUTPUT_FILE_TEMPLATE "received_file_%d.txt"

void errorHandling(const char *errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}

int createTCPSocket()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        errorHandling("Error creating socket\n");
    }

    printf("Server socket created successfully\n");
    return serverSocket;
}

void setupServerAddress(struct sockaddr_in *serverAddress)
{
    memset(serverAddress, 0, sizeof(*serverAddress));
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_addr.s_addr = INADDR_ANY;
    serverAddress->sin_port = htons(PORT);
}

void bindSocket(int serverSocket, const struct sockaddr *serverAddress)
{
    if (bind(serverSocket, serverAddress, sizeof(*serverAddress)) < 0)
    {
        errorHandling("Error binding socket");
    }

    printf("Server bound to port %d\n", PORT);
}

void *handleClient(void *arg)
{
    int clientSocket = *((int *)arg);
    free(arg);

    // Receive file name from client
    char fileName[MAX_BUFFER_SIZE];
    ssize_t recvLen = recv(clientSocket, fileName, sizeof(fileName) - 1, 0);
    if (recvLen < 0)
    {
        perror("Error receiving file name");
        close(clientSocket);
        return NULL;
    }
    fileName[recvLen] = '\0';

    printf("Receiving file: %s\n", fileName);

    // Create output file
    static int fileCount = 0;
    char outputFilename[MAX_BUFFER_SIZE];
    snprintf(outputFilename, sizeof(outputFilename), OUTPUT_FILE_TEMPLATE, fileCount++);
    FILE *file = fopen(outputFilename, "wb");
    if (file == NULL)
    {
        perror("Error opening file for writing");
        close(clientSocket);
        return NULL;
    }

    // Receive file data from client and write to file
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        fwrite(buffer, 1, bytesReceived, file);
    }

    if (bytesReceived < 0)
    {
        perror("Error receiving file data");
    }

    fclose(file);
    printf("File received successfully: %s\n", outputFilename);

    close(clientSocket);
    return NULL;
}

int main()
{
    int serverSocket;
    struct sockaddr_in serverAddress;

    serverSocket = createTCPSocket();
    setupServerAddress(&serverAddress);
    bindSocket(serverSocket, (struct sockaddr *)&serverAddress);

    if (listen(serverSocket, 5) < 0)
    {
        errorHandling("Error listening for connections");
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int *clientSocket = malloc(sizeof(int));
        if (clientSocket == NULL)
        {
            perror("Error allocating memory for client socket");
            continue;
        }

        // Accept connection from client
        *clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (*clientSocket < 0)
        {
            perror("Error accepting connection");
            free(clientSocket);
            continue;
        }

        // Create a new thread to handle the client
        pthread_t tid;
        if (pthread_create(&tid, NULL, handleClient, (void *)clientSocket) != 0)
        {
            perror("Error creating thread");
            close(*clientSocket);
            free(clientSocket);
            continue;
        }

        // Detach the thread to clean up resources automatically
        pthread_detach(tid);
    }

    close(serverSocket);

    return 0;
}
