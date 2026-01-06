#ifndef THREADS_H
#define THREADS_H

#include "menu.h"

// Deklarácie thread routines a správy vlákien
void* menu_input_thread(void* arg);
void* menu_recv_thread(void* arg);

#endif // THREADS_H
