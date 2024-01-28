#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

// #define SERVER_IP "127.0.0.1"
#define SERVER_IP "10.10.88.233"
int SERVER_PORT[] = {8080, 8081, 8082, 8083, 8084};
int NUM_SERVERS = sizeof(SERVER_PORT) / sizeof(SERVER_PORT[0]);
#define MESSAGE_INTERVAL_SECONDS 2
#define NUM_MESSAGES 5
#define MAX_BUFFER_SIZE 1024

// Function to handle errors
void handle_error(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

// Thread function for sending messages
void *send_message(void *args)
{
    int server_index = ((int *)args)[0];
    int client_socket = ((int *)args)[1];
    struct sockaddr_in server_address;

    // Set up the server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0)
    {
        handle_error("Invalid address");
    }
    server_address.sin_port = htons(SERVER_PORT[server_index]);
    socklen_t server_address_len = sizeof(server_address);

    for (int seq_num = 1; seq_num <= NUM_MESSAGES; ++seq_num)
    {
        // Prepare the message with sequence number
        char message[MAX_BUFFER_SIZE];
        snprintf(message, sizeof(message), "Message %d from client\n\n", seq_num);

        // Send the message to the server
        ssize_t send_len = sendto(client_socket, message, strlen(message), 0, (struct sockaddr *)&server_address, server_address_len);

        if (send_len < 0)
        {
            handle_error("Error sending data to server");
        }

        printf("Sent message to server %s:%d - Sequence Number: %d\n\n",
               inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port), seq_num);

        char buffer[MAX_BUFFER_SIZE];
        // Receive data from the server
        ssize_t recv_len = recvfrom(client_socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&server_address, &server_address_len);

        if (recv_len < 0)
        {
            handle_error("Error receiving data");
        }

        buffer[recv_len] = '\0';
        printf("Received message from server %s:%d: %s\n",
               inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port), buffer);

        // Wait for the specified interval before sending the next message
        sleep(MESSAGE_INTERVAL_SECONDS);
    }

    pthread_exit(NULL);
}

// Function to create and join threads
void create_and_join_threads(int client_socket)
{
    pthread_t threads[NUM_SERVERS];
    int thread_args[NUM_SERVERS][2];

    // Create threads for each server
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        thread_args[i][0] = i;
        thread_args[i][1] = client_socket;
        if (pthread_create(&threads[i], NULL, send_message, (void *)&thread_args[i]) != 0)
        {
            handle_error("Error creating thread");
        }
    }

    // Join threads
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            handle_error("Error joining thread");
        }
    }
}

int main()
{
    int client_socket;
    // Create a UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        handle_error("Error creating socket");
    }

    create_and_join_threads(client_socket);

    // Close the socket
    if (close(client_socket) != 0)
    {
        handle_error("Error closing socket");
    }

    return 0;
}
