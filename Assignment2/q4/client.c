#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CONN 10
#define MAX_FILENAME_LEN 256

void *send_file(void *filename);

int main()
{
    DIR *dir;
    struct dirent *entry;

    // Open directory
    if ((dir = opendir(".")) == NULL)
    {
        perror("Failed to open directory");
        exit(EXIT_FAILURE);
    }

    int num_files = 0;

    // Count number of files
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
            num_files++;
    }
    closedir(dir);

    printf("Number of files to send: %d\n", num_files);

    // Connect to server
    int sock_fd;
    struct sockaddr_in server_addr;

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    // Send files in parallel
    pthread_t threads[num_files];
    int i = 0;

    rewinddir(dir);

    // Create threads to send files
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char *filename = entry->d_name;
            pthread_create(&threads[i], NULL, send_file, (void *)filename);
            i++;
        }
    }
    closedir(dir);

    // Join threads
    for (int j = 0; j < num_files; j++)
    {
        pthread_join(threads[j], NULL);
    }

    // Close connection
    close(sock_fd);

    return 0;
}

void *send_file(void *filename)
{
    char *file = (char *)filename;
    int sock_fd;
    struct sockaddr_in server_addr;
    ssize_t bytes_sent;
    char buffer[1024];

    // Connect to server
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Connection failed");
        close(sock_fd);
        pthread_exit(NULL);
    }

    // Send filename
    if ((bytes_sent = send(sock_fd, file, strlen(file), 0)) == -1)
    {
        perror("Send failed");
        close(sock_fd);
        pthread_exit(NULL);
    }

    // Open file
    FILE *fptr = fopen(file, "rb");
    if (!fptr)
    {
        perror("File open failed");
        close(sock_fd);
        pthread_exit(NULL);
    }

    // Send file data
    while (!feof(fptr))
    {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), fptr);
        if (send(sock_fd, buffer, bytes_read, 0) == -1)
        {
            perror("Send failed");
            fclose(fptr);
            close(sock_fd);
            pthread_exit(NULL);
        }
    }

    // Close file and connection
    fclose(fptr);
    close(sock_fd);

    printf("File sent successfully: %s\n", file);

    pthread_exit(NULL);
}
