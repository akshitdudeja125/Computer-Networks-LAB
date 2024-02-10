#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 2048

void receive_folder(int server_socket, const char *folder_path)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while (1)
    {
        bytes_received = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);

        if (bytes_received == -1)
        {
            perror("Error receiving data");
            break;
        }

        if (bytes_received == 0)
        {
            printf("Folder received successfully.\n");
            break;
        }

        buffer[bytes_received] = '\0';
        // print file name
        printf("File name: %s\n", buffer);

        // Create directories and files
        FILE *file = fopen(buffer, "wb");
        if (file == NULL)
        {
            perror("Error creating file");
            exit(EXIT_FAILURE);
        }

        while (1)
        {
            bytes_received = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);

            if (bytes_received == -1)
            {
                perror("Error receiving data");
                break;
            }

            if (bytes_received == 0)
            {
                // End of file
                fclose(file);
                break;
            }

            fwrite(buffer, 1, bytes_received, file);
            buffer[bytes_received] = '\0';
        }
    }
}

int main()
{
    int server_socket;
    struct sockaddr_in server_addr;

    const char *folder_path = "received_folder"; // Adjust the folder name as needed
    mkdir(folder_path, 0777);                    // Create the folder
    chdir(folder_path);                          // Change the working directory to the created folder

    // Create socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to a specific port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("10.10.88.233");
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", SERVER_PORT);

    receive_folder(server_socket, folder_path);

    close(server_socket);
    return 0;
}