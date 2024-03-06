#include <iostream>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include "chip8.hpp"
#include <thread> 
#include <chrono> 


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


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: chip8 <ROM file>" << std::endl; 
        return 1; 
    }

    chip8 cpu = chip8(); 

    int w = 1024; 
    int h = 512; 

    SDL_Window* window = NULL; 

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "SDL could not be initialized. SDL_ERRor: %s\n", SDL_GetError(); 
        exit(1); 
    }
    
    window = SDL_CreateWindow
    (
        "CHIP_8 Emulator", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        w, h, SDL_WINDOW_SHOWN
    );

    if (window == NULL)
    {
        std::cout << "SDL could not be initialized. SDL_ERRor: %s\n", SDL_GetError(); 
        exit(2); 
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0); 
    SDL_RenderSetLogicalSize(renderer, w, h); 

    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer, 
            SDL_PIXELFORMAT_ARGB8888, 
            SDL_TEXTUREACCESS_STREAMING,
            64, 32); 


    uint32_t pixels[2048]; 

    load_file: 
    if (!cpu.load_file(argv[1]))
    {
        return 2; 
    }    

    while (true)
    {
        cpu.emulate_cycle(); 
        SDL_Event e; 
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                exit(0); 
            }

            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    exit(0); 
                }

                if (e.key.keysym.sym == SDLK_F1)
                {
                    goto load_file; 
                }

                for (int i = 0; i < 16; ++i)
                {
                    if (e.key.keysym.sym == keymap[i])
                    {
                        cpu.keypad[i] = 1; 
                    }
                }
            }

            if (e.type == SDL_KEYUP)
            {
                for (int i = 0; i < 16; ++i)
                {
                    if (e.key.keysym.sym == keymap[i])
                    {
                        cpu.keypad[i] = 0; 
                    }
                }
            }
        }

        if (cpu.draw_flag)
        {
            cpu.draw_flag = false; 

            for (int i = 0; i < 2048; ++i)
            {
                uint8_t pixel = cpu.video[i]; 
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000; 
            }

            SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32)); 
            SDL_RenderClear(renderer); 
            SDL_RenderCopy(renderer, sdlTexture, NULL, NULL); 
            SDL_RenderPresent(renderer); 
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1200)); 
    }
}