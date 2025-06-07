
#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include <stddef.h>

uint8_t getFontWidth();
uint8_t getFontHeight();
uint8_t * getFontChar(char c);
void setScale(int new_size);

#endif //FONT_H