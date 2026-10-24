
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 4096
#define FLAG 'F'
#define ESC 'E'

void algorithm(int client_fd)
{
    char data[MAX], frame[MAX];
    int i, pos = 0;

    printf("Enter data (F = FLAG, E = ESC): ");
    scanf("%4095s", data);

    frame[pos++] = FLAG;

    for (i = 0; data[i] != '\0'; i++) {
        if (data[i] == FLAG || data[i] == ESC)
            frame[pos++] = ESC;

        frame[pos++] = data[i];
    }

    frame[pos++] = FLAG;
    frame[pos] = '\0';

    printf("Stuffed frame: %s\n", frame);

    frame[pos++] = '\n';
    send(client_fd, frame, pos, 0);
}

int main()
{
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(client_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        return 1;
    }

    printf("Connected to server!\n");
    algorithm(client_fd);

    char buffer[1024];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (n > 0) {
        buffer[n] = '\0';
        printf("Server says: %s\n", buffer);
    }

    close(client_fd);
    return 0;
}
