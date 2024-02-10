#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define IP "127.0.0.1"
#define PORT 8085
#define MAX_BUFFER_SIZE 1024
#define INPUT_FILE "sample.txt"

void errorHandling(const char *errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}

int createTCPSocket()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        errorHandling("Error creating socket");
    }

    printf("Client socket created successfully\n");
    return clientSocket;
}

void setupServerAddress(struct sockaddr_in *serverAddress)
{
    memset(serverAddress, 0, sizeof(*serverAddress));
    serverAddress->sin_family = AF_INET;
    if (inet_pton(AF_INET, IP, &(serverAddress->sin_addr)) <= 0)
    {
        errorHandling("Invalid address");
    }
    serverAddress->sin_port = htons(PORT);
}

void sendFileThroughSocket(int clientSocket, struct sockaddr_in serverAddress)
{
    const char *filename = INPUT_FILE;

    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        errorHandling("Error opening file");
    }

    // Send the filename to the server
    if (send(clientSocket, filename, strlen(filename), 0) < 0)
    {
        fclose(file);
        close(clientSocket);
        errorHandling("Error sending filename");
    }

    // Send file data to the server
    char buffer[MAX_BUFFER_SIZE];
    size_t readLen;
    while ((readLen = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        if (send(clientSocket, buffer, readLen, 0) < 0)
        {
            fclose(file);
            close(clientSocket);
            errorHandling("Error sending data");
        }
    }

    fclose(file);
    printf("File sent successfully\n");
}

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;

    clientSocket = createTCPSocket();
    setupServerAddress(&serverAddress);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        errorHandling("Error connecting to server");
    }

    // Send the file to the server
    sendFileThroughSocket(clientSocket, serverAddress);

    close(clientSocket);

    return 0;
}
