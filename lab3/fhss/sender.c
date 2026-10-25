
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_K 8
#define MAX_DATA 4096
#define MAX_MESSAGE 32768

void algorithm(int client_fd)
{
    int k, size, i, j, n;
    int frequencyMap[1 << MAX_K];
    char binaryMap[1 << MAX_K][MAX_K + 1];
    char data[MAX_DATA];
    char message[MAX_MESSAGE];

    printf("Enter k: ");
    scanf("%d", &k);
    getchar();

    size = 1 << k;  // 2^k possible binary patterns

    // Create frequency map and binary map
    for (i = 0; i < size; i++) {
        frequencyMap[i] = 200 + i * 100;

        for (j = 0; j < k; j++) {
            // Shift bit j to the rightmost position
            // AND with 1 extracts that bit
            int bit = (i >> j) & 1;

            if (bit == 1)
                binaryMap[i][k - 1 - j] = '1';
            else
                binaryMap[i][k - 1 - j] = '0';
        }

        binaryMap[i][k] = '\0';
    }

    printf("\nFrequency Map:\n");

    for (i = 0; i < size; i++)
        printf("%s -> %d Hz\n",
               binaryMap[i], frequencyMap[i]);

    printf("\nEnter binary data: ");
    fgets(data, sizeof(data), stdin);
    data[strcspn(data, "\n")] = '\0';

    int len = strlen(data);
    int count = len / k;
    int pos = 0;

    // Send k first so the receiver can create its map
    pos += sprintf(message + pos, "%d ", k);

    printf("\nFrequency Hopping Sequence:\n");

    for (i = 0; i < count; i++) {
        n = 0;

        for (j = 0; j < k; j++) {
            // Shift left and append the next binary bit
            n = (n << 1) | (data[i * k + j] - '0');
        }

        printf("%d Hz\n", frequencyMap[n]);

        // Append only the frequency to the message
        pos += sprintf(message + pos, "%d ",
                       frequencyMap[n]);
    }

    // Newline marks the end of the message
    message[pos++] = '\n';

    send(client_fd, message, pos, 0);

    printf("\nFrequency sequence sent!\n");
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

    // 4. Run FHSS algorithm
    algorithm(client_fd);

    // 5. Receive reply
    char buffer[1024];
    ssize_t bytes = recv(client_fd, buffer,
                         sizeof(buffer) - 1, 0);

    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("Server says: %s\n", buffer);
    } else if (bytes == -1) {
        perror("recv");
    }

    // 6. Close socket
    close(client_fd);

    return 0;
}
