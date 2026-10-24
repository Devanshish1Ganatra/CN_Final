#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void printIP(char *label, uint32_t ip) {
    struct in_addr addr;
    addr.s_addr = htonl(ip);

    printf("%s: %s\n", label, inet_ntoa(addr));
}

void printBinary(uint32_t ip) {
    for (int i = 3; i >= 0; i--) {
        unsigned int octet = (ip >> (i * 8)) & 255;

        for (int j = 7; j >= 0; j--) {
            printf("%d", (octet >> j) & 1);
        }

        if (i != 0)
            printf(".");
    }

    printf("\n");
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    char ipText[50];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    listen(server_fd, 1);

    printf("Waiting for sender...\n");

    client_fd = accept(server_fd, (struct sockaddr *)&client, &len);

    int n = recv(client_fd, ipText, sizeof(ipText) - 1, 0);

    if (n <= 0) {
        close(client_fd);
        close(server_fd);
        return 0;
    }

    ipText[n] = '\0';

    struct in_addr parsed;

    if (inet_pton(AF_INET, ipText, &parsed) != 1) {
        printf("Invalid IPv4 address.\n");
        close(client_fd);
        close(server_fd);
        return 0;
    }

    uint32_t ip = ntohl(parsed.s_addr);
    unsigned int first = (ip >> 24) & 255;

    uint32_t mask;
    uint32_t hostMask;
    uint32_t networkID;
    uint32_t hostID;
    uint32_t broadcast;

    int networkBits, hostBits;
    char className;

    printf("\n========== IPv4 INFORMATION ==========\n");

    printIP("IP Address", ip);

    printf("Binary IP: ");
    printBinary(ip);

    if (first >= 1 && first <= 126) {
        className = 'A';
        mask = 0xFF000000;
        networkBits = 8;
        hostBits = 24;
    } else if (first >= 128 && first <= 191) {
        className = 'B';
        mask = 0xFFFF0000;
        networkBits = 16;
        hostBits = 16;
    } else if (first >= 192 && first <= 223) {
        className = 'C';
        mask = 0xFFFFFF00;
        networkBits = 24;
        hostBits = 8;
    } else if (first >= 224 && first <= 239) {
        printf("Class: D\n");
        printf("Purpose: Multicast\n");
        printf("Classful host and broadcast calculations do not apply.\n");
        goto extra;
    } else {
        printf("Class: E\n");
        printf("Purpose: Reserved / Experimental\n");
        printf("Classful host and broadcast calculations do not apply.\n");
        goto extra;
    }

    printf("Class: %c\n", className);

    hostMask = ~mask;
    networkID = ip & mask;
    hostID = ip & hostMask;
    broadcast = networkID | hostMask;

    printIP("Default Subnet Mask", mask);

    printf("Binary Subnet Mask: ");
    printBinary(mask);

    printIP("Network ID", networkID);
    printIP("Host ID", hostID);
    printIP("Broadcast Address", broadcast);

    printf("Network Bits: %d\n", networkBits);
    printf("Host Bits: %d\n", hostBits);

    printf("Total Addresses: %llu\n",
           1ULL << hostBits);

    printf("Usable Hosts: %llu\n",
           (1ULL << hostBits) - 2);

    printIP("First Usable Host", networkID + 1);
    printIP("Last Usable Host", broadcast - 1);

    printf("\nCalculation:\n");
    printf("Network ID = IP AND Subnet Mask\n");
    printf("Host ID = IP AND Inverted Subnet Mask\n");
    printf("Broadcast = Network ID OR Inverted Subnet Mask\n");

extra:
    if (first == 10 ||
        (first == 172 &&
         ((ip >> 16) & 255) >= 16 &&
         ((ip >> 16) & 255) <= 31) ||
        (first == 192 && ((ip >> 16) & 255) == 168)) {
        printf("Address Type: Private IPv4\n");
    } else if (first == 127) {
        printf("Address Type: Loopback\n");
    } else if (first == 169 && ((ip >> 16) & 255) == 254) {
        printf("Address Type: Link-local\n");
    } else {
        printf("Address Type: Not in standard private ranges\n");
    }

    printf("======================================\n");

    close(client_fd);
    close(server_fd);

    return 0;
}