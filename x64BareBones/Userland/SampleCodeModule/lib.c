#include "syscalls.h"
#include <stdint.h>
#include <stddef.h>
#include "lib.h"

int str_eq(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return a[i] == b[i];
}

int str_len(const char *s) {
    int l = 0;
    while (s[l]) l++;
    return l;
}

void print(const char *s) {
    _sys_write(SYS_WRITE, s, str_len(s));
}

void printChar(char c) {
    _sys_write(SYS_WRITE, &c, 1);
}

int read(char *buf, int count) {
    return _sys_read(SYS_READ, 0, buf, count);
}

void clearScreen() {
    _sys_clearScreen(SYS_CLEAR_SCREEN);
}

void getTime(rtc_time_t *tm) {
    _sys_get_time(SYS_GET_TIME, tm);
}

void getRegisters(uint64_t *regs) {
    _sys_get_registers(SYS_GET_REGS, regs);
}

void changeFontSize(int size) {
    _sys_changeFontSize(SYS_CHANGE_FONT_SIZE, size);
}

void playBeep(int channel, double freq, int duration) {
    _sys_playBeep(channel, freq, duration);
}

int int_to_str(int v, char *buf) {
    char tmp[16];
    int i = 0;
    if (v == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return 1;
    }
    int neg = 0;
    if (v < 0) { neg = 1; v = -v; }
    while (v > 0) {
        tmp[i++] = '0' + (v % 10);
        v /= 10;
    }
    int len = 0;
    if (neg) buf[len++] = '-';
    while (i--) buf[len++] = tmp[i];
    buf[len] = 0;
    return len;
}

// ============================================================================
// Benchmarking API Implementation
// ============================================================================

uint64_t bench_start(void) {
    return _sys_rdtsc(SYS_RDTSC);
}

uint64_t bench_stop(uint64_t start) {
    uint64_t end = _sys_rdtsc(SYS_RDTSC);
    return end - start;
}

uint64_t cycles_to_ms(uint64_t cycles) {
    return _sys_cycles_to_ms(SYS_CYCLES_TO_MS, cycles);
}

uint64_t cycles_to_us(uint64_t cycles) {
    return _sys_cycles_to_us(SYS_CYCLES_TO_US, cycles);
}

uint64_t get_tsc_freq(void) {
    return _sys_get_tsc_freq(SYS_GET_TSC_FREQ);
}

int has_tsc(void) {
    return _sys_has_tsc(SYS_HAS_TSC);
}

int has_invariant_tsc(void) {
    return _sys_has_invariant_tsc(SYS_HAS_INVARIANT_TSC);
}

// Helper para convertir uint64_t a string
static int uint64_to_str(uint64_t v, char *buf) {
    char tmp[32];
    int i = 0;
    if (v == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return 1;
    }
    while (v > 0) {
        tmp[i++] = '0' + (v % 10);
        v /= 10;
    }
    int len = 0;
    while (i > 0) buf[len++] = tmp[--i];
    buf[len] = 0;
    return len;
}

// Función simple para calcular raíz cuadrada (para desviación estándar)
static uint64_t sqrt_uint64(uint64_t n) {
    if (n == 0) return 0;
    uint64_t x = n;
    uint64_t y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

void print_stats(const char* name, uint64_t* samples, int count) {
    if (count <= 0) {
        print("No samples\n");
        return;
    }
    
    // Calcular promedio
    uint64_t sum = 0;
    uint64_t min = samples[0];
    uint64_t max = samples[0];
    
    for (int i = 0; i < count; i++) {
        sum += samples[i];
        if (samples[i] < min) min = samples[i];
        if (samples[i] > max) max = samples[i];
    }
    
    uint64_t avg = sum / count;
    
    // Calcular desviación estándar
    uint64_t variance_sum = 0;
    for (int i = 0; i < count; i++) {
        int64_t diff = samples[i] - avg;
        if (diff < 0) diff = -diff;
        variance_sum += (diff * diff);
    }
    uint64_t variance = variance_sum / count;
    uint64_t stddev = sqrt_uint64(variance);
    
    // Convertir a ms
    uint64_t avg_ms = cycles_to_ms(avg);
    uint64_t stddev_ms = cycles_to_ms(stddev);
    uint64_t min_ms = cycles_to_ms(min);
    uint64_t max_ms = cycles_to_ms(max);
    
    char buf[32];
    
    print("=== ");
    print(name);
    print(" ===\n");
    
    print("Samples: ");
    uint64_to_str(count, buf);
    print(buf);
    print("\n");
    
    print("Avg: ");
    uint64_to_str(avg_ms, buf);
    print(buf);
    print(" ms (");
    uint64_to_str(avg, buf);
    print(buf);
    print(" cycles)\n");
    
    print("StdDev: ");
    uint64_to_str(stddev_ms, buf);
    print(buf);
    print(" ms (");
    uint64_to_str(stddev, buf);
    print(buf);
    print(" cycles)\n");
    
    print("Min: ");
    uint64_to_str(min_ms, buf);
    print(buf);
    print(" ms (");
    uint64_to_str(min, buf);
    print(buf);
    print(" cycles)\n");
    
    print("Max: ");
    uint64_to_str(max_ms, buf);
    print(buf);
    print(" ms (");
    uint64_to_str(max, buf);
    print(buf);
    print(" cycles)\n");
    print("\n");
}
