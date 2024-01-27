#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define IP "10.10.88.233"
#define PORT 8080

void error(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

int createSocket()
{
    int socketDescriptor;
    if ((socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        error("Error creating socket");
    }
    else
    {
        printf("Client socket created successfully\n");
    }
    return socketDescriptor;
}

void setupServerAddress(struct sockaddr_in *serverAddr)
{
    memset(serverAddr, 0, sizeof(*serverAddr));
    serverAddr->sin_family = AF_INET;
    serverAddr->sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &serverAddr->sin_addr) <= 0)
    {
        error("Invalid address");
    }
}

void sendMessage(int socketDescriptor, struct sockaddr_in *serverAddr, char *buffer)
{
    printf("Enter a message to send to the server: ");
    fgets(buffer, MAX_BUFFER_SIZE, stdin);

    if (sendto(socketDescriptor, buffer, strlen(buffer), 0, (struct sockaddr *)serverAddr, sizeof(*serverAddr)) == -1)
    {
        error("Error sending message");
    }
    else
    {
        printf("Message sent to the server successfully\n");
    }
}

void receiveMessage(int socketDescriptor, struct sockaddr_in *serverAddr, char *buffer)
{
    socklen_t addrLen = sizeof(*serverAddr);
    if (recvfrom(socketDescriptor, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)serverAddr, &addrLen) == -1)
    {
        error("Error receiving message");
    }
    else
    {
        printf("Message received from the server\n");
    }
}

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE];

    // Create a UDP socket
    clientSocket = createSocket();

    // Set up the server address and port
    setupServerAddress(&serverAddr);

    // Send a message to the server
    sendMessage(clientSocket, &serverAddr, buffer);

    // Receive the response from the server
    receiveMessage(clientSocket, &serverAddr, buffer);

    // Display the received message
    printf("Received from server: %s", buffer);

    // Close the socket
    close(clientSocket);

    return 0;
}
