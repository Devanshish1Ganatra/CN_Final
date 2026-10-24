
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

void algorithm(int client_fd)
{
    char message[100], ch;
    int received = 0;
    int window, frames, tt, tp, lost;

    // Receive inputs from sender
    while (received < 99) {
        if (recv(client_fd, &ch, 1, 0) <= 0)
            return;

        if (ch == '\n')
            break;

        message[received++] = ch;
    }

    message[received] = '\0';

    sscanf(message, "%d %d %d %d %d",
           &window, &frames, &tt, &tp, &lost);

    int rtt = 2 * tp;
    int timeout = 3 * rtt;

    printf("\nGO-BACK-N ARQ TIMELINE\n");
    printf("Window = %d, Frames = %d\n", window, frames);
    printf("Tt = %d ms, Tp = %d ms\n", tt, tp);
    printf("RTT = %d ms, Timeout = %d ms\n\n",
           rtt, timeout);

    printf("%-12s | %-38s | %-32s\n",
           "Time (ms)", "SENDER", "RECEIVER");

    printf("------------|----------------------------------------|----------------------------------\n");

    int time = 0;

    // Initial transmission of the first window
    for (int p = 1; p <= window && p <= frames; p++) {
        int sendTime = time + tt;
        int arrival = sendTime + tp;

        printf("%d-%d        | Load frame P%d\n",
               time, sendTime, p);

        printf("%-12d | Send P%d -----------------------> | Arrives at %d ms\n",
               sendTime, p, arrival);

        if (p == lost) {
            printf("%-12d | P%d LOST\n", sendTime, p);
        } else if (p < lost) {
            int receiverEnd = arrival + tt;
            int ackArrival = receiverEnd + tp;

            printf("%d-%d        |                                    | Receive P%d, prepare ACK\n",
                   arrival, receiverEnd, p);

            printf("%d-%d        | <----------------------- ACK P%d | ACK returns at %d ms\n",
                   receiverEnd, ackArrival, p, ackArrival);
        } else {
            printf("%-12d |                                    | P%d discarded (out of order)\n",
                   arrival, p);
        }

        time = sendTime;
    }

    /*
       This example models one lost frame.
       For the diagram: W = 3, N = 5, lost = 2.
    */

    int lostSendTime = (lost - 1) * tt + tt;
    int timeoutAt = lostSendTime + timeout;

    // ACK for the first frame frees one window slot
    int firstAck = tt + tp + tt + tp;

    time = firstAck;

    // Send any new frame allowed by the sliding window
    for (int p = window + 1; p <= frames; p++) {
        if (p < lost + window) {
            int sendTime = time + tt;
            int arrival = sendTime + tp;

            printf("%d-%d        | Load frame P%d\n",
                   time, sendTime, p);

            printf("%-12d | Send P%d -----------------------> | Arrives at %d ms\n",
                   sendTime, p, arrival);

            printf("%-12d |                                    | P%d discarded (waiting for P%d)\n",
                   arrival, p, lost);

            time = sendTime;
        }
    }

    printf("\n%-12d | TIMEOUT for P%d\n", timeoutAt, lost);
    printf("%-12d | Go back to P%d; retransmit outstanding frames\n\n",
           timeoutAt, lost);

    // Retransmit lost packet without loading it again
    int retryArrival = timeoutAt + tp;
    int receiverEnd = retryArrival + tt;
    int ackArrival = receiverEnd + tp;

    printf("%-12d | Retransmit P%d ----------------> | Arrives at %d ms\n",
           timeoutAt, lost, retryArrival);

    printf("%d-%d        |                                    | Receive P%d, prepare ACK\n",
           retryArrival, receiverEnd, lost);

    printf("%d-%d        | <----------------------- ACK P%d | ACK returns at %d ms\n",
           receiverEnd, ackArrival, lost, ackArrival);

    // Retransmit the later outstanding frames
    time = timeoutAt;

    for (int p = lost + 1; p <= frames && p < lost + window; p++) {
        int sendTime = time + tt;
        int arrival = sendTime + tp;
        int receiverEnd = arrival + tt;
        int ack = receiverEnd + tp;

        printf("%d-%d        | Reload frame P%d\n",
               time, sendTime, p);

        printf("%-12d | Retransmit P%d ----------------> | Arrives at %d ms\n",
               sendTime, p, arrival);

        printf("%d-%d        |                                    | Receive P%d, prepare ACK\n",
               arrival, receiverEnd, p);

        printf("%d-%d        | <----------------------- ACK P%d | ACK returns at %d ms\n",
               receiverEnd, ack, p, ack);

        time = sendTime;
    }

    // For the demonstrated case, P5 can be sent after ACK P2.
    if (frames > window + 1) {
        int nextTime = ackArrival;
        int sendTime = nextTime + tt;
        int arrival = sendTime + tp;
        int receiverEnd = arrival + tt;
        int finalTime = receiverEnd + tp;

        printf("\n%d-%d        | Load next frame P%d\n",
               nextTime, sendTime, frames);

        printf("%-12d | Send P%d -----------------------> | Arrives at %d ms\n",
               sendTime, frames, arrival);

        printf("%d-%d        |                                    | Receive P%d, prepare ACK\n",
               arrival, receiverEnd, frames);

        printf("%d-%d        | <----------------------- ACK P%d | ACK returns at %d ms\n",
               receiverEnd, finalTime, frames, finalTime);

        time = finalTime;
    } else {
        time = ackArrival;
    }

    printf("\nTotal frames: %d\n", frames);
    printf("Lost frame: P%d\n", lost);
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
