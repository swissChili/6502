#include "dbg.h"
#include "cpu.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

bool debug_stmt(cpu_t *cpu, char *input, bool *running)
{
	char *tok = strtok(input, " \t\r\n\v");

	if (!tok || !*tok)
		return false;
	
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

				printf("Memory:\n");
				printf("\t$%x	%x\n", addr, cpu->mem[addr]);
				if (addr < 0xFFFF)
					printf("\t$%x	%x\n", addr + 1, cpu->mem[addr + 1]);
			}
			else
			{
				printf("Expected an address as an argument in the form "
					"$1234, not %s\n", tok);
			}
		}
		else
		{
			printf("Registers:\n");

			printf("\tPC:\t$%x\n", cpu->pc);
			#define R(r) printf("\t" #r ":\t$%x\n", cpu->regs[r]);
				REGISTERS
			#undef R
		}
	}
	else if (!strcmp(tok, "run"))
	{
		*running = true;
	}
	else if (!strcmp(tok, "quit") || !strcmp(tok, "exit"))
	{
		printf("Bye\n");
		return true;
	}
	else
	{
		printf("Unknown command %s\n", tok);
	}
	
	return false;
}

typedef struct
{
	mqd_t mq;
	cpu_t *cpu;
} debug_prompt_arg_t;

void debug_prompt(debug_prompt_arg_t *arg)
{
	mqd_t mq = arg->mq;
	cpu_t *cpu = arg->cpu;
	free(arg);

	bool running = true;
	while (running)
	{
		char *input = readline("\033[33m> \033[0m");
		if (!input || !*input)
			continue;

		if (!strcmp(input, "quit") || !strcmp(input, "exit"))
			running = false;

		mq_send(mq, input, strlen(input) + 1, 2);

		add_history(input);
		free(input);
	}
}

pthread_t start_debug_prompt(mqd_t mq, cpu_t *cpu)
{
	debug_prompt_arg_t *arg = malloc(sizeof(debug_prompt_arg_t));
	arg->mq = mq;
	arg->cpu = cpu;

	pthread_t thread;
	pthread_create(&thread, NULL, (void *(*)(void *))&debug_prompt, arg);
	return thread;
}
