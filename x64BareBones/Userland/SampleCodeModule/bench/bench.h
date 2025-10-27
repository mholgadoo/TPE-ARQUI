#ifndef BENCH_H
#define BENCH_H

#include <stdint.h>

// ============================================================================
// Benchmark Commands API
// ============================================================================

/**
 * @brief Ejecuta benchmark de FPS dibujando rectángulos durante N segundos
 * @param seconds Duración del benchmark en segundos
 */
void bench_fps(int seconds);

/**
 * @brief Ejecuta benchmark de operaciones FPU usando Mandelbrot set
 * @param size Tamaño del grid (size x size)
 */
void bench_fpu(int size);

/**
 * @brief Ejecuta benchmark de I/O
 * @param mode "kbd" para latencia de teclado, "fb" para framebuffer bandwidth
 */
void bench_io(const char* mode);

/**
 * @brief Muestra información del entorno (CPU, entorno virtualizado, etc)
 */
void bench_env(void);

#endif // BENCH_H

