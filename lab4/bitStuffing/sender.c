
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 10000

void algorithm(int client_fd)
{
    char data[MAX], frame[MAX];
    char flag[] = "01111110";
    int i, j, pos = 0, ones = 0;

    printf("Enter binary data: ");
    scanf("%9999s", data);

    // Add starting flag
    for (i = 0; i < 8; i++)
        frame[pos++] = flag[i];

    // Perform bit stuffing on original data
    for (i = 0; data[i] != '\0'; i++) {
        frame[pos++] = data[i];

        if (data[i] == '1') {
            ones++;

            if (ones == 5) {
                frame[pos++] = '0';  // Stuff one zero
                ones = 0;
            }
        } else {
            ones = 0;
        }
    }

    // Add ending flag
    for (i = 0; i < 8; i++)
        frame[pos++] = flag[i];

    printf("\nStuffed frame:\n");

    for (i = 0; i < pos; i++)
        printf("%c", frame[i]);

    printf("\n");

    // Add newline to mark the end of the TCP message
    frame[pos++] = '\n';

    send(client_fd, frame, pos, 0);

    printf("Frame sent successfully!\n");
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

    // 4. Run bit stuffing algorithm
    algorithm(client_fd);

    // 5. Receive reply
    char buffer[1024];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (n > 0) {
        buffer[n] = '\0';
        printf("Server says: %s\n", buffer);
    }

    // 6. Close socket
    close(client_fd);

    return 0;
}
