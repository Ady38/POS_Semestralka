#include "threads.h"
#include "input.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

void* menu_input_thread(void* arg) {
    Menu* menu = (Menu*)arg;
    char buffer[BUFFER_SIZE];
    enable_raw_mode();
    while (menu->running) {
        int c = getchar();
        if (c == EOF) break;
        buffer[0] = (char)c;
        buffer[1] = '\0';
        send(menu->client_fd, buffer, 1, 0);
        if (c == 'q' || c == 'Q') {
            printf("\nUkoncujem\n");
            menu->running = 0;
            break;
        }
    }
    disable_raw_mode();
    return NULL;
}

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
        system("clear");
        printf("\r%s\n", buffer);
    }
    return NULL;
}
