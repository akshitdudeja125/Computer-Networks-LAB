#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 80
#define BUFFER_SIZE 1024
#define FILENAME "response.html" // Name of the file to save the response

void send_http_request(int client_socket, const char *host, const char *path, const char *method) {
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "%s %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", method, path, host);

    ssize_t bytes_sent = send(client_socket, request, strlen(request), 0);
    if (bytes_sent < 0) {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }
}

void receive_http_response(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    int headers_done = 0; // Flag to indicate when headers are done

    FILE *file = fopen(FILENAME, "wb"); // Open file for writing in binary mode
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Receive and write the response to the file
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        if (!headers_done) {
            // Search for the double line break indicating end of headers
            char *end_of_headers = strstr(buffer, "\r\n\r\n");
            if (end_of_headers != NULL) {
                // Skip the headers and set the flag
                fwrite(end_of_headers + 4, 1, bytes_received - (end_of_headers - buffer) - 4, file);
                headers_done = 1;
            }
        } else {
            fwrite(buffer, 1, bytes_received, file); // Write content to file
        }
    }

    fclose(file); // Close the file
}

void perform_get_request(int client_socket, const char *host, const char *path) {
    send_http_request(client_socket, host, path, "GET");
    receive_http_response(client_socket);
}

int get_response_code(const char *response) {
    const char *code_start = strstr(response, "HTTP/1.1");
    if (code_start == NULL)
        return -1;

    // Move pointer to the beginning of the status code
    code_start += strlen("HTTP/1.1") + 1;

    // Extract the status code
    int code;
    sscanf(code_start, "%d", &code);
    return code;
}
void perform_post_request(int client_socket, const char *host, const char *path, const char *data) {
    send_http_request(client_socket, host, path, "POST");
    send(client_socket, data, strlen(data), 0);
    char response_buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_socket, response_buffer, BUFFER_SIZE, 0);
    int response_code = get_response_code(response_buffer);
    if (response_code == -1) {
        printf("Failed to extract response code\n");
    } else {
        printf("Status code: %d\n", response_code);
    }
    
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    const char *host = "www.example.com";
    const char *path = "/index.html";
    const char *post_data = "key1=value1&key2=value2"; // Example POST data

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating client socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("93.184.216.34"); // IPv4 address of www.example.com
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Perform GET request
    // printf("Performing GET request...\n");
    // perform_get_request(client_socket, host, path);

    // Perform POST request
    printf("\nPerforming POST request...\n");
    perform_post_request(client_socket, host, path, post_data);

    // Close the socket
    close(client_socket);

    return 0;
}
