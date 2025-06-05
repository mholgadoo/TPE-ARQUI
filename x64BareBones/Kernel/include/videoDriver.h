// Kernel/include/videoDriver.h
#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y);

void putChar(char c, uint32_t x, uint32_t y, uint32_t color);
void drawRect(uint32_t hexColor, uint64_t x, uint64_t y, uint64_t width, uint64_t height);

void clearScreen();

uint16_t getScreenWidth();
uint16_t getScreenHeight();

#endif // VIDEO_DRIVER_H
