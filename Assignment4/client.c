#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "utils.h"

#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define IP "127.0.0.1"
#define TIMEOUT 5
#define N_lost 3 // Every N_lost Data frame will be lost

int main()
{
    int sockfd;
    struct sockaddr_in serverAddress;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t server_addr_size;

    int frame_id = 0;
    Frame frame_send;
    Frame frame_recv;
    bool ack_recv = true;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    // serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (inet_pton(AF_INET, IP, &(serverAddress.sin_addr)) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    struct timeval tv;
    tv.tv_sec = TIMEOUT; // Timeout set to 5 seconds
    tv.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    while (1)
    {

        if (ack_recv == true)
        {
            frame_send.frame_kind = 1;
            frame_send.sq_no = frame_id;

            printf("Enter Data: ");
            scanf("%s", buffer);
            strcpy(frame_send.packet.data, buffer);

            if ((frame_id + 1) % N_lost)
            {
                sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
                printf("[+]Frame Send\n");
            }
        }

        int server_addr_size = sizeof(serverAddress);
        int recv_size = recvfrom(sockfd, &frame_recv, sizeof(frame_recv), 0, (struct sockaddr *)&serverAddress, &server_addr_size);

        if (recv_size < 0)
        {
            printf("[-]Timeout: Ack Not Received\n");
            ack_recv = false;
            sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
            printf("[+]Frame Resend\n");
        }
        else if (recv_size > 0)
        {
            if (frame_recv.frame_kind == 0 && frame_recv.sq_no == frame_id)
            {
                printf("[+]Ack Received\n");
                ack_recv = true;
                frame_id++;
            }
            else
            {
                printf("[-]Older Ack Received\n");
                ack_recv = false;
            }
        }
        else
        {
            printf("[-]Connection Closed\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}