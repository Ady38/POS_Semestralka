#ifndef NETWORK_H
#define NETWORK_H

#include "menu.h"

// Pripojí klienta k serveru na danej adrese a porte.
// menu: štruktúra Menu, kde sa uloží file descriptor socketu
// address: IP adresa servera
// port: port servera
// return: 0 pri úspechu, -1 pri chybe
int client_connect(Menu* menu, const char* address, int port);

// Odpojí klienta od servera a uzavrie socket
void client_disconnect(Menu* menu);

#endif // NETWORK_H
