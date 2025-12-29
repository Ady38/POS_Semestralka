#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

// "Objektová" štruktúra pre menu
typedef struct Menu {
    bool hra_pozastavena;
} Menu;

// Vytvorenie a zničenie menu
Menu* menu_vytvor(bool hra_pozastavena);
void menu_zrus(Menu* menu);

// Hlavné funkcie menu
void menu_zobraz(Menu* menu);
void menu_nova_hra(Menu* menu);
void menu_pripojit_sa_k_hre(Menu* menu);
void menu_pokracovat_v_hre(Menu* menu);
void menu_koniec(Menu* menu);

#endif // MENU_H

