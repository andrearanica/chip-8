/* chip-8.h
 * Interface file for 'chip_8' library
*/

#ifndef _CHIP_8_H
#define _CHIP_8_H

#include <stdint.h>
#include <stdbool.h>

#define DISPLAY_DIM_X 64
#define DISPLAY_DIM_Y 32
#define MEMORY_SIZE 4096

struct _chip_8 {
    uint16_t PC;
    uint8_t memory[MEMORY_SIZE];
    uint8_t registers[16];
    uint8_t SP;
    uint16_t stack[16];
    uint16_t I;
    uint8_t VF;
    uint8_t delay_timer;
    uint8_t sound_timer;

    int display[DISPLAY_DIM_Y * DISPLAY_DIM_X];
};

typedef struct _chip_8 * Chip8;

extern Chip8 chip8_new();
extern void  chip8_load_program(Chip8, char*);
extern void  chip8_run(Chip8, bool);

#endif