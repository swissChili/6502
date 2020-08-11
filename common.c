#include "common.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

catch_t g_catch[MAX_CATCH_LEN];
unsigned g_catch_len;

void unwind()
{
	for (int i = g_catch_len - 1; i >= 0; i--)
	{
		g_catch[i].fn(g_catch[i].arg);
	}
}

void throw_(const char *msg, const char *file, unsigned int line)
{
	fprintf(stderr, "\033[31mException thrown:\033[33m %s:%d\033[0m %s\n", file, line, msg);
	unwind();
	exit(1);
}

void catch_(handle_t hdl, intptr_t arg)
{
	if (g_catch_len > MAX_CATCH_LEN)
	{
		THROW("Catch overflow");
	}

	g_catch[g_catch_len++] = (catch_t){ .fn = hdl, .arg = arg };
}

void catch_signal(int sig)
{
	if (sig == SIGSEGV)
		throw_("Segmentation fault", "unknown", 0);
}

__attribute__((constructor)) void init_catch()
{
	signal(SIGSEGV, catch_signal);
}
