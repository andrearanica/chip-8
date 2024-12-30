/* chip-8.c
* Implementation file for the 'chip-8' library
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "chip-8.h"

#define BUFFER_DIM 256

void chip8_print_debug();

uint8_t chip8_char_sprites[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x50, 0x20, 0xF0, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x80, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x01, 0xF0  // 9
};

Chip8 chip8_new() {
    Chip8 chip_8 = (Chip8) malloc(sizeof(struct _chip_8));

    if (chip_8 == NULL) {
        fprintf(stderr, "CHIP8: cannot allocate Chip8");
        return NULL;
    }

    // Initialize emulator data
    chip_8->PC = 0x200;
    chip_8->SP = 0;
    chip_8->I = 0;
    chip_8->delay_timer = 0;
    chip_8->sound_timer = 0;

    int min_sprite_address = 0x050;
    for (int i = 0; i < sizeof(chip8_char_sprites); i++) {
        chip_8->memory[min_sprite_address+1] = chip8_char_sprites[i];
    }

    return chip_8;
}

void chip8_load_program(Chip8 chip8, char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        perror(path);
        exit(EXIT_FAILURE);
    }

    // I positionate the file pointer on the beginning of the ROM
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    
    fread(chip8->memory+0x200, sizeof(uint16_t), size, file);
}

void chip8_run(Chip8 chip8, bool debug) {
    uint16_t instruction;
    int opcode, x, kk, nnn, n;
    
    while(1) {
        instruction = chip8->memory[chip8->PC] << 8 | chip8->memory[chip8->PC+1];
        opcode = (instruction & 0xF000) >> 12;
        x = (instruction & 0x0F00) >> 8;
        kk = (instruction & 0x00FF);
        nnn = (instruction & 0x0FFF) > 4;
        n = (instruction & 0x000F);

        chip8->PC += 2;

        switch(opcode) {
            case 0x0:
                // 224 = 00E0 Clear screen
                if (instruction == 224) {
                    for (int i = 0; i < DISPLAY_DIM_X * DISPLAY_DIM_Y; i++) {
                        chip8->display[i] = 0;
                    }
                }
            case 0x6:
                // Load register
                chip8->registers[x] = kk;
                break;
            case 0xa:
                // Load inside I register
                chip8->I = nnn;
                break;
            case 0xd:
                // TODO Draw sprite
                break;
            default:
                fprintf(stderr, "Error: instruction '%04x' not supported\n", instruction);
                exit(EXIT_FAILURE);
                break;
        }
    
        if (debug) {
            system("clear");
            printf("########## REGISTERS ##########\n");

            printf("| PC | %d |\n", chip8->PC);
            printf("| I  | %d |\n", chip8->I);

            for (int i = 0; i < sizeof(chip8->registers); i++) {
                printf("| V%d | %d |\n", i, chip8->registers[i]);
            }
            printf("Instruction: %04x\n", instruction);
            printf("Opcode: %01x\n", opcode);
        }
        scanf("%s");
    }
}