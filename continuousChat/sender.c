
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/*
1. socket
2. configure server
3. bind
4. configure client
5. sendto / recvfrom
6. close
*/

int main(void)
{
    // 1. Create UDP socket
    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // 2. Prepare server address
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 3. Bind socket
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // 4. Prepare client address
    struct sockaddr_in client_addr = {0};

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(9001);
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 5. Continuous chat
    char buffer[1024];
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        printf("Server: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        sendto(server_fd, buffer, strlen(buffer) + 1, 0,
               (struct sockaddr *)&client_addr,
               sizeof(client_addr));

        if (strcmp(buffer, "close") == 0)
            break;

        ssize_t n = recvfrom(server_fd, buffer,
                             sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&client_addr,
                             &client_len);

        if (n == -1) {
            perror("recvfrom");
            break;
        }

        buffer[n] = '\0';
        printf("Client: %s\n", buffer);

        if (strcmp(buffer, "close") == 0)
            break;
    }

    // 6. Close socket
    close(server_fd);

    return 0;
}
