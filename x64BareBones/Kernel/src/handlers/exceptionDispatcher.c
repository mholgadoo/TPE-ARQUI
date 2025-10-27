#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <syscalls.h>
#include <time.h>
#include <interrupts.h>
#include <videoDriver.h>

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
    clearScreen();
    writeString("========================================\n\n", 43);
    
    writeString("ExceptionType: ", 6);
    writeString(msg, len);
    writeString("\n\n", 2);

    static const char *name[21] = {
      "RAX   ","RBX   ","RCX   ","RDX   ","RSI   ","RDI   ","RBP   ",
      "R8    ","R9    ","R10   ","R11   ","R12   ","R13   ","R14   ","R15   ",
      "RSP   ","RIP   ","CS    ","RFLAGS","URSP  ","USS   "
    };

    writeString("--- Register Dump ---\n", 22);
    
    // Imprimir en columnas de 2 para mejor legibilidad
    for (int i = 0; i < 21; i++) {
        writeString(name[i], 6);
        writeString(": 0x", 4);
        print_hex64(snapshot[i]);
        
        if (i % 2 == 1) {
            writeString("\n", 1);
        } else {
            writeString("    ", 4);
        }
    }
    
    if (21 % 2 != 0) writeString("\n", 1);
    
    writeString("\n========================================\n", 42);
	_sti();
	writeString("Press any key to return to shell...\n", 36);
	
    while (keyboard_getchar() == 0); // Espera a que el usuario presione una tecla
    clearScreen();
}