#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_BUFFER_SIZE 1024
#define NUM_MESSAGES 5

int main()
{
    int clientSocket;

    // Create a UDP socket
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    else
        printf("Client socket created successfully\n");

    // Details of the five UDP servers with the same IP
    const char *serverIP = "127.0.0.1";
    const int serverPorts[] = {8085, 8086}; // Port numbers of the servers
    const int nServers = sizeof(serverPorts) / sizeof(serverPorts[0]);
    struct sockaddr_in serverAddr[nServers];

    for (int i = 0; i < nServers; i++)
    {
        // Set up the server address
        memset(&serverAddr[i], 0, sizeof(serverAddr[i]));
        serverAddr[i].sin_family = AF_INET;

        // Convert IP address from string to binary form
        if (inet_pton(AF_INET, serverIP, &serverAddr[i].sin_addr) <= 0)
        {
            perror("Invalid address");
            exit(EXIT_FAILURE);
        }

        // Set the server port
        serverAddr[i].sin_port = htons(serverPorts[i]);
    }

    int seq_num = 1;

    while (1)
    {
        for (int i = 0; i < nServers; i++)
        {

            char buffer[MAX_BUFFER_SIZE];
            socklen_t addrLen = sizeof(serverAddr[i]);

            // Create the message with sequence number
            sprintf(buffer, "Message %d from client\n\n", seq_num);

            // Send the message to the server
            if (sendto(clientSocket, buffer, strlen(buffer), 0, (struct sockaddr *)&serverAddr[i], addrLen) == -1)
            {
                perror("Error sending message");
                break;
            }

            printf("Sent message to server %s:%d - Sequence Number: %d\n", inet_ntoa(serverAddr[i].sin_addr), ntohs(serverAddr[i].sin_port), seq_num);

            int recv_len = recvfrom(clientSocket, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&serverAddr[i], &addrLen);
            // printf("%d\n", recv_len);

            if (recv_len <= 0)
            {
                perror("Error receiving message");
                exit(EXIT_FAILURE);
            }

            buffer[recv_len] = '\0'; // Null-terminate the received data
            printf("Received message from server %s:%d: %s\n\n", inet_ntoa(serverAddr[i].sin_addr), ntohs(serverAddr[i].sin_port), buffer);
        }

        if (seq_num == NUM_MESSAGES)
            break;

        sleep(2); // Sleep for 2 seconds
        seq_num++;
    }

    // Close the socket
    close(clientSocket);

    return 0;
}
