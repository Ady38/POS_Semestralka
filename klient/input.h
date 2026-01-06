#ifndef INPUT_H
#define INPUT_H

#include <termios.h>

// Ukladá pôvodné nastavenia terminálu, aby sa dali obnoviť po ukončení raw režimu
extern struct termios orig_termios;

// Vypne raw režim a obnoví pôvodné nastavenia terminálu
void disable_raw_mode();
// Zapne raw režim (vypne echo a kanonický režim pre okamžité čítanie kláves)
void enable_raw_mode();

#endif // INPUT_H
