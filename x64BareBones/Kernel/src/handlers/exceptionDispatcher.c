#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <syscalls.h>
#include <time.h>
#include <interrupts.h>

#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_ID 6

static void zero_division();
static void invalid_opcode();

static const char *reg_names[] = {"RAX","RBX","RCX","RDX","RBP","RDI","RSI","R8","R9","R10","R11","R12","R13","R14","R15","RIP","CS","RFLAGS"};

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
    writeString("Exception: ", 11);
    writeString(msg, len);
    writeString("\n\nRegisters:\n", 12);

	uint64_t * regs;
	save_registers(regs);

    for (int i = 0; i < 18; i++) {
        writeString(reg_names[i], 3);
        writeString(": ", 2);
        print_hex64(regs[i]);
        writeString("\n", 1);
		writeString("\n", 1);
    }

	_sti();
	writeString("Press any key to continue...\n", 29);
	
	char c;
	while (keyboard_getchar(&c) == 0); // Espera a que el usuario presione una tecla
	clearScreen();
}