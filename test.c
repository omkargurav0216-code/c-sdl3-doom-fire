#include <SDL3/SDL.h>
#include <stdio.h>

int main(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    printf("SDL3 initialized successfully!\n");

    SDL_Quit();

    return 0;
}