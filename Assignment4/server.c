#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "utils.h"

#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define IP "127.0.0.1"
#define TIMEOUT 5
#define N_lost 2 // Every N_lost Acknowledgement frame will be lost
#define N_delayed 4 // Every N_delayed Acknowledgement Frame will be delayed

int main()
{
    int sockfd;
    struct sockaddr_in serverAddress, clientAddress;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t client_addr_size;

    int frame_id = 0;
    Frame frame_recv;
    Frame frame_send;

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

    if (bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    client_addr_size = sizeof(clientAddress);

    while (1)
    {

        int recv_size = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0, (struct sockaddr *)&clientAddress, &client_addr_size);

        if (recv_size < 0)
        {
            printf("[-]Error Receiving Frame\n");
        }
        else if (recv_size > 0)
        {
            if (frame_recv.frame_kind == 1 && frame_recv.sq_no == frame_id)
            {
                printf("[+]Frame Received: %s\n", frame_recv.packet.data);

                frame_send.frame_kind = 0;
                frame_send.sq_no = frame_id;

                if ((frame_id + 1) % N_delayed == 0)
                    sleep(TIMEOUT + 1);

                if ((frame_id + 1) % N_lost)
                {
                    sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&clientAddress, client_addr_size);
                    printf("[+]Ack Send\n");
                }
                
                frame_id++;
            }
            else
            {
                printf("[-]Older Frame Received\n");

                frame_send.frame_kind = 0;
                frame_send.sq_no = frame_id - 1;

                sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&clientAddress, client_addr_size);
                printf("[+]Ack Send\n");
            }
        }
        else
        {
            printf("[-]Connection Closed!\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}