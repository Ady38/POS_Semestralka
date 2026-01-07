#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <pthread.h>

// Objektová štruktúra pre hlavné menu klienta.
// Uchováva stav menu, informácie o spojení a vlákna pre vstup a prijímanie správ.
typedef struct Menu {
    bool hra_pozastavena; // True, ak je hra pozastavená a je možné pokračovať v hre
    int client_fd;        // File descriptor socketu klienta (pripojenie k serveru)
    volatile int running; // Príznak, či je klient v hernom cykle (1=beží, 0=koniec)
    volatile int paused;  // Príznak, či je hra pozastavená (1=pozastavená, 0=beží)
    pthread_t input_thread; // ID vlákna pre čítanie vstupu od používateľa
    pthread_t recv_thread;  // ID vlákna pre prijímanie správ zo servera
} Menu;

// Vytvorí a inicializuje štruktúru Menu.
// hra_pozastavena: nastaví, či je možné pokračovať v hre (napr. po odpojení)
// return: pointer na alokovanú štruktúru Menu
Menu* menu_vytvor(bool hra_pozastavena);

// Uvoľní pamäť alokovanú pre Menu
void menu_zrus(Menu* menu);

// Zobrazí hlavné menu a riadi interakciu používateľa (cyklus výberu možností)
void menu_zobraz(Menu* menu);

// Spustí novú hru: načíta parametre od používateľa, spustí server a pripojí sa
void menu_nova_hra(Menu* menu);

// Pripojí sa k existujúcej hre na serveri
void menu_pripojit_sa_k_hre(Menu* menu);

// Pokračuje v pozastavenej hre (ak je podporované)
void menu_pokracovat_v_hre(Menu* menu);

// Ukončí aplikáciu, uvoľní zdroje a ukončí proces
void menu_koniec(Menu* menu);

#endif // MENU_H

