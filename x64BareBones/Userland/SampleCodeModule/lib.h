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
#define SYS_RDTSC             10
#define SYS_GET_TSC_FREQ      11
#define SYS_CYCLES_TO_MS      12
#define SYS_CYCLES_TO_US      13
#define SYS_HAS_TSC           14
#define SYS_HAS_INVARIANT_TSC 15
#define SYS_PUT_CHAR          16

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

// ============================================================================
// Benchmarking API - Wrappers sencillos a TSC/PIT
// ============================================================================

/**
 * @brief Inicia una medición de tiempo
 * @return Valor del TSC al inicio
 */
uint64_t bench_start(void);

/**
 * @brief Finaliza una medición de tiempo
 * @param start Valor del TSC al inicio (obtenido con bench_start)
 * @return Número de ciclos transcurridos
 */
uint64_t bench_stop(uint64_t start);

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
 * @brief Obtiene la frecuencia del TSC en Hz
 * @return Frecuencia del TSC en Hz
 */
uint64_t get_tsc_freq(void);

/**
 * @brief Verifica si el CPU tiene TSC
 * @return 1 si tiene TSC, 0 en caso contrario
 */
int has_tsc(void);

/**
 * @brief Verifica si el CPU tiene TSC invariante
 * @return 1 si tiene TSC invariante, 0 en caso contrario
 */
int has_invariant_tsc(void);

/**
 * @brief Imprime estadísticas de un conjunto de muestras
 * @param name Nombre del benchmark
 * @param samples Array de muestras (en ciclos)
 * @param count Número de muestras
 */
void print_stats(const char* name, uint64_t* samples, int count);

#endif // LIB_H
