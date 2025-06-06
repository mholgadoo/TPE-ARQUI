#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>
#include <time.h>
#include <rtc.h>

void syscall_write(const char *str, int len);
uint64_t sys_read(int fd, char * buffer, int count);
void syscall_clear_screen();
uint64_t get_registers(uint64_t * arg1);
void get_time(rtc_time_t * arg1);
void play_sound(uint32_t frequency, uint32_t duration_ms);
void change_font_size(int new_size);

#endif
