#include "chip8.hpp"
#include <algorithm>
#include <fstream>
#include <random>
#include <cstring>
#include <cstdint>
#include <ctime>
#include <iostream>

std::ofstream log_file;

unsigned char chip8_fontset[80] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

chip8::chip8()
{
  log_file.open("chip8_instruction_log.txt");

  // seed rng
  srand(time(0));

  // set pc to 0x200, reset opode, index register, and stack pointer
  pc = 0x200;
  opcode = 0;
  I = 0;
  sp = 0;
  draw_flag = false;

  delay_timer = 0;
  sound_timer = 0;

  // clear memory, registers, stack, display
  std::fill(std::begin(memory), std::end(memory), 0);
  std::fill(std::begin(V), std::end(V), 0);
  std::fill(std::begin(stack), std::end(stack), 0);
  std::fill(std::begin(video), std::end(video), 0);

  // load fonts into memory
  for (int i = 0; i < 80; i++)
  {
    memory[i] = chip8_fontset[i];
  }

  table[0x0] = &chip8::Table0;
  table[0x1] = &chip8::op_1NNN;
  table[0x2] = &chip8::op_2NNN;
  table[0x3] = &chip8::op_3xkk;
  table[0x4] = &chip8::op_4xkk;
  table[0x5] = &chip8::op_5xy0;
  table[0x6] = &chip8::op_6xkk;
  table[0x7] = &chip8::op_7xkk;
  table[0x8] = &chip8::Table8;
  table[0x9] = &chip8::op_9xy0;
  table[0xA] = &chip8::op_Annn;
  table[0xB] = &chip8::op_Bnnn;
  table[0xC] = &chip8::op_Cxkk;
  table[0xD] = &chip8::op_Dxyn;
  table[0xE] = &chip8::TableE;
  table[0xF] = &chip8::TableF;

  std::fill(std::begin(table0), std::end(table0), &chip8::op_NULL);
  std::fill(std::begin(table8), std::end(table8), &chip8::op_NULL);
  std::fill(std::begin(tableE), std::end(tableE), &chip8::op_NULL);
  std::fill(std::begin(tableF), std::end(tableF), &chip8::op_NULL);

  table0[0x0] = &chip8::op_00E0;
  table0[0xE] = &chip8::op_00EE;

  table8[0x0] = &chip8::op_8xy0;
  table8[0x1] = &chip8::op_8xy1;
  table8[0x2] = &chip8::op_8xy2;
  table8[0x3] = &chip8::op_8xy3;
  table8[0x4] = &chip8::op_8xy4;
  table8[0x5] = &chip8::op_8xy5;
  table8[0x6] = &chip8::op_8xy6;
  table8[0x7] = &chip8::op_8xy7;
  table8[0xE] = &chip8::op_8xye;

  tableE[0x1] = &chip8::op_ExA1;
  tableE[0xE] = &chip8::op_Ex9E;

  tableF[0x7] = &chip8::op_Fx07;
  tableF[0xA] = &chip8::op_Fx0A;
  tableF[0x15] = &chip8::op_Fx15;
  tableF[0x18] = &chip8::op_Fx18;
  tableF[0x1E] = &chip8::op_Fx1E;
  tableF[0x29] = &chip8::op_Fx29;
  tableF[0x33] = &chip8::op_Fx33;
  tableF[0x55] = &chip8::op_Fx55;
  tableF[0x65] = &chip8::op_Fx65;
}

void chip8::decrement_timers()
{
  if (delay_timer > 0)
  {
    --delay_timer;
  }
}



bool chip8::load_file(char const *filename)
{

  // creates an input filestream object, opens the file in binary mode, which reads the data in its binary form with translation to preserve exact format
  // std::ios::ate (at end), sets the file pointer at the end of the file, useful to find the size of the file, since the file pointer will be at the end immediatley upon opening
  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  if (!file.is_open())
  {
    return false;
  }

  std::streampos size = file.tellg();
  size_t memory_size = sizeof(memory) - 0x200;
  if ((size) > memory_size)
  {
    return false;
  }

  file.seekg(0, std::ios::beg);
  file.read((char *)memory + 0x200, size);
  file.close();

  return true;
}

void chip8::emulate_cycle()
{

  // get the current instruction from memory, shift left 8 bits, to combine with the next half of the instruction
  opcode = (memory[pc] << 8u | memory[pc + 1]);
  if (log_file.is_open())
  {
    log_file << "PC: " << std::hex << pc << ", "
             << "opcode: " << opcode << std::endl;
  }
  pc += 2;
  /* get first nibble from the opcode, and use it to index into the correct tbale array
  table array contains pointers to member functions of the chip8 class,
  we use (*this).* to dereference a pointer to a member function of a class, 'this' is a pointer to the current instance of the chip8 class,
  '*' derefernces the pointer to the current instance of the class, resulting in the object itself, while the second "*" dereferences the pointer to the member function
  after dereferencing the pointer to the member function, the final set of parenthesis calls the memebr function with no args, as given by implementation*/
  ((*this).*(table[(opcode & 0xF000) >> 12]))();

  // decrement_timers();
}

