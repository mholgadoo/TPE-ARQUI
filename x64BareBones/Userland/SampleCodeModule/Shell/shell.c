// shell.c
#include "../shell.h"
#include <stdint.h>
#include <stddef.h>
#include "../syscalls.h"
#include "../lib.h"
#include "../bench/bench.h"

#define REG_COUNT 21

static char *username;


void shell_print_help() {
    print("=== M&M Shell - Available Commands ===\n\n");
    
    print("System Commands:\n");
    print("  help           - show this help\n");
    print("  clear          - clear screen\n");
    print("  time           - display system time\n");
    print("  regs           - display CPU registers\n");
    print("  fontscale 1-3  - change font size\n\n");
    
    print("Exception Tests (dump registers + return to shell):\n");
    print("  divzero        - trigger division by zero exception\n");
    print("  invopcode      - trigger invalid opcode exception\n\n");
    
    print("Games:\n");
    print("  pongis         - play pongis (shows FPS!)\n\n");
    
    print("Benchmarks:\n");
    print("  benchtest      - test benchmark timer\n");
    print("  bench fps [s]  - FPS benchmark (default: 5 sec)\n");
    print("  bench fpu [n]  - FPU benchmark Mandelbrot (default: 256)\n");
    print("  bench io [m]   - I/O benchmark (kbd/fb/all)\n");
    print("  bench env      - show environment info\n");
}

static int read_line(char *buf, int max) {
    int i = 0;
    while (i < max - 1) {
        char c = 0;
        int r = 0;
        do {
            r = read(&c, 1);
        } while (r == 0);
        if (c == '\n') {
            printChar('\n');
            break;
        }
        if (c == '\b') {
            if (i > 0) {
                i--;
                printChar('\b');
            }
        } else {
            buf[i++] = c;
            printChar(c);
        }
    }
    buf[i] = '\0';
    return i;
}

static void trigger_divzero() {
    // Usar assembly directo para garantizar división por cero
    __asm__ __volatile__(
        "mov $1, %%eax\n"      // eax = 1
        "xor %%edx, %%edx\n"   // edx = 0 (parte alta)
        "xor %%ebx, %%ebx\n"   // ebx = 0 (divisor)
        "div %%ebx\n"          // 1 / 0 → excepción #DE
        :
        :
        : "%eax", "%edx", "%ebx"
    );
}

static void trigger_invopcode() {
    __asm__ __volatile__("ud2");
}

static void print_time() {
    rtc_time_t tm;
    getTime(&tm);
    int hour = (tm.hour - 3 + 24) % 24;//restamos 3 por la zona horaria UTC-3
    char buf[9];
    buf[2] = ':'; buf[5] = ':'; buf[8] = '\0';
    buf[0] = '0' + (hour / 10);
    buf[1] = '0' + (hour % 10); 
    buf[3] = '0' + (tm.min  / 10);
    buf[4] = '0' + (tm.min  % 10);
    buf[6] = '0' + (tm.sec  / 10);
    buf[7] = '0' + (tm.sec  % 10);
    print(buf);
    print("\n");
}

static const char *regName[REG_COUNT] = {
  "RAX","RBX","RCX","RDX","RSI","RDI","RBP",
  "R8","R9","R10","R11","R12","R13","R14","R15",
  "RSP","RIP","CS","RFLAGS","URSP","USS"
};

static void print_hex64(uint64_t v) {
    char buf[17]; buf[16]=0;
    for (int i=15;i>=0;i--) { int d=v&0xF;
        buf[i]= d<10?'0'+d:'A'+(d-10);
        v>>=4;
    }
    print(buf);
}

static void print_regs(void)
{
    uint64_t regs[REG_COUNT];
    getRegisters(regs);  // Pasar el puntero directo, no &regs
    for (int i = 0; i < REG_COUNT; i++) {
        print(regName[i]);
        print(": 0x");
        print_hex64(regs[i]);
        print("\n");
    }
}

