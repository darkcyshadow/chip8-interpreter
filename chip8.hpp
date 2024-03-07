#ifndef chip8_h
#define chip8_h

#include <cstdlib>
#include <ctime> 


class chip8
{
private:
    // memory size of 4k, char is 1 byte
    unsigned char memory[4096];
    // current opcode, size of each opcode is 2 bytes
    unsigned short opcode;
    // registers v0-vf, each register is 8 bits
    unsigned short V[16];
    // index register
    unsigned short I;
    // program counter
    unsigned short pc;

     // delay timer
    unsigned char delay_timer;
    // sound timer
    unsigned char sound_timer;
    // 16-level stack
    unsigned short stack[16];
    // stack pointer
    unsigned short sp;

    //const int mem_start = 0x200;

    /* memory map:
    0x000-0x1FF - chip 8 interpreter/font set
    0x050-0x0A0 - used for the built in 4x5 pixel font set (0-F)
    0x200-0xFFF - program ROM and work RAM
    */

    void op_NULL();

    void op_1NNN();
    void op_2NNN();
    void op_3xkk();
    void op_4xkk();
    void op_5xy0();
    void op_6xkk();
    void op_7xkk();
    void op_9xy0();
    void op_Annn();
    void op_Bnnn();
    void op_Cxkk();
    void op_Dxyn();

    void op_8xy0();
    void op_8xy1();
    void op_8xy2();
    void op_8xy3();
    void op_8xy4();
    void op_8xy5();
    void op_8xy6();
    void op_8xy7();
    void op_8xye();

    void op_00EE();
    void op_00E0();

    void op_Ex9E();
    void op_ExA1();

    void op_Fx07();
    void op_Fx0A();
    void op_Fx15();
    void op_Fx18();
    void op_Fx1E();
    void op_Fx29();
    void op_Fx33();
    void op_Fx55();
    void op_Fx65();
    void Table0();
    void Table8();
    void TableE();
    void TableF();

    // using chip8_func = void (chip8::*)();
    typedef void (chip8::*chip8_func)();

    chip8_func table[0xF + 1];
    chip8_func table0[0xE + 1];
    chip8_func table8[0xE + 1];
    chip8_func tableE[0xE + 1];
    chip8_func tableF[0x65 + 1];

    /*nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
    n or nibble - A 4-bit value, the lowest 4 bits of the instruction
    x - A 4-bit value, the lower 4 bits of the high byte of the instruction
    y - A 4-bit value, the upper 4 bits of the low byte of the instruction
    kk or byte - An 8-bit value, the lowest 8 bits of the instruction*/

    // defined chip8_func as a type for pointers to member functions of chip8 that take in no args and return void

    //

    // take note of the opcodes, the first nibble goes from 0 - F, and we can use the first nibble to index into the correct opcode table
    // ex) op_Fx07 & 0xF000U returns 0xF000, and shift it 12 bits to the right '>> 12u' we get 0xF, indexing into tableF
    // further, we can extract the lowest bits by taking Fx07 & 0x000Fu, resulting in 0x7

public:
    // video screen
    unsigned int video[62 * 34];
    unsigned short keypad[16]; 
    bool draw_flag; 
   

    chip8();

    void emulate_cycle();
    bool load_file(char const *filename);
    void decrement_timers(); 
    

   
};
#endif