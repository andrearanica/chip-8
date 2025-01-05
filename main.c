#include <stdio.h>
#include <stdlib.h>
#include "chip-8.h"

int main(int argc, void** argv) {
    Chip8 chip8 = chip8_new();
    
    if (argc == 2) {
        // TODO show roms menu and load rom
    } else if (argc == 2) {
        chip8_load_program(chip8, argv[1]);
    }
    
    chip8_run(chip8, true);

    exit(EXIT_SUCCESS);
}