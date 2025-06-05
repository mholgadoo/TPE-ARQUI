#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#include <stdint.h>

void keyboard_handler();     // se llama por interrupcion
char keyboard_getchar();     // pop del buffer

#endif
