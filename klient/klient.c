#include <stdio.h>
#include "menu.h"
#include <stdlib.h>

int main(void) {
    system("clear"); // vyprazdni konzolu na zaciatku
    Menu* menu = menu_vytvor(false); // predpokladame, ze hra nie je pozastavena na zaciatku
    menu_zobraz(menu);
    return 0;
}
