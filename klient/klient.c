#include <stdio.h>
#include "menu.h"
#include <stdlib.h>

// Hlavný vstupný bod klienta
// Inicializuje menu a spustí hlavný cyklus menu
int main(void) {
    system("clear"); // vyprazdni konzolu na zaciatku
    Menu* menu = menu_vytvor(false); // predpokladame, ze hra nie je pozastavena na zaciatku
    menu_zobraz(menu); // zobrazí hlavné menu a riadi interakciu používateľa
    return 0;
}
