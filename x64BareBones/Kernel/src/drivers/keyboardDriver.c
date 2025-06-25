#include <keyboardDriver.h>
#include <lib.h>
#include "font.h"
#include <stdint.h>
#include "registers.h"
#include <videoDriver.h>
#include <interrupts.h>

extern uint8_t inb(uint16_t port);  // asm i/o
extern void outb(uint16_t port, uint8_t value);

#define KEYS_AMOUNT 58
#define BUFFER_SIZE 256

static char keyBuffer[BUFFER_SIZE];
static int head = 0;
static int tail = 0;

static const char *regName[REG_COUNT] = {
    "RAX","RBX","RCX","RDX","RSI","RDI","RBP",
    "R8","R9","R10","R11","R12","R13","R14","R15",
    "RSP","RIP","CS","RFLAGS","URSP","USS"
};

static void show_registers(void) {
    uint64_t regs[REG_COUNT];
    get_saved_registers(regs);
    writeString("Registers snapshot:\n", 20);
    for (int i = 0; i < REG_COUNT; i++) {
        writeString(regName[i], 3);
        writeString(": 0x", 4);
        print_hex64(regs[i]);
        writeString("\n", 1);
    }

    /*
     * We are still inside the keyboard interrupt handler when this
     * function is called.  Send the End Of Interrupt (EOI) to the PIC
     * manually so new keyboard interrupts are delivered while we wait
     * for the user to press a key to continue.
     */
    outb(0x20, 0x20);

    _sti();
    writeString("Press any key to continue...\n", 29);
    while (keyboard_getchar() == 0);
    clearScreen();
}

static const char scanCodeTable[KEYS_AMOUNT][2] = {
    {0, 0}, {27, 27}, {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'},
    {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'}, {'-', '_'}, {'=', '+'}, {'\b', '\b'},
    {'\t', '\t'}, {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'}, {'t', 'T'}, {'y', 'Y'},
    {'u', 'U'}, {'i', 'I'}, {'o', 'O'}, {'p', 'P'}, {'[', '{'}, {']', '}'}, {'\n', '\n'},
    {0, 0}, {'a', 'A'}, {'s', 'S'}, {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'},
    {'j', 'J'}, {'k', 'K'}, {'l', 'L'}, {';', ':'}, {'\'', '\"'}, {'`', '~'}, {0, 0},
    {'\\', '|'}, {'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, {'b', 'B'}, {'n', 'N'},
    {'m', 'M'}, {',', '<'}, {'.', '>'}, {'/', '?'}, {0, 0}, {0, 0}, {0, 0}, {' ', ' '}
};


static int isBufferEmpty() {
    return head == tail;
}

static int isBufferFull() {
    return ((tail + 1) % BUFFER_SIZE) == head;
}

static void pushKey(char c) {
    if (!isBufferFull()) {
        keyBuffer[tail] = c;
        tail = (tail + 1) % BUFFER_SIZE;
    }
}

char keyboard_getchar() {
    if (isBufferEmpty())
        return 0;

    char c = keyBuffer[head];
    head = (head + 1) % BUFFER_SIZE;
    return c;
}


static int shift = 0;   // flags del teclado
static int capsLock = 0;
static int ctrlPressed = 0;

void keyboard_handler() {
    uint8_t scancode = inb(0x60);
    char ascii = 0;

    if (scancode == 0x1D){
        ctrlPressed = 1;
    }else if (scancode == 0x9D){
        ctrlPressed = 0;
    }

    if(scancode == 0x2A || scancode == 0x36) {
        shift = 1;
    }
    if(scancode == 0xAA || scancode == 0xB6) {
        shift = 0;
    }
    if(scancode == 0x3A) {
        capsLock = !capsLock;  // Toggle Caps Lock
    }

    if (scancode >= KEYS_AMOUNT)
        return;

    char base = scanCodeTable[scancode][0];
    char shifted = scanCodeTable[scancode][1];

    // logica de mayuscula para letras
    if (base >= 'a' && base <= 'z') {
        int upper = capsLock ^ shift;
        ascii = upper ? shifted : base;
    } else {
        ascii = shift ? shifted : base;
    }

    if (ascii != 0) {
        if (ctrlPressed && (ascii =='r' || ascii =='R')) {
            uint64_t *snapshot = get_registers();
            save_snapshot(snapshot);
            show_registers();
            return;
        }
        if (ctrlPressed && (ascii == 'l' || ascii == 'L')) {
            clearScreen();
            return;
        }
        pushKey(ascii);
    }
}