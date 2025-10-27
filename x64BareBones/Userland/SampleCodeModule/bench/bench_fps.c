#include "bench.h"
#include "../lib.h"
#include "../syscalls.h"

#define SYS_DRAW_RECT 4
#define SYS_SLEEP 3

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

void bench_fps(int seconds) {
    print("=== FPS Benchmark ===\n");
    print("Dibujando rectangulos durante ");
    char buf[32];
    int_to_str(seconds, buf);
    print(buf);
    print(" segundos...\n\n");
    
    if (seconds <= 0) seconds = 5;
    
    // Calculamos cuántos ticks son N segundos
    // Asumimos ~18.2 ticks/segundo
    uint64_t target_ticks = seconds * 18;
    
    #define MAX_FRAMES 1000
    uint64_t frame_times[MAX_FRAMES];
    int frame_count = 0;
    
    uint64_t start_time = bench_start();
    uint64_t start_tick = _sys_get_ticks(5);
    
    // Loop de dibujado
    while ((_sys_get_ticks(5) - start_tick) < target_ticks && frame_count < MAX_FRAMES) {
        uint64_t frame_start = bench_start();
        
        clearScreen();
        
        // Texto informativo (arriba a la izquierda)
        print("=== FPS Benchmark en progreso ===\n");
        print("Frame: ");
        char frame_buf[16];
        int_to_str(frame_count + 1, frame_buf);
        print(frame_buf);
        print("\n\n");
        
        // Dibujar rectángulos a la DERECHA y ABAJO del texto
        // Desde x=400 para no tapar texto
        int rect_count = 20;
        for (int i = 0; i < rect_count; i++) {
            int x = 400 + ((i * 50) % (SCREEN_WIDTH - 450));  // Empieza en x=400
            int y = 100 + ((i * 30) % (SCREEN_HEIGHT - 150)); // Empieza en y=100
            uint32_t color = 0xFF0000 | (i * 0x001100);
            _sys_drawRect(SYS_DRAW_RECT, color, x, y, 80, 60);
        }
        
        // Pequeño delay para simular frame rate real
        _sys_sleep(SYS_SLEEP, 1);
        
        frame_times[frame_count++] = bench_stop(frame_start);
    }
    
    uint64_t total_time = bench_stop(start_time);
    
    clearScreen();
    
    // Calcular estadísticas
    if (frame_count == 0) {
        print("No se pudieron medir frames\n");
        return;
    }
    
    uint64_t total_cycles = 0;
    uint64_t min_cycles = frame_times[0];
    uint64_t max_cycles = frame_times[0];
    
    for (int i = 0; i < frame_count; i++) {
        total_cycles += frame_times[i];
        if (frame_times[i] < min_cycles) min_cycles = frame_times[i];
        if (frame_times[i] > max_cycles) max_cycles = frame_times[i];
    }
    
    uint64_t avg_cycles = total_cycles / frame_count;
    uint64_t total_ms = cycles_to_ms(total_time);
    uint64_t avg_ms = cycles_to_ms(avg_cycles);
    
    // Calcular desviación estándar
    uint64_t variance_sum = 0;
    for (int i = 0; i < frame_count; i++) {
        int64_t diff = frame_times[i] - avg_cycles;
        if (diff < 0) diff = -diff;
        variance_sum += (diff * diff);
    }
    uint64_t stddev_cycles = 0;
    if (frame_count > 0) {
        uint64_t variance = variance_sum / frame_count;
        // sqrt simple
        uint64_t x = variance;
        uint64_t y = (x + 1) / 2;
        while (y < x) {
            x = y;
            y = (x + variance / x) / 2;
        }
        stddev_cycles = x;
    }
    
    // Calcular FPS
    int fps_total = 0;
    if (total_ms > 0) {
        fps_total = (frame_count * 1000) / total_ms;
    }
    
    // Imprimir resultados en formato tabular
    print("=== Resultados FPS Benchmark ===\n\n");
    
    print("Frames totales:     ");
    uint64_to_str_helper(frame_count, buf);
    print(buf);
    print("\n");
    
    print("Tiempo total:       ");
    uint64_to_str_helper(total_ms, buf);
    print(buf);
    print(" ms\n");
    
    print("FPS promedio:       ");
    int_to_str(fps_total, buf);
    print(buf);
    print(" fps\n");
    
    print("Frame avg:          ");
    uint64_to_str_helper(avg_ms, buf);
    print(buf);
    print(" ms (");
    uint64_to_str_helper(avg_cycles, buf);
    print(buf);
    print(" cycles)\n");
    
    print("Frame min:          ");
    uint64_to_str_helper(cycles_to_ms(min_cycles), buf);
    print(buf);
    print(" ms\n");
    
    print("Frame max:          ");
    uint64_to_str_helper(cycles_to_ms(max_cycles), buf);
    print(buf);
    print(" ms\n");
    
    print("StdDev:             ");
    uint64_to_str_helper(cycles_to_ms(stddev_cycles), buf);
    print(buf);
    print(" ms\n");
    
    print("\n=== Fin Benchmark ===\n\n");
}

