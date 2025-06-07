#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <sys/types.h>
#include <stdint.h>
typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} rtc_time_t;

//agrego los numeros para que se cargue en los registros igual que lo recibe syscall dispatcher
void _sys_write(uint64_t syscall_number, const char *str, int len);
void _sys_clearScreen(uint64_t syscall_number);
ssize_t _sys_read(uint64_t syscall_number, int fd, char *buf, int count);
void _sys_sleep(uint64_t syscall_number, uint64_t ticks);
void _sys_drawRect(uint64_t syscall_number, uint32_t color, uint64_t x, uint64_t y, uint64_t width, uint64_t height);
uint64_t _sys_get_ticks(uint64_t syscall_number);
uint64_t _sys_get_registers(uint64_t syscall_number, uint64_t * regs);
void _sys_get_time(uint64_t syscall_number,rtc_time_t *time);
void _sys_playBeep(uint64_t syscall_number, uint32_t frequency, uint32_t duration_ms);
void _sys_changeFontSize(uint64_t syscall_number, int new_size);

#endif
