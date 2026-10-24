
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
1. socket
2. configure
3. connect
4. send/recv
5. close
*/
int main()
{
    // 1. Create socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    // 2. Configure server address
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    // 3. Connect to server
    if (connect(client_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        return 1;
    }

    printf("Connected to server!\n");

    // 4. Send message
    const char *message = "Hello Server";

    if (send(client_fd, message, strlen(message), 0) == -1) {
        perror("send");
        close(client_fd);
        return 1;
    }

    // 5. Receive reply
    char buffer[1024];

    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (n > 0) {
        buffer[n] = '\0';
        printf("Server says: %s\n", buffer);
    } else if (n == 0) {
        printf("Server closed its sending side.\n");
    } else {
        perror("recv");
    }

    // 6. Close socket
    close(client_fd);

    return 0;
}
  