
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void algorithm(int client_fd)
{
    int window, frames, tt, tp;
    char message[100];

    printf("Enter window size: ");
    scanf("%d", &window);

    printf("Enter number of frames: ");
    scanf("%d", &frames);

    printf("Enter Tt (ms): ");
    scanf("%d", &tt);

    printf("Enter Tp (ms): ");
    scanf("%d", &tp);

    // Send inputs to receiver
    sprintf(message, "%d %d %d %d\n",
            window, frames, tt, tp);

    send(client_fd, message, strlen(message), 0);

    // Receive timeline and completion time
    char buffer[4096];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (n > 0) {
        buffer[n] = '\0';
        printf("\n%s", buffer);
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

    // 4. Run Sliding Window algorithm
    algorithm(client_fd);

    // 5. Close socket
    close(client_fd);

    return 0;
}
