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

#define CONNECTION_PORT 8080
#define CONNECTION_ADDRESS "10.10.88.233"

int main()
{
    int request_sock, domain, type, protocol;

    // Create a UDP socket
    request_sock = socket(AF_INET, SOCK_DGRAM, 0); // Domain, Type, Protocol // UDP

    char buffer[1024];

    if (request_sock == -1)
    {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket created successfully\n");
    }

    struct sockaddr_in server, client;
    socklen_t addr_len = sizeof(client);

    // Initialize server address structure
    server.sin_family = AF_INET;              // address family (defines the structure and the type of the address that socket can communicate with)
    server.sin_port = htons(CONNECTION_PORT); // converts the unsigned short integer from host byte order to network byte order
    // Convert IP address from string to binary form

    if (inet_pton(AF_INET, CONNECTION_ADDRESS, &server.sin_addr) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the server address
    int bind_desc = bind(request_sock, (struct sockaddr *)&server, sizeof(server));
    if (bind_desc == -1)
    {
        perror("Could not bind to server address");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Binded successfully\n");
    }

    while (1)
    {
        // Receive data from the client
        int recv_len = recvfrom(request_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client, &addr_len);
        if (recv_len < 0)
        {
            perror("Error in recvfrom");
            break;
        }

        buffer[recv_len] = '\0'; // Null-terminate the received data
        printf("Received message from client: %s\n", buffer);

        // Send a response back to the client
        char *response = "Hello from server\n";
        if (sendto(request_sock, response, strlen(response), 0, (struct sockaddr *)&client, addr_len) < 0)
        {
            perror("Error in sendto");
            break;
        }

        printf("Response sent to the client.\n");
    }

    // Close the socket before exiting
    close(request_sock);

    return 0;
}
