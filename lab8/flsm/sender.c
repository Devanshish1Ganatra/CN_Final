
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock;
    struct sockaddr_in server;

    char ip[50];
    int prefix, requiredSubnets;
    char message[100];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    printf("Enter network IPv4 address: ");
    scanf("%49s", ip);

    printf("Enter existing prefix length: ");
    scanf("%d", &prefix);

    printf("Enter required number of subnets: ");
    scanf("%d", &requiredSubnets);

    sprintf(message, "%s %d %d",
            ip, prefix, requiredSubnets);

    send(sock, message, strlen(message) + 1, 0);

    printf("\nInputs sent to receiver.\n");

    close(sock);
    return 0;
}
