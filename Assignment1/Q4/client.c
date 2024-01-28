#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>

#define IP "10.10.88.233"
#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define INPUT_FOLDER_PATH "client_files_1"

void dieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}

int createUDPSocket()
{
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1)
    {
        dieWithError("Error creating socket");
    }

    printf("Client socket created successfully\n");
    return clientSocket;
}

void setupServerAddress(struct sockaddr_in *serverAddress)
{
    memset(serverAddress, 0, sizeof(*serverAddress));
    serverAddress->sin_family = AF_INET;
    // serverAddress->sin_addr.s_addr = inet_addr(IP);
    if (inet_pton(AF_INET, IP, &(serverAddress->sin_addr)) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    serverAddress->sin_port = htons(PORT);
}

const char *extractFileName(const char *filePath)
{
    const char *fileName = strrchr(filePath, '/');

    // If '/' is not found, check for '\' (Windows path separator)
    if (fileName == NULL)
        fileName = strrchr(filePath, '\\');

    // If neither '/' nor '\' is found, the entire path is the file name
    if (fileName == NULL)
        fileName = filePath;
    else
        fileName++; // Move to the next character after the last occurrence of '/' or '\'

    return fileName;
}

void sendFileThroughSocket(int clientSocket, struct sockaddr_in serverAddress, const char *filePath, const char *fileName)
{
    FILE *file = fopen(filePath, "rb");
    if (file == NULL)
        dieWithError("Error opening file");

    // const char *fileName = extractFileName(filePath);
    printf("Sending file name: %s\n", fileName);

    // Sending the file name to the server
    if (sendto(clientSocket, fileName, strlen(fileName), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Error sending data");
        fclose(file);
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        char buffer[MAX_BUFFER_SIZE];
        size_t read_len = fread(buffer, 1, sizeof(buffer) - 1, file);

        if (read_len <= 0)
        {
            break;
        }

        ssize_t send_len = sendto(clientSocket, buffer, read_len, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

        if (send_len < 0)
        {
            perror("Error sending data");
            fclose(file);
            close(clientSocket);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);

    // Send an empty message to the server to indicate the end of file
    if (sendto(clientSocket, "", 0, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Error sending data");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("File sent successfully\n");
    }
}

void sendFolderThroughSocket(int clientSocket, struct sockaddr_in serverAddress, const char *folderPath)
{
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(folderPath)) == NULL)
    {
        dieWithError("Error opening folder");
    }

    // // Send the directory name to the server
    // if (sendto(clientSocket, folderPath, strlen(folderPath), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    // {
    //     perror("Error sending data");
    //     closedir(dir);
    //     close(clientSocket);
    //     exit(EXIT_FAILURE);
    // }
    // // print the folder name
    // printf("Sending folder name: %s\n", folderPath);

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char filePath[MAX_BUFFER_SIZE];
            snprintf(filePath, sizeof(filePath) - 1, "%s/%s", folderPath, entry->d_name);
            sendFileThroughSocket(clientSocket, serverAddress, filePath, entry->d_name);
        }
    }

    // send an end of folder transfer
    if (sendto(clientSocket, "", 0, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Error sending data");
        closedir(dir);
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("End of folder transfer\n");
    }

    closedir(dir);
}

int main()
{

    int clientSocket = createUDPSocket();

    struct sockaddr_in serverAddress;
    setupServerAddress(&serverAddress);

    sendFolderThroughSocket(clientSocket, serverAddress, INPUT_FOLDER_PATH);

    close(clientSocket);

    return 0;
}
