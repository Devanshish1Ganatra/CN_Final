
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX 1000

void algorithm(int client_fd)
{
    char message[100], ch;
    int received = 0;
    int window, frames, tt, tp;

    // Receive inputs from sender
    while (received < 99) {
        if (recv(client_fd, &ch, 1, 0) <= 0)
            return;

        if (ch == '\n')
            break;

        message[received++] = ch;
    }

    message[received] = '\0';

    sscanf(message, "%d %d %d %d",
           &window, &frames, &tt, &tp);

    int ackTime[MAX];
    int completed[MAX] = {0};
    int startTime[MAX], sendTime[MAX], arrival[MAX];
    int receiverEnd[MAX];
    int currentTime = 0;
    int outstanding = 0;

    printf("\nSLIDING WINDOW (NOISELESS CHANNEL)\n");
    printf("Window = %d, Frames = %d, Tt = %d ms, Tp = %d ms\n\n",
           window, frames, tt, tp);

    printf("%-12s | %-34s | %-34s\n",
           "Time (ms)", "SENDER", "RECEIVER");

    printf("------------|------------------------------------|------------------------------------\n");

    for (int p = 0; p < frames; p++) {

        // If window is full, wait for the earliest ACK
        while (outstanding >= window) {
            int earliest = -1;

            for (int i = 0; i < p; i++) {
                if (completed[i] == 0) {
                    if (earliest == -1 ||
                        ackTime[i] < ackTime[earliest]) {
                        earliest = i;
                    }
                }
            }

            currentTime = ackTime[earliest];

            // All ACKs arriving by this time free window slots
            for (int i = 0; i < p; i++) {
                if (completed[i] == 0 &&
                    ackTime[i] <= currentTime) {
                    completed[i] = 1;
                    outstanding--;
                }
            }
        }

        // Load and send the next frame
        startTime[p] = currentTime;
        sendTime[p] = startTime[p] + tt;
        arrival[p] = sendTime[p] + tp;
        receiverEnd[p] = arrival[p] + tt;
        ackTime[p] = receiverEnd[p] + tp;

        printf("%d-%d        | Load frame P%d\n",
               startTime[p], sendTime[p], p + 1);

        printf("%-12d | Send P%d -----------------------> | Arrives at %d ms\n",
               sendTime[p], p + 1, arrival[p]);

        printf("%d-%d        |                                    | Receive P%d, prepare ACK\n",
               arrival[p], receiverEnd[p], p + 1);

        printf("%d-%d        | <----------------------- ACK P%d | ACK arrives at %d ms\n",
               receiverEnd[p], ackTime[p], p + 1, ackTime[p]);

        outstanding++;
        currentTime = sendTime[p];
    }

    // Wait for all remaining ACKs
    int totalTime = currentTime;

    for (int i = 0; i < frames; i++) {
        if (ackTime[i] > totalTime)
            totalTime = ackTime[i];
    }

    printf("\nTotal frames sent       : %d\n", frames);
    printf("Window size             : %d\n", window);
    printf("Total completion time   : %d ms\n", totalTime);

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
