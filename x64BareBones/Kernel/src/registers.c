#include <string.h>
#include "registers.h"

#define REG_COUNT 21

uint64_t saved_registers[REG_COUNT];

static const uint8_t map[REG_COUNT] = {
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20
};

void save_snapshot(const uint64_t *raw)
{
    for (int i = 0; i < REG_COUNT; i++)
        saved_registers[i] = raw[ map[i] ];
}

uint64_t get_saved_registers(uint64_t *dst)
{
    memcpy((void*)dst, (void*)saved_registers, REG_COUNT * sizeof(uint64_t)); //por ahora imprime basura si no hubo ctrl r
    return 1;
}