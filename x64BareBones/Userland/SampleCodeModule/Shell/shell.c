// shell.c
#include "../shell.h"
#include <stdint.h>
#include <stddef.h>
#include "../syscalls.h"
#include "../lib.h"

static char *username;
static int fontScale = 1;


void shell_print_help() {
    print("Available commands:\n");
    print("help       - show this help\n");
    print("divzero    - trigger division by zero\n");
    print("invopcode  - trigger invalid opcode\n");
    print("time       - display system time\n");
    print("regs       - display CPU registers\n");
    print("clear      - clear screen\n");
    print("fontscale 1      - lowest font size\n");
    print("fontscale 2      - middle font size\n");
    print("fontscale 3      - bigger font size\n");
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
    int x = 1 / 0;
}

static void trigger_invopcode() {
    __asm__ __volatile__("ud2");
}

static void print_time() {
    rtc_time_t tm;
    getTime(&tm);
    char buf[9];
    buf[2] = ':'; buf[5] = ':'; buf[8] = '\0';
    buf[0] = '0' + (tm.hour / 10);
    buf[1] = '0' + (tm.hour % 10);
    buf[3] = '0' + (tm.min  / 10);
    buf[4] = '0' + (tm.min  % 10);
    buf[6] = '0' + (tm.sec  / 10);
    buf[7] = '0' + (tm.sec  % 10);
    print(buf);
    print("\n");
}

static void print_hex64(uint64_t value) {
    char buf[17];
    buf[16] = '\0';
    for (int i = 15; i >= 0; i--) {
        int d = value & 0xF;
        buf[i] = d < 10 ? '0' + d : 'A' + (d - 10);
        value >>= 4;
    }
    print(buf);
}

static void print_regs() {
    static const char *names[17] = {
        "RAX","RBX","RCX","RDX",
        "RSI","RDI","RBP","RSP",
        "R8","R9","R10","R11",
        "R12","R13","R14","R15",
        "RIP"
    };
    uint64_t regs[17];
    getRegisters(regs);
    for (int i = 0; i < 17; i++) {
        print(names[i]);
        print(": 0x");
        print_hex64(regs[i]);
        print("\n");
    }
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
    else
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
