#include "common.h"
#include "cpu.h"
#include "dbg.h"
#include "gui.h"
#include "screen.h"

#include <bits/getopt_core.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/stat.h>


void cleanup_screen_thread(pthread_t thread)
{
	puts("Cleaning up screen...");
	pthread_cancel(thread);
}

void cleanup_debug_prompt_thread(pthread_t thread)
{
	puts("Cleaning up debug prompt...");
	pthread_cancel(thread);
}

int main(int argc, char **argv)
{
	bool disflag = false,
		runflag = false,
		helpflag = false,
		debugflag = false,
		should_read = false,
		guiflag = false,
		scrflag = false,
		nohaltflag = false;

	int disasm_len = 0;

	FILE *input = stdin;

	char c;

	while ((c = getopt(argc, argv, "HDsdrhgi:n:")) != -1)
	{
		switch (c)
		{
		case 'H':
			nohaltflag = true;
			break;
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
			"	-s use SDL screen (faster than GUI debugger)\n"
			"	-H keep running after CPU halts (useful on windows and to look at screen)\n"
			"	-d disassemble input\n"
			"	-r run input\n"
			"	-D open CLI debug prompt (like gdb)\n"
			"	-i <input> set input file, defaults to standard input\n"
			"	-n <number> number of instructions to disassemble, 0 for all\n"
			"	-h, -? show this help page\n");
		return 0;
	}

	cpu_t cpu;
	mqd_t mq_to_cpu;

	struct mq_attr attrs;
	attrs.mq_maxmsg = 10;
	attrs.mq_msgsize = MQ_BUF_LEN;

	if (should_read)
	{
		cpu = new_cpu();
		fread(cpu.mem + 0x600, 0xFFFF - 0x600, 1, input);
		
		int unlink = mq_unlink(MQ_NAME);
		if (unlink < 0 && errno != ENOENT)
		{
			printf("Warning: mq_unlink() error: %d %s\n", errno, strerror(errno));
		}
		
		mq_to_cpu = mq_open(MQ_NAME, O_RDWR | O_CREAT | O_NONBLOCK, S_IWUSR|S_IRUSR, &attrs);
		printf("error after mq_open (%ld) = %d %s\n", attrs.mq_msgsize, errno, strerror(errno));
		ASSERT("Open message queue for emulator", mq_to_cpu > 0)

		mq_send(mq_to_cpu, "init", 5, 2);
	}
	else
	{
		puts("6502 toolchain by swissChili <swisschili.sh>");
		printf("%s -h  for help\n", argv[0]);
	}

	if (scrflag)
	{
		CATCH(&cleanup_screen_thread, start_screen_thread(cpu.mem + CPU_FB_ADDR));
	}

	if (guiflag && scrflag)
	{
		THROW("-g and -s cannot be used together");
	}

	if (guiflag)
	{
		start_gui(mq_to_cpu, &cpu);
		run_mq(&cpu, mq_to_cpu);
	}
	else if (disflag)
	{
		disas_num(&cpu, 64);
	}
	else if (runflag)
	{
		run(&cpu);
		if (nohaltflag)
		{
			puts("Press any key to exit");
			getchar();
		}
	}
	else if (debugflag)
	{
		CATCH(&cleanup_debug_prompt_thread, start_debug_prompt(mq_to_cpu, &cpu));
		run_mq(&cpu, mq_to_cpu);
	}

	if (should_read)
	{
		free_cpu(&cpu);
		mq_close(mq_to_cpu);
	}
}
