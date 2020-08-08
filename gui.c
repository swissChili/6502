#include "gui.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear/nuklear.h"
#include "nuklear/demo/sdl_opengl3/nuklear_sdl_gl3.h"
#include "screen.h"

#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 640
#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

void gui(cpu_t *cpu)
{
	SDL_Window *win;
	SDL_GLContext glContext;
	int win_width, win_height;
	bool running = true;
	bool cpu_running = false;

	struct nk_context *ctx;
	struct nk_colorf bg =
	{
		.r = 0.29f,
		.g = 0.28f,
		.b = 0.50f,
		.a = 1.0f,
	};
	struct nk_color selected =
	{
		.r = 28,
		.g = 234,
		.b = 79,
		.a = 255,
	};

	uint16_t disas_start = 0x600,
		disas_end = 0x600 + 32;

	uint8_t screen_scale = 4;

	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS);
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	win = SDL_CreateWindow("6502",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);
	glContext = SDL_GL_CreateContext(win);
	SDL_GetWindowSize(win, &win_width, &win_height);

	/* OpenGL setup */
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to setup GLEW\n");
		exit(1);
	}

	ctx = nk_sdl_init(win);

	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	struct nk_font *font = nk_font_atlas_add_default(atlas, 16, NULL);
	nk_sdl_font_stash_end();
	//nk_style_load_all_cursors(ctx, atlas->cursors);
	nk_style_set_font(ctx, &font->handle);

	while (running)
	{
		SDL_Event evt;
		nk_input_begin(ctx);
		while (SDL_PollEvent(&evt))
		{
			if (evt.type == SDL_QUIT) goto cleanup;
			nk_sdl_handle_event(&evt);
		}
		nk_input_end(ctx);

		if (cpu_running && cpu->running)
			step(cpu);

		if (!cpu->running)
			cpu_running = false;

		if (nk_begin(ctx, "Registers", nk_rect(50, 300, 500, 90),
			NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
			NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
		{
			nk_layout_row_dynamic(ctx, 30, 4);
			cpu->pc = nk_propertyi(ctx, "PC", 0, cpu->pc, 0xFFFF, 1, 20.0f);
			cpu->regs[A] = nk_propertyi(ctx, "A", 0, cpu->regs[A], 0xFF, 1, 20.0f);
			cpu->regs[X] = nk_propertyi(ctx, "X", 0, cpu->regs[X], 0xFF, 1, 20.0f);
			cpu->regs[Y] = nk_propertyi(ctx, "Y", 0, cpu->regs[Y], 0xFF, 1, 20.0f);
		}
		nk_end(ctx);

		if (nk_begin(ctx, "Screen", nk_rect(50, 400, 150, 220),
			NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
			NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
		{
			nk_layout_row_dynamic(ctx, 24, 1);
			screen_scale = nk_propertyi(ctx, "Scale", 1, screen_scale, 8, 1, 1);
			
			nk_layout_row_static(ctx, screen_scale * 32, screen_scale * 32, 1);
			screen(ctx, cpu->mem + CPU_FB_ADDR, screen_scale);
		}
		nk_end(ctx);

		if (nk_begin(ctx, "Disassembler", nk_rect(330, 50, 300, 200),
			NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
			NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
		{
			nk_layout_row_dynamic(ctx, 30, 2);
			disas_start = nk_propertyi(ctx, "Start", 0, disas_start, 0xFFFF, 1, 20.0f);
			disas_end = nk_propertyi(ctx, "End", 0, disas_end, 0xFFFF, 1, 20.0f);

			uint16_t pc = cpu->pc;

			for (cpu->pc = disas_start; cpu->pc < disas_end;)
			{
				nk_layout_row_begin(ctx, NK_STATIC, 24, 2);

				uint16_t this_pc = cpu->pc;
			
				char addr[6];
				sprintf(addr, "$%x", this_pc);

				nk_layout_row_push(ctx, 48);
				nk_label(ctx, addr, NK_TEXT_LEFT);

				nk_layout_row_push(ctx, 120);
				char *line = disas_step(cpu);
				if (pc == this_pc)
				{
					nk_label_colored(ctx, line, NK_TEXT_LEFT, selected);
				}
				else
				{
					nk_label(ctx, line, NK_TEXT_LEFT);
				}
				free(line);
			}

			cpu->pc = pc;
		}
		nk_end(ctx);

		if (nk_begin(ctx, "Debugger", nk_rect(50, 50, 230, 150),
			NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
			NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
		{
			nk_layout_row_dynamic(ctx, 30, 2);
			nk_label(ctx, cpu->running ? "CPU Running" : "CPU Halted", NK_TEXT_LEFT);
			if (nk_button_label(ctx, "Reset"))
			{
				puts("cpu reset");
				reset(cpu);
			}

			nk_layout_row_dynamic(ctx, 30, 2);
			if (nk_button_label(ctx, "Step"))
			{
				printf("step pressed!\n");
				step(cpu);
			}

			if (nk_button_label(ctx, cpu_running ? "Stop" : "Run"))
			{
				cpu_running = !cpu_running;
				puts(cpu_running ? "cpu running" : "cpu stopped");
			}
		}
		nk_end(ctx);

		SDL_GetWindowSize(win, &win_width, &win_height);
		glViewport(0, 0, win_width, win_height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(bg.r, bg.g, bg.b, bg.a);
		nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
		SDL_GL_SwapWindow(win);
	}

cleanup:
	nk_sdl_shutdown();
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(win);
	SDL_Quit();
}
