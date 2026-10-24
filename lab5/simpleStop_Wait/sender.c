
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void algorithm(int client_fd)
{
    int tt, tp, packets;
    char message[100];

    printf("Enter transmission/loading time (Tt) in ms: ");
    scanf("%d", &tt);

    printf("Enter propagation time (Tp) in ms: ");
    scanf("%d", &tp);

    printf("Enter number of packets: ");
    scanf("%d", &packets);

    // Send inputs to the receiver
    sprintf(message, "%d %d %d\n", tt, tp, packets);
    send(client_fd, message, strlen(message), 0);

    printf("\nInputs sent to receiver.\n");

    // Receive completion time
    char buffer[1024];
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (n > 0) {
        buffer[n] = '\0';
        printf("%s\n", buffer);
    }
}

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
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // 3. Connect to server
    if (connect(client_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        return 1;
    }

    printf("Connected to server!\n");

    // 4. Run Stop-and-Wait algorithm
    algorithm(client_fd);

    // 5. Close socket
    close(client_fd);

    return 0;
}
