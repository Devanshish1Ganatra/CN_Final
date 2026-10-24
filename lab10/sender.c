
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define CLIENT_PORT 9001
#define CHUNK_SIZE 1024
#define MAX_RETRIES 5

struct Packet {
    int type;
    int seq;
    int size;
    char data[CHUNK_SIZE];
};

int main() {
    int client_fd, choice;
    struct sockaddr_in client_addr, server_addr;
    socklen_t server_len = sizeof(server_addr);

    char filename[100];
    char filepath[150];

    printf("Choose file to send:\n");
    printf("1. file.txt\n");
    printf("2. video.mp4\n");
    printf("3. audio.mp3\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 1)
        strcpy(filename, "file.txt");
    else if (choice == 2)
        strcpy(filename, "video.mp4");
    else if (choice == 3)
        strcpy(filename, "audio.mp3");
    else {
        printf("Invalid choice.\n");
        return 1;
    }

    sprintf(filepath, "senderFiles/%s", filename);

    FILE *fp = fopen(filepath, "rb");

    if (fp == NULL) {
        perror("File open");
        return 1;
    }

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_fd == -1) {
        perror("socket");
        fclose(fp);
        return 1;
    }

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(client_fd, (struct sockaddr *)&client_addr,
             sizeof(client_addr)) == -1) {
        perror("bind");
        fclose(fp);
        close(client_fd);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    struct Packet packet, ack;
    fd_set readfds;
    struct timeval timeout;

    int retries, success, seq = 0;
    size_t bytes;

    /* Send filename */
    memset(&packet, 0, sizeof(packet));
    packet.type = 0;
    packet.seq = 0;
    packet.size = strlen(filename);
    strcpy(packet.data, filename);

    success = 0;

    for (retries = 0; retries < MAX_RETRIES; retries++) {
        sendto(client_fd, &packet, sizeof(packet), 0,
               (struct sockaddr *)&server_addr, server_len);

        FD_ZERO(&readfds);
        FD_SET(client_fd, &readfds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        if (select(client_fd + 1, &readfds, NULL, NULL,
                   &timeout) > 0) {
            recvfrom(client_fd, &ack, sizeof(ack), 0, NULL, NULL);

            if (ack.type == 3 && ack.seq == 0) {
                success = 1;
                break;
            }
        }

        printf("Retrying filename...\n");
    }

    if (!success) {
        printf("Receiver not responding.\n");
        fclose(fp);
        close(client_fd);
        return 1;
    }

    printf("Sending %s...\n", filename);

    /* Send file chunks */
    while ((bytes = fread(packet.data, 1, CHUNK_SIZE, fp)) > 0) {
        packet.type = 1;
        packet.seq = seq;
        packet.size = bytes;

        success = 0;

        for (retries = 0; retries < MAX_RETRIES; retries++) {
            sendto(client_fd, &packet, sizeof(packet), 0,
                   (struct sockaddr *)&server_addr, server_len);

            FD_ZERO(&readfds);
            FD_SET(client_fd, &readfds);
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            if (select(client_fd + 1, &readfds, NULL, NULL,
                       &timeout) > 0) {
                recvfrom(client_fd, &ack, sizeof(ack), 0, NULL, NULL);

                if (ack.type == 3 && ack.seq == seq + 1) {
                    success = 1;
                    break;
                }
            }

            printf("Retrying chunk %d...\n", seq);
        }

        if (!success) {
            printf("Transfer failed at chunk %d.\n", seq);
            fclose(fp);
            close(client_fd);
            return 1;
        }

        printf("Sent chunk %d (%zu bytes)\n", seq, bytes);
        seq++;
    }

    fclose(fp);

    /* Send end-of-file */
    memset(&packet, 0, sizeof(packet));
    packet.type = 2;
    packet.seq = seq;
    packet.size = 0;

    success = 0;

    for (retries = 0; retries < MAX_RETRIES; retries++) {
        sendto(client_fd, &packet, sizeof(packet), 0,
               (struct sockaddr *)&server_addr, server_len);

        FD_ZERO(&readfds);
        FD_SET(client_fd, &readfds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        if (select(client_fd + 1, &readfds, NULL, NULL,
                   &timeout) > 0) {
            recvfrom(client_fd, &ack, sizeof(ack), 0, NULL, NULL);

            if (ack.type == 3 && ack.seq == seq + 1) {
                success = 1;
                break;
            }
        }
    }

    if (success)
        printf("File transfer completed successfully!\n");
    else
        printf("Final ACK not received.\n");

    close(client_fd);
    return 0;
}
