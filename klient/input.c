#include "input.h"
#include <unistd.h>
#include <stdlib.h>

// Globálna premenná na uloženie pôvodných nastavení terminálu
struct termios orig_termios;

// Obnoví pôvodné nastavenia terminálu (vypne raw mód)
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// Zapne raw mód: vypne echo a kanonický režim, aby bolo možné čítať klávesy okamžite
void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode); // zabezpečí obnovu po ukončení programu
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}