void chip8::Table0()
{
  ((*this).*(table0[opcode & 0x000F]))();
}

void chip8::Table8()
{
  ((*this).*(table8[opcode & 0x000F]))();
}

void chip8::TableE()
{
  ((*this).*(tableE[opcode & 0x000F]))();
}

void chip8::TableF()
{
  ((*this).*(tableF[opcode & 0x00FF]))();
}

// clear the display
void chip8::op_00E0()
{
  for (int i = 0; i < 2048; i++)
  {
    video[i] = 0;
  }
  draw_flag = true;
}

// return from subroutine
// set the pc to address at top of stack, subtract 1 from sp
void chip8::op_00EE()
{
  --sp;
  pc = stack[sp];
}

// set the pc to nnn
void chip8::op_1NNN()
{
  uint16_t address = opcode & 0x0FFF;
  pc = address;
}

// call subroutine at nnn, increments the stack pointer, puts current pc on top of stack, then sets pc to nnnn
void chip8::op_2NNN()
{

  stack[sp] = pc;
  ++sp;
  uint16_t address = opcode & 0x0FFF;
  pc = address;
}

// skips next instruction if Vx = kk, compares register Vx to kk, if equal, increment pc by 2 to skip next instruction
void chip8::op_3xkk()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t kk = opcode & 0x00FF;

  if (V[vx] == kk)
  {
    pc += 2;
  }
}

// skips next instruction if Vx != kk
void chip8::op_4xkk()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t kk = opcode & 0x00FF;

  if (V[vx] != kk)
  {
    pc += 2;
  }
}

// skips next instruction if registed Vx == Vy
void chip8::op_5xy0()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t vy = (opcode & 0x00F0) >> 4;

  if (V[vx] == V[vy])
  {
    pc += 2;
  }
}

// ld vx, byte, puts value kk inside register vx
void chip8::op_6xkk()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t kk = opcode & 0x00FF;

  V[vx] = kk;
}

// add vx, byte - adds value kk to register vx, stores result in vx
void chip8::op_7xkk()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t kk = opcode & 0x00FF;

  V[vx] += kk;
}

// ld vx, vy - sets register vx with value in register vy
void chip8::op_8xy0()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t vy = (opcode & 0x00F0) >> 4;

  V[vx] = V[vy];
}

// or vx, vy - stores value of bitwise OR vx, vy in register vx
void chip8::op_8xy1()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t vy = (opcode & 0x00F0) >> 4;

  V[vx] |= V[vy];
}

// and vx, vy - stores value of bitwise AND vx, vy in register vx
void chip8::op_8xy2()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t vy = (opcode & 0x00F0) >> 4;

  V[vx] &= V[vy];
}

// xor vx, vy - stores value of bitwise XOR vx, vy, in register vx
void chip8::op_8xy3()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t vy = (opcode & 0x00F0) >> 4;

  V[vx] ^= V[vy];
}

// add vx, vy - set vx = vx + vy, set vf = carry, the values of vx and vy are added together, if result is greater than 8 bits, VF is set to 1, otherwise 0
void chip8::op_8xy4()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t vy = (opcode & 0x00F0) >> 4;
  uint16_t sum = V[vx] + V[vy];
  uint8_t temp = 0; 
  if (sum > 255)
  {
    temp = 1;
  }
  else
  {
    temp = 0;
  }
  V[vx] = sum & 0xFF;
  V[0xF] = temp; 
}

// sub vx, vy - set vx = vx - vy, if vx > vy, vf set to 1, otherwise 0, then vy subtracted from vx and result stored in vx;
void chip8::op_8xy5()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t vy = (opcode & 0x00F0) >> 4;
  uint16_t diff = V[vx] - V[vy];
  uint8_t temp = 0;
  if (V[vx] >= V[vy])
  {
    temp = 1;
  }
  else
  {
    temp = 0;
  }
  V[vx] = diff;
  V[0xF] = temp;
}

// shr vx {, vy} - if the least significant bit of vx is 1, then vf is set to 1, otherwise 0, then vx divided by 2
void chip8::op_8xy6()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t temp;
  if ((V[vx] & 0x1) == 1)
  {
    temp = 1;
  }
  else
  {
    temp = 0;
  }
  V[vx] /= 2;
  V[0xF] = temp;
}

// subn vx, vy - set vx = vy - vx, if vy > vx, vf set to 1, otherwise 0, then store diff in vx
void chip8::op_8xy7()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t vy = (opcode & 0x00F0) >> 4;
  uint8_t diff = V[vy] - V[vx];
  uint8_t temp = 0;
  if (V[vy] >= V[vx])
  {
    temp = 1;
  }
  else
  {
    temp = 0;
  }
  V[vx] = diff;
  V[0xF] = temp;
}

// shl vx, {. vy} - if most significant bit of vx is 1, then vf is set to 1, otherwise 0, then vx *= 2
void chip8::op_8xye()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t temp = (V[vx] & 0x80) >> 7;
  V[vx] <<= 1;
  V[0xF] = temp;
}

