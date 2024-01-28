#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define NUM_MESSAGES 5
#define SERVER_IP "127.0.0.1"

// Function to handle errors
void handle_error(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

// Function to create a UDP socket
int create_udp_socket()
{
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1)
    {
        handle_error("Error creating socket");
    }
    else
    {
        printf("Client socket created successfully\n");
    }
    return socket_fd;
}

// Function to set up server address
void setup_server_address(struct sockaddr_in *server_addr, const char *server_ip, int server_port)
{
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;

    if (inet_pton(AF_INET, server_ip, &(server_addr->sin_addr)) <= 0)
    {
        handle_error("Invalid address");
    }

    server_addr->sin_port = htons(server_port);
}

// Function to send message to server
void send_message(int socket_fd, const char *message, const struct sockaddr_in *server_addr, socklen_t addr_len)
{
    if (sendto(socket_fd, message, strlen(message), 0, (struct sockaddr *)server_addr, addr_len) == -1)
    {
        handle_error("Error sending message");
    }
}

// Function to receive message from server
void receive_message(int socket_fd, char *buffer, struct sockaddr_in *server_addr, socklen_t *addr_len)
{
    int recv_len = recvfrom(socket_fd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)server_addr, addr_len);
    if (recv_len <= 0)
    {
        handle_error("Error receiving message");
    }

    buffer[recv_len] = '\0'; // Null-terminate the received data
}

// Function to handle communication with servers
void communicate_with_servers()
{
    int client_socket = create_udp_socket();

    const char *server_ip = SERVER_IP;
    const int server_ports[] = {8080, 8081};
    const int n_servers = sizeof(server_ports) / sizeof(server_ports[0]);
    struct sockaddr_in server_addr[n_servers];

    for (int i = 0; i < n_servers; i++)
    {
        setup_server_address(&server_addr[i], server_ip, server_ports[i]);
    }

    int seq_num = 1;

    while (1)
    {
        for (int i = 0; i < n_servers; i++)
        {
            char buffer[MAX_BUFFER_SIZE];
            socklen_t addr_len = sizeof(server_addr[i]);

            // Create the message with sequence number
            sprintf(buffer, "Message %d from client\n\n", seq_num);

            // Send the message to the server
            send_message(client_socket, buffer, &server_addr[i], addr_len);

            printf("Sent message to server %s:%d - Sequence Number: %d\n",
                   inet_ntoa(server_addr[i].sin_addr), ntohs(server_addr[i].sin_port), seq_num);

            // Receive message from the server
            receive_message(client_socket, buffer, &server_addr[i], &addr_len);

            printf("Received message from server %s:%d: %s\n\n",
                   inet_ntoa(server_addr[i].sin_addr), ntohs(server_addr[i].sin_port), buffer);
        }

        if (seq_num == NUM_MESSAGES)
            break;

        sleep(2); // Sleep for 2 seconds
        seq_num++;
    }

    // Close the socket
    close(client_socket);
}

int main()
{
    communicate_with_servers();
    return 0;
}
