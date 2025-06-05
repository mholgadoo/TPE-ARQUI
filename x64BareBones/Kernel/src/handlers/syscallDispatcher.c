#include <syscalls.h>

uint64_t syscallDispatcher(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    switch (syscall_number) {
        case 0:
            syscall_write((char)arg1);
            return 0;
        case 1:
            return (uint64_t)syscall_read();
        case 2:
            syscall_clear_screen();
            return 0;
        default:
            return -1;
    }
}
