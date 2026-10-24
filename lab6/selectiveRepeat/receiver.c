
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX 100

void algorithm(int client_fd)
{
    char message[100], ch;
    int received = 0;
    int window, frames, tt, tp, lost;

    // Receive inputs
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

    int sendTime[MAX] = {0};
    int ackTime[MAX] = {0};
    int acked[MAX] = {0};
    int sent[MAX] = {0};
    int retransmitted[MAX] = {0};

    printf("\nSELECTIVE REPEAT ARQ TIMELINE\n");
    printf("Window = %d, Frames = %d\n", window, frames);
    printf("Tt = %d ms, Tp = %d ms\n", tt, tp);
    printf("RTT = %d ms, Timeout = %d ms\n\n",
           rtt, timeout);

    printf("%-12s | %-38s | %-32s\n",
           "Time (ms)", "SENDER", "RECEIVER");

    printf("------------|----------------------------------------|----------------------------------\n");

    int base = 1;
    int nextFrame = 1;
    int loadTime = 0;
    int currentTime = 0;

    // Continue until every frame is acknowledged
    while (base <= frames) {

        // Send frames while they fit inside the window
        while (nextFrame <= frames &&
               nextFrame < base + window) {

            int p = nextFrame;

            int loadStart = loadTime;
            sendTime[p] = loadStart + tt;
            loadTime = sendTime[p];
            sent[p] = 1;

            printf("%d-%d        | Load frame P%d\n",
                   loadStart, sendTime[p], p);

            printf("%-12d | Send P%d -----------------------> | Arrives at %d ms\n",
                   sendTime[p], p, sendTime[p] + tp);

            if (p == lost) {
                // First transmission is lost
                ackTime[p] = sendTime[p] + timeout;

                printf("%-12d | P%d LOST; waiting for timeout\n\n",
                       sendTime[p], p);
            } else {
                int arrival = sendTime[p] + tp;
                int receiverEnd = arrival + tt;

                ackTime[p] = receiverEnd + tp;

                printf("%d-%d        |                                    | Receive/buffer P%d\n",
                       arrival, receiverEnd, p);

                printf("%d-%d        | <----------------------- ACK P%d | ACK arrives at %d ms\n\n",
                       receiverEnd, ackTime[p], p, ackTime[p]);
            }

            nextFrame++;
        }

        // Find the next ACK or timeout event
        int eventTime = 1000000;
        int eventFrame = -1;

        for (int p = 1; p < nextFrame; p++) {
            if (sent[p] && !acked[p] &&
                ackTime[p] < eventTime) {
                eventTime = ackTime[p];
                eventFrame = p;
            }
        }

        if (eventFrame == -1)
            break;

        currentTime = eventTime;

        if (eventFrame == lost && !retransmitted[lost]) {
            // Timeout: retransmit the lost frame without reloading
            printf("%-12d | TIMEOUT for P%d\n",
                   currentTime, lost);

            printf("%-12d | Retransmit P%d ----------------> | Arrives at %d ms\n",
                   currentTime, lost, currentTime + tp);

            int arrival = currentTime + tp;
            int receiverEnd = arrival + tt;
            int retryAck = receiverEnd + tp;

            printf("%d-%d        |                                    | Receive P%d; buffered frames can be delivered in order\n",
                   arrival, receiverEnd, lost);

            printf("%d-%d        | <----------------------- ACK P%d | ACK arrives at %d ms\n\n",
                   receiverEnd, retryAck, lost, retryAck);

            // Schedule the ACK after successful retransmission
            retransmitted[lost] = 1;
            ackTime[lost] = retryAck;
        } else {
            // This frame is now acknowledged
            acked[eventFrame] = 1;

            printf("%-12d | ACK received for P%d\n",
                   currentTime, eventFrame);
        }

        // Slide the window only across consecutive ACKed frames
        while (base <= frames && acked[base])
            base++;

        // Newly available window slots are used on the next iteration
        if (currentTime > loadTime)
            loadTime = currentTime;
    }

    int totalTime = currentTime;

    printf("\nTotal frames           : %d\n", frames);
    printf("Lost first transmission: P%d\n", lost);
    printf("Total completion time  : %d ms\n", totalTime);

    char reply[100];
    sprintf(reply, "Total completion time: %d ms", totalTime);
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
