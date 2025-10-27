#include "bench.h"
#include "../lib.h"
#include "../syscalls.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// Helper para convertir uint64_t a string
static void uint64_to_str_helper(uint64_t v, char *buf) {
    char tmp[32];
    int i = 0;
    if (v == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return;
    }
    while (v > 0) {
        tmp[i++] = '0' + (v % 10);
        v /= 10;
    }
    int len = 0;
    while (i > 0) buf[len++] = tmp[--i];
    buf[len] = 0;
}

void bench_env(void) {
    clearScreen();
    print("=== Environment Detection ===\n\n");
    
    char buf[64];
    
    // Información de pantalla
    print("--- Display ---\n");
    print("Resolution:         ");
    int_to_str(SCREEN_WIDTH, buf);
    print(buf);
    print("x");
    int_to_str(SCREEN_HEIGHT, buf);
    print(buf);
    print("\n\n");
    
    // Información de CPU
    print("--- CPU ---\n");
    
    // TSC Support
    print("TSC Support:        ");
    print(has_tsc() ? "YES" : "NO");
    print("\n");
    
    print("Invariant TSC:      ");
    int inv_tsc = has_invariant_tsc();
    print(inv_tsc ? "YES" : "NO");
    print("\n");
    
    // Frecuencia TSC
    uint64_t freq = get_tsc_freq();
    print("TSC Frequency:      ");
    uint64_to_str_helper(freq, buf);
    print(buf);
    print(" Hz\n");
    
    // Estimar frecuencia en GHz
    uint64_t freq_mhz = freq / 1000000;
    print("                    ~");
    uint64_to_str_helper(freq_mhz, buf);
    print(buf);
    print(" MHz\n\n");
    
    // Detección de entorno virtualizado (heurística)
    print("--- Environment ---\n");
    
    // Heurística 1: TSC invariante suele indicar VM moderna
    if (inv_tsc) {
        print("Likely Virtual:     YES (Invariant TSC present)\n");
        print("Possible VM:        QEMU/VirtualBox/VMware\n");
    } else {
        print("Likely Virtual:     NO (or older VM)\n");
        print("Possible:           Bare Metal or old VM\n");
    }
    
    print("\n");
    
    // Información adicional útil para el informe
    print("--- Benchmark Info ---\n");
    print("Para comparacion entre entornos, copie estos datos:\n\n");
    
    print("Entorno:            [Manual: QEMU/VBox/PC1/PC2]\n");
    print("TSC Freq:           ");
    uint64_to_str_helper(freq, buf);
    print(buf);
    print(" Hz\n");
    print("Resolution:         ");
    int_to_str(SCREEN_WIDTH, buf);
    print(buf);
    print("x");
    int_to_str(SCREEN_HEIGHT, buf);
    print(buf);
    print("\n");
    print("Invariant TSC:      ");
    print(inv_tsc ? "YES" : "NO");
    print("\n");
    
    print("\nNota: Use estos datos para crear la tabla\n");
    print("comparativa en el informe.\n");
    
    print("\n=== Fin Detection ===\n\n");
}

