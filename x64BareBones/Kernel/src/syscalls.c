#include <syscalls.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <stdint.h>
#include <audioDriver.h>
#include <font.h>

#define CHAR_COLOR 0xFFFFFF
#define CHAR_START_X 10
#define CHAR_START_Y 10

#define STDIN 0

static uint32_t cursor_x = CHAR_START_X;
static uint32_t cursor_y = CHAR_START_Y;

extern uint8_t inb(uint16_t port);

extern void outb(uint16_t port, uint8_t value);

void syscall_write(const char *str, int len) {
    writeString(str, len);
}

uint64_t sys_read(int fd, char * buffer, int count) {
    if (fd != STDIN) {
        return 0;
    }
    int i = 0;
    for (; i < count; i++) {
        char c = keyboard_getchar();
        if (c == 0) {
            break;
        }
        buffer[i] = c;
    }
    return i;
}

void syscall_clear_screen() {
    clearScreen();
}

uint64_t get_registers(uint64_t * buffer){
    save_registers(buffer);
    return 0;
}

//---

static uint8_t read_rtc_register(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

void get_time(rtc_time_t *buffer) {
    // espera a que el rtc no este actualizando
    outb(0x70, 0x0A);
    while (inb(0x71) & 0x80);

    // lee los valores
    buffer->sec   = read_rtc_register(0x00);
    buffer->min   = read_rtc_register(0x02);
    buffer->hour  = read_rtc_register(0x04);
    buffer->day   = read_rtc_register(0x07);
    buffer->month = read_rtc_register(0x08);
    buffer->year  = read_rtc_register(0x09);

    outb(0x70, 0x0B);
    uint8_t regB = inb(0x71);
    if (!(regB & 0x04)) {  // Si esta en BCD, convertir
        buffer->sec   = (buffer->sec   & 0x0F) + ((buffer->sec   / 16) * 10);
        buffer->min   = (buffer->min   & 0x0F) + ((buffer->min   / 16) * 10);
        buffer->hour  = (buffer->hour  & 0x0F) + ((buffer->hour  / 16) * 10);
        buffer->day   = (buffer->day   & 0x0F) + ((buffer->day   / 16) * 10);
        buffer->month = (buffer->month & 0x0F) + ((buffer->month / 16) * 10);
        buffer->year  = (buffer->year  & 0x0F) + ((buffer->year  / 16) * 10);
    }
}

//---

void play_sound(uint32_t frequency, uint32_t duration_ms) {
    playBeep(frequency);
    uint64_t ticks;

    if (duration_ms < 25)
        ticks = 1;
    else
        ticks = duration_ms / 25;
    sleep(ticks);

    stopBeep();
}

void change_font_size(int new_size) {
    setScale(new_size);
}


