#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024
#define IP "127.0.0.1"
void handle_error(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

int create_udp_socket()
{
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0)
    {
        handle_error("Error creating socket");
    }
    return server_socket;
}

void setup_server_address(struct sockaddr_in *server_address, int port)
{
    memset(server_address, 0, sizeof(*server_address));
    server_address->sin_family = AF_INET;

    if (inet_pton(AF_INET, IP, &(server_address->sin_addr)) <= 0)
    {
        handle_error("Invalid address");
    }

    server_address->sin_port = htons(port);
}

void bind_socket(int server_socket, struct sockaddr_in *server_address)
{
    if (bind(server_socket, (struct sockaddr *)server_address, sizeof(*server_address)) < 0)
    {
        handle_error("Error binding socket");
    }
}

void receive_and_send(int server_socket, struct sockaddr_in *client_address, socklen_t *client_address_len)
{
    char buffer[MAX_BUFFER_SIZE];

    ssize_t recv_len = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)client_address, client_address_len);

    if (recv_len < 0)
    {
        handle_error("Error receiving data");
    }

    buffer[recv_len] = '\0';
    printf("Received message from client %s:%d: %s\n\n", inet_ntoa(client_address->sin_addr), ntohs(client_address->sin_port), buffer);

    // Send a reply to the client
    char reply[MAX_BUFFER_SIZE];
    sprintf(reply, "The following message was received: %s\n\n", buffer);
    sendto(server_socket, reply, strlen(reply), 0, (struct sockaddr *)client_address, *client_address_len);

    // Print the reply at the server
    printf("Sent reply to client %s:%d: %s\n\n", inet_ntoa(client_address->sin_addr), ntohs(client_address->sin_port), reply);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Create a UDP socket
    server_socket = create_udp_socket();
    printf("Socket created to port.\n\n");

    // Set up the server address
    setup_server_address(&server_address, port);

    // Bind the socket to the address
    bind_socket(server_socket, &server_address);
    printf("Server bound to port %d\n\n", port);

    while (1)
    {
        receive_and_send(server_socket, &client_address, &client_address_len);
    }

    // Close the socket (unreachable in this example)
    close(server_socket);

    return 0;
}
