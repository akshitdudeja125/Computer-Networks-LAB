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
#define TIMEOUT 5
// #define N_lost 3       // Every N_lost Data frame will be lost
#define P 1.0                 // Probability with which Data frame will be lost
#define FILENAME "sample.txt" // Name of the file to be sent

int main()
{
    srand(time(NULL));

    int sockfd;
    struct sockaddr_in serverAddress;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t server_addr_size;

    FILE *file;
    ssize_t bytes_read;

    file = fopen(FILENAME, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int frame_id = 0;
    Frame frame_send;
    Frame frame_recv;
    bool ack_recv = true;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
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

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    while (1)
    {
        if (ack_recv == true)
        {
            frame_send.frame_kind = 1;
            frame_send.sq_no = frame_id;

            bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE, file);

            if (bytes_read == 0)
            {
                printf("[+]File Sent\n");
            }
            else if (bytes_read < 0)
            {
                perror("Error reading file");
                exit(EXIT_FAILURE);
            }
            else
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
                        if (bytes_read == 0)
                            break;
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

    fclose(file);
    close(sockfd);

    gettimeofday(&end_time, NULL);

    fclose(file);
    close(sockfd);

    long seconds = end_time.tv_sec - start_time.tv_sec;
    long microseconds = end_time.tv_usec - start_time.tv_usec;
    if (microseconds < 0)
    {
        seconds--;
        microseconds += 1000000;
    }
    printf("Total time taken: %ld seconds and %ld microseconds\n", seconds, microseconds);

    return 0;
}
