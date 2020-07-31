#include "cpu.h"
#include "instructions.h"

#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define die(m, ...) \
	printf("\033[31m" m "\033[0m\n", ##__VA_ARGS__); \
	exit(1);

#define warn(m, ...) \
	printf("\033[33m" m "\033[0m\n", ##__VA_ARGS__);

cpu_t new_cpu()
{
	cpu_t cpu = { 0 };
	cpu.regs[SP] = 0xFD; // stack at is 0x100 + SP
	cpu.pc = 0; // arbitrary program counter start
	cpu.running = true;
	cpu.mem = malloc(0xFFFF);
	memset(cpu.mem, 0, 0xFFFF);

	if (!cpu.mem)
	{
		die("Could not allocate memory for CPU");
	}

	return cpu;
}

void stack_push(cpu_t *cpu, uint8_t v)
{
	cpu->mem[cpu->regs[SP]-- + 0x100] = v;
}

uint8_t stack_pop(cpu_t *cpu)
{
	return cpu->mem[cpu->regs[SP]++ + 0x100];
}

void free_cpu(cpu_t *cpu)
{
	free(cpu->mem);
}

void stat_nz(cpu_t *cpu, int8_t v)
{
	cpu->status.negative = v < 0;
	cpu->status.zero = v == 0;
}

// Used to check for overflow, is c unique?
bool last_unique(bool a, bool b, bool c)
{
	return a == b && a != c;
}

void stat_cv(cpu_t *cpu, uint8_t a, uint8_t b, uint8_t c)
{
	cpu->status.overflow = last_unique(a >> 7, b >> 7, c >> 7);
	cpu->status.carry = c < a || c < b;
}

void execute(cpu_t *cpu, const char *mnemonic, uint8_t op, arg_t a)
{
	// used to save space
	#define REGS \
		R(X) R(A) R(Y)

	switch (op) {
		// Load and store instructions:
		#define R(reg) \
			case LD##reg: \
				cpu->regs[reg] = a.val; \
				stat_nz(cpu, a.val); \
				break;

			REGS

		#undef R

		#define R(reg) \
			case ST##reg: \
				cpu->mem[a.ptr] = cpu->regs[reg]; \
				break; \

			REGS

		#undef R

		// Arithmetic instructions:
		// NOTE: binary coded decimals are NOT SUPPORTED because I don't want
		// to implement them.
		case ADC:
		{
			uint8_t sum = cpu->regs[A] + a.val + cpu->status.carry;
			// signed overflow
			stat_cv(cpu, cpu->regs[A], a.val + cpu->status.carry, sum);
			stat_nz(cpu, sum);
			cpu->regs[A] = sum;
			break;
		}

		case SBC:
		{
			uint8_t diff = cpu->regs[A] - a.val - !cpu->status.carry;
			stat_cv(cpu, cpu->regs[A], a.val - !cpu->status.carry, diff);
			stat_nz(cpu, diff);
			cpu->regs[A] = diff;
			break;
		}

		case INC:
			cpu->mem[a.ptr]++;
			stat_nz(cpu, cpu->mem[a.ptr]);
			break;

		case INX:
			cpu->regs[X]++;
			stat_nz(cpu, cpu->regs[X]);
			break;

		case INY:
			cpu->regs[Y]++;
			stat_nz(cpu, cpu->regs[Y]);
			break;

		case DEC:
			cpu->mem[a.ptr]--;
			stat_nz(cpu, cpu->mem[a.ptr]);
			break;
	}
	#undef REGS
}

uint16_t le_to_native(uint8_t a, uint8_t b)
{
#ifdef LITTLE_ENDIAN
	return b << 8 | a;
#else
	return a << 8 | b;
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
			// Aparently, PC should will point to the NEXT opcode
			// I can't find any documentation on this unfortunately, but
			// I have discovered this through testing the output of other
			// assemblers.
			uint16_t pc = cpu->pc + 1;
			return arg_ptr(cpu, f, (int8_t)cpu->mem[cpu->pc++] + pc);
		}

		case AM_IND:
		{
			uint16_t addr = fetch_le(cpu);

			if (f & FETCH_NO_INDIRECTION)
				return arg_imm(addr);

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

			if (f & FETCH_NO_INDIRECTION)
				return arg_imm(zp);

			uint16_t addr = zp + cpu->regs[X];
			uint16_t indirect = le_to_native(cpu->mem[addr], cpu->mem[addr + 1]);
			return arg_ptr(cpu, f, indirect);
		}

		case AM_ZIY:
		{
			uint8_t zp = cpu->mem[cpu->pc++];

			if (f & FETCH_NO_INDIRECTION)
				return arg_imm(zp);

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
	printf("$%x", cpu->pc);
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
			warn("\tUndefined opcode %x", op);
	}
}

void disas(cpu_t *cpu)
{
	// Raw binary, no way to know what's code what isn't
	while (cpu->pc < 0xFFFF)
	{
		disas_step(cpu);
	}
}
