#include "syscalls.h"
#include <stdint.h>
#include <stddef.h>
#include "lib.h"

int str_eq(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return a[i] == b[i];
}

int str_len(const char *s) {
    int l = 0;
    while (s[l]) l++;
    return l;
}

void print(const char *s) {
    _sys_write(SYS_WRITE, s, str_len(s));
}

void printChar(char c) {
    _sys_write(SYS_WRITE, &c, 1);
}

int read(char *buf, int count) {
    return _sys_read(SYS_READ, 0, buf, count);
}

void clearScreen() {
    _sys_clearScreen(SYS_CLEAR_SCREEN);
}

void getTime(rtc_time_t *tm) {
    _sys_get_time(SYS_GET_TIME, tm);
}

void getRegisters(uint64_t *regs) {
    _sys_get_registers(SYS_GET_REGS, regs);
}

void changeFontSize(int size) {
    _sys_changeFontSize(SYS_CHANGE_FONT_SIZE, size);
}

void playBeep(int channel, double freq, int duration) {
    _sys_playBeep(channel, freq, duration);
}

int int_to_str(int v, char *buf) {
    char tmp[16];
    int i = 0;
    if (v == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return 1;
    }
    int neg = 0;
    if (v < 0) { neg = 1; v = -v; }
    while (v > 0) {
        tmp[i++] = '0' + (v % 10);
        v /= 10;
    }
    int len = 0;
    if (neg) buf[len++] = '-';
    while (i--) buf[len++] = tmp[i];
    buf[len] = 0;
    return len;
}
