#include "bench.h"
#include "../lib.h"
#include "../syscalls.h"

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

/**
 * Calcula iteraciones de Mandelbrot para un punto complejo (x0, y0)
 * Retorna el número de iteraciones antes de divergir
 */
static int mandelbrot_iterations(double x0, double y0, int max_iter) {
    double x = 0.0;
    double y = 0.0;
    int iteration = 0;
    
    while (x*x + y*y <= 4.0 && iteration < max_iter) {
        double xtemp = x*x - y*y + x0;
        y = 2.0*x*y + y0;
        x = xtemp;
        iteration++;
    }
    
    return iteration;
}

void bench_fpu(int size) {
    print("=== FPU Benchmark (Mandelbrot Set) ===\n");
    char buf[32];
    
    if (size <= 0) size = 256;
    if (size > 1024) size = 1024; // Límite razonable
    
    print("Grid size: ");
    int_to_str(size, buf);
    print(buf);
    print("x");
    print(buf);
    print("\n");
    
    int max_iterations = 256;
    print("Max iterations: ");
    int_to_str(max_iterations, buf);
    print(buf);
    print("\n\n");
    
    print("Calculando...\n");
    
    // Repetir el benchmark 3 veces para obtener muestras
    #define NUM_RUNS 3
    uint64_t run_times[NUM_RUNS];
    
    for (int run = 0; run < NUM_RUNS; run++) {
        uint64_t start = bench_start();
        
        // Calcular Mandelbrot set
        uint64_t total_iterations = 0;
        
        for (int py = 0; py < size; py++) {
            for (int px = 0; px < size; px++) {
                // Mapear coordenadas de pixel a plano complejo
                // x: [-2.5, 1.0], y: [-1.0, 1.0]
                double x0 = -2.5 + (3.5 * px) / size;
                double y0 = -1.0 + (2.0 * py) / size;
                
                int iters = mandelbrot_iterations(x0, y0, max_iterations);
                total_iterations += iters;
            }
        }
        
        run_times[run] = bench_stop(start);
        
        print("Run ");
        int_to_str(run + 1, buf);
        print(buf);
        print(": ");
        uint64_to_str_helper(cycles_to_ms(run_times[run]), buf);
        print(buf);
        print(" ms\n");
    }
    
    // Calcular estadísticas
    uint64_t total_cycles = 0;
    uint64_t min_cycles = run_times[0];
    uint64_t max_cycles = run_times[0];
    
    for (int i = 0; i < NUM_RUNS; i++) {
        total_cycles += run_times[i];
        if (run_times[i] < min_cycles) min_cycles = run_times[i];
        if (run_times[i] > max_cycles) max_cycles = run_times[i];
    }
    
    uint64_t avg_cycles = total_cycles / NUM_RUNS;
    uint64_t avg_ms = cycles_to_ms(avg_cycles);
    
    // Calcular operaciones de punto flotante
    // Cada iteración de Mandelbrot: ~6 operaciones FP (2 mult, 2 add, 1 comp, etc)
    // Aproximación conservadora: 6 FLOPS por iteración
    uint64_t total_ops = (uint64_t)size * size * max_iterations * 6;
    
    // MFLOPS = (operaciones / tiempo_segundos) / 1000000
    uint64_t mflops = 0;
    if (avg_ms > 0) {
        // ops / (ms / 1000) / 1000000 = ops * 1000 / ms / 1000000 = ops / ms / 1000
        mflops = total_ops / avg_ms / 1000;
    }
    
    print("\n=== Resultados FPU Benchmark ===\n\n");
    
    print("Operaciones FP:     ~");
    uint64_to_str_helper(total_ops / 1000000, buf);
    print(buf);
    print(" millones\n");
    
    print("Tiempo promedio:    ");
    uint64_to_str_helper(avg_ms, buf);
    print(buf);
    print(" ms\n");
    
    print("Tiempo minimo:      ");
    uint64_to_str_helper(cycles_to_ms(min_cycles), buf);
    print(buf);
    print(" ms\n");
    
    print("Tiempo maximo:      ");
    uint64_to_str_helper(cycles_to_ms(max_cycles), buf);
    print(buf);
    print(" ms\n");
    
    print("MFLOPS (aprox):     ");
    uint64_to_str_helper(mflops, buf);
    print(buf);
    print("\n");
    
    print("\n=== Fin Benchmark ===\n\n");
}

