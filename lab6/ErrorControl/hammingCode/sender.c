
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 1000

char xorBit(char a, char b)
{
    if (a == b)
        return '0';
    return '1';
}

int isParityPosition(int pos)
{
    int p = 1;

    while (p < pos)
        p = p * 2;

    if (p == pos)
        return 1;

    return 0;
}

void algorithm(int client_fd)
{
    char data[MAX], bits[MAX], codeword[MAX];
    int choice, errorPos, m, r = 0, n, pos = 0;

    printf("Choose simulation:\n");
    printf("1. Correct transmission\n");
    printf("2. Error transmission\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    printf("Enter data bits: ");
    scanf("%999s", data);

    m = strlen(data);

    // Find the number of parity bits
    while (1) {
        int power = 1;

        for (int i = 0; i < r; i++)
            power = power * 2;

        if (power >= m + r + 1)
            break;

        r++;
    }

    n = m + r;

    // Initialize all positions, counted from the right
    for (int i = 1; i <= n; i++)
        bits[i] = '0';

    int dataIndex = m - 1;

    // Place data in non-parity positions
    for (int i = 1; i <= n; i++) {
        if (!isParityPosition(i)) {
            bits[i] = data[dataIndex];
            dataIndex--;
        }
    }

    // Calculate even parity bits using our XOR function
    for (int p = 1; p <= n; p = p * 2) {
        char parity = '0';

        for (int i = 1; i <= n; i++) {
            if ((i / p) % 2 == 1 && i != p)
                parity = xorBit(parity, bits[i]);
        }

        bits[p] = parity;
    }

    // Convert to normal left-to-right codeword
    for (int i = n; i >= 1; i--)
        codeword[pos++] = bits[i];

    codeword[pos] = '\0';

    printf("\nGenerated Hamming codeword: %s\n", codeword);
    printf("Total bits: %d\n", n);

    if (choice == 2) {
        printf("Enter bit position to flip (left to right, 1-%d): ",
               n);
        scanf("%d", &errorPos);

        int index = errorPos - 1;

        // Flip the selected bit before transmission
        if (codeword[index] == '0')
            codeword[index] = '1';
        else
            codeword[index] = '0';

        printf("Corrupted codeword sent: %s\n", codeword);
    } else {
        printf("Correct codeword sent: %s\n", codeword);
    }

    // Send the received candidate codeword
    strcat(codeword, "\n");
    send(client_fd, codeword, strlen(codeword), 0);

    char reply[300];
    int received = recv(client_fd, reply, sizeof(reply) - 1, 0);

    if (received > 0) {
        reply[received] = '\0';
        printf("Receiver says: %s\n", reply);
    }
}

int main()
{
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(client_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        return 1;
    }

    printf("Connected to server!\n");
    algorithm(client_fd);

    close(client_fd);
    return 0;
}
