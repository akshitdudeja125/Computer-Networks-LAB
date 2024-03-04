#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 3005
#define BUFFER_SIZE 1e9
#define IP "127.0.0.1"

const char *extractFileExtension(const char *file_name)
{
    const char *dot = strrchr(file_name, '.');
    if (!dot || dot == file_name)
        return "";
    return dot + 1;
}

const char *generateMimeType(const char *file_ext)
{
    if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0)
        return "text/html";
    else if (strcasecmp(file_ext, "txt") == 0)
        return "text/plain";
    else if (strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0)
        return "image/jpeg";
    else if (strcasecmp(file_ext, "png") == 0)
        return "image/png";
    else
        return "application/octet-stream";
}

char *decodeURL(const char *src)
{
    size_t src_len = strlen(src);
    char *decoded = (char *)malloc(src_len + 1);
    size_t decoded_len = 0;

    for (size_t i = 0; i < src_len; i++)
    {
        if (src[i] == '%' && i + 2 < src_len)
        {
            char hex_val_str[3] = {src[i + 1], src[i + 2], '\0'};
            int hex_val = (int)strtol(hex_val_str, NULL, 16);
            decoded[decoded_len++] = (char)hex_val;
            i += 2;
        }
        else
            decoded[decoded_len++] = src[i];
    }

    decoded[decoded_len] = '\0';
    return decoded;
}

void generateResponse(const char *file_name, const char *file_ext, char *response, size_t *response_len)
{
    const char *mime_type = generateMimeType(file_ext);
    char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(header, BUFFER_SIZE, "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: %s\r\n\r\n",
             mime_type);

    int file_fd = open(file_name, O_RDONLY);
    if (file_fd == -1)
    {
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 404 Not Found\r\n"
                                        "Content-Type: text/plain\r\n\r\n"
                                        "404 Not Found");
        *response_len = strlen(response);
        return;
    }

    struct stat file_stat;
    fstat(file_fd, &file_stat);
    off_t file_size = file_stat.st_size;

    *response_len = 0;
    memcpy(response, header, strlen(header));
    *response_len += strlen(header);

    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, response + *response_len, BUFFER_SIZE - *response_len)) > 0)
        *response_len += bytes_read;

    free(header);
    close(file_fd);
}

void *handleClient(void *arg)
{
    int client_socket = *((int *)arg);
    char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    printf("client_socket: %d\n", client_socket);
    printf("Buffer: %s\n", buffer);
    if (bytes_received > 0)
    {
        char *get_ptr = strstr(buffer, "GET /");
        if (get_ptr != NULL)
        {
            char *http_ptr = strstr(get_ptr, " HTTP/1");
            if (http_ptr != NULL)
            {
                *http_ptr = '\0';

                char *file_name = decodeURL(get_ptr + 5);

                char file_ext[32];
                strcpy(file_ext, extractFileExtension(file_name));

                char *response = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
                size_t response_len;
                generateResponse(file_name, file_ext, response, &response_len);

                send(client_socket, response, response_len, 0);

                free(response);
                free(file_name);
            }
        }
    }
    close(client_socket);
    free(arg);
    free(buffer);
    return NULL;
}

int main(int argc, char *argv[])
{
    int server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    if (inet_pton(AF_INET, IP, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on Port %d\n", PORT);
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_socket = malloc(sizeof(int));

        if ((*client_socket = accept(server_socket,
                                     (struct sockaddr *)&client_addr,
                                     &client_addr_len)) < 0)
        {
            perror("Accept Failed");
            continue;
        }

        // create a new thread to handle client request
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handleClient, (void *)client_socket);
        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}