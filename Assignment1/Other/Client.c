#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define MESSAGE_INTERVAL_SECONDS 2
#define NUM_MESSAGES 5

int main()
{
    int client_socket;
    struct sockaddr_in server_address;

    // Create a UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address.sin_port = htons(SERVER_PORT);

    for (int seq_num = 1; seq_num <= NUM_MESSAGES; ++seq_num)
    {
        // Prepare the message with sequence number
        char message[50];
        sprintf(message, "Message %d from client", seq_num);

        // Send the message to the server
        ssize_t send_len = sendto(client_socket, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));

        if (send_len < 0)
        {
            perror("Error sending data");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        printf("Sent message %d to server\n", seq_num);

        // Wait for the specified interval before sending the next message
        sleep(MESSAGE_INTERVAL_SECONDS);
    }

    // Close the socket
    close(client_socket);

    return 0;
}