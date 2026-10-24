
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock;
    struct sockaddr_in server;

    int V, E, source;
    int u, v, w;
    char message[4096];
    int pos = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    printf("Enter number of vertices: ");
    scanf("%d", &V);

    printf("Enter number of edges: ");
    scanf("%d", &E);

    pos = sprintf(message, "%d %d ", V, E);

    printf("Enter each edge as: source destination weight\n");

    for (int i = 0; i < E; i++) {
        scanf("%d %d %d", &u, &v, &w);
        pos += sprintf(message + pos, "%d %d %d ", u, v, w);
    }

    printf("Enter source vertex: ");
    scanf("%d", &source);

    pos += sprintf(message + pos, "%d", source);

    send(sock, message, strlen(message) + 1, 0);

    printf("Graph sent to receiver.\n");

    close(sock);
    return 0;
}
