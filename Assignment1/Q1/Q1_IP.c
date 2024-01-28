#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define IP "135.123.2.26"

int main()
{
    int server_socket;
    struct sockaddr_in server_address;

    // Try to bind to a specific IP address (replace 'x.x.x.x' with the desired IP)
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    // server_address.sin_addr.s_addr = inet_addr("135.123.2.26");
    if (inet_pton(AF_INET, IP, &(server_address.sin_addr)) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    server_address.sin_port = htons(8080); // Example port 8080

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error binding to the specified IP address");
        printf("Error code: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    printf("Successfully bound to the specified IP address.\n");
    close(server_socket);
    return 0;
}

// Error code 99 corresponds to the EADDRNOTAVAIL error.This error code indicates that the requested address is not available on the machine.
