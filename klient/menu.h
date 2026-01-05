#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <pthread.h>

// "Objektová" štruktúra pre menu
typedef struct Menu {
    bool hra_pozastavena;
    int client_fd;
    volatile int running;
    pthread_t input_thread;
    pthread_t recv_thread;
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

// Thread routines
void* menu_input_thread(void* arg);
void* menu_recv_thread(void* arg);

#endif // MENU_H

