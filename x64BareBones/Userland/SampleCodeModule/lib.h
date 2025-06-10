// lib.h
#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stddef.h>
#include "syscalls.h"


#define SYS_WRITE             0
#define SYS_READ              1
#define SYS_CLEAR_SCREEN      2
#define SYS_GET_REGS          6
#define SYS_GET_TIME          7
#define SYS_CHANGE_FONT_SIZE  9

int str_len(const char *s);
int str_eq(const char *a, const char *b);

void print(const char *s);

void printChar(char c);

int read(char *buf, int count);

void clearScreen(void);

void getTime(rtc_time_t *tm);

void getRegisters(uint64_t *regs);

void changeFontSize(int size);

void playBeep(int channel, double freq, int duration);

int int_to_str(int v, char *buf);

#endif // LIB_H
