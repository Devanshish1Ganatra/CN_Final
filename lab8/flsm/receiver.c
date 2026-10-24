
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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

    char message[100];
    char ipText[50];

    int prefix, requiredSubnets;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    listen(server_fd, 1);

    printf("Waiting for sender...\n");

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client, &len);

    int n = recv(client_fd, message, sizeof(message) - 1, 0);

    if (n <= 0) {
        close(client_fd);
        close(server_fd);
        return 0;
    }

    message[n] = '\0';

    sscanf(message, "%49s %d %d",
           ipText, &prefix, &requiredSubnets);

    struct in_addr parsed;

    if (inet_pton(AF_INET, ipText, &parsed) != 1 ||
        prefix < 0 || prefix > 32 ||
        requiredSubnets < 1) {
        printf("Invalid input.\n");
        close(client_fd);
        close(server_fd);
        return 0;
    }

    uint32_t ip = ntohl(parsed.s_addr);

    uint32_t originalMask = 0;

    if (prefix > 0) {
        originalMask = 0xFFFFFFFFU << (32 - prefix);
    }

    uint32_t networkAddress = ip & originalMask;

    int borrowedBits = 0;
    uint64_t possibleSubnets = 1;

    while (possibleSubnets < (uint64_t)requiredSubnets) {
        borrowedBits++;
        possibleSubnets = possibleSubnets * 2;
    }

    int newPrefix = prefix + borrowedBits;

    printf("\n========== FLSM RESULTS ==========\n");

    printIP("Input IP Address", ip);
    printf("Original Prefix: /%d\n", prefix);
    printIP("Original Subnet Mask", originalMask);
    printIP("Original Network Address", networkAddress);

    printf("\n--- Subnet Calculations ---\n");
    printf("Required Subnets: %d\n", requiredSubnets);
    printf("Borrowed Bits: %d\n", borrowedBits);
    printf("New Prefix: /%d\n", newPrefix);
    printf("Possible Equal-Size Subnets: %llu\n",
           (unsigned long long)possibleSubnets);

    uint32_t newMask = 0;

    if (newPrefix > 0) {
        newMask = 0xFFFFFFFFU << (32 - newPrefix);
    }

    uint32_t subnetHostMask = ~newMask;

    printIP("New Subnet Mask", newMask);

    int hostBits = 32 - newPrefix;

    printf("Host Bits per Subnet: %d\n", hostBits);

    uint64_t addressesPerSubnet = 1ULL << hostBits;

    printf("Total Addresses per Subnet: %llu\n",
           (unsigned long long)addressesPerSubnet);

    if (hostBits >= 2) {
        printf("Usable Hosts per Subnet: %llu\n",
               (unsigned long long)(addressesPerSubnet - 2));
    } else if (hostBits == 1) {
        printf("Usable Hosts per Subnet: 2 (special /31 case)\n");
    } else {
        printf("Usable Hosts per Subnet: 1 address (/32)\n");
    }

    printf("\n--- Subnet Details ---\n");

    uint64_t blockSize = addressesPerSubnet;

    for (int i = 0; i < requiredSubnets; i++) {
        uint64_t subnetStart =
            (uint64_t)networkAddress + (uint64_t)i * blockSize;

        if (subnetStart + blockSize - 1 > 0xFFFFFFFFULL) {
            printf("Subnet range exceeds IPv4 address space.\n");
            break;
        }

        uint32_t subnetNetwork = (uint32_t)subnetStart;
        uint32_t subnetBroadcast =
            (uint32_t)(subnetStart + blockSize - 1);

        printf("\nSubnet %d\n", i + 1);

        printIP("Network Address", subnetNetwork);

        if (hostBits >= 2) {
            printIP("First Usable Host", subnetNetwork + 1);
            printIP("Last Usable Host", subnetBroadcast - 1);
            printIP("Broadcast Address", subnetBroadcast);
        } else if (hostBits == 1) {
            printIP("First Address", subnetNetwork);
            printIP("Last Address", subnetBroadcast);
            printf("Broadcast: Not used in the usual /31 point-to-point case\n");
        } else {
            printIP("Single Address", subnetNetwork);
            printf("Separate host range/broadcast: Not applicable\n");
        }
    }

    printf("\n--- FLSM Formulas ---\n");
    printf("Borrowed bits = smallest n where 2^n >= required subnets\n");
    printf("New prefix = original prefix + borrowed bits\n");
    printf("Subnet address = original network + subnet index * block size\n");
    printf("Block size = 2^(32 - new prefix)\n");

    printf("==================================\n");

    close(client_fd);
    close(server_fd);

    return 0;
}
