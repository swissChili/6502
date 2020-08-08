#pragma once

#include <stdint.h>

#ifndef SCREEN_ONLY_SDL

#undef NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear/nuklear.h"

void screen(struct nk_context *ctx, uint8_t *mem, uint8_t size);

#endif

#include <SDL2/SDL.h>


typedef struct
{
	SDL_Window *win;
	SDL_Renderer *r;
	uint8_t size;
} sdl_screen_t;

// draw the CPU screen
sdl_screen_t new_sdl_screen(uint8_t size);
void sdl_screen(sdl_screen_t *scr, uint8_t *mem);
