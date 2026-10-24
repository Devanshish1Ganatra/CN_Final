
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

void algorithm(int client_fd)
{
    char message[1024], ch;
    int received = 0;
    int tt, tp, packets, lostCount;
    int lost[100], time = 0;

    // Receive inputs
    while (received < 1023) {
        if (recv(client_fd, &ch, 1, 0) <= 0)
            return;

        if (ch == '\n')
            break;

        message[received++] = ch;
    }

    message[received] = '\0';

    sscanf(message, "%d %d %d %d",
           &tt, &tp, &packets, &lostCount);

    // Read the lost packet numbers
    char *token = strtok(message, " ");

    for (int i = 0; i < 4; i++)
        token = strtok(NULL, " ");

    for (int i = 0; i < lostCount; i++) {
        sscanf(token, "%d", &lost[i]);
        token = strtok(NULL, " ");
    }

    int rtt = 2 * tp;
    int timeout = 3 * rtt;

    printf("\nSTOP-AND-WAIT ARQ TIMELINE\n");
    printf("Tt = %d ms, Tp = %d ms\n", tt, tp);
    printf("RTT = %d ms, Timeout = %d ms\n\n",
           rtt, timeout);

    printf("%-12s | %-38s | %-32s\n",
           "Time (ms)", "SENDER", "RECEIVER");

    printf("------------|----------------------------------------|----------------------------------\n");

    for (int p = 1; p <= packets; p++) {
        int isLost = 0;

        for (int i = 0; i < lostCount; i++) {
            if (lost[i] == p)
                isLost = 1;
        }

        // Load the packet only once
        int sendStart = time + tt;

        printf("%d-%d        | Load packet P%d\n",
               time, sendStart, p);

        printf("%-12d | Send P%d -----------------------> | Expected arrival: %d ms\n",
               sendStart, p, sendStart + tp);

        if (isLost) {
            // Timeout is measured from the first transmission
            int timeoutAt = sendStart + timeout;

            printf("%-12d | P%d LOST\n", sendStart, p);
            printf("%-12d | Waiting for ACK\n", sendStart);
            printf("%-12d | TIMEOUT for P%d\n\n",
                   timeoutAt, p);

            // Retransmit immediately: do not load the packet again
            int retrySend = timeoutAt;
            int retryArrival = retrySend + tp;
            int receiverEnd = retryArrival + tt;
            int ackArrival = receiverEnd + tp;

            printf("%-12d | Retransmit P%d ----------------> | Arrives at %d ms\n",
                   retrySend, p, retryArrival);

            printf("%d-%d        |                                    | Process P%d, prepare ACK\n",
                   retryArrival, receiverEnd, p);

            printf("%d-%d        | <----------------------- ACK P%d | ACK returns to sender\n",
                   receiverEnd, ackArrival, p);

            printf("%-12d | P%d completed after retransmission\n\n",
                   ackArrival, p);

            time = ackArrival;
        } else {
            int arrival = sendStart + tp;
            int receiverEnd = arrival + tt;
            int ackArrival = receiverEnd + tp;

            printf("%d-%d        |                                    | Receive P%d, prepare ACK\n",
                   arrival, receiverEnd, p);

            printf("%d-%d        | <----------------------- ACK P%d | ACK returns to sender\n",
                   receiverEnd, ackArrival, p);

            printf("%-12d | P%d completed\n\n", ackArrival, p);

            time = ackArrival;
        }
    }

    printf("Total packets        : %d\n", packets);
    printf("Lost first attempts  : %d\n", lostCount);
    printf("RTT                  : %d ms\n", rtt);
    printf("Timeout              : %d ms\n", timeout);
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

    // 5. Accept
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    // 6. Run algorithm
    algorithm(client_fd);

    // 7. Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}
