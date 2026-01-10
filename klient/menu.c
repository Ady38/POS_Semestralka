#include "menu.h"
#include "threads.h"
#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>


// Vytvorí a inicializuje štruktúru Menu
Menu* menu_vytvor(bool hra_pozastavena) {
    Menu* menu = (Menu*)malloc(sizeof(Menu));
    if (menu) menu->hra_pozastavena = hra_pozastavena;
    return menu;
}

// Uvoľní pamäť Menu
void menu_zrus(Menu* menu) {
    free(menu);
}

// Pomocná funkcia: zistí voľný port
int najdi_volny_port() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0; // systém vyberie voľný port
    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    socklen_t len = sizeof(addr);
    getsockname(sockfd, (struct sockaddr*)&addr, &len);
    int port = ntohs(addr.sin_port);
    close(sockfd);
    return port;
}

// Spustí novú hru: načíta parametre, spustí server a pripojí sa
void menu_nova_hra(Menu* menu) {
    // Bezpečne ukonči staré spojenie a vlákna, ak boli spustené
    if (menu->client_fd > 0 || menu->running) {
        menu->running = 0;
        if (menu->input_thread) {
            pthread_cancel(menu->input_thread);
            pthread_join(menu->input_thread, NULL);
            menu->input_thread = 0;
        }
        if (menu->recv_thread) {
            pthread_cancel(menu->recv_thread);
            pthread_join(menu->recv_thread, NULL);
            menu->recv_thread = 0;
        }
        if (menu->client_fd > 0) {
            client_disconnect(menu);
            menu->client_fd = -1;
        }
        menu->paused = 0;
        menu->hra_pozastavena = 0;
    }
    printf("\n[Nova hra]\n");
    int size, mode, end_mode, game_time = 0;
    // Zadanie veľkosti sveta
    printf("Zadajte velkost sveta (10-30): ");
    while (scanf("%d", &size) != 1 || size < 10 || size > 30) {
        printf("Neplatna velkost. Zadajte cislo 10-30: ");
        while (getchar() != '\n');
    }
    // Zadanie herného režimu
    printf("Zadajte herny rezim (0=bez prekazok, 1=s prekazami): ");
    while (scanf("%d", &mode) != 1 || (mode != 0 && mode != 1)) {
        printf("Neplatny rezim. Zadajte 0 alebo 1: ");
        while (getchar() != '\n');
    }
    // Zadanie režimu ukončenia
    printf("Zadajte rezim ukoncenia (0=standard, 1=casovy): ");
    while (scanf("%d", &end_mode) != 1 || (end_mode != 0 && end_mode != 1)) {
        printf("Neplatny rezim. Zadajte 0 alebo 1: ");
        while (getchar() != '\n');
    }
    // Ak je časový režim, načítaj čas
    if (end_mode == 1) {
        printf("Zadajte cas pre hru v sekundach (>0): ");
        while (scanf("%d", &game_time) != 1 || game_time <= 0) {
            printf("Neplatny cas. Zadajte kladne cislo: ");
            while (getchar() != '\n');
        }
    }
    int port = najdi_volny_port();
    printf("Pouzije sa port: %d\n", port);
    // Spusti server s parametrami podľa zadaných hodnôt a portom
    char cmd[128];
    if (end_mode == 1)
        snprintf(cmd, sizeof(cmd), "./server %d %d %d %d %d &", port, size, mode, end_mode, game_time);
    else
        snprintf(cmd, sizeof(cmd), "./server %d %d %d %d &", port, size, mode, end_mode);
    system(cmd); // spusti server na Linuxe
    sleep(1); // krátka pauza na inicializáciu servera
    menu_pripojit_sa_k_hre_port(menu, port);
}

// Pokračovanie v pozastavenej hre
void menu_pokracovat_v_hre(Menu* menu) {
    menu->paused = 0;
    menu->hra_pozastavena = 0;
    if (menu->client_fd > 0) {
        menu->running = 1;
        pthread_create(&menu->input_thread, NULL, menu_input_thread, menu);
        pthread_create(&menu->recv_thread, NULL, menu_recv_thread, menu);
        pthread_join(menu->input_thread, NULL);
        pthread_join(menu->recv_thread, NULL);
        client_disconnect(menu);
    }
}

// Ukončí aplikáciu a uvoľní zdroje
void menu_koniec(Menu* menu) {
    printf("\nKoniec aplikacie.\n");
    menu_zrus(menu);
    exit(0);
}

// Zobrazí hlavné menu a riadi interakciu používateľa
void menu_zobraz(Menu* menu) {
    int volba;
    while (1) {
        // Výpis možností menu
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
        // Načíta voľbu používateľa
        if (scanf("%d", &volba) != 1) {
            while (getchar() != '\n');
            printf("Neplatna volba. Skuste znova.\n");
            continue;
        }
        // Spracovanie voľby
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

// Pripojí sa k serveru na zadanom porte a spustí vlákna pre vstup a prijímanie správ
void menu_pripojit_sa_k_hre_port(Menu* menu, int port) {
    // Skontroluj, či server beží (pripojenie sa podarilo)
    if (client_connect(menu, "127.0.0.1", port) != 0) {
        printf("Server sa nespustil alebo sa ukončil.\n");
        return;
    }
    printf("Stlac klavesu (q = koniec):\n");
    menu->running = 1;
    pthread_create(&menu->input_thread, NULL, menu_input_thread, menu);
    pthread_create(&menu->recv_thread, NULL, menu_recv_thread, menu);
    pthread_join(menu->input_thread, NULL);
    pthread_join(menu->recv_thread, NULL);
    client_disconnect(menu);
    // Ak bol klient odpojený počas pauzy, automaticky spusti novú hru
    if (menu->hra_pozastavena) {
        menu->hra_pozastavena = 0;
        menu_nova_hra(menu);
        return;
    }
}

void menu_pripojit_sa_k_hre(Menu* menu) {
    // Pôvodná implementácia pre ručné zadanie portu
    int port = 38200;
    printf("Zadajte port servera (default 38200): ");
    int input = 0;
    if (scanf("%d", &input) == 1 && input > 0) port = input;
    if (client_connect(menu, "127.0.0.1", port) != 0) {
        return;
    }
    printf("Stlac klavesu (q = koniec):\n");
    menu->running = 1;
    pthread_create(&menu->input_thread, NULL, menu_input_thread, menu);
    pthread_create(&menu->recv_thread, NULL, menu_recv_thread, menu);
    pthread_join(menu->input_thread, NULL);
    pthread_join(menu->recv_thread, NULL);
    client_disconnect(menu);
}
