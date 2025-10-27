#include <bench_timer.h>
#include <stdint.h>
#include <time.h>

// ============================================================================
// Variables globales para calibración
// ============================================================================

static uint64_t tsc_frequency_hz = 0;   // Frecuencia del TSC en Hz
static int tsc_calibrated = 0;          // Flag de calibración

// ============================================================================
// Funciones de detección de capacidades
// ============================================================================

int has_tsc(void) {
    uint32_t eax, ebx, ecx, edx;
    cpuid_info(0x1, &eax, &ebx, &ecx, &edx);
    // TSC está en bit 4 de EDX
    return (edx & (1 << 4)) != 0;
}

int has_invariant_tsc(void) {
    uint32_t eax, ebx, ecx, edx;
    // Primero verificar que CPUID extended está disponible
    cpuid_info(0x80000000, &eax, &ebx, &ecx, &edx);
    if (eax < 0x80000007) {
        return 0;
    }
    // Invariant TSC está en bit 8 de EDX de la hoja 0x80000007
    cpuid_info(0x80000007, &eax, &ebx, &ecx, &edx);
    return (edx & (1 << 8)) != 0;
}

// ============================================================================
// Calibración usando el PIT (Programmable Interval Timer)
// ============================================================================

/**
 * El PIT en x86 opera a 1.193182 MHz (≈1193182 Hz)
 * Lo usamos para medir cuántos ciclos TSC pasan en un tiempo conocido
 */
#define PIT_FREQUENCY 1193182ULL

/**
 * @brief Calibra el TSC usando el timer del sistema (PIT-based)
 * 
 * Estrategia:
 * 1. Esperamos un número conocido de ticks del sistema
 * 2. Medimos cuántos ciclos TSC pasan durante ese tiempo
 * 3. Calculamos la frecuencia del TSC
 * 
 * El timer del sistema (timer_handler en time.c) se ejecuta cada tick.
 * Por defecto, el PIT está configurado para ~18.2 Hz (55ms por tick aprox)
 */
void calibrate_tsc(void) {
    if (!has_tsc()) {
        tsc_calibrated = 0;
        return;
    }

    // Vamos a medir durante aproximadamente 100ms (más preciso que 1 tick)
    // Usamos get_ticks() que incrementa con cada IRQ0 del PIT
    
    const uint64_t calibration_ticks = 10; // ~550ms a 18.2 Hz
    
    uint64_t start_tick = get_ticks();
    uint64_t start_tsc = rdtsc();
    
    // Esperar los ticks de calibración
    while ((get_ticks() - start_tick) < calibration_ticks) {
        // Busy wait
    }
    
    uint64_t end_tsc = rdtsc();
    uint64_t end_tick = get_ticks();
    
    uint64_t elapsed_ticks = end_tick - start_tick;
    uint64_t elapsed_cycles = end_tsc - start_tsc;
    
    // El PIT típico en modo 2 se configura para ~18.2 Hz (cada ~55ms)
    // Si asumimos 18.2 ticks/segundo:
    // frecuencia_tsc = ciclos / tiempo_en_segundos
    // tiempo_en_segundos = ticks / 18.2
    
    // Para mayor precisión, asumimos que el PIT está en ~1193182/65536 ≈ 18.2065 Hz
    const uint64_t pit_hz = 18; // Aproximación conservadora
    
    if (elapsed_ticks > 0) {
        // tsc_freq = (cycles * pit_hz) / ticks
        tsc_frequency_hz = (elapsed_cycles * pit_hz) / elapsed_ticks;
        tsc_calibrated = 1;
    } else {
        // Fallback: asumir una frecuencia típica de 2 GHz
        tsc_frequency_hz = 2000000000ULL;
        tsc_calibrated = 1;
    }
}

uint64_t get_tsc_frequency(void) {
    if (!tsc_calibrated) {
        calibrate_tsc();
    }
    return tsc_frequency_hz;
}

// ============================================================================
// Funciones de conversión
// ============================================================================

uint64_t cycles_to_ms(uint64_t cycles) {
    if (!tsc_calibrated) {
        calibrate_tsc();
    }
    
    if (tsc_frequency_hz == 0) {
        return 0;
    }
    
    // ms = (cycles * 1000) / freq_hz
    // Evitamos overflow dividiendo primero si es posible
    if (cycles > (UINT64_MAX / 1000)) {
        return (cycles / tsc_frequency_hz) * 1000;
    } else {
        return (cycles * 1000) / tsc_frequency_hz;
    }
}

uint64_t cycles_to_us(uint64_t cycles) {
    if (!tsc_calibrated) {
        calibrate_tsc();
    }
    
    if (tsc_frequency_hz == 0) {
        return 0;
    }
    
    // us = (cycles * 1000000) / freq_hz
    // Dividimos en dos pasos para evitar overflow
    uint64_t ms = cycles_to_ms(cycles);
    uint64_t remaining_cycles = cycles - ((ms * tsc_frequency_hz) / 1000);
    uint64_t us_from_remaining = (remaining_cycles * 1000000) / tsc_frequency_hz;
    
    return (ms * 1000) + us_from_remaining;
}

uint64_t ms_to_cycles(uint64_t ms) {
    if (!tsc_calibrated) {
        calibrate_tsc();
    }
    
    if (tsc_frequency_hz == 0) {
        return 0;
    }
    
    // cycles = (ms * freq_hz) / 1000
    if (ms > (UINT64_MAX / tsc_frequency_hz)) {
        return (ms / 1000) * tsc_frequency_hz;
    } else {
        return (ms * tsc_frequency_hz) / 1000;
    }
}

