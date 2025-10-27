#include "bench.h"
#include "../lib.h"
#include "../syscalls.h"

#define SYS_READ 1
#define SYS_DRAW_RECT 4

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

// Usar la función str_eq de lib.h (ya está declarada)
extern int str_eq(const char *a, const char *b);

/**
 * Benchmark de latencia de teclado
 * Mide el tiempo desde que el usuario presiona una tecla hasta que
 * la recibimos en userland
 */
static void bench_keyboard_latency(void) {
    print("=== Keyboard Latency Benchmark ===\n");
    print("Presiona 10 teclas rapido...\n");
    print("(El benchmark mide el tiempo de respuesta)\n\n");
    
    #define NUM_SAMPLES 10
    uint64_t samples[NUM_SAMPLES];
    int sample_count = 0;
    
    // Limpiar buffer
    char c;
    while (_sys_read(SYS_READ, 0, &c, 1) > 0);
    
    while (sample_count < NUM_SAMPLES) {
        // Esperar a que haya una tecla disponible
        uint64_t start = bench_start();
        
        // Busy-wait hasta que read retorne algo
        while (_sys_read(SYS_READ, 0, &c, 1) == 0) {
            // Nota: Esto mide la latencia desde que llamamos read
            // hasta que obtenemos la tecla. No es la latencia real del IRQ,
            // pero da una aproximación del overhead del sistema
        }
        
        uint64_t cycles = bench_stop(start);
        samples[sample_count++] = cycles;
        
        printChar(c);
    }
    
    print("\n\n");
    
    // Calcular estadísticas
    uint64_t total = 0;
    uint64_t min_val = samples[0];
    uint64_t max_val = samples[0];
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        total += samples[i];
        if (samples[i] < min_val) min_val = samples[i];
        if (samples[i] > max_val) max_val = samples[i];
    }
    
    uint64_t avg = total / NUM_SAMPLES;
    
    char buf[32];
    
    print("Samples:            ");
    uint64_to_str_helper(NUM_SAMPLES, buf);
    print(buf);
    print("\n");
    
    print("Latencia promedio:  ");
    uint64_to_str_helper(cycles_to_us(avg), buf);
    print(buf);
    print(" us (");
    uint64_to_str_helper(avg, buf);
    print(buf);
    print(" cycles)\n");
    
    print("Latencia min:       ");
    uint64_to_str_helper(cycles_to_us(min_val), buf);
    print(buf);
    print(" us\n");
    
    print("Latencia max:       ");
    uint64_to_str_helper(cycles_to_us(max_val), buf);
    print(buf);
    print(" us\n");
    
    print("\n");
}

/**
 * Benchmark de ancho de banda del framebuffer
 * Mide la velocidad de dibujado de rectángulos
 */
static void bench_framebuffer_bandwidth(void) {
    print("=== Framebuffer Bandwidth Benchmark ===\n");
    print("Dibujando rectangulos...\n\n");
    
    #define NUM_ITERS 100
    uint64_t samples[NUM_ITERS];
    
    // Test: dibujar un rectángulo grande muchas veces
    int rect_width = 200;
    int rect_height = 200;
    uint64_t pixels_per_iter = rect_width * rect_height;
    
    for (int i = 0; i < NUM_ITERS; i++) {
        uint64_t start = bench_start();
        
        // Dibujar rectángulo a la DERECHA del texto (x=450 en vez de 100)
        _sys_drawRect(SYS_DRAW_RECT, 0xFF0000, 450, 150, rect_width, rect_height);
        
        samples[i] = bench_stop(start);
    }
    
    // Calcular estadísticas
    uint64_t total = 0;
    uint64_t min_val = samples[0];
    uint64_t max_val = samples[0];
    
    for (int i = 0; i < NUM_ITERS; i++) {
        total += samples[i];
        if (samples[i] < min_val) min_val = samples[i];
        if (samples[i] > max_val) max_val = samples[i];
    }
    
    uint64_t avg = total / NUM_ITERS;
    uint64_t avg_us = cycles_to_us(avg);
    
    // Calcular ancho de banda en MB/s
    // pixels * 4 bytes/pixel = bytes
    // bytes / (tiempo_us / 1000000) = bytes/s
    // bytes/s / 1048576 = MB/s
    uint64_t bytes_per_rect = pixels_per_iter * 4; // 4 bytes por pixel (RGBA)
    uint64_t bandwidth_mbps = 0;
    if (avg_us > 0) {
        // MB/s = (bytes * 1000000 / us) / 1048576
        bandwidth_mbps = (bytes_per_rect * 1000000) / avg_us / 1048576;
    }
    
    char buf[32];
    
    print("Rectangulo:         ");
    int_to_str(rect_width, buf);
    print(buf);
    print("x");
    int_to_str(rect_height, buf);
    print(buf);
    print(" pixels\n");
    
    print("Iteraciones:        ");
    uint64_to_str_helper(NUM_ITERS, buf);
    print(buf);
    print("\n");
    
    print("Tiempo promedio:    ");
    uint64_to_str_helper(avg_us, buf);
    print(buf);
    print(" us\n");
    
    print("Tiempo min:         ");
    uint64_to_str_helper(cycles_to_us(min_val), buf);
    print(buf);
    print(" us\n");
    
    print("Tiempo max:         ");
    uint64_to_str_helper(cycles_to_us(max_val), buf);
    print(buf);
    print(" us\n");
    
    print("Ancho de banda:     ~");
    uint64_to_str_helper(bandwidth_mbps, buf);
    print(buf);
    print(" MB/s\n");
    
    print("\n");
}

void bench_io(const char* mode) {
    clearScreen();
    
    if (str_eq(mode, "kbd") || str_eq(mode, "keyboard")) {
        bench_keyboard_latency();
    } else if (str_eq(mode, "fb") || str_eq(mode, "framebuffer")) {
        bench_framebuffer_bandwidth();
    } else {
        print("Modo de I/O no reconocido.\n");
        print("Modos disponibles:\n");
        print("  kbd  - Latencia de teclado\n");
        print("  fb   - Ancho de banda framebuffer\n\n");
        
        // Por defecto, ejecutar ambos
        print("Ejecutando ambos benchmarks...\n\n");
        bench_keyboard_latency();
        print("Presiona una tecla para continuar...\n");
        char c;
        while (_sys_read(SYS_READ, 0, &c, 1) == 0);
        clearScreen();
        bench_framebuffer_bandwidth();
    }
    
    print("=== Fin Benchmark ===\n\n");
}

