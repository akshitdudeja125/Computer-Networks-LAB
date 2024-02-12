#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>

#define PORT 8085
#define MAX_FILENAME_LEN 256
#define MAX_BUFFER_SIZE 1024
#define IP "127.0.0.1"
#define FOLDER_PATH "client_files"
#define SLEEP_TIME 1

int send_file(const char *filename, long *file_size)
{
    printf("Sending file: %s\n", filename);
    int sock_fd;
    struct sockaddr_in server_addr;
    ssize_t bytes_sent;

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
    if (send(sock_fd, filename, strlen(filename), 0) == -1)
    {
        perror("Send failed");
        close(sock_fd);
        return -1;
    }

    sleep(SLEEP_TIME);

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
    while (1)
    {
        char buffer[MAX_BUFFER_SIZE];
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), fptr);
        if (bytes_read < 0)
        {
            perror("File read failed");
            fclose(fptr);
            close(sock_fd);
            return -1;
        }
        if (bytes_read == 0)
        {
            break;
        }
        if (send(sock_fd, buffer, bytes_read, 0) == -1)
        {
            perror("Send failed");
            fclose(fptr);
            close(sock_fd);
            return -1;
        }
        *file_size += bytes_read; // Add the size of current chunk to file size
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

    struct timeval start_time, end_time;
    double total_time;

    gettimeofday(&start_time, NULL); // Start the timer

    // Open directory
    if ((dir = opendir(FOLDER_PATH)) == NULL)
    {
        perror("Failed to open directory");
        exit(EXIT_FAILURE);
    }
    int count_files = 0;
    long total_file_size = 0; // Variable to hold the total size of all files
    // Send files sequentially
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char *filename = entry->d_name;
            count_files++;
            long file_size;
            if (send_file(filename, &file_size) == -1)
            {
                printf("Failed to send file: %s\n", filename);
            }
            else
            {
                total_file_size += file_size; // Update total file size
            }
        }
    }

    closedir(dir);

    gettimeofday(&end_time, NULL); // End the timer

    total_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("Total time taken: %.6f seconds\n", total_time - (count_files * SLEEP_TIME));
    printf("Total size of all files: %ld bytes\n", total_file_size);

    return 0;
}
