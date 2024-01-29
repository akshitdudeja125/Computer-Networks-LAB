#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define IP "10.10.151.28"
// #define IP "127.0.0.1"
#define PORT 8080
#define MAX_BUFFER_SIZE 2048
#define OUTPUT_FOLDER_PATH "recieved_files"

void dieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}

int createUDPSocket()
{
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0); // Internetwork(TCP,UDP)//Data socket //Default address
    if (serverSocket == -1)
    {
        dieWithError("Error creating socket");
    }

    printf("Server socket created successfully\n");
    return serverSocket;
}

void setupServerAddress(struct sockaddr_in *serverAddress)
{
    memset(serverAddress, 0, sizeof(*serverAddress));
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(PORT); // htons converts host byte order to network byte order
    // serverAddress->sin_addr.s_addr = INADDR_ANY;
    if (inet_pton(AF_INET, IP, &(serverAddress->sin_addr)) <= 0) // Convert IPv4 and IPv6 addresses from text to binary form
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
}

void bindSocket(int serverSocket, const struct sockaddr *serverAddress)
{
    // This is the server's socket, so we need to bind it to a specific port number
    if (bind(serverSocket, serverAddress, sizeof(*serverAddress)) < 0)
    {
        dieWithError("Error binding socket");
    }

    printf("Server bound to port %d\n", PORT);
}

void recieveFiles(int serverSocket, const char *outputFolderPath)
{
    while (1)
    {
        printf("Waiting for file name\n");
        char buffer[MAX_BUFFER_SIZE];
        struct sockaddr_in clientAddress;
        socklen_t client_address_len = sizeof(clientAddress);

        // Receive the filename from the client
        ssize_t recv_len = recvfrom(serverSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&clientAddress, &client_address_len);
        if (recv_len < 0)
        {
            dieWithError("Error receiving data");
        }
        if (recv_len == 0)
        {
            return;
        }
        buffer[recv_len] = '\0';
        printf("Receiving file: %s\n", buffer);

        char outputFilename[MAX_BUFFER_SIZE];
        snprintf(outputFilename, sizeof(outputFilename) - 1, "%s/%s", outputFolderPath, buffer);
        printf("Output file: %s\n", outputFilename);

        FILE *file = fopen(outputFilename, "wb");
        if (file == NULL)
            dieWithError("Error opening file for writing");

        char fileBuffer[MAX_BUFFER_SIZE];
        // int count = 0;
        while (1)
        {
            // printf("%s\n", fileBuffer);
            recv_len = recvfrom(serverSocket, fileBuffer, sizeof(fileBuffer) - 1, 0, (struct sockaddr *)&clientAddress, &client_address_len);
            // printf("%ld %d\n", recv_len, count);
            // count++;
            if (recv_len < 0)
            {
                printf("received %ld bytes\n", recv_len);
                dieWithError("Error receiving data");
            }

            if (recv_len == 0)
            {
                printf("received 0 bytes\n");
                break; // End of file
            }

            fileBuffer[recv_len] = '\0';
            fwrite(fileBuffer, 1, recv_len, file);
        }

        fclose(file);
        printf("File received successfully: %s\n", outputFilename);
    }
}

void receiveFolder(int serverSocket, const char *outputFolderPath)
{
    DIR *outputFolder = opendir(outputFolderPath);
    if (outputFolder == NULL && mkdir(outputFolderPath, 0777) != 0) // 0777:read,write and execute permissions for user,group and others
    {
        dieWithError("Error creating output folder");
    }
    else
    {
        if (outputFolder != NULL)
        {
            closedir(outputFolder);
        }
    }

    recieveFiles(serverSocket, outputFolderPath);

    printf("All files received\n");
}

int main()
{
    int serverSocket;
    struct sockaddr_in serverAddress;

    serverSocket = createUDPSocket();
    setupServerAddress(&serverAddress);

    bindSocket(serverSocket, (struct sockaddr *)&serverAddress);

    // Receive the entire folder from the client
    receiveFolder(serverSocket, OUTPUT_FOLDER_PATH);

    close(serverSocket);

    return 0;
}