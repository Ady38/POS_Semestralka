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
#include <pthread.h>

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
    system("./server &"); // spusti server na pozadi
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
        system("clear");
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
        printf("\rEcho zo servera: %s\n", buffer);
    }
    return NULL;
}

void menu_pripojit_sa_k_hre(Menu* menu) {
    menu->client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(menu->client_fd < 0){
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
        close(menu->client_fd);
        return;
    }

    if(connect(menu->client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Pripojenie zlyhalo");
        close(menu->client_fd);
        return;
    }
    printf("Pripojeny k serveru\n");
    printf("Stlac klavesu (q = koniec):\n");

    menu->running = 1;
    pthread_create(&menu->input_thread, NULL, menu_input_thread, menu);
    pthread_create(&menu->recv_thread, NULL, menu_recv_thread, menu);
    pthread_join(menu->input_thread, NULL);
    pthread_join(menu->recv_thread, NULL);

    close(menu->client_fd);
    printf("Klient je ukonceny\n");
}
