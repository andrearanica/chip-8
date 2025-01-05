/* chip-8.c
* Implementation file for the 'chip-8' library
*/

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "chip-8.h"

#define BUFFER_DIM 256
#define TIMER_INTERVAL 1000 / 60 // 60 Hz

void chip8_draw_screen(Chip8, SDL_Renderer*);
void chip8_draw_sprite(Chip8, int, int, int);
void chip8_handle_keyboard(Chip8, SDL_Event*);

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

    int min_sprite_address = 0x50;
    for (int i = 0; i < sizeof(chip8_char_sprites); i++) {
        chip8->memory[min_sprite_address+i] = chip8_char_sprites[i];
    }
}

void chip8_run(Chip8 chip8, bool debug) {
    uint16_t instruction;
    int opcode, x, y, kk, nnn, n, result, value;
    uint8_t random_number;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        SDL_Log("Error: cannot initialize SDL");
        exit(EXIT_FAILURE);
    }

    SDL_Window* win = SDL_CreateWindow("Chip 8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 256, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, 64, 32);

    SDL_Event event;
    bool running = true;
    while(running) {
        if (chip8->delay_timer > 0) {
            chip8->delay_timer -= 1;
        }
        
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
                            chip8->registers[0xF] = 1;
                        } else {
                            chip8->registers[0xF] = 0;
                        }
                        break;
                    case 5:
                        if (chip8->registers[x] > chip8->registers[y]) {
                            chip8->registers[0xF] = 1;
                        } else {
                            chip8->registers[0xF] = 0;
                        }
                        chip8->registers[x] -= chip8->registers[y];
                        break;
                    case 6:
                        chip8->registers[x] >>= 1;
                        break;
                    case 7:
                        if (chip8->registers[y] > chip8->registers[x]) {
                            chip8->registers[0xF] = 1;
                        } else {
                            chip8->registers[0xF] = 0;
                        }
                        chip8->registers[x] = chip8->registers[y] - chip8->registers[x];
                        break;
                    case 0xE:
                        chip8->registers[x] <<= 1;
                        break;
                    default:
                        fprintf(stderr, "Error: instruction '%04X' not supported\n", instruction);
                        exit(EXIT_FAILURE);
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
            case 0xc:
                random_number = rand();
                chip8->registers[x] = random_number & kk;
                break;
            case 0xd:
                chip8_draw_sprite(chip8, chip8->registers[x], chip8->registers[y], n);
                break;
            case 0xe:
                switch(kk) {
                    case 0xA1:
                        value = chip8->registers[x];
                        if (chip8->keyboard[value] != 1) {
                            chip8->PC += 2;
                        }
                        break;
                    case 0x9E:
                        value = chip8->registers[x];
                        if (chip8->keyboard[value] == 1) {
                            chip8->PC += 2;
                        }
                        break;
                    default:
                        fprintf(stderr, "Error: instruction '%04X' not supported\n", instruction);
                        exit(EXIT_FAILURE);
                }
                break;
            case 0xf:
                switch (kk) {
                    case 0x07:
                        chip8->registers[x] = chip8->delay_timer;
                        break;
                    case 0x15:
                        chip8->delay_timer = chip8->registers[x];
                        break;
                    case 0x18:
                        chip8->sound_timer = chip8->registers[x];
                        break;
                    case 0x1E:
                        chip8->I += chip8->registers[x];
                        break;
                    case 0x33:
                        value = chip8->registers[x];
                        chip8->memory[chip8->I] = (value / 100) % 10;
                        chip8->memory[chip8->I+1] = (value / 10) % 10;
                        chip8->memory[chip8->I+2] = value % 10;
                        break;
                    case 0x29:
                        value = chip8->registers[x];
                        value = 0x50 + value;
                        chip8->I = value;
                        break;
                    case 0x55:
                        for (int i = 0; i <= x; i++) {
                            chip8->memory[chip8->I + i] = chip8->registers[i];
                        }
                        break;
                    case 0x65:
                        for (int i = 0; i <= x; i++) {
                            chip8->registers[i] = chip8->memory[chip8->I + i];
                        }
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

        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    chip8_handle_keyboard(chip8, &event);
                    break;
                case SDL_KEYUP:
                    chip8_handle_keyboard(chip8, &event);
                    break;
                case SDL_QUIT:
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(win);
                    SDL_Quit();
                    exit(EXIT_SUCCESS);
                    break;
                default:
                    break;
            }
        }

        if (chip8->sound_timer != 0) {
            // TODO beep
        }

        if (chip8->delay_timer > 0) {
            chip8->delay_timer -= 1;
        }
        chip8_draw_screen(chip8, renderer);
        SDL_Delay(1000 / 250);
    }
}

void chip8_draw_sprite(Chip8 chip8, int x, int y, int n) {
    uint8_t sprite_row = 0;
    int sprite, mask, old_pixel, screen_index;

    chip8->registers[0xF] = 0;

    for (int i = 0; i < n; i++) {
        sprite = chip8->memory[chip8->I + i];
        for (int j = 0; j < 8; j++) {
            screen_index = (x+j)+(y*DISPLAY_DIM_X);
            old_pixel = chip8->display[screen_index];
            if ((sprite & 0x80) != 0) {
                if (old_pixel == 1) {
                    chip8->registers[0xF] = 1;
                }
                chip8->display[screen_index] = 1 ^ old_pixel;
            }
            sprite = sprite << 1;
        }
        y++;
    }
}

void chip8_draw_screen(Chip8 chip8, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < DISPLAY_DIM_Y; i++) {
        for (int j = 0; j < DISPLAY_DIM_X; j++) {
            if (chip8->display[i * DISPLAY_DIM_X + j] == 1) {
                SDL_RenderDrawPoint(renderer, j, i);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void chip8_handle_keyboard(Chip8 chip8, SDL_Event* event) {
    bool key_down = (event->type == SDL_KEYDOWN);

    int address;
    switch(event->key.keysym.sym) {
        case SDLK_1:
            address = 0x1;
            break;
        case SDLK_2:
            address = 0x2;
            break;
        case SDLK_3:
            address = 0x3;
            break;
        case SDLK_4:
            address = 0xC;
            break;
        case SDLK_q:
            address = 0x4;
            break;
        case SDLK_w:
            address = 0x5;
            break;
        case SDLK_e:
            address = 0x6;
            break;
        case SDLK_r:
            address = 0xD;
            break;
        case SDLK_a:
            address = 0x7;
            break;
        case SDLK_s:
            address = 0x8;
            break;
        case SDLK_d:
            address = 0x9;
            break;
        case SDLK_f:
            address = 0xe;
            break;
        case SDLK_z:
            address = 0xA;
            break;
        case SDLK_x:
            address = 0x0;
            break;
        case SDLK_c:
            address = 0xB;
            break;
        case SDLK_v:
            address = 0xF;
            break;
    }

    chip8->keyboard[address] = key_down;
}