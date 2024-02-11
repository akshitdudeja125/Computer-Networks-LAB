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

#define PORT 8085
#define MAX_CONN 10
#define MAX_FILENAME_LEN 256
#define MAX_BUFFER_SIZE 1024
#define IP "127.0.0.1"
#define FOLDER_PATH "client_files"

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
    // server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
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
    if ((bytes_sent = send(sock_fd, file, strlen(file), 0)) == -1)
    {
        perror("Send failed");
        close(sock_fd);
        pthread_exit(NULL);
    }

    sleep(2);

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
    while (!feof(fptr))
    {
        char buffer[MAX_BUFFER_SIZE];
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), fptr);
        if (bytes_read == -1)
        {
            perror("File read failed");
            fclose(fptr);
            close(sock_fd);
            pthread_exit(NULL);
        }
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

    int num_files = 0;

    // Count number of files
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
            num_files++;
    }
    // closedir(dir);

    printf("Number of files to send: %d\n", num_files);

    // Send files in parallel
    pthread_t threads[num_files];
    int i = 0;

    rewinddir(dir);

    // Create threads to send files
    while ((entry = readdir(dir)) != NULL)
    {
        // char filename[MAX_FILENAME_LEN];
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

    return 0;
}

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <dirent.h>

// #define PORT 8085
// #define MAX_FILENAME_LEN 256
// #define MAX_BUFFER_SIZE 1024
// #define IP "127.0.0.1"
// #define FOLDER_PATH "client_files"

// void send_file(const char *filename)
// {
//     printf("Sending file: %s\n", filename);
//     int sock_fd;
//     struct sockaddr_in server_addr;
//     ssize_t bytes_sent;
//     char buffer[MAX_BUFFER_SIZE];

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
//     if ((bytes_sent = send(sock_fd, filename, strlen(filename), 0)) == -1)
//     {
//         perror("Send failed");
//         close(sock_fd);
//         exit(EXIT_FAILURE);
//     }

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
//     while (!feof(fptr))
//     {
//         size_t bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE, fptr);
//         if (bytes_read == -1)
//         {
//             perror("File read failed");
//             fclose(fptr);
//             close(sock_fd);
//             exit(EXIT_FAILURE);
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
// }

// int main()
// {
//     DIR *dir;
//     struct dirent *entry;

//     // Open directory
//     if ((dir = opendir(FOLDER_PATH)) == NULL)
//     {
//         perror("Failed to open directory");
//         exit(EXIT_FAILURE);
//     }

//     // Send files in parallel
//     while ((entry = readdir(dir)) != NULL)
//     {
//         if (entry->d_type == DT_REG)
//         {
//             // Fork a child process to send each file
//             pid_t child_pid = fork();
//             if (child_pid == -1)
//             {
//                 perror("Fork failed");
//                 continue;
//             }
//             else if (child_pid == 0)
//             {
//                 // Child process
//                 char *filename = entry->d_name;
//                 send_file(filename);
//                 exit(EXIT_SUCCESS);
//             }
//             // Parent process continues to fork new child processes
//         }
//     }

//     closedir(dir);

//     return 0;
// }