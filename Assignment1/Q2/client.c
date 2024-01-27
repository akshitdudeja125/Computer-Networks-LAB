// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <pthread.h>

// #define MAX_BUFFER_SIZE 1024

// int main()
// {
//     int clientSocket;

//     // Create a UDP socket
//     if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
//     {
//         perror("Error creating socket");
//         exit(EXIT_FAILURE);
//     }
//     else
//     {
//         printf("Client socket created successfully\n");
//     }

//     // Details of the five UDP servers with the same IP
//     const char *serverIP = "10.10.88.233";
//     const int serverPorts[] = {8080, 8081}; // Port numbers of the servers
//     const int nServers = sizeof(serverPorts) / sizeof(serverPorts[0]);

//     while (1)
//     {
//         for (int i = 0; i < nServers; i++)
//         {
//             struct sockaddr_in serverAddr;

//             // Set up the server address
//             memset(&serverAddr, 0, sizeof(serverAddr));
//             serverAddr.sin_family = AF_INET;

//             // Convert IP address from string to binary form
//             if (inet_pton(AF_INET, serverIP, &serverAddr.sin_addr) <= 0)
//             {
//                 perror("Invalid address");
//                 exit(EXIT_FAILURE);
//             }

//             serverAddr.sin_port = htons(serverPorts[i]); // Set the server port

//             char buffer[MAX_BUFFER_SIZE];
//             int sequenceNumber = 1;

//             // Create the message with sequence number
//             snprintf(buffer, sizeof(buffer), "%d", sequenceNumber);

//             // Send the message to the server
//             if (sendto(clientSocket, buffer, strlen(buffer), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
//             {
//                 perror("Error sending message");
//                 break;
//             }

//             printf("Sent message to server %s:%d - Sequence Number: %d\n", serverIP, serverPorts[i], sequenceNumber);

//             socklen_t addrLen = sizeof(serverAddr);
//             int recv_len = recvfrom(clientSocket, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&serverAddr, &addrLen);
//             printf("recv from %s:%d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
//             printf("%d\n", recv_len);
//             if (recv_len <= 0)
//             {
//                 perror("Error receiving message");
//                 exit(EXIT_FAILURE);
//             }

//             buffer[recv_len] = '\0'; // Null-terminate the received data
//             printf("Received response from server %s:%d - %s\n", serverIP, serverPorts[i], buffer);

//             sequenceNumber++;

//             sleep(2); // Sleep for 2 seconds
//         }
//     }

//     // Close the socket
//     close(clientSocket);

//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_BUFFER_SIZE 1024

struct ThreadData
{
    int clientSocket;
    const char *serverIP;
    int serverPort;
};

void *sendToServer(void *arg)
{
    struct ThreadData *threadData = (struct ThreadData *)arg;
    struct sockaddr_in serverAddr;

    // Set up the server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, threadData->serverIP, &serverAddr.sin_addr) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_port = htons(threadData->serverPort); // Set the server port

    char buffer[MAX_BUFFER_SIZE];
    int sequenceNumber = 1;

    socklen_t addrLen = sizeof(serverAddr);
    if (connect(client_socket, (struct sockaddr *)&server_address, addrLen) < 0)
    {
        printf("\n Error : Connect Failed \n");
        pthread_exit(NULL);
    }

    while (1)
    {
        // Create the message with sequence number
        snprintf(buffer, sizeof(buffer), "%d", sequenceNumber);

        // Send the message to the server
        if (sendto(threadData->clientSocket, buffer, strlen(buffer), 0, (struct sockaddr *)&serverAddr, addrLen) == -1)
        {
            perror("Error sending message");
            break;
        }

        printf("Sent message to server %s:%d - Sequence Number: %d\n", threadData->serverIP, threadData->serverPort, sequenceNumber);

        int recv_len = recvfrom(threadData->clientSocket, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&serverAddr, &addrLen);
        printf("recv from %s:%d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
        printf("%d\n", recv_len);
        if (recv_len <= 0)
        {
            perror("Error receiving message");
            pthread_exit(NULL);
        }

        buffer[recv_len] = '\0'; // Null-terminate the received data
        printf("Received response from server %s:%d - %s\n", threadData->serverIP, threadData->serverPort, buffer);

        sequenceNumber++;
        if (sequenceNumber <= 5)
            break;

        sleep(2); // Sleep for 2 seconds
    }

    pthread_exit(NULL);
}

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
    {
        printf("Client socket created successfully\n");
    }

    // Details of the five UDP servers with the same IP
    const char *serverIP = "10.10.88.233";
    const  int serverPorts[] = {8080}; // Port numbers of the servers
    const int nServers = sizeof(serverPorts) / sizeof(serverPorts[0]);

    // Create threads for each server
    pthread_t threads[nServers];
    struct ThreadData threadData[nServers];

    for (int i = 0; i < nServers; ++i)
    {
        threadData[i].clientSocket = clientSocket;
        threadData[i].serverIP = serverIP;
        threadData[i].serverPort = serverPorts[i];

        if (pthread_create(&threads[i], NULL, sendToServer, (void *)&threadData[i]) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < 2; ++i)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            perror("Error joining thread");
            exit(EXIT_FAILURE);
        }
    }

    // Close the socket
    close(clientSocket);

    return 0;
}
