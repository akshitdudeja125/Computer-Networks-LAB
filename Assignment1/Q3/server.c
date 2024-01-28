#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define IP "10.10.88.233"

#define OUTPUT_FILE_TEMPLATE "received_file_%d.txt"

void errorHandling(const char *errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}

int createUDPSocket()
{
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
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
    serverAddress->sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &(serverAddress->sin_addr)) <= 0)
    {
        errorHandling("Invalid address");
    }
}

void bindSocket(int serverSocket, const struct sockaddr *serverAddress)
{
    if (bind(serverSocket, serverAddress, sizeof(*serverAddress)) < 0)
    {
        errorHandling("Error binding socket");
    }

    printf("Server bound to port %d\n", PORT);
}

void receiveFile(int serverSocket)
{

    // Recieve file name from client to indicate start of file transfer
    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in clientAddress;
    socklen_t client_address_len = sizeof(clientAddress);

    ssize_t recv_len = recvfrom(serverSocket, buffer, sizeof(buffer)-1, 0, (struct sockaddr *)&clientAddress, &client_address_len);

    if (recv_len < 0)
    {
        errorHandling("Error receiving data");
    }
    else if (recv_len == 0)
    {
        errorHandling("Client disconnected");
    }

    buffer[recv_len] = '\0';
    char *fileName = buffer;

    printf("Receiving file: %s\n", fileName);

    static int fileCount = 0; // Keep track of the number of received files

    char outputFilename[MAX_BUFFER_SIZE];
    snprintf(outputFilename, sizeof(outputFilename), OUTPUT_FILE_TEMPLATE, fileCount);

    FILE *file = fopen(outputFilename, "wb");
    if (file == NULL)
    {
        errorHandling("Error opening file for writing");
    }

    while (1)
    {
        char buffer[MAX_BUFFER_SIZE];
        ssize_t recv_len = recvfrom(serverSocket, buffer, sizeof(buffer)-1, 0, (struct sockaddr *)&clientAddress, &client_address_len);
        buffer[recv_len] = '\0';
        printf("%d\n", recv_len);
        if (recv_len < 0)
        {
            errorHandling("Error receiving data");
            break;
        }

        if (recv_len == 0)
        {
            printf("Client disconnected");
            break; // End of file
        }

        fwrite(buffer, 1, recv_len, file);
    }

    fclose(file);
    printf("File received successfully: %s\n", outputFilename);
    fileCount++; // Increment file count for the next file
}

int main()
{
    int serverSocket;
    struct sockaddr_in serverAddress;

    serverSocket = createUDPSocket();
    setupServerAddress(&serverAddress);
    bindSocket(serverSocket, (struct sockaddr *)&serverAddress);

    while (1)
    {
        receiveFile(serverSocket);
    }

    close(serverSocket);

    return 0;
}
