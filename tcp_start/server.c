#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


/*
1. socket
2. configure
3. bind
4. listen
5. accept
6. send/recv
7. close
*/
int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr;

    char buffer[1024];

    // 1. Create a TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // 2. Configure the server address
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 3. Bind the socket to port 8080
    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // 4. Start listening for connections
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Server waiting on port 8080...\n");

    // 5. Accept one client
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    // 6. Receive a message from the client
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (n > 0) {
        buffer[n] = '\0';
        printf("Client says: %s\n", buffer);

        // 7. Send a reply
        const char *reply = "Hello Client!";
        if (send(client_fd, reply, strlen(reply), 0) == -1) {
            perror("send");
        }
    } else if (n == 0) {
        printf("Client disconnected.\n");
    } else {
        perror("recv");
    }

    // 8. Close the sockets
    close(client_fd);
    close(server_fd);

    return 0;
}
