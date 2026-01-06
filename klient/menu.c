#include "menu.h"
#include "threads.h"
#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 38200

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
    int size, mode, end_mode, game_time = 0;
    printf("Zadajte velkost sveta (10-30): ");
    while (scanf("%d", &size) != 1 || size < 10 || size > 30) {
        printf("Neplatna velkost. Zadajte cislo 10-30: ");
        while (getchar() != '\n');
    }
    printf("Zadajte herny rezim (0=bez prekazok, 1=s prekazkami): ");
    while (scanf("%d", &mode) != 1 || (mode != 0 && mode != 1)) {
        printf("Neplatny rezim. Zadajte 0 alebo 1: ");
        while (getchar() != '\n');
    }
    printf("Zadajte rezim ukoncenia (0=standard, 1=casovy): ");
    while (scanf("%d", &end_mode) != 1 || (end_mode != 0 && end_mode != 1)) {
        printf("Neplatny rezim. Zadajte 0 alebo 1: ");
        while (getchar() != '\n');
    }
    if (end_mode == 1) {
        printf("Zadajte cas pre hru v sekundach (>0): ");
        while (scanf("%d", &game_time) != 1 || game_time <= 0) {
            printf("Neplatny cas. Zadajte kladne cislo: ");
            while (getchar() != '\n');
        }
    }
    // Spusti server s parametrami
    char cmd[128];
    if (end_mode == 1)
        snprintf(cmd, sizeof(cmd), "./server %d %d %d %d &", size, mode, end_mode, game_time);
    else
        snprintf(cmd, sizeof(cmd), "./server %d %d %d &", size, mode, end_mode);
    system(cmd);
    sleep(1); // krátka pauza na inicializáciu servera
    menu_pripojit_sa_k_hre(menu);
}

void menu_pokracovat_v_hre(Menu* menu) {
    (void)menu;
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

void menu_pripojit_sa_k_hre(Menu* menu) {
    if (client_connect(menu, "127.0.0.1", PORT) != 0) {
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
