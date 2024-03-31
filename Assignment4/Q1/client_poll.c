#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <poll.h>
#include "../utils.h"

#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define IP "127.0.0.1"
#define TIMEOUT 5000 // Timeout in milliseconds
// #define N_lost 3     // Every N_lost Data frame will be lost
#define P 0.7 // Probability with which Data frame will be lost

int main()
{
    srand(time(NULL));

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

    server_addr_size = sizeof(serverAddress);

    while (1)
    {
        if (ack_recv == true)
        {
            frame_send.frame_kind = 1;
            frame_send.sq_no = frame_id;

            printf("Enter Data: ");
            scanf("%s", buffer);
            strcpy(frame_send.packet.data, buffer);

            double random_number = (double)rand() / RAND_MAX;

            // if ((frame_id + 1) % N_lost)
            if (random_number <= P)
            {
                sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&serverAddress, server_addr_size);
                printf("[+]Frame Sent\n");
            }
        }

        struct pollfd fds[1];
        fds[0].fd = sockfd;
        fds[0].events = POLLIN;

        int poll_result = poll(fds, 1, TIMEOUT);

        if (poll_result == -1)
        {
            perror("Poll Failed");
            exit(EXIT_FAILURE);
        }
        else if (poll_result == 0)
        {
            printf("[-]Timeout: Ack Not Received\n");
            ack_recv = false;
            sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&serverAddress, server_addr_size);
            printf("[+]Frame Resent\n");
        }
        else
        {
            if (fds[0].revents & POLLIN)
            {
                int recv_size = recvfrom(sockfd, &frame_recv, sizeof(frame_recv), 0, (struct sockaddr *)&serverAddress, &server_addr_size);

                if (recv_size < 0)
                {
                    perror("Recvfrom Failed");
                    exit(EXIT_FAILURE);
                }
                else if (recv_size > 0)
                {
                    if (frame_recv.frame_kind == 0 && frame_recv.sq_no == frame_id)
                    {
                        printf("[+]Ack Received\n");
                        ack_recv = true;
                        frame_id++;
                    }
                    else if (frame_recv.frame_kind == 0 && frame_recv.sq_no < frame_id)
                    {
                        printf("[-]Duplicate Ack Received\n");
                        ack_recv = false;
                    }
                    else if (frame_recv.frame_kind == 1)
                    {
                        printf("[-]Why is Server sending Data Frame???\n");
                    }
                    else
                    {
                        printf("[-]Invalid Frame Received\n");
                    }
                }
                else
                {
                    printf("[-]Connection Closed\n");
                    break;
                }
            }
        }
        printf("\n\n");
    }

    close(sockfd);
    return 0;
}
