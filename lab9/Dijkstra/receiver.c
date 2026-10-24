
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAXV 50
#define MAXE 500
#define INF LLONG_MAX

struct Edge {
    int u, v, w;
};

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    char message[4096];
    int V, E, source;
    struct Edge edges[MAXE];
    long long dist[MAXV];
    int visited[MAXV] = {0};
    int weightValid = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    listen(server_fd, 1);

    printf("Waiting for sender...\n");
    client_fd = accept(server_fd, (struct sockaddr *)&client, &len);

    int n = recv(client_fd, message, sizeof(message) - 1, 0);

    if (n <= 0) {
        close(client_fd);
        close(server_fd);
        return 0;
    }

    message[n] = '\0';

    char *token = strtok(message, " ");
    if (token == NULL) return 0;
    sscanf(token, "%d", &V);

    token = strtok(NULL, " ");
    if (token == NULL) return 0;
    sscanf(token, "%d", &E);

    if (V < 1 || V > MAXV || E < 0 || E > MAXE) {
        printf("Invalid graph size.\n");
        close(client_fd);
        close(server_fd);
        return 0;
    }

    for (int i = 0; i < E; i++) {
        token = strtok(NULL, " ");
        if (token == NULL) return 0;
        sscanf(token, "%d", &edges[i].u);

        token = strtok(NULL, " ");
        if (token == NULL) return 0;
        sscanf(token, "%d", &edges[i].v);

        token = strtok(NULL, " ");
        if (token == NULL) return 0;
        sscanf(token, "%d", &edges[i].w);

        if (edges[i].u < 0 || edges[i].u >= V ||
            edges[i].v < 0 || edges[i].v >= V) {
            printf("Invalid edge vertex.\n");
            close(client_fd);
            close(server_fd);
            return 0;
        }

        if (edges[i].w < 0) {
            weightValid = 0;
        }
    }

    token = strtok(NULL, " ");
    if (token == NULL) return 0;
    sscanf(token, "%d", &source);

    if (source < 0 || source >= V) {
        printf("Invalid source vertex.\n");
        close(client_fd);
        close(server_fd);
        return 0;
    }

    if (!weightValid) {
        printf("Dijkstra cannot be used with negative edge weights.\n");
        close(client_fd);
        close(server_fd);
        return 0;
    }

    for (int i = 0; i < V; i++) {
        dist[i] = INF;
    }
    dist[source] = 0;

    for (int count = 0; count < V; count++) {
        int u = -1;

        for (int i = 0; i < V; i++) {
            if (!visited[i] && dist[i] != INF &&
                (u == -1 || dist[i] < dist[u])) {
                u = i;
            }
        }

        if (u == -1) break;

        visited[u] = 1;

        for (int j = 0; j < E; j++) {
            if (edges[j].u == u) {
                int v = edges[j].v;
                int w = edges[j].w;

                if (!visited[v] && dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                }
            }
        }
    }

    printf("\n===== DIJKSTRA =====\n");

    for (int i = 0; i < V; i++) {
        if (dist[i] == INF) {
            printf("Distance to %d: INF (unreachable)\n", i);
        } else {
            printf("Distance to %d: %lld\n", i, dist[i]);
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
