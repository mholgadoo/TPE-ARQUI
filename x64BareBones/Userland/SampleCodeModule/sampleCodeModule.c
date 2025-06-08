#include "syscalls.h"
#include <stdint.h>
#include "syscalls.h"    // Debe tener los prototipos de _sys_write y _sys_read
#include "shell.h"       // Debe tener el prototipo de start_shell


#define SYS_READ 1
#define USERNAME_MAX_LEN 32

int main() {
    // Llamar a tu shell
     shell_run(0);
    return 0;
}

/*
int main() {
    char c;
    while (1) {
        if (_sys_read(SYS_READ, 0, &c, 1) > 0) {
            _sys_write(0, &c, 1);
        }
    }
    return 0;
}

//mains de prueba
// Espera unos ticks entre cada cambio, para que veas bien el efecto
void delay(int ticks) {
    _sys_sleep(3, ticks);
}

int main() {
    const char *msg = "Cambio";
    int sizes[] = {1, 2, 3, 2, 1};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    for (int i = 0; i < num_sizes; ++i) {
        _sys_clearScreen(2);
        _sys_changeFontSize(9, sizes[i]);
        _sys_write(0, msg, 6);
        delay(60); // espera ~1 segundo si tu timer es 60 ticks por segundo
    }
    // Dejá el tamaño en 1 al finalizar
    _sys_changeFontSize(0,1);

    return 0;
}


int main() {
    // Beep de 1 kHz por 500 ms
    _sys_playBeep(8, 1000, 500);
    _sys_sleep(3, 20); // Espera un rato para no solapar sonidos
    _sys_playBeep(8, 500, 1000); // 500 Hz por 1 segundo
    return 0;
}

void print_time(const rtc_time_t *t) {
    char buf[32];

    // Imprime hora:min:seg
    int len = 0;
    buf[len++] = (t->hour / 10) + '0';
    buf[len++] = (t->hour % 10) + '0';
    buf[len++] = ':';
    buf[len++] = (t->min / 10) + '0';
    buf[len++] = (t->min % 10) + '0';
    buf[len++] = ':';
    buf[len++] = (t->sec / 10) + '0';
    buf[len++] = (t->sec % 10) + '0';
    buf[len++] = '\n';
    buf[len] = 0;

    _sys_write(0, buf, len);
}

int main() {
    rtc_time_t t;

    while (1) {
        _sys_get_time(7, &t);
        print_time(&t);

        // Espera 1 segundo (ajustá si tu sleep usa ticks en vez de ms)
        _sys_sleep(3, 18); // 18 ticks = 1 segundo si tu PIT está en 18.2Hz
    }

    return 0;
}


int int_to_ascii(uint64_t value, char *buf);

const char *reg_names[] = {
    "RAX", "RBX", "RCX", "RDX",
    "RSI", "RDI", "RBP", "RSP",
    "R8", "R9", "R10", "R11",
    "R12", "R13", "R14", "R15"
};

int int_to_ascii(uint64_t value, char *buf) {
    char temp[32];
    int i = 0;
    if (value == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return 1;
    }
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }
    int len = i;
    for (int j = 0; j < len; j++) {
        buf[j] = temp[len - j - 1];
    }
    buf[len] = 0;
    return len;
}

int main() {
    uint64_t buffer[16];
        
    while (1) {
        _sys_clearScreen(2);
        _sys_get_registers(6, buffer);

        for (int i = 0; i < 16; ++i) {
            _sys_write(0, reg_names[i], 3);
            _sys_write(0, ": ", 2);

            char numbuf[32];
            int len = int_to_ascii(buffer[i], numbuf);
            _sys_write(0, numbuf, len);
            _sys_write(0, "\n", 1);
        }
        _sys_sleep(3, 20); // Espera ~2 segundos si tu tick es 1/10s
    }
    return 0;
}


// Convierte un uint64_t a string decimal. Retorna longitud del strin

int main() {
    uint64_t ticks;
    while (1) {
		_sys_sleep(3, 20);
        // Mostramos ticks usando tu write
        char buf[32];
        ticks = _sys_get_ticks(5);
        int len = int_to_ascii(ticks, buf); // función auxiliar que convierta ticks a string
        _sys_write(0, buf, len);
        _sys_write(0, "\n", 1);
    }
    return 0;
}


int main() {
    // Dibuja varios rectángulos de distintos colores, posiciones y tamaños
    _sys_drawRect(4, 0xFF0000,  50,  50, 100, 60);   // Rojo
    _sys_drawRect(4, 0x00FF00, 200,  80, 150, 40);   // Verde
    _sys_drawRect(4, 0x0000FF, 100, 180,  80, 80);   // Azul
    _sys_drawRect(4, 0xFFFF00, 220, 150,  60, 90);   // Amarillo

    // También podés probar rectángulos que se solapan
    _sys_drawRect(4, 0xFFFFFF, 80, 70, 60, 120);     // Blanco, superpuesto

    // Esperá con sleep para ver bien los resultados (si la pantalla se limpia con un clear después)
    _sys_sleep(3,30);
	int64_t ticks = _sys_get_ticks(5);


    // Opcional: limpiar pantalla y dibujar otro rectángulo
    _sys_clearScreen(2);
    _sys_drawRect(4, 0xFF00FF, 100, 100, 200, 50);   // Violeta después de limpiar

    // Fin: loop infinito (o return 0, depende si tu kernel lo soporta)
    while(1);
    // return 0; // Si preferís
}

int main3() {
    while (1) {
        _sys_write("ZZZ...\n", 7);
        _sys_sleep(10);            // Dormí 100 ticks (~1 segundo si tu PIT es 100Hz)
        _sys_write("Desperte!\n", 11);
        _sys_sleep(100);
    }
    return 0;
}

int main2() {
    char c;
    while (1) {
        if (_sys_read(0, &c, 1) > 0) {
            _sys_write(&c, 1);
        }
    }
    return 0;
}


//Prueba sys_write y sys_clearScreen
int main1() {
    _sys_write("Hola mundo!\n", 12);
    _sys_clearScreen();
    _sys_write("Pantalla limpia\n", 16);
    return 0;
}
	*/
