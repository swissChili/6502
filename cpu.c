#include "cpu.h"
#include "instructions.h"

#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define die(m, ...) \
	printf("\033[31mError: " m "\033[0m\n", ##__VA_ARGS__); \
	exit(1);

cpu_t new_cpu()
{
	cpu_t cpu = { 0 };
	cpu.regs[SR] = UNUSED; // unused flag always set
	cpu.regs[SP] = 0xFD; // stack at is 0x100 + SP
	cpu.pc = 0; // arbitrary program counter start
	cpu.mem = malloc(0xFFFF);
	memset(cpu.mem, 0, 0xFFFF);

	if (!cpu.mem)
	{
		die("Could not allocate memory for CPU");
	}

	return cpu;
}

void free_cpu(cpu_t *cpu)
{
	free(cpu->mem);
}

void execute(cpu_t *cpu, const char *mnemonic, uint8_t op, arg_t addr)
{
	switch (op) {
		
	}
}

uint16_t le_to_native(uint8_t a, uint8_t b)
{
#ifdef LITTLE_ENDIAN
	return a << 8 | b;
#else
	return b << 8 | a;
#endif
}

uint16_t fetch_le(cpu_t *cpu)
{
	uint8_t a = cpu->mem[cpu->pc++];
	uint8_t b = cpu->mem[cpu->pc++];
	return le_to_native(a, b);
}

arg_t arg_imm(uint16_t a)
{
	return (arg_t){ a, a };
}

arg_t arg_ptr(cpu_t *c, uint flags, uint16_t p)
{
	if (flags & FETCH_NO_INDIRECTION)
		return arg_imm(p);

	return (arg_t){ c->mem[p], p };
}

arg_t arg(uint16_t v, uint16_t a)
{
	return (arg_t){ v, a };
}

arg_t fetch_addr(cpu_t *cpu, uint8_t am, uint f)
{
	switch (am)
	{
		case AM_ACC:
		case AM_IMP:
			return arg_imm(0);

		// In both cases return immediate 8 bit value
		case AM_IMM:
		case AM_ZP:
			return arg_imm(cpu->mem[cpu->pc++]);

		case AM_ABS:
			return arg_ptr(cpu, f, fetch_le(cpu));

		case AM_REL:
		{
			// PC should point to the opcode
			// braces needed to avoid c stupidity
			uint16_t pc = cpu->pc - 1;
			return arg_ptr(cpu, f, cpu->mem[cpu->pc++] + pc);
		}

		case AM_IND:
		{
			uint16_t addr = fetch_le(cpu);
			uint8_t low = cpu->mem[addr],
				high = cpu->mem[addr + 1];

			return arg_ptr(cpu, f, le_to_native(low, high));
		}

		case AM_AX:
			return arg_ptr(cpu, f, fetch_le(cpu) + cpu->regs[X]);

		case AM_AY:
			return arg_ptr(cpu, f, fetch_le(cpu) + cpu->regs[Y]);

		case AM_ZPX:
			return arg_ptr(cpu, f, cpu->mem[cpu->pc++] + cpu->regs[X]);

		case AM_ZPY:
			return arg_ptr(cpu, f, cpu->mem[cpu->pc++] + cpu->regs[Y]);

		case AM_ZIX:
		{
			uint8_t zp = cpu->mem[cpu->pc++];
			uint16_t addr = zp + cpu->regs[X];
			uint16_t indirect = le_to_native(cpu->mem[addr], cpu->mem[addr + 1]);
			return arg_ptr(cpu, f, indirect);
		}

		case AM_ZIY:
		{
			uint8_t zp = cpu->mem[cpu->pc++];
			uint16_t base = le_to_native(cpu->mem[zp], cpu->mem[zp + 1]);
			return arg_ptr(cpu, f, base + cpu->regs[Y]);
		}

		default:
			die("Unknown address mode %x", am);
			__builtin_unreachable();
	}
}

void step(cpu_t *cpu)
{
	switch (cpu->mem[cpu->pc++])
	{
#define INST(mn, am, op) \
		case op: \
			execute(cpu, #mn, mn, fetch_addr(cpu, am, 0)); \
			break;

		INSTRUCTIONS

#undef INST

		default:
			die("Undefined opcode");
	}
}

void dump_inst(cpu_t *cpu, const char *mn, uint16_t addr, uint8_t am)
{
	printf("\t%s\t", mn);

	switch (am)
	{
		case AM_IMM:
			printf("#");
		case AM_REL:
		case AM_ABS:
		case AM_ZP:
			printf("$%x", addr);
			break;

		case AM_IND:
			printf("($%x)", addr);
			break;

		case AM_AX:
		case AM_ZPX:
			printf("$%x, X", addr);
			break;

		case AM_AY:
		case AM_ZPY:
			printf("$%x, Y", addr);
			break;

		case AM_ZIX:
			printf("($%x, X)", addr);
			break;

		case AM_ZIY:
			printf("($%x), Y", addr);
			break;
	}

	printf("\n");
}

void disas_step(cpu_t *cpu)
{
	printf("%x", cpu->pc);
	uint8_t op = cpu->mem[cpu->pc++];
	switch (op)
	{
#define INST(mn, am, op) \
		case op: \
			dump_inst(cpu, #mn, \
				fetch_addr(cpu, am, FETCH_NO_INDIRECTION).ptr, am); \
			break;

		INSTRUCTIONS

#undef INST

		default:
			die("Undefined opcode %x", op);
	}
}

void disas(cpu_t *cpu)
{
	while (cpu->pc < 0xFFFF)
	{
		disas_step(cpu);
	}
}
