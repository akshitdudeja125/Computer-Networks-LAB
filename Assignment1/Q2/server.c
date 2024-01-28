#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8086
#define MAX_BUFFER_SIZE 1024

int main()
{
    int server_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Create a UDP socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0)
    {
        perror("Error creating socket\n\n");
        exit(EXIT_FAILURE);
    }

    printf("Socket created to port.\n\n");

    // Set up the server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;

    // server_address.sin_addr.s_addr = INADDR_ANY;
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        perror("Invalid address\n\n");
        exit(EXIT_FAILURE);
    }
    server_address.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Error binding socket\n\n");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server bound to port %d\n\n", PORT);

    while (1)
    {
        char buffer[MAX_BUFFER_SIZE];

        // Receive data from the client
        ssize_t recv_len = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &client_address_len);

        if (recv_len < 0)
        {
            perror("Error receiving data\n\n");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        buffer[recv_len] = '\0';
        printf("Received message from client %s:%d: %s\n\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), buffer);

        // Send a reply to the client
        char reply[MAX_BUFFER_SIZE];
        sprintf(reply, "THe following messaage was recieved: %s\n\n", buffer);
        sendto(server_socket, reply, strlen(reply), 0, (struct sockaddr *)&client_address, client_address_len);

        // Print the reply at the server
        printf("Sent reply to client %s:%d: %s\n\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), reply);
    }

    // Close the socket (unreachable in this example)
    close(server_socket);

    return 0;
}