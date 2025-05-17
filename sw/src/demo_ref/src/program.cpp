#include <SDL.h>
#include <stdbool.h>

extern "C" {
#include "sw_rasterizer.h"
#include "graphite.h"
}

#include <sim.h>

static int screen_width = 320;
static int screen_height = 240;
static int screen_scale = 3;

static SDL_Renderer* renderer;

bool g_rasterizer_barycentric = true;

void draw_pixel(int x, int y, int color) {

    // Constants taken from https://stackoverflow.com/a/9069480

    int r5 = color >> 11;
    int g6 = (color >> 5) & 0x3F;
    int b5 = color & 0x1F;

    int r = (r5 * 527 + 23) >> 6;
    int g = (g6 * 259 + 33) >> 6;
    int b = (b5 * 527 + 23) >> 6;

    SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPoint(renderer, x, y);
}

int main() {
    sw_init_rasterizer_standard(screen_width, screen_height, draw_pixel);
    sw_init_rasterizer_barycentric(screen_width, screen_height, draw_pixel);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window =
        SDL_CreateWindow("XGDemo Reference Implementation", SDL_WINDOWPOS_CENTERED_DISPLAY(1),
                         SDL_WINDOWPOS_UNDEFINED, screen_width * screen_scale, screen_height * screen_scale, 0);

    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_RenderSetScale(renderer, (float)screen_scale, (float)screen_scale);

    graphite_init(renderer, screen_width, screen_height);

    sim_run();

    SDL_DestroyWindow(window);
    SDL_Quit();

    sw_dispose_rasterizer_barycentric();
    sw_dispose_rasterizer_standard();

    return 0;
}
