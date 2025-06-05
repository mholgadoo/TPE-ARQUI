#include <syscalls.h>
#include <videoDriver.h>
#include <keyboardDriver.h>

#define CHAR_COLOR 0xFFFFFF
#define CHAR_START_X 10
#define CHAR_START_Y 10
#define CHAR_SPACING 8

static uint32_t cursor_x = CHAR_START_X;
static uint32_t cursor_y = CHAR_START_Y;

void syscall_write(char c) {
    if (c == '\n') {
        cursor_y += getFontHeight();
        cursor_x = CHAR_START_X;
    } else if (c == '\t') {
        cursor_x += 4 * CHAR_SPACING;
    } else if (c == '\b') {
        if (cursor_x >= CHAR_START_X + CHAR_SPACING) {
            cursor_x -= CHAR_SPACING;
            putChar(' ', cursor_x, cursor_y, 0x000000);
        }
    } else {
        putChar(c, cursor_x, cursor_y, CHAR_COLOR);
        cursor_x += CHAR_SPACING;
    }
}

char syscall_read() {
    char c = 0;
    while ((c = keyboard_getchar()) == 0);  // espera activa
    return c;
}

void syscall_clear_screen() {
    clearScreen();
    cursor_x = CHAR_START_X;
    cursor_y = CHAR_START_Y;
}
