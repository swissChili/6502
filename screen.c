#include "screen.h"
#include "cpu.h"
#include "common.h"

#include <SDL2/SDL.h>
#include <pthread.h>


struct nk_color byte_to_color(uint8_t b)
{
	struct nk_color c;
	c.r = (b >> 5) * (255 / 0b111);
	c.g = ((b >> 2) & 0b111) * (255 / 0b111);
	c.b = (b & 0b11) * (255 / 0b11);
	c.a = 255;
	return c;
}

void screen(struct nk_context *ctx, uint8_t *mem, uint8_t size)
{
	struct nk_command_buffer *out = nk_window_get_canvas(ctx);

	struct nk_rect bounds;
	enum nk_widget_layout_states state = nk_widget(&bounds, ctx);

	if (!state)
		return;

	nk_fill_rect(out, bounds, 0, nk_rgb(0, 0, 0));


	for (int i = 0; i < CPU_FB_H; i++)
	{
		for (int j = 0; j < CPU_FB_W; j++)
		{
			if (mem[i + CPU_FB_H * j])
			{
				nk_fill_rect(out,
					nk_rect(bounds.x + i * size, bounds.y + j * size,
						size, size), 0.0f,
					byte_to_color(mem[i + CPU_FB_H * j]));
			}
		}
	}
}

sdl_screen_t new_sdl_screen(uint8_t size)
{
	sdl_screen_t scr;
	scr.win = SDL_CreateWindow("6502",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		size * 32,
		size * 32,
		0);
	ASSERT("Create Screen SDL_Window", scr.win);
	scr.size = size;
	scr.r = SDL_CreateRenderer(scr.win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	ASSERT("Create SDL_Renderer", scr.r);
	scr.tex = SDL_CreateTexture(scr.r, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING,
		CPU_FB_W, CPU_FB_H);
	ASSERT("Create SDL_Texture", scr.tex);

	return scr;
}

void free_sdl_screen(sdl_screen_t *scr)
{
	puts("Freed SDL Screen");
	SDL_DestroyTexture(scr->tex);
	SDL_DestroyRenderer(scr->r);
	SDL_DestroyWindow(scr->win);
}

bool sdl_screen(sdl_screen_t *scr, uint8_t *mem, bool dirty)
{
	static bool texture_set = false;
	if (!texture_set)
	{
		SDL_UpdateTexture(scr->tex, NULL, mem, CPU_FB_W);
	}

	SDL_Event e;

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT:
				return true;
		}
	}

	if (dirty)
	{
		SDL_RenderClear(scr->r);
		SDL_RenderCopy(scr->r, scr->tex, NULL, NULL);
		SDL_RenderPresent(scr->r);	
	}

	return false;
}

bool g_screen_thread_halt = false;

void *screen_thread(uint8_t *mem)
{
	sdl_screen_t scr = new_sdl_screen(8);
	
	pthread_cleanup_push(((void (*)(void *))&free_sdl_screen), ((void *)&scr));

	while (true)
	{
		if (sdl_screen(&scr, mem, true) || g_screen_thread_halt)
			break;
	}

	pthread_cleanup_pop(true);

	pthread_exit(NULL);
}

pthread_t start_screen_thread(uint8_t *mem)
{
	pthread_t thread;
	pthread_create(&thread, NULL, (void *(*)(void *))&screen_thread, mem);
	return thread;
}
