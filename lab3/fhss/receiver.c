
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_K 8
#define MAX_DATA 4096
#define MAX_MESSAGE 32768

void algorithm(int client_fd)
{
    int k, size, i, j, n, freq;
    int frequencyMap[1 << MAX_K];
    char binaryMap[1 << MAX_K][MAX_K + 1];
    char message[MAX_MESSAGE];
    char data[MAX_DATA];

    // Receive the complete message, ending at newline
    int received = 0;
    char ch;

    while (received < MAX_MESSAGE - 1) {
        ssize_t bytes = recv(client_fd, &ch, 1, 0);

        if (bytes <= 0) {
            printf("Connection closed.\n");
            return;
        }

        if (ch == '\n')
            break;

        message[received++] = ch;
    }

    message[received] = '\0';

    // The first number in the message is k
    char *token = strtok(message, " ");
    k = atoi(token);

    size = 1 << k;

    // Create the same frequency map and binary map
    for (i = 0; i < size; i++) {
        frequencyMap[i] = 200 + i * 100;

        for (j = 0; j < k; j++) {
            // Extract bit j using right shift and AND
            int bit = (i >> j) & 1;

            if (bit == 1)
                binaryMap[i][k - 1 - j] = '1';
            else
                binaryMap[i][k - 1 - j] = '0';
        }

        binaryMap[i][k] = '\0';
    }

    printf("Received k = %d\n", k);
    printf("\nReceiver Frequency Map:\n");

    for (i = 0; i < size; i++)
        printf("%s -> %d Hz\n",
               binaryMap[i], frequencyMap[i]);

    printf("\nReceived Frequency Hopping Sequence:\n");

    int pos = 0;

    // Remaining numbers are the frequencies
    token = strtok(NULL, " ");

    while (token != NULL) {
        freq = atoi(token);
        n = -1;

        // Find the matching frequency
        for (i = 0; i < size; i++) {
            if (frequencyMap[i] == freq) {
                n = i;
                break;
            }
        }

        if (n != -1) {
            printf("%d Hz -> %s\n", freq, binaryMap[n]);

            // Append the corresponding binary pattern
            for (j = 0; j < k; j++)
                data[pos++] = binaryMap[n][j];
        }

        token = strtok(NULL, " ");
    }

    data[pos] = '\0';

    printf("\nOriginal Data Received: %s\n", data);
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

    // Allow reuse of the port
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

    // 5. Accept one client
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    // 6. Run FHSS algorithm
    algorithm(client_fd);

    // 7. Send reply
    const char *reply = "FHSS data received successfully!";

    send(client_fd, reply, strlen(reply), 0);

    // 8. Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}
