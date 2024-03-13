# CHIP-8 Emulator

## Overview

This project is a CHIP-8 emulator, written in C++, designed to run CHIP-8 games and applications. CHIP-8 is an interpreted programming language developed in the mid-1970s for use on 8-bit microcomputers. This emulator replicates the functionality of the CHIP-8 system, allowing users to load and execute CHIP-8 programs on modern hardware.

## Features

- Full emulation of the CHIP-8 instruction set
- Support for loading and running CHIP-8 programs (.ch8 files)
- Graphical rendering using SDL2
- Keyboard mapping for CHIP-8 hex keypad

## Building the Project

The project uses SDL2 for rendering and input handling. Ensure you have SDL2 installed on your system before compiling.

### Dependencies

- SDL2
- C++ compiler (g++, clang++, etc.)

### Compilation

Use the following command to compile the project (assuming `g++` is your compiler, or clang on macos):
```
linux : g++ -std=c++11 main.cpp chip8.cpp -lSDL2 -o chip8_emulator

macos:  clang++ -std=c++11 main.cpp chip8.cpp -I/Library/Frameworks/SDL2.framework/Headers -F/Library/Frameworks -framework SDL2
```

## Running the Emulator

To run the emulator, use the following command, replacing `[ROM_FILE]` with the path to your CHIP-8 ROM file:
```
./a.out [ROM_FILE] // replace with path to rom, for example if in current directory, ./a.out pong2.ch
```

## Keyboard Mapping

The original CHIP-8 used a 16-key hexadecimal keypad. This emulator maps those keys to the following keys on a standard QWERTY 

| Original | Emulated |
|----------|----------|
| 1 2 3 C  | 1 2 3 4  |
| 4 5 6 D  | Q W E R  |
| 7 8 9 E  | A S D F  |
| A 0 B F  | Z X C V  |


## Configuration

Currently, the emulator does not support external configuration files. All configurations, such as emulation speed and key mappings, must be done within the source code.

## Contributing

Contributions to this project are welcome! Please feel free to fork the repository, make your changes, and submit a pull request.

## License

This project is open-source and available under the MIT License. See the LICENSE file for more details.





