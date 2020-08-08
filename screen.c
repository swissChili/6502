#include "screen.h"
#include "cpu.h"

struct nk_color byte_to_color(uint8_t b)
{
	struct nk_color c;
	c.r = (b >> 6) * (255 / 0b11);
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

	//nk_fill_rect(out, bounds, 0, nk_rgb(255, 0, 0));

	//return;

	for (int i = 0; i < CPU_FB_H; i++)
	{
		for (int j = 0; j < CPU_FB_W; j++)
		{
			nk_fill_rect(out,
				nk_rect(bounds.x + i * size, bounds.y + j * size,
					size, size), 0.0f,
				byte_to_color(mem[i * CPU_FB_H + j]));
		}
	}
}
