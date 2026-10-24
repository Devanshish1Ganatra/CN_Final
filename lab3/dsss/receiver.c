
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_DATA 4096
#define MAX_MESSAGE 32768

void algorithm(int client_fd)
{
    char spreadingPattern[] = "10110111000";
    char receivedData[MAX_MESSAGE];
    char originalData[MAX_DATA];

    int n = strlen(spreadingPattern);
    int received = 0;
    int i, j, pos = 0;
    char ch;

    // Receive data until the sender's newline
    while (received < MAX_MESSAGE - 1) {
        ssize_t bytes = recv(client_fd, &ch, 1, 0);

        if (bytes <= 0)
            return;

        if (ch == '\n')
            break;

        receivedData[received++] = ch;
    }

    receivedData[received] = '\0';

    printf("\nReceived Spread Data:\n%s\n", receivedData);

    printf("\nDecoding Data:\n");

    // Each original bit is represented by 11 bits
    for (i = 0; i < received; i += n) {
        int match = 1;
        int complementMatch = 1;

        // Compare this group with the spreading pattern
        for (j = 0; j < n; j++) {
            if (receivedData[i + j] != spreadingPattern[j])
                match = 0;

            // Check whether it matches the complement
            if (spreadingPattern[j] == '1') {
                if (receivedData[i + j] != '0')
                    complementMatch = 0;
            } else {
                if (receivedData[i + j] != '1')
                    complementMatch = 0;
            }
        }

        if (match == 1) {
            originalData[pos++] = '1';
            printf("Pattern matched     -> 1\n");
        } else if (complementMatch == 1) {
            originalData[pos++] = '0';
            printf("Complement matched  -> 0\n");
        }
    }

    originalData[pos] = '\0';

    printf("\nOriginal Data Received: %s\n", originalData);
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

    // 5. Accept one client
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    // 6. Run DSSS algorithm
    algorithm(client_fd);

    // 7. Send reply
    const char *reply = "DSSS data received successfully!";
    send(client_fd, reply, strlen(reply), 0);

    // 8. Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}
