#include <iostream>
#include <SDL2/SDL.h>
// #include <glad/glad.h>
#include "chip8.hpp"
#include <thread>
#include <unistd.h>

// clang++ main.cpp ./glad/src/glad.c -I/Library/Frameworks/SDL2.framework/Headers -I./glad/include -F/Library/Frameworks -framework SDL2

unsigned short keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int main(int argc, char* argv[])
{
    if (argc <= 1){
        exit(1); 
    }

    chip8 cpu; 
    if (!cpu.load_file(argv[1])){
        exit(1); 
    }

    SDL_Window *window; 
    SDL_Renderer *renderer;
    SDL_Texture *texture; 

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0){
        SDL_Quit(); 
        exit(1); 
    }

    window = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN); 
    if (window == nullptr){
        SDL_Quit(); 
        exit(1); 
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr){
        SDL_Quit(); 
        exit(1); 
    }

    SDL_RenderSetLogicalSize(renderer, 640, 320); 
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (texture == nullptr)
    {
        SDL_Quit();
        exit(1);
    }

    while (true)
    {
        cpu.emulate_cycle(); 
        SDL_Event event; 
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                exit(0); 
            }
            if (event.type == SDL_KEYDOWN){
                if (event.key.keysym.sym == SDLK_ESCAPE){
                    exit(0); 
                }
                for (int i = 0; i < 16; i++){
                    if (event.key.keysym.sym == keymap[i]){
                        cpu.keypad[i] = 1; 
                    }
                }
            }

            if (event.type == SDL_KEYUP){
                for (int i = 0; i < 16; i++){
                    if (event.key.keysym.sym == keymap[i]){
                        cpu.keypad[i] = 0; 
                    }
                }
            }
        }
        if (cpu.draw_flag){
            cpu.draw_flag = false; 
            uint32_t pixels[32 * 64]; 
            for (int i = 0; i < 2048; i++){
                if (cpu.video[i] == 0){
                    pixels[i] = 0xFF000000; 
                } else 
                {
                    pixels[i] = 0xFFFFFFFF; 
                }

            }
            SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t)); 
            SDL_RenderClear(renderer); 
            SDL_RenderCopy(renderer, texture, NULL, NULL); 
            SDL_RenderPresent(renderer); 
        }

        usleep(1600); 
    }
}