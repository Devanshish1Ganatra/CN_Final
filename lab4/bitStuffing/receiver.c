
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX 10000

void algorithm(int client_fd)
{
    char frame[MAX], data[MAX];
    char ch;
    int received = 0, pos = 0;
    int i, ones = 0;

    // Receive the complete frame until newline
    while (received < MAX - 1) {
        ssize_t n = recv(client_fd, &ch, 1, 0);

        if (n <= 0)
            return;

        if (ch == '\n')
            break;

        frame[received++] = ch;
    }

    frame[received] = '\0';

    printf("\nReceived frame:\n%s\n", frame);

    // Skip the 8-bit starting flag and ending flag
    for (i = 8; i < received - 8; i++) {
        data[pos++] = frame[i];

        if (frame[i] == '1') {
            ones++;

            if (ones == 5) {
                i++;       // Skip the stuffed zero
                ones = 0;
            }
        } else {
            ones = 0;
        }
    }

    data[pos] = '\0';

    printf("\nOriginal data received:\n%s\n", data);
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

    // 6. Run bit stuffing algorithm
    algorithm(client_fd);

    // 7. Send reply
    const char *reply = "Bit stuffing completed successfully!";
    send(client_fd, reply, strlen(reply), 0);

    // 8. Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}
