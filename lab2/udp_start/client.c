
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/*
1. socket
2. configure client
3. bind
4. configure server
5. sendto/ recvfrom
6. close
*/
int main(void)
{
    // 1. Create UDP socket
    int client_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    // 2. Prepare client address
    struct sockaddr_in client_addr = {0};

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(9001);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 3. Bind client socket
    if (bind(client_fd,
             (struct sockaddr *)&client_addr,
             sizeof(client_addr)) == -1) {
        perror("bind");
        close(client_fd);
        return 1;
    }

    // 4. Prepare server address
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 5. Receive message from server
    char buffer[1024];

    ssize_t n = recvfrom(client_fd, buffer,
                         sizeof(buffer) - 1, 0,
                         NULL, NULL);

    if (n == -1) {
        perror("recvfrom");
        close(client_fd);
        return 1;
    }

    buffer[n] = '\0';
    printf("Server: %s\n", buffer);

    // 6. Send reply to server
    const char *message = "Hello Server";

    if (sendto(client_fd, message, strlen(message), 0,
               (struct sockaddr *)&server_addr,
               sizeof(server_addr)) == -1) {
        perror("sendto");
        close(client_fd);
        return 1;
    }

    printf("Reply sent to server!\n");

    // 7. Close socket
    close(client_fd);

    return 0;
}
