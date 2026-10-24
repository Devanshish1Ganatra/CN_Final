
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void algorithm(int client_fd)
{
    int tt, tp, packets, lostCount;
    int lost[100];
    char message[1024];
    int pos = 0;

    printf("Enter Tt (ms): ");
    scanf("%d", &tt);

    printf("Enter Tp (ms): ");
    scanf("%d", &tp);

    printf("Enter total number of packets: ");
    scanf("%d", &packets);

    printf("Enter number of lost packets: ");
    scanf("%d", &lostCount);

    printf("Enter lost packet numbers: ");
    for (int i = 0; i < lostCount; i++)
        scanf("%d", &lost[i]);

    // Send all inputs to the receiver
    pos += sprintf(message + pos, "%d %d %d %d",
                   tt, tp, packets, lostCount);

    for (int i = 0; i < lostCount; i++)
        pos += sprintf(message + pos, " %d", lost[i]);

    message[pos++] = '\n';
    send(client_fd, message, pos, 0);

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

    // 4. Run algorithm
    algorithm(client_fd);

    // 5. Close socket
    close(client_fd);

    return 0;
}
