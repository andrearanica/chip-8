#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>
#include "chip-8.h"

Chip8 chip8;

void loop();

int main(int argc, char** argv) {
    chip8 = chip8_new();
    bool show_debug = false;

    chip8_load_program(chip8, "./chip8-roms/games/Pong (1 player).ch8");
    
    emscripten_set_main_loop(loop, 0, 1);

    exit(EXIT_SUCCESS);
}

void loop() {
    chip8_run_instruction(chip8, false);
    exit(0);
}