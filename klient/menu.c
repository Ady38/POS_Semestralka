#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>

#define PORT 38200
#define BUFFER_SIZE 1024

// Vytvorenie a zrusenie menu
Menu* menu_vytvor(bool hra_pozastavena) {
    Menu* menu = (Menu*)malloc(sizeof(Menu));
    if (menu) menu->hra_pozastavena = hra_pozastavena;
    return menu;
}

void menu_zrus(Menu* menu) {
    free(menu);
}

// Funkcie menu
void menu_nova_hra(Menu* menu) {
    printf("\n[Nova hra]\n");
    // Spusti server v tomto procese
    system("server/server &"); // spusti server na pozadi
    sleep(1); // krátka pauza na inicializáciu servera
    menu_pripojit_sa_k_hre(menu);
    // TODO: Vyber rezimu, typu sveta, nacitanie zo suboru, volba poctu hracov
    // menu->hra_pozastavena = false; // pripadne upravit stav
}

void menu_pokracovat_v_hre(Menu* menu) {
    printf("\n[Pokracovanie v hre]\n");
    // TODO: Pokracovanie v pozastavenej hre
}

void menu_koniec(Menu* menu) {
    printf("\nKoniec aplikacie.\n");
    menu_zrus(menu);
    exit(0);
}

void menu_zobraz(Menu* menu) {
    int volba;
    while (1) {
        printf("\n--- HLAVNE MENU ---\n");
        printf("1. Nova hra\n");
        printf("2. Pripojenie k hre\n");
        if (menu->hra_pozastavena) {
            printf("3. Pokracovanie v hre\n");
            printf("4. Koniec\n");
        } else {
            printf("3. Koniec\n");
        }
        printf("Zadajte volbu: ");
        if (scanf("%d", &volba) != 1) {
            while (getchar() != '\n');
            printf("Neplatna volba. Skuste znova.\n");
            continue;
        }
        if (volba == 1) {
            menu_nova_hra(menu);
        } else if (volba == 2) {
            menu_pripojit_sa_k_hre(menu);
        } else if (menu->hra_pozastavena && volba == 3) {
            menu_pokracovat_v_hre(menu);
        } else if ((menu->hra_pozastavena && volba == 4) || (!menu->hra_pozastavena && volba == 3)) {
            menu_koniec(menu);
        } else {
            printf("Neplatna volba. Skuste znova.\n");
        }
    }
}

// Pomocné funkcie na nastavenie terminálu do raw režimu
struct termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void menu_pripojit_sa_k_hre(Menu* menu) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0){
        perror("Vytvorenie socketu zlyhalo");
        return;
    }
    printf("Klient Socket vytvoreny\n");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Neplatna adresa");
        close(client_fd);
        return;
    }

    if(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Pripojenie zlyhalo");
        close(client_fd);
        return;
    }
    printf("Pripojeny k serveru\n");
    printf("Stlac klavesu (q = koniec):\n");

    char buffer[BUFFER_SIZE];
    enable_raw_mode();
    fd_set readfds;
    int running = 1;
    while(running) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client_fd, &readfds);
        int maxfd = (STDIN_FILENO > client_fd ? STDIN_FILENO : client_fd) + 1;
        int activity = select(maxfd, &readfds, NULL, NULL, NULL);
        if(activity < 0) {
            perror("Select zlyhal");
            break;
        }
        // User input
        if(FD_ISSET(STDIN_FILENO, &readfds)) {
            int c = getchar();
            if (c == EOF) break;
            buffer[0] = (char)c;
            buffer[1] = '\0';
            send(client_fd, buffer, 1, 0);
            if(c == 'q' || c == 'Q'){
                printf("\nUkoncujem\n");
                running = 0;
                continue;
            }
            system("clear"); // clear only after sending a message
        }
        // Server message
        if(FD_ISSET(client_fd, &readfds)) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
            if(bytes_read <= 0){
                printf("\nServer sa odpojil\n");
                break;
            }
            buffer[bytes_read] = '\0';
            printf("\rEcho zo servera: %s\n", buffer);
        }
    }
    disable_raw_mode();
    close(client_fd);
    printf("Klient je ukonceny\n");
}
