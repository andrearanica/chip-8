# Chip 8

C emulator that executes Chip 8 ROMs.

## Introduction

Chip8 is a programming languages that provides a simple instruction set to make simple video games.  
This emulator is written in C and is able to run Chip8 programs, managing I/O operations with the keyboard and output on a 64x32 display.

## Installation

First, make sure you have installed SDL2 running the following command.
``` bash
sudo apt-get install libsdl2-2.0-0
```

After that, you can compile the source code with your favourite compiler.
``` bash
gcc chip-8.h chip-8.h chip-8.c -o chip-8
```

Finally, you can run the emulator calling the executable file and passing the path to the ROM you want to play.
``` bash
./chip-8 ./roms/PATH_TO_ROM
```

## Credits

I took inspiration and ROMS from [https://github.com/Klairm/chip8](https://github.com/Klairm/chip8)