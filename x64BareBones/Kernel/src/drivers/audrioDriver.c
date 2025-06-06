#include <audioDriver.h>

extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);

#define PIT_FREQUENCY 1193182 //ejemplo

uint64_t playBeep(uint32_t f) {
    uint32_t div = PIT_FREQUENCY / f;

    // Configure PIT
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t) (div & 0xFF));
    outb(0x42, (uint8_t) ((div >> 8) & 0xFF));

    // play sound
    uint8_t aux = inb(0x61);
    if (aux != (aux | 3)) {
        outb(0x61, aux | 3);
    }
    return 1;
}

uint64_t stopBeep() {
    uint8_t aux = inb(0x61) & 0xFC;
    outb(0x61, aux);
    return 1;
}