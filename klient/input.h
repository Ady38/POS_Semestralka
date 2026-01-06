#ifndef INPUT_H
#define INPUT_H

#include <termios.h>

extern struct termios orig_termios;

void disable_raw_mode();
void enable_raw_mode();

#endif // INPUT_H
