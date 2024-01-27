// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include <unistd.h>

// #define SERVER_IP "127.0.0.1"
// // #define SERVER_PORT 12345
// int SERVER_PORT[] = {12345, 12346};
// int NUM_SERVERS = sizeof(SERVER_PORT) / sizeof(SERVER_PORT[0]);
// #define MESSAGE_INTERVAL_SECONDS 2
// #define NUM_MESSAGES 5

// int main()
// {
//     int client_socket;
//     struct sockaddr_in server_address[NUM_SERVERS];

//     // Create a UDP socket
//     client_socket = socket(AF_INET, SOCK_DGRAM, 0);
//     if (client_socket < 0)
//     {
//         perror("Error creating socket");
//         exit(EXIT_FAILURE);
//     }

//     // Set up the server address
//     for (int i = 0; i < NUM_SERVERS; i++)
//     {
//         memset(&server_address[i], 0, sizeof(server_address[i]));
//         server_address[i].sin_family = AF_INET;
//         server_address[i].sin_addr.s_addr = inet_addr(SERVER_IP);
//         server_address[i].sin_port = htons(SERVER_PORT[i]);
//     }

//     for (int seq_num = 1; seq_num <= NUM_MESSAGES; ++seq_num)
//     {
//         // Prepare the message with sequence number
//         char message[50];
//         char buffer[1024];
//         sprintf(message, "Message %d from client", seq_num);

//         // Send the message to the server
//         for (int i = 0; i < NUM_SERVERS; i++)
//         {
//             socklen_t server_address_len = sizeof(server_address[i]);
//             ssize_t send_len = sendto(client_socket, message, strlen(message), 0, (struct sockaddr *)&server_address[i], server_address_len);

//             if (send_len < 0)
//             {
//                 printf("Error sending data to server with port: %d", SERVER_PORT[i]);
//                 close(client_socket);
//                 exit(EXIT_FAILURE);
//             }

//             printf("Sent message %d to server with port: %d\n", seq_num, SERVER_PORT[i]);

//             // Receive data from the server
//             ssize_t recv_len = recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_address[i], &server_address_len);

//             if (recv_len < 0)
//             {
//                 perror("Error receiving data");
//                 close(client_socket);
//                 exit(EXIT_FAILURE);
//             }

//             buffer[recv_len] = '\0';
//             printf("Received message from client %s:%d: %s\n", inet_ntoa(server_address[i].sin_addr), ntohs(server_address[i].sin_port), buffer);
//         }

//         // Wait for the specified interval before sending the next message
//         sleep(MESSAGE_INTERVAL_SECONDS);
//     }

//     // Close the socket
//     close(client_socket);

//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
int SERVER_PORT[] = {12345, 12346};
int NUM_SERVERS = sizeof(SERVER_PORT) / sizeof(SERVER_PORT[0]);
#define MESSAGE_INTERVAL_SECONDS 2
#define NUM_MESSAGES 5

// Thread function for sending messages
void *send_message(void *args)
{
    int server_index = *((int *)args);
    int client_socket;
    struct sockaddr_in server_address;

    // Create a UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("Error creating socket");
        pthread_exit(NULL);
    }

    // Set up the server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    // server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0)
    {
        perror("Invalid address");
        pthread_exit(NULL);
    }
    server_address.sin_port = htons(SERVER_PORT[server_index]);
    socklen_t server_address_len = sizeof(server_address);

    if (connect(client_socket, (struct sockaddr *)&server_address, server_address_len) < 0)
    {
        printf("\n Error : Connect Failed \n");
        pthread_exit(NULL);
    }

    for (int seq_num = 1; seq_num <= NUM_MESSAGES; ++seq_num)
    {
        // Prepare the message with sequence number
        char message[50];
        sprintf(message, "Message %d from client", seq_num);
        char buffer[1024];

        // Send the message to the server
        ssize_t send_len = sendto(client_socket, message, strlen(message), 0, (struct sockaddr *)&server_address, server_address_len);
        // printf("%ld : %d\n", send_len, SERVER_PORT[server_index]);

        if (send_len < 0)
        {
            printf("Error sending data to server with port: %d\n", SERVER_PORT[server_index]);
            close(client_socket);
            break;
        }

        printf("Sent message %d to server with port: %d\n", seq_num, SERVER_PORT[server_index]);

        // Receive data from the server
        ssize_t recv_len = recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_address, &server_address_len);
        // printf("%ld : %d\n", recv_len, SERVER_PORT[server_index]);

        if (recv_len < 0)
        {
            perror("Error receiving data!");
            close(client_socket);
            break;
        }

        buffer[recv_len] = '\0';
        printf("Received message from client %s:%d: %s\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port), buffer);

        // Wait for the specified interval before sending the next message
        sleep(MESSAGE_INTERVAL_SECONDS);
    }

    // Close the socket
    close(client_socket);

    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[NUM_SERVERS];
    int thread_args[NUM_SERVERS];

    // Create threads for each server
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        thread_args[i] = i;
        if (pthread_create(&threads[i], NULL, send_message, (void *)&thread_args[i]) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }

    // Join threads
    for (int i = 0; i < NUM_SERVERS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
