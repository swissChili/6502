#include "cpu.h"
#include "dbg.h"
#include "gui.h"
#include "screen.h"

#include <bits/getopt_core.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern sdl_screen_t *g_scr;

int main(int argc, char **argv)
{
	bool disflag = false,
		runflag = false,
		helpflag = false,
		debugflag = false,
		should_read = false,
		guiflag = false,
		scrflag = false;

	int disasm_len = 0;

	FILE *input = stdin;

	char c;

	while ((c = getopt(argc, argv, "Dsdrhgi:n:")) != -1)
	{
		switch (c)
		{
		case 'd':
			disflag = true;
			should_read = true;
			break;
		case 'r':
			runflag = true;
			should_read = true;
			break;
		case 'D':
			debugflag = true;
			should_read = true;
			break;
		case 'g':
			guiflag = true;
			should_read = true;
			break;
		case 'i':
			input = fopen(optarg, "r");
			break;
		case 'n':
			disasm_len = atoi(optarg);
			break;
		case 's':
			scrflag = true;
			break;
		case 'h':
		case '?':
			helpflag = 1;
			break;
		}
	}

	if (helpflag)
	{
		printf("6502 emulator, disassembler and debugger\n"
			"Usage:\n"
			"	-g use GUI\n"
			"	-d disassemble input\n"
			"	-r run input\n"
			"	-D open CLI debug prompt (like gdb)\n"
			"	-i <input> set input file, defaults to standard input\n"
			"	-n <number> number of instructions to disassemble, 0 for all\n"
			"	-h, -? show this help page\n");
		return 0;
	}

	cpu_t cpu;

	if (should_read)
	{
		cpu = new_cpu();
		fread(cpu.mem + 0x600, 0xFFFF - 0x600, 1, input);
	}
	else
	{
		puts("6502 toolchain by swissChili <swisschili.sh>");
		printf("%s -h  for help\n", argv[0]);
	}

	if (scrflag)
	{
#ifndef NO_PTHREAD
		start_screen_thread(cpu.mem + CPU_FB_ADDR);
#else
		sdl_screen_t scr = new_sdl_screen(8);
		g_scr = &scr;
#endif
	}

	if (guiflag)
	{
		gui(&cpu);
	}
	else if (disflag)
	{
		disas_num(&cpu, 12);
	}
	else if (runflag)
	{
		run(&cpu);
	}
	else if (debugflag)
	{
		debug(&cpu);
	}
	
	if (scrflag)
		free_sdl_screen(g_scr);

	if (should_read)
		free_cpu(&cpu);
}
