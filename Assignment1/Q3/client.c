#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define INPUT_FILE "sample.txt"
#define OUTPUT_FILE "received_file.txt"

void errorHandling(const char *errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}

int createUDPSocket()
{
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
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
    // serverAddress->sin_addr.s_addr = inet_addr(IP);
    if (inet_pton(AF_INET, IP, &(serverAddress->sin_addr)) <= 0)
        errorHandling("Invalid address");
    serverAddress->sin_port = htons(PORT);
}

void sendFileThroughSocket(int clientSocket, struct sockaddr_in serverAddress)
{
    const char *filename = INPUT_FILE;

    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        errorHandling("Error in opening file");
    }

    // print the filename
    printf("Sending %s to the server\n", filename);

    if (sendto(clientSocket, filename, strlen(filename), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        fclose(file);
        close(clientSocket);
        errorHandling("Error sending filename");
    }

    while (1)
    {
        char buffer[MAX_BUFFER_SIZE];
        size_t read_len = fread(buffer, 1, sizeof(buffer), file);

        if (read_len <= 0)
        {
            break;
        }

        ssize_t send_len = sendto(clientSocket, buffer, read_len, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

        if (send_len < 0)
        {
            fclose(file);
            close(clientSocket);
            errorHandling("Error sending data");
        }
        else
        {
            printf("Sent %ld bytes\n", send_len);
        }
    }

    fclose(file);

    // Send an empty message to the server to indicate the end of file
    if (sendto(clientSocket, "", 0, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        close(clientSocket);
        errorHandling("Error sending data");
    }
    else
    {
        printf("File sent successfully\n");
    }
}

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;

    clientSocket = createUDPSocket();
    setupServerAddress(&serverAddress);

    // Send the file to the server
    sendFileThroughSocket(clientSocket, serverAddress);

    close(clientSocket);

    return 0;
}
