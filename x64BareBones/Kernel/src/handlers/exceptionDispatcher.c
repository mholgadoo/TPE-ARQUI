#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <syscalls.h>
#include <time.h>
#include <interrupts.h>
#include <interrupts.h>

#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_ID 6

static void zero_division();
static void invalid_opcode();

extern uint64_t snapshot[];

static const char *regName[21] = {
 "RAX","RBX","RCX","RDX","RSI","RDI","RBP",
 "R8","R9","R10","R11","R12","R13","R14","R15",
 "RSP","RIP","CS","RFLAGS","URSP","USS"
};

void exceptionDispatcher(int exception) {
	if (exception == ZERO_EXCEPTION_ID)
		zero_division();
	else if (exception == INVALID_OPCODE_ID) {
		invalid_opcode();
	}
	//si no por ahora no hace nada
}

static void zero_division() {
	printException("Division by zero", 16);
}

static void invalid_opcode() {
	printException("Invalid opcode", 15);
}

void printException(const char *msg, int len) {
    uint64_t *raw = get_registers();   /* snapshot recién capturado */
    save_snapshot(raw);                /* lo congelo para comando regs */

    writeString("Exception: ", 11);
    writeString(msg, len);
    writeString("\n\nRegisters:\n", 12);

    static const char *name[21] = {
      "RAX","RBX","RCX","RDX","RSI","RDI","RBP",
      "R8","R9","R10","R11","R12","R13","R14","R15",
      "RSP","RIP","CS","RFLAGS","URSP","USS"
    };

    for (int i = 0; i < 21; i++) {
        print(name[i]); print(": 0x"); print_hex64(raw[i]); print("\n");
    }

	_sti();
	writeString("Press any key to continue...\n", 29);
	
	char c;
	while (keyboard_getchar(&c) == 0); // Espera a que el usuario presione una tecla
	clearScreen();
}