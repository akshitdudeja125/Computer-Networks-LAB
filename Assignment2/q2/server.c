#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "postfix_evaluator.h"

#define PORT 8080
#define MAX_PENDING_CONNECTIONS 5
#define MAX_BUFFER_SIZE 1024
#define IP "10.10.88.233"

void *handle_client(void *arg)
{
    int client_socket = *((int *)arg);
    free(arg); // Free memory allocated for the argument

    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;

    // Receive expression from client
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received < 0)
    {
        perror("Error receiving data from client");
        close(client_socket);
        pthread_exit(NULL);
    }
    else if (bytes_received == 0)
    {
        // Connection closed by client
        close(client_socket);
        pthread_exit(NULL);
    }

    buffer[bytes_received] = '\0';
    // Evaluate expression
    int errorCode = 0;                  // Initialize errorCode to 0
    char errorMessage[MAX_BUFFER_SIZE]; // Initialize errorMessage buffer
    double result = evaluatePostfix(buffer, &errorCode, errorMessage);
    printf("Result %lf\n", result);
    printf("Error Code %d\n", errorCode);
    if (errorCode == 0)
    {
        // Send result to client
        char result_str[MAX_BUFFER_SIZE];
        int result_length = snprintf(result_str, sizeof(result_str), "%lf", result);
        if (result_length < 0 || result_length >= sizeof(result_str))
        {
            perror("Error formatting result");
        }
        if (send(client_socket, result_str, strlen(result_str), 0) < 0)
        {
            perror("Error sending result to client");
            close(client_socket);
            pthread_exit(NULL);
        }
    }
    else
    {
        if (send(client_socket, errorMessage, strlen(errorMessage), 0) < 0)
        {
            perror("Error sending result to client");
            close(client_socket);
            pthread_exit(NULL);
        }
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, IP, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Couldn't bind socket to address");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_PENDING_CONNECTIONS) < 0)
    {
        perror("Error listening for connections");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Accept connection from client
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Error accepting connection");
            continue;
        }

        // Create thread to handle client request
        pthread_t tid;
        int *client_socket_ptr = (int *)malloc(sizeof(int));
        if (client_socket_ptr == NULL)
        {
            perror("Error allocating memory");
            close(client_socket);
            continue;
        }
        *client_socket_ptr = client_socket;
        if (pthread_create(&tid, NULL, handle_client, (void *)client_socket_ptr) != 0)
        {
            perror("Error creating thread");
            close(client_socket);
            free(client_socket_ptr);
            continue;
        }

        // Detach thread
        pthread_detach(tid);
    }

    close(server_socket);
    return 0;
}
