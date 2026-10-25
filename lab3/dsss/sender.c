
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_DATA 4096
#define MAX_MESSAGE 32768

void algorithm(int client_fd)
{
    char spreadingPattern[] = "10110111000";
    char data[MAX_DATA];
    char message[MAX_MESSAGE];

    int n = strlen(spreadingPattern);
    int i, j, pos = 0;

    printf("Enter binary data: ");
    scanf("%4095s", data);

    printf("\nSpreading Pattern: %s\n", spreadingPattern);

    // Spread each input bit
    for (i = 0; data[i] != '\0'; i++) {
        printf("\nData bit %c: ", data[i]);

        for (j = 0; j < n; j++) {
            char bit;

            if (data[i] == '1') {
                // For 1, use the original pattern
                bit = spreadingPattern[j];
            } else {
                // For 0, complement the pattern
                if (spreadingPattern[j] == '1')
                    bit = '0';
                else
                    bit = '1';
            }

            message[pos++] = bit;
            printf("%c", bit);
        }
    }

    message[pos] = '\0';

    printf("\n\nTransmitted Data:\n%s\n", message);

    // Send the complete spread data
    message[pos++] = '\n';
    send(client_fd, message, pos, 0);

    printf("\nSpread data sent successfully!\n");
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

    // 4. Run DSSS algorithm
    algorithm(client_fd);

    // 5. Receive reply
    char buffer[1024];
    ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("Server says: %s\n", buffer);
    }

    // 6. Close socket
    close(client_fd);

    return 0;
}
