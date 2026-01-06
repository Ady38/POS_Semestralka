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
