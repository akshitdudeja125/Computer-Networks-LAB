#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

// Function to handle errors and exit
void handle_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int createSocket()
{
    int socketDescriptor;
    if ((socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        handle_error("Could not create socket");
    }
    else
    {
        printf("Socket created successfully\n");
    }
    return socketDescriptor;
}

void initializeServerAddress(struct sockaddr_in *server, int port)
{
    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    if (inet_pton(AF_INET, "10.10.88.233", &(server->sin_addr)) <= 0)
    {
        handle_error("Invalid address");
    }
}

void bindSocket(int socketDescriptor, struct sockaddr_in *server)
{
    if (bind(socketDescriptor, (struct sockaddr *)server, sizeof(*server)) == -1)
    {
        handle_error("Could not bind to server address");
    }
    else
    {
        printf("Binded successfully\n");
    }
}

void processClientRequests(int socketDescriptor)
{
    struct sockaddr_in client;
    socklen_t addr_len = sizeof(client);
    char buffer[MAX_BUFFER_SIZE];

    while (1)
    {
        int recv_len = recvfrom(socketDescriptor, buffer, sizeof(buffer), 0, (struct sockaddr *)&client, &addr_len);
        if (recv_len < 0)
        {
            handle_error("Error in recvfrom");
        }

        buffer[recv_len] = '\0'; // Null-terminate the received data
        printf("Received message from client: %s\n", buffer);

        char response[MAX_BUFFER_SIZE];
        snprintf(response, MAX_BUFFER_SIZE, "Received message from client: %s", buffer);

        if (sendto(socketDescriptor, response, strlen(response), 0, (struct sockaddr *)&client, addr_len) < 0)
        {
            handle_error("Error in sendto");
        }

        printf("Response sent to the client.\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    // int port = 8085; // If you want to use a specific port, uncomment this line

    // Create a UDP socket
    int request_sock = createSocket();

    struct sockaddr_in server;
    initializeServerAddress(&server, port);

    // Bind the socket to the server address
    bindSocket(request_sock, &server);

    // Process client requests
    processClientRequests(request_sock);

    // Close the socket before exiting (the process will never reach here in the current infinite loop)
    close(request_sock);

    return 0;
}
