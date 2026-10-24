
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define CHUNK_SIZE 1024

struct Packet {
    int type;
    int seq;
    int size;
    char data[CHUNK_SIZE];
};

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    struct Packet packet, ack;
    char filename[100];
    char filepath[150];

    FILE *fp = NULL;
    int expected_seq = 0;
    int completed = 0;

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    printf("Waiting for sender...\n");

    while (1) {
        int n = recvfrom(server_fd, &packet, sizeof(packet), 0,
                         (struct sockaddr *)&client_addr, &client_len);

        if (n < (int)(3 * sizeof(int)))
            continue;

        memset(&ack, 0, sizeof(ack));
        ack.type = 3;

        /* Receive filename */
        if (packet.type == 0) {
            if (packet.size <= 0 || packet.size >= sizeof(filename))
                continue;

            if (fp == NULL && !completed) {
                memcpy(filename, packet.data, packet.size);
                filename[packet.size] = '\0';

                if (strcmp(filename, "file.txt") != 0 &&
                    strcmp(filename, "video.mp4") != 0 &&
                    strcmp(filename, "audio.mp3") != 0) {
                    printf("Invalid filename.\n");
                    continue;
                }

                sprintf(filepath, "receiverFiles/%s", filename);

                fp = fopen(filepath, "wb");

                if (fp == NULL) {
                    perror("File open");
                    continue;
                }

                expected_seq = 0;

                printf("\nReceiving: %s\n", filename);
                printf("Saving to: %s\n", filepath);
            }

            if (fp != NULL && !completed) {
                ack.seq = 0;

                sendto(server_fd, &ack, sizeof(ack), 0,
                       (struct sockaddr *)&client_addr, client_len);
            }
        }

        /* Receive file chunks */
        else if (packet.type == 1 && fp != NULL && !completed) {
            if (packet.size < 0 || packet.size > CHUNK_SIZE)
                continue;

            if (packet.seq == expected_seq) {
                size_t written = fwrite(packet.data, 1,
                                        packet.size, fp);

                if (written != (size_t)packet.size) {
                    perror("File write");
                    fclose(fp);
                    fp = NULL;
                    continue;
                }

                printf("Received chunk %d (%d bytes)\n",
                       packet.seq, packet.size);

                expected_seq++;
            }

            /* ACK new or duplicate chunk */
            if (packet.seq < expected_seq) {
                ack.seq = packet.seq + 1;

                sendto(server_fd, &ack, sizeof(ack), 0,
                       (struct sockaddr *)&client_addr, client_len);
            }
        }

        /* File transfer complete */
        else if (packet.type == 2 && (fp != NULL || completed)) {
            if (!completed && packet.seq == expected_seq) {
                fclose(fp);
                fp = NULL;
                completed = 1;

                printf("\nFile received successfully!\n");
                printf("Saved file: %s\n", filepath);
            }

            if (completed && packet.seq == expected_seq) {
                ack.seq = packet.seq + 1;

                sendto(server_fd, &ack, sizeof(ack), 0,
                       (struct sockaddr *)&client_addr, client_len);
            }
        }
    }

    close(server_fd);
    return 0;
}
