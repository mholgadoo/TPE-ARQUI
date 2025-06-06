#include <time.h>
#include <stdint.h>

static unsigned long ticks = 0;

void sleep(uint64_t pauseTicks) {
    uint64_t start = ticks;
	_sti();
    while ((ticks - start) < pauseTicks) {
    }
	_cli(); //no se por que al entrar a sleep estan las interrupciones desactivadas asi que las activo y por las dudas las desactivo
}

void timer_handler() {
	ticks++;
}

uint64_t get_ticks() {
    return ticks;
}
