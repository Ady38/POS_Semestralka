#include "threads.h"
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

// Vlákno pre čítanie vstupu od používateľa a odosielanie na server
// menu: pointer na Menu, kde je socket a príznak running
void* menu_input_thread(void* arg) {
    Menu* menu = (Menu*)arg;
    char buffer[BUFFER_SIZE];
    enable_raw_mode(); // zapne raw mód pre okamžité čítanie kláves
    while (menu->running) {
        int c = getchar(); // načítaj znak
        if (c == EOF) break;
        buffer[0] = (char)c;
        buffer[1] = '\0';
        send(menu->client_fd, buffer, 1, 0); // pošli znak na server
        if (c == 'q' || c == 'Q') { // ak používateľ stlačí q, ukonči
            printf("\nUkoncujem\n");
            menu->running = 0;
            break;
        }
    }
    disable_raw_mode(); // obnoví pôvodné nastavenia terminálu
    return NULL;
}

// Vlákno pre prijímanie správ zo servera a ich zobrazovanie
// menu: pointer na Menu, kde je socket a príznak running
void* menu_recv_thread(void* arg) {
    Menu* menu = (Menu*)arg;
    char buffer[BUFFER_SIZE];
    while (menu->running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(menu->client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            printf("\nServer sa odpojil\n");
            menu->running = 0;
            break;
        }
        buffer[bytes_read] = '\0';
        system("clear"); // vyčistí obrazovku
        printf("\r%s\n", buffer); // vypíše stav sveta
    }
    return NULL;
}
