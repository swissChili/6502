#include "cpu.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	printf("6502 Emulator\n");

	uint8_t disflag = 0,
		runflag = 0,
		helpflag = 0;

	char c;

	while ((c = getopt(argc, argv, "drh")) != -1)
	{
		switch (c)
		{
			case 'd':
				disflag = 1;
				break;
			case 'r':
				runflag = 1;
				break;
			case 'h':
			case '?':
				helpflag = 1;
				break;
		}
	}

	if (helpflag)
	{
		printf("-r to run, -d to disassemble");
		return 0;
	}

	cpu_t cpu = new_cpu();
	fread(cpu.mem, 0xFFFF, 1, stdin);

	if (disflag)
	{
		disas(&cpu);
	}

	free_cpu(&cpu);
}
