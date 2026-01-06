#ifndef NETWORK_H
#define NETWORK_H

#include "menu.h"

// Deklarácie sieťových funkcií a štruktúr
int client_connect(Menu* menu, const char* address, int port);
void client_disconnect(Menu* menu);

#endif // NETWORK_H
