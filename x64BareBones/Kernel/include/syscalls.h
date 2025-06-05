#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

void syscall_write(char c);
char syscall_read();
void syscall_clear_screen();

#endif