static void test_benchmark(void) {
    print("=== Benchmark Timer Test ===\n");
    
    // Verificar soporte de TSC
    print("TSC Support: ");
    print(has_tsc() ? "YES\n" : "NO\n");
    
    print("Invariant TSC: ");
    print(has_invariant_tsc() ? "YES\n" : "NO\n");
    
    // Mostrar frecuencia TSC
    uint64_t freq = get_tsc_freq();
    print("TSC Frequency: ");
    print_hex64(freq);
    print(" Hz\n\n");
    
    // Test simple: medir el tiempo de algunas operaciones
    print("Testing benchmark functions...\n");
    
    #define NUM_SAMPLES 10
    uint64_t samples[NUM_SAMPLES];
    
    // Test 1: Medir tiempo de llamar a bench_start/stop
    print("Test 1: Overhead de bench_start/bench_stop\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        uint64_t start = bench_start();
        samples[i] = bench_stop(start);
    }
    print_stats("Overhead", samples, NUM_SAMPLES);
    
    // Test 2: Medir un loop simple
    print("Test 2: Loop de 1000 iteraciones\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        uint64_t start = bench_start();
        volatile int x = 0;
        for (int j = 0; j < 1000; j++) {
            x++;
        }
        samples[i] = bench_stop(start);
    }
    print_stats("Loop 1000", samples, NUM_SAMPLES);
    
    // Test 3: Medir tiempo de una syscall
    print("Test 3: Syscall get_tsc_freq()\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        uint64_t start = bench_start();
        get_tsc_freq();
        samples[i] = bench_stop(start);
    }
    print_stats("Syscall", samples, NUM_SAMPLES);
    
    print("=== Test Complete ===\n");
}

// Helper para parsear un número de un string
static int parse_int(const char *str) {
    int result = 0;
    int i = 0;
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return result;
}

// Helper para verificar si un string comienza con un prefijo
static int starts_with(const char *str, const char *prefix) {
    int i = 0;
    while (prefix[i] != '\0') {
        if (str[i] != prefix[i]) return 0;
        i++;
    }
    return 1;
}

// Helper para extraer el argumento después de un espacio
static const char* get_arg(const char *line, const char *cmd) {
    int cmd_len = 0;
    while (cmd[cmd_len]) cmd_len++;
    
    // Saltar el comando
    const char *p = line + cmd_len;
    
    // Saltar espacios
    while (*p == ' ') p++;
    
    return p;
}

static void setUsername(const char *name) {
    if (name == NULL || name[0] == '\0') {
        username = "User";
    } else {
        username = (char *)name;
    }
}

static void commandProc(const char *line) {
    if (str_eq(line, "help"))
        shell_print_help();
    else if (str_eq(line, "divzero"))
        trigger_divzero();
    else if (str_eq(line, "invopcode"))
        trigger_invopcode();
    else if (str_eq(line, "time"))
        print_time();
    else if (str_eq(line, "regs"))
        print_regs();
    else if (str_eq(line, "clear"))
        clearScreen();
    else if (str_eq(line, "fontscale 1"))
        changeFontSize(1);
    else if (str_eq(line, "fontscale 2"))
        changeFontSize(2);
    else if (str_eq(line, "fontscale 3"))
        changeFontSize(3);
    else if (str_eq(line, "playbeep")) {
        print("Playing beep sound...\n");
        playBeep(8, 220, 200);      //A
        playBeep(8, 249.94, 200);   //B
        playBeep(8, 261.63, 200);   //C
        playBeep(8, 293.66, 200);   //D
        playBeep(8, 329.63, 200);   //E
        playBeep(8, 349.23, 200);   //F
        playBeep(8, 392, 200);      //G
    } else if (str_eq(line, "pongis"))
        pongis_game();
    else if (str_eq(line, "benchtest"))
        test_benchmark();
    // Comandos bench
    else if (starts_with(line, "bench fps")) {
        const char *arg = get_arg(line, "bench fps");
        int seconds = 5;
        if (arg[0] >= '0' && arg[0] <= '9') {
            seconds = parse_int(arg);
        }
        bench_fps(seconds);
    } else if (starts_with(line, "bench fpu")) {
        const char *arg = get_arg(line, "bench fpu");
        int size = 256;
        if (arg[0] >= '0' && arg[0] <= '9') {
            size = parse_int(arg);
        }
        bench_fpu(size);
    } else if (starts_with(line, "bench io")) {
        const char *arg = get_arg(line, "bench io");
        if (arg[0] == '\0') {
            bench_io("all");
        } else {
            bench_io(arg);
        }
    } else if (str_eq(line, "bench env")) {
        bench_env();
    } else if (str_eq(line, "bench")) {
        print("Benchmark commands:\n");
        print("  bench fps [seconds]  - FPS benchmark\n");
        print("  bench fpu [size]     - FPU benchmark (Mandelbrot)\n");
        print("  bench io [mode]      - I/O benchmark (kbd/fb/all)\n");
        print("  bench env            - Environment info\n");
    } else
        print("Unknown command\n");
}

void shell_run(const char *name) {
    setUsername(name);
    char line[64];
    while (1) {
        print("@");
        print(username);
        print("$> ");
        read_line(line, sizeof(line));
        if (line[0] == '\0') continue;
        commandProc(line);
    }
}
