#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 80

int main()
{
    int server_socket;
    struct sockaddr_in server_address;

    // Try to bind to a reserved port (replace 80 with the desired port)
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Bind to localhost
    server_address.sin_port = htons(PORT);                     // Example reserved port 80

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error binding to the reserved port");
        printf("Error code: %d\n", errno);
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Successfully bound to the reserved port.\n");
    close(server_socket);
    return 0;
}

// Error code 13 corresponds to EACCES(Permission denied), indicating that the process does not have the necessary privileges to bind to a port below 1024.
