#ifndef BENCH_TIMER_H
#define BENCH_TIMER_H

#include <stdint.h>

// ============================================================================
// API de Timing - Medición de rendimiento usando TSC (Time Stamp Counter)
// ============================================================================

/**
 * @brief Lee el Time Stamp Counter del CPU
 * @return Valor de 64 bits del TSC
 */
uint64_t rdtsc(void);

/**
 * @brief Ejecuta la instrucción CPUID para obtener información del CPU
 * @param leaf Hoja de información CPUID a consultar
 * @param eax Puntero para almacenar EAX
 * @param ebx Puntero para almacenar EBX
 * @param ecx Puntero para almacenar ECX
 * @param edx Puntero para almacenar EDX
 */
void cpuid_info(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);

/**
 * @brief Calibra el TSC usando el PIT (Programmable Interval Timer)
 * Debe llamarse una vez al inicio del sistema para calcular la frecuencia del TSC
 */
void calibrate_tsc(void);

/**
 * @brief Obtiene la frecuencia del TSC en Hz
 * @return Frecuencia del TSC en Hz (ciclos por segundo)
 */
uint64_t get_tsc_frequency(void);

/**
 * @brief Convierte ciclos de TSC a milisegundos
 * @param cycles Número de ciclos TSC
 * @return Tiempo en milisegundos
 */
uint64_t cycles_to_ms(uint64_t cycles);

/**
 * @brief Convierte ciclos de TSC a microsegundos
 * @param cycles Número de ciclos TSC
 * @return Tiempo en microsegundos
 */
uint64_t cycles_to_us(uint64_t cycles);

/**
 * @brief Convierte milisegundos a ciclos de TSC
 * @param ms Tiempo en milisegundos
 * @return Número de ciclos TSC
 */
uint64_t ms_to_cycles(uint64_t ms);

/**
 * @brief Inicia una medición de tiempo
 * @return Valor del TSC al inicio
 */
static inline uint64_t bench_start(void) {
    return rdtsc();
}

/**
 * @brief Finaliza una medición de tiempo
 * @param start Valor del TSC al inicio (obtenido con bench_start)
 * @return Número de ciclos transcurridos
 */
static inline uint64_t bench_stop(uint64_t start) {
    uint64_t end = rdtsc();
    return end - start;
}

/**
 * @brief Verifica si el TSC está disponible en este CPU
 * @return 1 si TSC está disponible, 0 en caso contrario
 */
int has_tsc(void);

/**
 * @brief Verifica si el TSC es invariante (no cambia con frecuencia del CPU)
 * @return 1 si TSC es invariante, 0 en caso contrario
 */
int has_invariant_tsc(void);

#endif // BENCH_TIMER_H

