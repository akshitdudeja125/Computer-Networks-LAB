#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define IP "10.10.88.233"

void send_expression(const char *expression)
{
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
        exit(EXIT_FAILURE);

    // Set up server address
    server_addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, IP, &(server_addr.sin_addr)) <= 0)
    {
        // handle_error("Invalid address");
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Send expression to server
    send(client_socket, expression, strlen(expression), 0);

    // Receive result from server
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received < 0)
    {
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_received] = '\0';
    printf("Result from server: %s\n", buffer);

    close(client_socket);
}

int main()
{
    char expression[1024];

    printf("Enter the postfix expression: ");
    fgets(expression, sizeof(expression), stdin);
    expression[strcspn(expression, "\n")] = '\0'; // Remove newline character

    send_expression(expression);

    return 0;
}
