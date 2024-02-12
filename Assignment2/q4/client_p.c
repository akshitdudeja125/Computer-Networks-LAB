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
#include <sys/time.h>

#define PORT 8085
#define MAX_FILENAME_LEN 256
#define MAX_BUFFER_SIZE 1024
#define IP "127.0.0.1"
#define FOLDER_PATH "client_files"
#define SLEEP_TIME 1
long max_file_size = 0;

void *send_file(void *filename)
{
    printf("Sending file: %s\n", (char *)filename);
    char *file = (char *)filename;
    int sock_fd;
    struct sockaddr_in server_addr;
    ssize_t bytes_sent;

    // Connect to server
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Connection failed");
        close(sock_fd);
        pthread_exit(NULL);
    }

    // Send filename
    if (send(sock_fd, file, strlen(file), 0) == -1)
    {
        perror("Send failed");
        close(sock_fd);
        pthread_exit(NULL);
    }

    sleep(SLEEP_TIME);

    // Open file
    char filePath[MAX_BUFFER_SIZE];
    snprintf(filePath, MAX_BUFFER_SIZE, "%s/%s", FOLDER_PATH, file);

    FILE *fptr = fopen(filePath, "rb");
    if (!fptr)
    {
        perror("File open failed");
        close(sock_fd);
        pthread_exit(NULL);
    }

    // Send file data
    long file_size_temp = 0;
    while (1)
    {
        char buffer[MAX_BUFFER_SIZE];
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), fptr);
        if (bytes_read < 0)
        {
            perror("File read failed");
            fclose(fptr);
            close(sock_fd);
            pthread_exit(NULL);
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
            pthread_exit(NULL);
        }
        file_size_temp += bytes_read;
    }

    if (max_file_size < file_size_temp)
        max_file_size = file_size_temp;

    // Close file and connection
    fclose(fptr);
    close(sock_fd);

    printf("File sent successfully: %s\n", file);

    pthread_exit(NULL);
}

int main()
{
    DIR *dir;
    struct dirent *entry;

    // Start time measurement
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Open directory
    if ((dir = opendir(FOLDER_PATH)) == NULL)
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

    printf("Number of files to send: %d\n", num_files);

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

    // End time measurement
    gettimeofday(&end, NULL);
    double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;

    printf("Time taken to send all files: %.6f seconds\n", time_taken - SLEEP_TIME);
    printf("Max size of all files: %ld bytes\n", max_file_size);

    return 0;
}

// ************** Using fork() **************

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <dirent.h>
// #include <sys/time.h>

// #define PORT 8085
// #define MAX_FILENAME_LEN 256
// #define MAX_BUFFER_SIZE 1024
// #define IP "127.0.0.1"
// #define FOLDER_PATH "client_files"
// #define SLEEP_TIME 1

// void send_file(const char *filename)
// {
//     printf("Sending file: %s\n", filename);
//     int sock_fd;
//     struct sockaddr_in server_addr;
//     ssize_t bytes_sent;

//     // Connect to server
//     if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
//     {
//         perror("Socket creation failed");
//         exit(EXIT_FAILURE);
//     }

//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(PORT);
//     if (inet_pton(AF_INET, IP, &(server_addr.sin_addr)) <= 0)
//     {
//         perror("Invalid address");
//         exit(EXIT_FAILURE);
//     }

//     if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
//     {
//         perror("Connection failed");
//         close(sock_fd);
//         exit(EXIT_FAILURE);
//     }

//     // Send filename
//     if (send(sock_fd, filename, strlen(filename), 0) == -1)
//     {
//         perror("Send failed");
//         close(sock_fd);
//         exit(EXIT_FAILURE);
//     }

//     sleep(SLEEP_TIME);

//     // Open file
//     char filePath[MAX_BUFFER_SIZE];
//     snprintf(filePath, MAX_BUFFER_SIZE, "%s/%s", FOLDER_PATH, filename);

//     FILE *fptr = fopen(filePath, "rb");
//     if (!fptr)
//     {
//         perror("File open failed");
//         close(sock_fd);
//         exit(EXIT_FAILURE);
//     }

//     // Send file data
//     while (1)
//     {
//         char buffer[MAX_BUFFER_SIZE];
//         size_t bytes_read = fread(buffer, 1, sizeof(buffer), fptr);
//         if (bytes_read < 0)
//         {
//             perror("File read failed");
//             fclose(fptr);
//             close(sock_fd);
//             exit(EXIT_FAILURE);
//         }
//         if (bytes_read == 0)
//         {
//             break;
//         }
//         if (send(sock_fd, buffer, bytes_read, 0) == -1)
//         {
//             perror("Send failed");
//             fclose(fptr);
//             close(sock_fd);
//             exit(EXIT_FAILURE);
//         }
//     }

//     // Close file and connection
//     fclose(fptr);
//     close(sock_fd);

//     printf("File sent successfully: %s\n", filename);

//     exit(EXIT_SUCCESS);
// }

// int main()
// {
//     DIR *dir;
//     struct dirent *entry;

//     // Start time measurement
//     struct timeval start, end;
//     gettimeofday(&start, NULL);

//     // Open directory
//     if ((dir = opendir(FOLDER_PATH)) == NULL)
//     {
//         perror("Failed to open directory");
//         exit(EXIT_FAILURE);
//     }

//     int num_files = 0;

//     // Count number of files
//     while ((entry = readdir(dir)) != NULL)
//     {
//         if (entry->d_type == DT_REG)
//             num_files++;
//     }

//     printf("Number of files to send: %d\n", num_files);

//     // Fork processes to send files
//     rewinddir(dir);

//     while ((entry = readdir(dir)) != NULL)
//     {
//         if (entry->d_type == DT_REG)
//         {
//             char *filename = entry->d_name;
//             pid_t pid = fork();
//             if (pid == -1)
//             {
//                 perror("Fork failed");
//                 continue;
//             }
//             else if (pid == 0) // Child process
//             {
//                 send_file(filename);
//             }
//         }
//     }

//     closedir(dir);

//     // Wait for all child processes to finish
//     int status;
//     for (int i = 0; i < num_files; i++)
//     {
//         wait(&status);
//     }

//     // End time measurement
//     gettimeofday(&end, NULL);
//     double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
//     time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;

//     printf("Time taken to send all files: %.6f seconds\n", time_taken - SLEEP_TIME);

//     return 0;
// }
