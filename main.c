#include <stdio.h>
#include <stdlib.h>
#include "chip-8.h"

int main(int argc, void** argv) {
    if (argc <= 1) {
        printf("Please provide the program path\n");
        exit(EXIT_FAILURE);
    }
    
    Chip8 chip8 = chip8_new();
    chip8_load_program(chip8, argv[1]);
    chip8_run(chip8, true);

    exit(EXIT_SUCCESS);
}