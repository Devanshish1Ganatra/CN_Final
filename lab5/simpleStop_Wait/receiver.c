
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

void algorithm(int client_fd)
{
    char message[100];
    char ch;
    int received = 0;

    int tt, tp, packets;

    // Receive inputs from sender
    while (received < 99) {
        if (recv(client_fd, &ch, 1, 0) <= 0)
            return;

        if (ch == '\n')
            break;

        message[received++] = ch;
    }

    message[received] = '\0';

    sscanf(message, "%d %d %d", &tt, &tp, &packets);

    int time = 0;

    printf("\nSTOP-AND-WAIT (NOISELESS CHANNEL)\n");
    printf("Tt = %d ms, Tp = %d ms\n\n", tt, tp);

    printf("%-12s | %-35s | %s\n",
           "Time (ms)", "Sender", "Receiver");

    printf("--------------------------------------------------------------------------\n");

    for (int i = 1; i <= packets; i++) {
        int loadEnd = time + tt;
        int packetArrival = loadEnd + tp;
        int receiverLoadEnd = packetArrival + tt;
        int ackArrival = receiverLoadEnd + tp;

        // Sender loads the packet
        printf("%d-%d       | Load packet P%d\n",
               time, loadEnd, i);

        // Packet travels to receiver
        printf("%d-%d       | Send P%d -------------> | Arrives at %d ms\n",
               loadEnd, packetArrival, i, packetArrival);

        // Receiver processes packet and prepares ACK
        printf("%d-%d       |                                  | Process P%d, prepare ACK\n",
               packetArrival, receiverLoadEnd, i);

        // ACK travels back to sender
        printf("%d-%d       | <------------- ACK P%d | ACK arrives at sender\n",
               receiverLoadEnd, ackArrival, i);

        printf("%d ms       | P%d completed\n\n", ackArrival, i);

        time = ackArrival;
    }

    printf("Total packets sent: %d\n", packets);
    printf("Total completion time: %d ms\n", time);

    char reply[100];
    sprintf(reply, "Total completion time: %d ms", time);

    send(client_fd, reply, strlen(reply), 0);
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr;

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // 2. Configure server address
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    // 3. Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // 4. Listen
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Server waiting on port 8080...\n");

    // 5. Accept client
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    // 6. Run Stop-and-Wait algorithm
    algorithm(client_fd);

    // 7. Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}
