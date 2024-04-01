#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../utils.h"

#define PORT 8080
#define MAX_BUFFER_SIZE 1024
#define N 1 // Receiver Window Size
#define IP "127.0.0.1"
#define TIMEOUT 5
// #define N_lost 2 // Every N_lost Acknowledgement frame will be lost
// #define N_delayed 4 // Every N_delayed Acknowledgement Frame will be delayed
#define P 1.0                        // Probability with which Acknowledgement frame will be lost
#define FILENAME "received_file.txt" // Name of the file to save on the server

int main()
{
    srand(time(NULL));

    int sockfd;
    struct sockaddr_in serverAddress, clientAddress;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t client_addr_size;

    FILE *file;
    ssize_t bytes_received;

    file = fopen(FILENAME, "wb");
    if (file == NULL)
    {
        perror("Error creating file");
        exit(EXIT_FAILURE);
    }

    int start = 0;
    bool comp = false;
    Frame frame_recv;
    Frame frame_send;
    Frame frames[N];

    for (int i = 0; i < N; i++)
    {
        frames[i].packet.data[0] = '\0';
    }

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

    printf("[+]Server Started\n\n");

    client_addr_size = sizeof(clientAddress);

    while (1)
    {
        int bytes_received = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0, (struct sockaddr *)&clientAddress, &client_addr_size);

        if (bytes_received < 0)
        {
            printf("[-]Error Receiving Frame\n");
            exit(EXIT_FAILURE);
        }
        else if (bytes_received > 0)
        {
            if (frame_recv.frame_kind == 1)
            {

                if (frame_recv.sq_no == start)
                {
                    frames[frame_recv.sq_no % N] = frame_recv;

                    int i;
                    for (i = start; i < start + N; i++)
                    {
                        if (frames[i % N].packet.data[0] == '\0')
                        {
                            break;
                        }

                        printf("[+]Frame %d Received:\n", frame_recv.sq_no);

                        if (frame_recv.packet.data[0] == 'E' && frame_recv.packet.data[1] == 'O' && frame_recv.packet.data[2] == 'F')
                        {
                            printf("[+]File Transfer Complete\n");
                            comp = true;
                        }
                        else if (fwrite(frames[i % N].packet.data, 1, MAX_BUFFER_SIZE, file) != MAX_BUFFER_SIZE)
                        {
                            perror("Error writing to file");
                            exit(EXIT_FAILURE);
                        }

                        frames[i % N].packet.data[0] = '\0';
                    }
                    start = (comp) ? 0 : i;
                    comp = false;

                    frame_send.sq_no = i - 1;
                }
                else if (frame_recv.sq_no > start && frame_recv.sq_no < start + N)
                {
                    frames[frame_recv.sq_no % N] = frame_recv;
                    printf("[-]Frame Out of Order\n");
                    frame_send.sq_no = start - 1;
                }
                else
                {
                    if (frame_recv.sq_no < start)
                        printf("[-]Frame Already Received\n");
                    if (frame_recv.sq_no >= start + N)
                        printf("[-]Frame Out of Order\n");
                    frame_send.sq_no = start - 1;
                }

                double random_number = (double)rand() / RAND_MAX;

                // if ((frame_id + 1) % N_delayed == 0)
                //     sleep(TIMEOUT + 1);

                if (random_number <= P)
                // if ((frame_id + 1) % N_lost)
                {
                    sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr *)&clientAddress, client_addr_size);
                    printf("[+]Ack %d Send\n", frame_send.sq_no);
                }
            }
            else if (frame_recv.frame_kind == 0)
            {
                printf("[-]Why is Client sending Acknowledgement Frame???\n");
            }
            else
            {
                printf("[-]Invalid Frame Received\n");
            }
        }
        else
        {
            printf("[-]Connection Closed!\n");
            break;
        }
        printf("\n\n");
    }

    fclose(file);
    close(sockfd);
    return 0;
}