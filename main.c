#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chip-8.h"

int main(int argc, void** argv) {
    Chip8 chip8 = chip8_new();
    bool show_debug = false;

    if (argc == 1) {
        // TODO show roms menu and load rom
    } else if (argc > 1) {
        chip8_load_program(chip8, argv[1]);
        show_debug = (argv[2] != NULL) && (strstr(argv[2], "debug") != NULL);
    }
    
    chip8_run(chip8, show_debug);

    exit(EXIT_SUCCESS);
}