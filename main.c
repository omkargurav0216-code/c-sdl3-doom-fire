#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define WIDTH 320
#define HEIGHT 240

// DOOM Fire state variables
#define FIRE_W WIDTH
#define FIRE_H HEIGHT
#define FIRE_PALETTE_SIZE 37

// Intensity of every fire pixel
static uint8_t fire_pixels[FIRE_W * FIRE_H];

// Classic 37-color fire palette
static const uint32_t fire_palette[FIRE_PALETTE_SIZE] = {
    0x070707, 0x1F0707, 0x2F0F07, 0x470F07, 0x571707, 0x671F07, 0x771F07,
    0x8F2707, 0x9F2F07, 0xAF3F07, 0xBF4707, 0xC74707, 0xDF4F07, 0xDF5707,
    0xDF5707, 0xD75F07, 0xD75F07, 0xD7670F, 0xCF6F0F, 0xCF770F, 0xCF7F0F,
    0xCF8717, 0xC78717, 0xC78F17, 0xC7971F, 0xBF9F1F, 0xBF9F1F, 0xBFA727,
    0xBFA727, 0xBFAF2F, 0xB7AF2F, 0xB7B72F, 0xB7B737, 0xCFCF6F, 0xDFDF9F,
    0xEFEFC7, 0xFFFFFF};

// 2D image is stored as a single contiguous 1D array
static uint32_t framebuffer[WIDTH * HEIGHT];

void put_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
    {
        return;
    }
    // convert x and y into framebuffer array index
    framebuffer[WIDTH * y + x] = color;
}

void clear(uint32_t color)
{
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        framebuffer[i] = color;
    }
}

int main(void)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;

    const double target_frame = 1.0 / 60.0; // --> 0.016

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL Init failed: %s \n", SDL_GetError());
        return EXIT_FAILURE;
    }

    window = SDL_CreateWindow("SDL Framebuffer", WIDTH * 3, HEIGHT * 3, 0);

    if (window == NULL)
    {
        fprintf(stderr, "SDL CreateWindow failed: %s \n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    renderer = SDL_CreateRenderer(window, NULL);

    if (renderer == NULL)
    {
        fprintf(stderr, "SDL CreateRenderer failed: %s \n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    if (texture == NULL)
    {
        fprintf(stderr, "SDL CreateTexture failed: %s \n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // nearest-neighbor scaling to preserve the pixelated appearance
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    uint8_t is_running = 1;
    uint32_t frame = 1;

    // Fire Init
    for (int i = 0; i < FIRE_W * FIRE_H; i++)
    {
        fire_pixels[i] = 0;
    }
    // Seed the bottom row with max intensity, --> fire's source
    for (int i = 0; i < FIRE_W; i++)
    {
        fire_pixels[(FIRE_H - 1) * FIRE_W + i] = FIRE_PALETTE_SIZE - 1;
    }

    while (is_running)
    {
        uint64_t start = SDL_GetPerformanceCounter();

        // Poll the events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                is_running = 0;
            }
        }

        clear(0x000000);

        // Fire Update
        for (uint32_t x = 0; x < FIRE_W; x++)
        {
            for (uint32_t y = 1; y < FIRE_H; y++)
            {
                uint32_t src = y * FIRE_W + x;

                uint32_t rand_idx = rand() % 4;

                int32_t dst_x = (int32_t)x - (int32_t)rand_idx + 1;

                if (dst_x < 0 || dst_x >= FIRE_W)
                {
                    continue;
                }

                uint32_t dst = (y - 1) * FIRE_W + dst_x;

                uint8_t src_val = fire_pixels[src];

                uint8_t decay = rand_idx & 1;

                fire_pixels[dst] =
                    (src_val > decay) ? (src_val - decay) : 0;
            }
        }
        // Fire Render
        for (uint32_t y = 0; y < FIRE_H; y++)
        {
            for (uint32_t x = 0; x < FIRE_W; x++)
            {
                uint8_t idx = fire_pixels[y * FIRE_W + x];

                // Manipulate framebuffer array
                put_pixel(x, y, fire_palette[idx]);
            }
        }

        // // Hz lines
        // for (int y = 0; y < HEIGHT; y++)
        // {
        //     for (int x = 0; x < WIDTH; x++)
        //     {
        //         if (x % 5 == 0)
        //         {
        //             put_pixel(x, y, 0x00FFFF);
        //         }
        //     }
        // }

        // // animation
        // int x = frame % 320;
        // int y = HEIGHT / 2;
        // put_pixel(x, y, 0x00FF00);

        // Copy the contents of the framebuffer to the texture
        // Pitch: number of bytes occupied by 1 row of pixels. (Width of the image/ texture in bytes)
        SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t));

        // Display the window & renderer
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        uint64_t end = SDL_GetPerformanceCounter();

        double elapsed = (double)(end - start) / (double)SDL_GetPerformanceFrequency();

        // Cap the framerate to 60 FPS
        if (elapsed < target_frame)
        {
            SDL_Delay((target_frame - elapsed) * 1000.0); // Sleep until target for 60 FPS is reached
        }

        frame += 1;
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}