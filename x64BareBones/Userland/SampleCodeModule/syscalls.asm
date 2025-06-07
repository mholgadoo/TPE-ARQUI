global _sys_write
global _sys_clearScreen
global _sys_read
global _sys_sleep
global _sys_drawRect
global _sys_get_ticks
global _sys_get_registers
global _sys_get_time
global _sys_playBeep
global _sys_changeFontSize

section .text

; _sys_write(const char *str, int len)
_sys_write:
    int 0x80
    ret

; _sys_clearScreen()
_sys_clearScreen:
    int 0x80
    ret

; ssize_t _sys_read(int fd, char *buf, int count)
_sys_read:
    int 0x80
    ret

; void _sys_sleep(uint64_t ticks)
_sys_sleep:
    int 0x80
    ret

_sys_drawRect:
    int 0x80
    ret

section .text
_sys_get_ticks:
    int 0x80
    ret

_sys_get_registers:
    mov rax, 6
    int 0x80
    ret

_sys_get_time:
    mov rax, 7
    int 0x80
    ret

_sys_playBeep:
    mov rax, 8
    int 0x80
    ret

_sys_changeFontSize:
    mov rax, 9
    int 0x80
    ret
