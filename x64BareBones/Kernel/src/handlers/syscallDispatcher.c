#include <syscalls.h>
#include <videoDriver.h>
#include <keyboardDriver.h>
#include <time.h>
#include <registers.h>
#include <bench_timer.h>

//llamado desde interrupts.asm, que es llamado desde wrapper de syscall en userland
uint64_t syscallDispatcher(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall_number) {
        case 0:
            syscall_write((const char*)arg1, (int)arg2);
            return 0;
        case 1:
            return sys_read((int)arg1, (char *)arg2, (int)arg3);        
        case 2:
            syscall_clear_screen();
            return 0;
        case 3:
            sleep(arg1);
            return 0;
        case 4:
            drawRect((uint32_t)arg1, arg2, arg3, arg4, arg5);
            return 0;
        case 5:
            return get_ticks();
        case 6:
            get_saved_registers((uint64_t*)arg1);
            return 0;
        case 7:
            get_time((rtc_time_t*)arg1);
            return 0;
        case 8:
            play_sound((uint32_t)arg1, (uint32_t)arg2);
            return 0;
        case 9:
            change_font_size((int)arg1);
            return 0;
        case 10:
            // Leer TSC
            return rdtsc();
        case 11:
            // Obtener frecuencia TSC en Hz
            return get_tsc_frequency();
        case 12:
            // Convertir ciclos a milisegundos
            return cycles_to_ms(arg1);
        case 13:
            // Convertir ciclos a microsegundos
            return cycles_to_us(arg1);
        case 14:
            // Verificar si tiene TSC
            return has_tsc();
        case 15:
            // Verificar si tiene TSC invariante
            return has_invariant_tsc();
        case 16:
            // putChar - dibujar un carácter en posición específica
            putChar((char)arg1, (uint32_t)arg2, (uint32_t)arg3, (uint32_t)arg4);
            return 0;
        default:
            return -1;
    }
}