// sne vx, vy - skip next instruction if vx != vy
void chip8::op_9xy0()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t vy = (opcode & 0x00F0) >> 4;
  if (V[vx] != V[vy])
  {
    pc += 2;
  }
}

// ld i, addr, the value of resiter I is set to nnn
void chip8::op_Annn()
{
  uint16_t address = opcode & 0x0FFF;
  I = address;
}

// jp v0, addr - the program counter is set to nnn plus the value of v0
void chip8::op_Bnnn()
{
  uint16_t address = opcode & 0x0FFF;
  pc = address + V[0x0];
}

// rnd vx, byte - generate a random number from 0-255, AND with value kk, and store in register vx
void chip8::op_Cxkk()
{
  uint8_t random_number = static_cast<unsigned short>(rand() % 256);
  uint8_t kk = opcode & 0x00FF;
  uint8_t vx = (opcode & 0x0F00) >> 8;
  V[vx] = kk & random_number;
}

// drw vx, vy, nibble - display n byte sprite starting at memory location I at (Vx, vy), set vf = collision
/* the interpreter reads n bytes from memory, starting at the address sotres in I, these bytes are then displayed as sptires on screen at coordinates (vx, vy)
the sprites are XORed onto the existing screen, if this causes any pixels to be erased, vf is set to 1, otherwise 0, if the sprite is positioned to part of it
is outisde the coordinates of the display, it wraps around to the opposide side of the screen
width of 8 pixels, and height of N pixels, */
void chip8::op_Dxyn()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t vy = (opcode & 0x00F0) >> 4;
  uint8_t byte = opcode & 0x000F;

  uint8_t x_pos = V[vx] % 64;
  uint8_t y_pos = V[vy] % 32;

  V[0xF] = 0;

  for (uint8_t row = 0; row < byte; row++)
  {
    uint8_t sprite_byte = memory[I + row];
    for (uint8_t col = 0; col < 8; col++)
    {
      // checks each byte in the sprite to see whether it is on, shift by col from 0 - 8 as they are stored left to right, and we want to check every col

      uint8_t sprite_pixel = sprite_byte & (0x80u >> col);
      // stores address of the screen pixel to be drawn, multiply
      uint32_t *screen_pixel = &video[(y_pos + row) * 64 + (x_pos + col)];

      if (sprite_pixel)
      {
        if (*screen_pixel == 1)
        {
          V[0xF] = 1;
        }
        *screen_pixel ^= 1;
      }
    }
  }
  draw_flag = true;
}

// skp vx - skips next instruction if key with the value of vx is pressed
void chip8::op_Ex9E()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  if (keypad[V[vx] & 0xF])
  {
    pc += 2;
  }
}

// sknp vx - skips next instrucion if key with value of vx is not pressed
void chip8::op_ExA1()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  if (!keypad[V[vx] & 0xF])
  {
    pc += 2;
  }
}

// ld vx, dt - value of delay timer is placed into vx
void chip8::op_Fx07()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  V[vx] = delay_timer;
}

// ld vx, k - wait for a key press store the value of key in vx, all execution stops until a key is pressed, then the value of that key is stored in vx
void chip8::op_Fx0A()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  bool key_press = false;
  for (int i = 0; i < 16; ++i)
  {
    if (keypad[i] == 1)
    {
      V[vx] = i;
      key_press = true;
    }
  }
  if (!key_press)
  {
    pc -= 2; // decrement pc to repeat this instruction until a key is pressed
  }
}

// ld dt, vx - set delay timer with value of vx
void chip8::op_Fx15()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  delay_timer = V[vx];
}

// ld st, vx - set sound timer to value of vx
void chip8::op_Fx18()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  sound_timer = V[vx];
}

// add i, vx - values of I and vx are added and stored in I
void chip8::op_Fx1E()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  I += V[vx];
}

// ld f, vx - value of I is set to the location for the hexadeciaml sprite corresponding to the value of vx
void chip8::op_Fx29()
{
  uint8_t vx = opcode & 0x0F00 >> 8;
  I = memory[(V[vx] * 5) & 0xF];
}

// ld b, vx - interpreter takes decimal vaue of vx, places 100's digit at memory location I, 10's digit at I + 1, 1's digit at I + 2
void chip8::op_Fx33() {
  uint8_t vx = (opcode & 0x0F00) >> 8;
  uint8_t value = V[vx];

  memory[I + 2] = value % 10;
  value /= 10;

  memory[I + 1] = value % 10;
  value /= 10;

  memory[I] = value % 10;
}


// ld I, vx - stores registers v0-vx in memory starting at location I
void chip8::op_Fx55()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  for (int i = 0; i <= vx; i++)
  {
    memory[I++] = V[i];
  }
}

// ld vx, I - reads registers v0-vx from memory starting at location I
void chip8::op_Fx65()
{
  uint8_t vx = (opcode & 0x0F00) >> 8;
  for (int i = 0; i <= vx; i++)
  {
    V[i] = memory[I++];
  }
}

void chip8::op_NULL() {}
