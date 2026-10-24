#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock;
    struct sockaddr_in server;

    char ip[50];
    int prefix, n;
    int hosts[50];
    char message[2048];
    int pos;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    printf("Enter network IPv4 address: ");
    scanf("%49s", ip);

    printf("Enter original prefix length: ");
    scanf("%d", &prefix);

    printf("Enter number of subnets: ");
    scanf("%d", &n);

    if (n < 1 || n > 50) {
        printf("Number of subnets must be between 1 and 50.\n");
        close(sock);
        return 0;
    }

    for (int i = 0; i < n; i++) {
        printf("Enter required hosts for subnet %d: ", i + 1);
        scanf("%d", &hosts[i]);
    }

    pos = sprintf(message, "%s %d %d", ip, prefix, n);

    for (int i = 0; i < n; i++) {
        pos += sprintf(message + pos, " %d", hosts[i]);
    }

    send(sock, message, strlen(message) + 1, 0);

    printf("\nVLSM input sent to receiver.\n");

    close(sock);
    return 0;
}