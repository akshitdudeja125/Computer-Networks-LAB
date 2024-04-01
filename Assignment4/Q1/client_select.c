#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include "../utils.h"

#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define IP "127.0.0.1"
#define TIMEOUT_SEC 5  // Timeout in seconds
#define TIMEOUT_USEC 0 // Timeout in microseconds
// #define N_lost 3       // Every N_lost Data frame will be lost
// #define P 1.0                 // Probability with which Data frame will be lost
#define FILENAME "sample.txt" // Name of the file to be sent

int func(double P)
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
                printf("\n[+]File Read Complete\n");
                strcpy(frame_send.packet.data, "EOF");
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

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = TIMEOUT_USEC;

        int select_result = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (select_result == -1)
        {
            perror("Select Failed");
            exit(EXIT_FAILURE);
        }
        else if (select_result == 0)
        {
            printf("[-]Timeout: Ack Not Received\n");
            ack_recv = false;
            sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&serverAddress, server_addr_size);
            printf("[+]Frame Resent\n");
        }
        else
        {
            if (FD_ISSET(sockfd, &readfds))
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

    long seconds = end_time.tv_sec - start_time.tv_sec;
    long microseconds = end_time.tv_usec - start_time.tv_usec;
    if (microseconds < 0)
    {
        seconds--;
        microseconds += 1000000;
    }
    // printf("Total time taken: %ld seconds and %ld microseconds\n", seconds, microseconds);
    // get time in miliseconds
    double final_time = seconds + microseconds / 1000000.0;
    printf("%lf,%lf\n", 1 - P, final_time);

    return 0;
}

int main()
{
    double p_values[] = {1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1};
    for (int i = 0; i < sizeof(p_values) / sizeof(int); i++)
    {
        // printf("\n[+]P = %lf\n", p_values[i]);
        func(p_values[i]);
        // printf("\n\n");
    }
}
