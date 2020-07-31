#include "dbg.h"
#include "cpu.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <stdlib.h>

void debug(cpu_t *cpu)
{
	while (true)
	{
		char *input = readline("\033[33m> \033[0m");
		if (!input)
			continue;

		char *tok = strtok(input, " \t\r\n\v");
		
		if (!strcmp(tok, "step") || !strcmp(tok, "s"))
		{
			step(cpu);
		}
		else if (!strcmp(tok, "show") || !strcmp(tok, "print"))
		{
			if ((tok = strtok(NULL, " ")))
			{
				char *ok = 0;
				if (tok[0] == '$')
				{
					uint16_t addr = strtol(tok + 1, &ok, 16);

					if (ok == 0)
					{
						printf("Memory:\n");
						printf("\t$%x	%x\n", addr, cpu->mem[addr]);
						printf("\t$%x	%x\n", addr + 1, cpu->mem[addr + 1]);
					}
				}
				else
				{
					printf("Registers:\n");

					#define R(r) printf("\t" #r ":	%x\n", cpu->regs[r]);
						REGISTERS
					#undef R
				}
			}
		}

		add_history(input);
	}
}
