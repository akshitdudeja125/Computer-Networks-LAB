#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <dirent.h>

#define PORT 8085
#define MAX_FILENAME_LEN 256
#define MAX_BUFFER_SIZE 1024
#define IP "127.0.0.1"
#define FOLDER_PATH "client_files_1"

int send_file(const char *filename)
{
    printf("Sending file: %s\n", filename);
    int sock_fd;
    struct sockaddr_in server_addr;
    ssize_t bytes_sent;
    char buffer[MAX_BUFFER_SIZE];

    // Connect to server
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address");
        return -1;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Connection failed");
        close(sock_fd);
        return -1;
    }

    // Send filename
    if ((bytes_sent = send(sock_fd, filename, strlen(filename), 0)) == -1)
    {
        perror("Send failed");
        close(sock_fd);
        return -1;
    }

    // Open file
    char filePath[MAX_BUFFER_SIZE];
    snprintf(filePath, MAX_BUFFER_SIZE, "%s/%s", FOLDER_PATH, filename);

    FILE *fptr = fopen(filePath, "rb");
    if (!fptr)
    {
        perror("File open failed");
        close(sock_fd);
        return -1;
    }

    // Send file data
    while (!feof(fptr))
    {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), fptr);
        if (bytes_read == -1)
        {
            perror("File read failed");
            fclose(fptr);
            close(sock_fd);
            return -1;
        }
        if (send(sock_fd, buffer, bytes_read, 0) == -1)
        {
            perror("Send failed");
            fclose(fptr);
            close(sock_fd);
            return -1;
        }
    }

    // Close file and connection
    fclose(fptr);
    close(sock_fd);

    printf("File sent successfully: %s\n", filename);

    return 0;
}

int main()
{
    DIR *dir;
    struct dirent *entry;

    // Open directory
    if ((dir = opendir(FOLDER_PATH)) == NULL)
    {
        perror("Failed to open directory");
        exit(EXIT_FAILURE);
    }

    // Send files sequentially
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char *filename = entry->d_name;
            if (send_file(filename) == -1)
            {
                printf("Failed to send file: %s\n", filename);
            }
        }
    }

    closedir(dir);

    return 0;
}