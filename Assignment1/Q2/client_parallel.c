#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
// #include <fcntl.h>

#define SERVER_IP "127.0.0.1"
int SERVER_PORT[] = {8085, 8086};
int NUM_SERVERS = sizeof(SERVER_PORT) / sizeof(SERVER_PORT[0]);
#define MESSAGE_INTERVAL_SECONDS 2
#define NUM_MESSAGES 5
#define MAX_BUFFER_SIZE 1024

// Thread function for sending messages
void *send_message(void *args)
{
    int server_index = ((int *)args)[0];
    int client_socket = ((int *)args)[1];
    struct sockaddr_in server_address;

    // Set up the server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    // server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0)
    {
        perror("Invalid address\n\n");
        pthread_exit(NULL);
    }
    server_address.sin_port = htons(SERVER_PORT[server_index]);
    socklen_t server_address_len = sizeof(server_address);

    // if (connect(client_socket, (struct sockaddr *)&server_address, server_address_len) < 0)
    // {
    //     printf("\n Error : Connect Failed \n");
    //     pthread_exit(NULL);
    // }

    for (int seq_num = 1; seq_num <= NUM_MESSAGES; ++seq_num)
    {
        // Prepare the message with sequence number
        char message[MAX_BUFFER_SIZE];
        sprintf(message, "Message %d from client\n\n", seq_num);

        // Send the message to the server
        ssize_t send_len = sendto(client_socket, message, strlen(message), 0, (struct sockaddr *)&server_address, server_address_len);

        // Debug
        // printf("%ld : %d : %d\n\n", send_len, SERVER_PORT[server_index], seq_num);

        if (send_len < 0)
        {
            printf("Error sending data to server with port: %d\n\n", SERVER_PORT[server_index]);
            close(client_socket);
            break;
        }

        printf("Sent message to server %s:%d - Sequence Number: %d\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port), seq_num);

        char buffer[MAX_BUFFER_SIZE];
        // Receive data from the server
        ssize_t recv_len = recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_address, &server_address_len);

        // Debug
        // printf("%ld : %d : %d\n\n", recv_len, SERVER_PORT[server_index], seq_num);

        if (recv_len < 0)
        {
            perror("Error receiving data!\n\n");
            break;
        }

        buffer[recv_len] = '\0';
        printf("Received message from server %s:%d: %s\n\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port), buffer);

        // Wait for the specified interval before sending the next message
        sleep(MESSAGE_INTERVAL_SECONDS);
    }

    pthread_exit(NULL);
}

int main()
{
    int client_socket;
    // Create a UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("Error creating socket\n\n");
        pthread_exit(NULL);
    }

    // Make the socket non-blocking
    // int flags = fcntl(client_socket, F_GETFL, 0);
    // if (flags == -1)
    // {
    //     perror("Error getting socket flags\n\n");
    //     close(client_socket);
    //     pthread_exit(NULL);
    // }

    // if (fcntl(client_socket, F_SETFL, flags | O_NONBLOCK) == -1)
    // {
    //     perror("Error setting socket to non-blocking\n\n");
    //     close(client_socket);
    //     pthread_exit(NULL);
    // }

    pthread_t threads[NUM_SERVERS];
    int thread_args[NUM_SERVERS][2];

    // Create threads for each server
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        thread_args[i][0] = i;
        thread_args[i][1] = client_socket;
        if (pthread_create(&threads[i], NULL, send_message, (void *)&thread_args[i]) != 0)
        {
            perror("Error creating thread\n\n");
            exit(EXIT_FAILURE);
        }
    }

    // Join threads
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            perror("Error joining thread\n\n");
            exit(EXIT_FAILURE);
        }
    }

    // Close the socket
    close(client_socket);

    return 0;
}
