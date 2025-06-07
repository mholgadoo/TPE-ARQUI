#include <time.h>
#include <stdint.h>

static void int_20();

void irqDispatcher(uint64_t irq) {
	//putPixel(0x00FF0000, 200, 200); //este si
	switch (irq) {
		case 0:
			int_20();
			break;
		case 1:
			//putPixel(0x00FF0000, 20, 20); //este no
			keyboard_handler();
			break;
		
	}
	return;
}

void int_20() {
	timer_handler();
}

void int_21() {
    keyboard_handler();
}
