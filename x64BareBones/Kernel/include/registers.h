/* registers.h */
#ifndef REGISTERS_H
#define REGISTERS_H
#include <stdint.h>

#define REG_COUNT 21

extern uint64_t snapshot[REG_COUNT];



extern volatile uint64_t snapshot_saved[REG_COUNT];

void save_snapshot(const uint64_t *raw);   /* re-acomoda a snapshot_saved */
uint64_t * get_registers(void);            /* devuelve raw (definido en ASM) */
extern uint64_t get_saved_registers(uint64_t *dst);

#endif
