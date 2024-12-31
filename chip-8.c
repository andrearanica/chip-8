/* chip-8.c
* Implementation file for the 'chip-8' library
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "chip-8.h"

#define BUFFER_DIM 256

void chip8_print_debug();
void chip8_draw_sprite(Chip8, int, int, int);

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
    0xF0, 0x90, 0xF0, 0x01, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
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

    return chip_8;
}

void chip8_load_program(Chip8 chip_8, char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        perror(path);
        exit(EXIT_FAILURE);
    }

    // I positionate the file pointer on the beginning of the ROM
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    fread(chip_8->memory+0x200, sizeof(uint16_t), size, file);

    int min_sprite_address = 0x50;
    for (int i = 0; i < sizeof(chip8_char_sprites); i++) {
        chip_8->memory[min_sprite_address+i] = chip8_char_sprites[i];
    }
}

void chip8_run(Chip8 chip8, bool debug) {
    uint16_t instruction;
    int opcode, x, y, kk, nnn, n, result, value;
    
    while(1) {
        instruction = chip8->memory[chip8->PC] << 8 | chip8->memory[chip8->PC+1];
        opcode = (instruction & 0xF000) >> 12;
        x = (instruction & 0x0F00) >> 8;
        y = (instruction & 0x00F0) >> 4;
        kk = (instruction & 0x00FF);
        nnn = (instruction & 0x0FFF);
        n = (instruction & 0x000F);

        chip8->PC += 2;

        switch(opcode) {
            case 0x0:
                // 224 = 00E0 Clear screen
                if (instruction == 224) {
                    for (int i = 0; i < DISPLAY_DIM_X * DISPLAY_DIM_Y; i++) {
                        chip8->display[i] = 0;
                    }
                // 238 = 00EE Return
                } else if (instruction == 238) { 
                    chip8->PC = chip8->stack[chip8->SP];
                    chip8->SP--;
                } else {
                    fprintf(stderr, "Error: instruction '%04x' not supported\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 0x1:
                chip8->PC = nnn;
                break;
            case 0x2:
                chip8->SP++;
                chip8->stack[chip8->SP] = chip8->PC;
                chip8->PC = nnn;
                break;
            case 0x3:
                if (chip8->registers[x] == kk) {
                    chip8->PC += 2;
                }
                break;
            case 0x4:
                if (chip8->registers[x] != kk) {
                    chip8->PC += 2;
                }
                break;
            case 0x5:
                if (chip8->registers[x] == chip8->registers[y]) {
                    chip8->PC += 2;
                }
                break;
            case 0x6:
                // Load register
                chip8->registers[x] = kk;
                break;
            case 0x7:
                chip8->registers[x] += kk;
                break;
            case 0x8:
                switch(n) {
                    case 0:
                        chip8->registers[x] = chip8->registers[y];
                        break;
                    case 1:
                        chip8->registers[x] = (chip8->registers[x] | chip8->registers[y]);
                        break;
                    case 2:
                        chip8->registers[x] = (chip8->registers[x] & chip8->registers[y]);
                        break;
                    case 3:
                        chip8->registers[x] = (chip8->registers[x] ^ chip8->registers[y]);
                        break;
                    case 4:
                        result = chip8->registers[x] + chip8->registers[y];
                        chip8->registers[x] = result;
                        if (result > 255) {
                            chip8->VF = 1;
                        } else {
                            chip8->VF = 0;
                        }
                        break;
                    case 5:
                        if (chip8->registers[x] > chip8->registers[y]) {
                            chip8->VF = 1;
                        } else {
                            chip8->VF = 0;
                        }
                        chip8->registers[x] -= chip8->registers[y];
                        break;
                    case 6:
                        chip8->registers[x] >>= chip8->registers[y];
                        break;
                    case 7:
                        if (chip8->registers[y] > chip8->registers[x]) {
                            chip8->VF = 1;
                        } else {
                            chip8->VF = 0;
                        }
                        chip8->registers[x] = chip8->registers[y] - chip8->registers[x];
                        break;
                    case 0xE:
                        chip8->registers[x] <<= chip8->registers[y];
                        break;
                    default:
                        fprintf(stderr, "Error: instruction '%04X' not supported\n", instruction);
                }
                break;
            case 0x9:
                if (chip8->registers[x] != chip8->registers[y]) {
                    chip8->PC += 2;
                }
                break;
            case 0xa:
                // Load inside I register
                chip8->I = nnn;
                break;
            case 0xd:
                chip8_draw_sprite(chip8, chip8->registers[x], chip8->registers[y], n);
                break;
            case 0xf:
                printf("%d", kk);
                switch (kk) {
                    case 0x55:
                        for (int i = 0; i < x; i++) {
                            chip8->memory[chip8->I + i] = chip8->registers[i];
                        }
                        break;
                    case 0x65:
                        for (int i = 0; i < x; i++) {
                            chip8->registers[i] = chip8->memory[chip8->I + i];
                        }
                        break;
                    case 0x33:
                        value = chip8->registers[x];
                        chip8->memory[chip8->I] = (value / 100) % 10;
                        chip8->memory[chip8->I+1] = (value / 10) % 10;
                        chip8->memory[chip8->I+2] = value % 10;
                        break;
                    default:
                        fprintf(stderr, "Error: instruction '%04X' not supported\n", instruction);
                        exit(EXIT_FAILURE);
                        break;
                }
                break;
            default:
                fprintf(stderr, "Error: instruction '%04x' not supported\n", instruction);
                exit(EXIT_FAILURE);
                break;
        }

        usleep(50000);
        system("clear");

        for (int i = 0; i < DISPLAY_DIM_Y; i++) {
            for (int j = 0; j < DISPLAY_DIM_X; j++) {
                if (chip8->display[i * DISPLAY_DIM_X + j] == 1) {
                    printf("█");
                } else {
                    printf(" ");
                }
            }
            printf("\n");
        }

        if (false) {
            system("clear");
            int buffer_n = 0;
            int memory_buffer[MEMORY_SIZE] = {};
            int addresses_buffer[MEMORY_SIZE] = {};
            for (int i = 0; i < MEMORY_SIZE; i++) {
                if (chip8->memory[i] != 0) {
                    memory_buffer[buffer_n] = chip8->memory[i];
                    addresses_buffer[buffer_n] = i;
                    buffer_n++;
                }
            }

            

            printf("## REGISTERS ##\t\t\t\t\t## DISPLAY ##\n");

            for (int i = 0; i < sizeof(chip8->registers); i++) {
                printf("| V%d\t | %d\t |\n", i, chip8->registers[i]);
            }
            printf("| PC\t | %d\t |\n", chip8->PC);
            printf("| I\t | %d\t |\n", chip8->I);

            for (int i = 0; i < buffer_n; i++) {
                // printf("%d\t| %d\t|\n", addresses_buffer[i], memory_buffer[i]);
            }

            printf("Instruction: %04x\n", instruction);
            printf("Opcode: %01x\n", opcode);
            printf("x: %d\n", x);
            printf("y: %d\n", y);

            // scanf("%s");
        }
    }
}

void chip8_draw_sprite(Chip8 chip8, int x, int y, int n) {
    uint8_t sprite_row = 0;
    int sprite, mask, old_pixel;

    for (int i = 0; i < n; i++) {
        sprite = chip8->memory[chip8->I + i];
        for (int j = 0; j < 8; j++) {
            if (sprite & 0x80) {
                old_pixel = chip8->display[(x+j)+(y*DISPLAY_DIM_X)];

                if (old_pixel == 1) {
                    chip8->VF = 1;
                }

                chip8->display[(x+j)+(y*DISPLAY_DIM_X)] = 1;
            }
            sprite = sprite << 1;
        }
        printf("\n");
        y++;
    }
}