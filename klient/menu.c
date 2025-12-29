#include "menu.h"
#include <stdio.h>
#include <stdlib.h>

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
    // TODO: Vyber rezimu, typu sveta, nacitanie zo suboru, volba poctu hracov
    // menu->hra_pozastavena = false; // pripadne upravit stav
}

void menu_pripojit_sa_k_hre(Menu* menu) {
    printf("\n[Pripojenie k hre]\n");
    // TODO: Pripojenie k existujucej hre, pozastavenie pohybu hadikov na 3 sekundy
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
