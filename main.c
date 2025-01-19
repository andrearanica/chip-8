#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chip-8.h"

int main(int argc, char** argv) {
    Chip8 chip8 = chip8_new();
    bool show_debug = false;

    if (argc == 1) {
        // TODO show roms menu and load rom
    } else if (argc > 1) {
        chip8_load_program(chip8, "./chip8-roms/games/Pong (1 player).ch8");
        show_debug = (argv[2] != NULL) && (strstr(argv[2], "debug") != NULL);
    }
    
    chip8_run(chip8, show_debug);

    exit(EXIT_SUCCESS);
}