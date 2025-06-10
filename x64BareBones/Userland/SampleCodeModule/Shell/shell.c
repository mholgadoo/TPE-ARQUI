#include "../shell.h"
#include <stdint.h>
#include <stddef.h>
#include "../syscalls.h"

#define SYS_WRITE             0
#define SYS_READ              1
#define SYS_CLEAR_SCREEN      2
#define SYS_GET_REGS          6
#define SYS_GET_TIME          7
#define SYS_CHANGE_FONT_SIZE  9


static char *username;
static int fontScale = 1;

static int str_eq(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) {
        i++;
    }
    return a[i] == b[i];
}

static int str_len(const char *s) {
    int l = 0;
    while (s[l]) l++;
    return l;
}

void shell_print_help() {
    _sys_write(SYS_WRITE,
        "Available commands:\n", 
        str_len("Available commands:\n"));
    _sys_write(SYS_WRITE,
        "help       - show this help\n", 
        str_len("help       - show this help\n"));
    _sys_write(SYS_WRITE,
        "divzero    - trigger division by zero\n", 
        str_len("divzero    - trigger division by zero\n"));
    _sys_write(SYS_WRITE,
        "invopcode  - trigger invalid opcode\n", 
        str_len("invopcode  - trigger invalid opcode\n"));
    _sys_write(SYS_WRITE,
        "time       - display system time\n", 
        str_len("time       - display system time\n"));
    _sys_write(SYS_WRITE,
        "regs       - display CPU registers\n", 
        str_len("regs       - display CPU registers\n"));
    _sys_write(SYS_WRITE,
        "clear      - clear screen\n", 
        str_len("clear      - clear screen\n"));
    _sys_write(SYS_WRITE,
        "font+      - increase font size\n", 
        str_len("font+      - increase font size\n"));
    _sys_write(SYS_WRITE,
        "font-      - decrease font size\n", 
        str_len("font-      - decrease font size\n"));
}

static int read_line(char *buf, int max) {
    int i = 0;
    while (i < max - 1) {
        char c = 0;
        int r = 0;
        do {
            r = _sys_read(SYS_READ, 0, &c, 1);
        } while (r == 0);  // aspera hasta que haya input

        if (c == '\n') {
            _sys_write(SYS_WRITE, "\n", 1);
            break;
        }
        if (c == '\b') {
            if (i > 0) {
                i--;
                _sys_write(SYS_WRITE, "\b", 1);
            }
        } else {
            buf[i++] = c;
            _sys_write(SYS_WRITE, &c, 1);
        }
    }
    buf[i] = '\0';
    return i;
}

static void trigger_divzero() {
    int x = 1/0;
}

static void trigger_invopcode() {
    //testeo rapido CAMBIAR
    __asm__ __volatile__("ud2");
}


static void print_time() {
    rtc_time_t tm;
    _sys_get_time(SYS_GET_TIME, &tm);
    char buf[9];
    buf[2] = ':'; buf[5] = ':'; buf[8] = '\0';
    buf[0] = '0' + (tm.hour / 10);
    buf[1] = '0' + (tm.hour % 10);
    buf[3] = '0' + (tm.min  / 10);
    buf[4] = '0' + (tm.min  % 10);
    buf[6] = '0' + (tm.sec  / 10);
    buf[7] = '0' + (tm.sec  % 10);
    _sys_write(SYS_WRITE, buf, 8);
    _sys_write(SYS_WRITE, "\n", 1);
}

static void print_hex64(uint64_t value) {
    char buf[16];
    for (int i = 15; i >= 0; i--) {
        int d = value & 0xF;
        buf[i] = d < 10 ? '0' + d : 'A' + (d - 10);
        value >>= 4;
    }
    _sys_write(SYS_WRITE, buf, 16);
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
    _sys_get_registers(SYS_GET_REGS, regs);

    for (int i = 0; i < 17; i++) {
        _sys_write(SYS_WRITE, names[i], str_len(names[i]));
        _sys_write(SYS_WRITE, ": 0x", 4);
        print_hex64(regs[i]);
        _sys_write(SYS_WRITE, "\n", 1);
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
        _sys_clearScreen(SYS_CLEAR_SCREEN);
    else if (str_eq(line, "fontscale 1"))
       _sys_changeFontSize(SYS_CHANGE_FONT_SIZE, 1);
    else if (str_eq(line, "fontscale 2"))
            _sys_changeFontSize(SYS_CHANGE_FONT_SIZE, 2);
    else if (str_eq(line, "fontscale 3"))
        _sys_changeFontSize(SYS_CHANGE_FONT_SIZE, 3);
    else if (str_eq(line, "playbeep")) {
        _sys_write(SYS_WRITE, "Playing beep sound...\n", str_len("Playing beep sound...\n"));
        _sys_playBeep(8, 1000, 440);  // Play a beep at 440 Hz
    }
    else
       _sys_write(SYS_WRITE, "Unknown command\n",str_len("Unknown command\n"));
}
void shell_run(const char *name) {
    setUsername(name);
    char line[64];
    while (1) {
        _sys_write(SYS_WRITE, "@", 1);
        _sys_write(SYS_WRITE, username, str_len(username));
        _sys_write(SYS_WRITE, "$> ", 3);

        read_line(line, sizeof(line));
        if (line[0] == '\0') continue;

        commandProc(line);
    }
}
