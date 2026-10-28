#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in server, client;
    socklen_t clientSize;

    char ip[50];
    char result[1000];

    int a, b, c, d;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        return 1;
    }

    listen(serverSocket, 5);

    printf("Server waiting for client...\n");

    clientSize = sizeof(client);

    clientSocket = accept(serverSocket,
                           (struct sockaddr *)&client,
                           &clientSize);

    if (clientSocket < 0)
    {
        printf("Accept failed\n");
        return 1;
    }

    printf("Client connected.\n");

    while (1)
    {
        memset(ip, '\0', sizeof(ip));

        read(clientSocket, ip, sizeof(ip));

        if (strcmp(ip, "exit") == 0)
            break;

        if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
        {
            strcpy(result, "Invalid IP Address\n");
            write(clientSocket, result, strlen(result));
            continue;
        }

        if (a < 0 || a > 255 ||
            b < 0 || b > 255 ||
            c < 0 || c > 255 ||
            d < 0 || d > 255)
        {
            strcpy(result, "Invalid IP Address\n");
            write(clientSocket, result, strlen(result));
            continue;
        }

        /* Class A */
        if (a >= 1 && a <= 126)
        {
            sprintf(result,
                    "IP Address        : %s\n"
                    "Class             : A\n"
                    "No. of Networks   : 2^7 - 2 = 126\n"
                    "Hosts / Network   : 2^24 - 2 = 16777214\n"
                    "LBA               : %d.0.0.0\n"
                    "DBA               : %d.255.255.255\n",
                    ip, a, a);
        }

        /* Class B */
        else if (a >= 128 && a <= 191)
        {
            sprintf(result,
                    "IP Address        : %s\n"
                    "Class             : B\n"
                    "No. of Networks   : 2^14 = 16384\n"
                    "Hosts / Network   : 2^16 - 2 = 65534\n"
                    "LBA               : %d.%d.0.0\n"
                    "DBA               : %d.%d.255.255\n",
                    ip, a, b, a, b);
        }

        /* Class C */
        else if (a >= 192 && a <= 223)
        {
            sprintf(result,
                    "IP Address        : %s\n"
                    "Class             : C\n"
                    "No. of Networks   : 2^21 = 2097152\n"
                    "Hosts / Network   : 2^8 - 2 = 254\n"
                    "LBA               : %d.%d.%d.0\n"
                    "DBA               : %d.%d.%d.255\n",
                    ip, a, b, c, a, b, c);
        }

        /* Class D */
        else if (a >= 224 && a <= 239)
        {
            sprintf(result,
                    "IP Address        : %s\n"
                    "Class             : D\n"
                    "No. of Networks   : Not Applicable\n"
                    "Hosts / Network   : Not Applicable\n"
                    "LBA               : Not Applicable\n"
                    "DBA               : Not Applicable\n",
                    ip);
        }

        /* Class E */
        else if (a >= 240 && a <= 255)
        {
            sprintf(result,
                    "IP Address        : %s\n"
                    "Class             : E\n"
                    "No. of Networks   : Not Applicable\n"
                    "Hosts / Network   : Not Applicable\n"
                    "LBA               : Not Applicable\n"
                    "DBA               : Not Applicable\n",
                    ip);
        }

        /* 0 and 127 */
        else
        {
            sprintf(result,
                    "IP Address        : %s\n"
                    "Invalid / Reserved IP Address\n",
                    ip);
        }


        write(clientSocket, result, strlen(result));
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}
