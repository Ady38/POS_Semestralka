#ifndef THREADS_H
#define THREADS_H

#include "menu.h"

// Vlákno pre čítanie vstupu od používateľa a odosielanie na server
void* menu_input_thread(void* arg);
// Vlákno pre prijímanie správ zo servera a ich zobrazovanie
void* menu_recv_thread(void* arg);

#endif // THREADS_H
