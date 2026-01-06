#include "network.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 38200

int client_connect(Menu* menu, const char* address, int port) {
    menu->client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(menu->client_fd < 0){
        perror("Vytvorenie socketu zlyhalo");
        return -1;
    }
    printf("Klient Socket vytvoreny\n");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
        perror("Neplatna adresa");
        close(menu->client_fd);
        return -1;
    }

    if(connect(menu->client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Pripojenie zlyhalo");
        close(menu->client_fd);
        return -1;
    }
    printf("Pripojeny k serveru\n");
    return 0;
}

void client_disconnect(Menu* menu) {
    close(menu->client_fd);
    printf("Klient je ukonceny\n");
}
