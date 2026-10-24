#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

struct Subnet {
    int id;
    int hosts;
    int hostBits;
    int prefix;
    uint64_t blockSize;
    uint32_t network;
    uint32_t broadcast;
};

void printIP(char *label, uint32_t ip) {
    struct in_addr address;
    char text[INET_ADDRSTRLEN];

    address.s_addr = htonl(ip);
    inet_ntop(AF_INET, &address, text, sizeof(text));

    printf("%s: %s\n", label, text);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    char message[2048];
    char ipText[50];

    int prefix, n;
    int hosts[50];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    listen(server_fd, 1);

    printf("Waiting for sender...\n");

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client, &len);

    int received = recv(client_fd, message,
                        sizeof(message) - 1, 0);

    if (received <= 0) {
        close(client_fd);
        close(server_fd);
        return 0;
    }

    message[received] = '\0';

    int count = sscanf(message, "%49s %d %d",
                       ipText, &prefix, &n);

    if (count != 3 || n < 1 || n > 50 ||
        prefix < 0 || prefix > 32) {
        printf("Invalid input.\n");
        close(client_fd);
        close(server_fd);
        return 0;
    }

    char *token = strtok(message, " ");
    token = strtok(NULL, " ");
    token = strtok(NULL, " ");

    for (int i = 0; i < n; i++) {
        token = strtok(NULL, " ");

        if (token == NULL) {
            printf("Missing host requirements.\n");
            close(client_fd);
            close(server_fd);
            return 0;
        }

        hosts[i] = 0;

        if (sscanf(token, "%d", &hosts[i]) != 1 ||
            hosts[i] < 1) {
            printf("Invalid host requirement.\n");
            close(client_fd);
            close(server_fd);
            return 0;
        }
    }

    struct in_addr parsed;

    if (inet_pton(AF_INET, ipText, &parsed) != 1) {
        printf("Invalid IPv4 address.\n");
        close(client_fd);
        close(server_fd);
        return 0;
    }

    uint32_t ip = ntohl(parsed.s_addr);

    uint32_t originalMask = 0;

    if (prefix > 0) {
        originalMask = 0xFFFFFFFFU << (32 - prefix);
    }

    uint32_t network = ip & originalMask;

    struct Subnet subnet[50];

    for (int i = 0; i < n; i++) {
        subnet[i].id = i + 1;
        subnet[i].hosts = hosts[i];

        int bits = 0;
        uint64_t block = 1;

        while (block < (uint64_t)hosts[i] + 2) {
            block *= 2;
            bits++;
        }

        subnet[i].hostBits = bits;
        subnet[i].prefix = 32 - bits;
        subnet[i].blockSize = block;
    }

    /* Sort by descending host requirement */
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (subnet[j].hosts > subnet[i].hosts) {
                struct Subnet temp = subnet[i];
                subnet[i] = subnet[j];
                subnet[j] = temp;
            }
        }
    }

    printf("\n========== VLSM RESULTS ==========\n");

    printIP("Input IP Address", ip);
    printf("Original Prefix: /%d\n", prefix);
    printIP("Original Subnet Mask", originalMask);
    printIP("Original Network Address", network);

    printf("\nSubnet requirements (largest first):\n");

    uint64_t nextAddress = network;
    uint64_t networkEnd = (uint64_t)network +
                          (1ULL << (32 - prefix)) - 1;

    for (int i = 0; i < n; i++) {
        uint64_t block = subnet[i].blockSize;

        /* Align next address to the subnet block boundary */
        uint64_t aligned =
            ((nextAddress + block - 1) / block) * block;

        if (subnet[i].prefix < prefix ||
            aligned + block - 1 > networkEnd ||
            aligned + block - 1 > 0xFFFFFFFFULL) {
            printf("\nNot enough address space for all subnets.\n");
            close(client_fd);
            close(server_fd);
            return 0;
        }

        subnet[i].network = (uint32_t)aligned;
        subnet[i].broadcast = (uint32_t)(aligned + block - 1);

        nextAddress = aligned + block;

        printf("\nSubnet for original requirement #%d\n",
               subnet[i].id);

        printf("Required Hosts: %d\n", subnet[i].hosts);
        printf("Host Bits: %d\n", subnet[i].hostBits);
        printf("New Prefix: /%d\n", subnet[i].prefix);

        uint32_t mask = 0;

        if (subnet[i].prefix > 0) {
            mask = 0xFFFFFFFFU << (32 - subnet[i].prefix);
        }

        printIP("Subnet Mask", mask);
        printIP("Network Address", subnet[i].network);

        if (subnet[i].hostBits >= 2) {
            printIP("First Usable Host",
                    subnet[i].network + 1);
            printIP("Last Usable Host",
                    subnet[i].broadcast - 1);
            printIP("Broadcast Address",
                    subnet[i].broadcast);

            printf("Usable Hosts: %llu\n",
                   (unsigned long long)(block - 2));
        } else if (subnet[i].hostBits == 1) {
            printIP("First Address", subnet[i].network);
            printIP("Last Address", subnet[i].broadcast);
            printf("Usable Hosts: 2 (special /31 case)\n");
            printf("Broadcast: Not used in the usual /31 point-to-point case\n");
        } else {
            printIP("Single Address", subnet[i].network);
            printf("Usable Hosts: 1 address (/32)\n");
            printf("Broadcast: Not applicable\n");
        }
    }

    printf("\n--- VLSM Formulas ---\n");
    printf("Required block size = next power of 2 >= hosts + 2\n");
    printf("New prefix = 32 - host bits\n");
    printf("Subnets are allocated from largest to smallest.\n");
    printf("==================================\n");

    close(client_fd);
    close(server_fd);

    return 0;
}