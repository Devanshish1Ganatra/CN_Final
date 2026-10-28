#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int clientSocket;
    struct sockaddr_in server;

    char ip[50];
    char result[1000];


    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket,
                (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        printf("Connection failed\n");
        return 1;
    }

    printf("Connected to server.\n");

    while (1)
    {
        printf("\nEnter IP address (or exit): ");
        scanf("%s", ip);

        write(clientSocket, ip, strlen(ip) + 1);

        if (strcmp(ip, "exit") == 0)
            break;

        memset(result, '\0', sizeof(result));

        read(clientSocket, result, sizeof(result));

        printf("\n----------- RESULT -----------\n");
        printf("%s", result);
        printf("------------------------------\n");
    }

    close(clientSocket);

    return 0;
}
