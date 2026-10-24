
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX 4096
#define FLAG 'F'
#define ESC 'E'

void algorithm(int client_fd)
{
    char frame[MAX], data[MAX], ch;
    int received = 0, pos = 0, i;

    // Read the complete frame until newline
    while (received < MAX - 1) {
        if (recv(client_fd, &ch, 1, 0) <= 0)
            return;

        if (ch == '\n')
            break;

        frame[received++] = ch;
    }

    frame[received] = '\0';

    printf("Received frame: %s\n", frame);

    // Skip starting FLAG and ending FLAG
    for (i = 1; i < received - 1; i++) {
        if (frame[i] == ESC)
            i++;  // Next byte is escaped data

        data[pos++] = frame[i];
    }

    data[pos] = '\0';

    printf("Original data: %s\n", data);
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Server waiting on port 8080...\n");
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    algorithm(client_fd);

    const char *reply = "Byte stuffing completed!";
    send(client_fd, reply, strlen(reply), 0);

    close(client_fd);
    close(server_fd);
    return 0;
}
