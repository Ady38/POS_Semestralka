#include "network.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// Pripojí klienta k serveru na danej adrese a porte
// menu: štruktúra Menu, kde sa uloží file descriptor socketu
// address: IP adresa servera
// port: port servera
// return: 0 pri úspechu, -1 pri chybe
int client_connect(Menu* menu, const char* address, int port) {
    menu->client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(menu->client_fd < 0){
        perror("Vytvorenie socketu zlyhalo");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
        perror("Neplatna adresa");
        close(menu->client_fd);
        return -1;
    }

    // Pokus o pripojenie k serveru
    if(connect(menu->client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Pripojenie zlyhalo");
        close(menu->client_fd);
        return -1;
    }
    printf("Pripojeny k serveru\n");
    return 0;
}

// Odpojí klienta od servera a uzavrie socket
void client_disconnect(Menu* menu) {
    close(menu->client_fd);
}
