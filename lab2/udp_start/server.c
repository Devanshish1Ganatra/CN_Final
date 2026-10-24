
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
5. sendto/ recvfrom
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

    // 5. Send message to client
    const char *message = "Hello Client";

    sendto(server_fd, message, strlen(message), 0,
           (struct sockaddr *)&client_addr,
           sizeof(client_addr));

    printf("Message sent to client!\n");

    close(server_fd);

    return 0;
}
