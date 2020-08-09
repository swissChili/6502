#include "cpu.h"
#include "common.h"
#include "instructions.h"
#define SCREEN_ONLY_SDL
#include "screen.h"

#include "dbg.h"
#include <errno.h>
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define die(m, ...) \
	printf("\033[31m" m "\033[0m\n", ##__VA_ARGS__); \
	exit(1);

#define warn(m, ...) \
	printf("\033[33m" m "\033[0m\n", ##__VA_ARGS__);


sdl_screen_t *g_scr = NULL;


void reset(cpu_t *cpu)
{
	cpu->regs[SP] = 0xFF; // stack at is 0x100 + SP
	cpu->pc = 0x600; // arbitrary program counter start
	cpu->running = true;
	memset(cpu->mem + 0x100, 0, 0xFE);
}

cpu_t new_cpu()
{
	cpu_t cpu = { 0 };
	cpu.regs[SP] = 0xFF; // stack at is 0x100 + SP
	cpu.pc = 0x600; // arbitrary program counter start
	cpu.running = true;
	cpu.mem = malloc(0xFFFF);
	cpu.screen_dirty = true;
	memset(cpu.mem, 0, 0xFFFF);

	ASSERT("Allocate memory for CPU", cpu.mem);

	return cpu;
}

uint16_t le_to_native(uint8_t a, uint8_t b)
{
#ifdef LITTLE_ENDIAN
	return b << 8 | a;
#else
	return a << 8 | b;
#endif
}

void native_to_le(uint16_t n, uint8_t *a, uint8_t *b)
{
#ifdef LITTLE_ENDIAN
	*a = n >> 8;
	*b = n & 0xFF;
#else
	*a = n & 0xFF;
	*b = n >> 8;
#endif
}

void stack_push(cpu_t *cpu, uint8_t v)
{
	cpu->mem[cpu->regs[SP]-- + 0x100] = v;
}

void stack_pushle(cpu_t *cpu, uint16_t v)
{
	uint8_t a, b;
	native_to_le(v, &a, &b);
	// push in "reverse" order so that the address is stored as LE
	stack_push(cpu, b);
	stack_push(cpu, a);
}

uint8_t stack_pop(cpu_t *cpu)
{
	return cpu->mem[cpu->regs[SP]++ + 0x100];
}

uint16_t stack_pople(cpu_t *cpu)
{
	uint8_t a = stack_pop(cpu);
	uint8_t b = stack_pop(cpu);
	return le_to_native(a, b);
}

void free_cpu(cpu_t *cpu)
{
	free(cpu->mem);
}

// rotate right
uint8_t ror(uint8_t a, uint8_t n)
{
	return (a >> n) | (a << (8 - n));
}

// rotate left
uint8_t rol(uint8_t a, uint8_t n)
{
	return (a << n) | (a >> (8 - n));
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

void cmp(cpu_t *cpu, uint8_t reg, uint8_t mem)
{
	cpu->status.negative = 0;
	cpu->status.zero = 0;
	cpu->status.carry = 0;
	if (cpu->regs[reg] < mem)
	{
		cpu->status.negative = 1;
	}
	else if (cpu->regs[reg] == mem)
	{
		cpu->status.zero = 1;
		cpu->status.carry = 1;
	}
	else
	{
		cpu->status.carry = 1;
	}
}

uint16_t scr_dirty(cpu_t *cpu, uint16_t mem)
{
	if (mem >= 0x200 && mem <= 0x200 + 32 * 32)
		cpu->screen_dirty = true;

	return mem;
}

void execute(cpu_t *cpu, const char *mnemonic, uint8_t op, arg_t a, uint8_t am)
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
				cpu->mem[scr_dirty(cpu, a.ptr)] = cpu->regs[reg]; \
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
			cpu->mem[scr_dirty(cpu, scr_dirty(cpu, a.ptr))]++;
			stat_nz(cpu, cpu->mem[scr_dirty(cpu, a.ptr)]);
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
			cpu->mem[scr_dirty(cpu, a.ptr)]--;
			stat_nz(cpu, cpu->mem[scr_dirty(cpu, a.ptr)]);
			break;

		case DEX:
			cpu->regs[X]--;
			stat_nz(cpu, cpu->regs[X]);
			break;

		case DEY:
			cpu->regs[Y]--;
			stat_nz(cpu, cpu->regs[Y]);
			break;

		case ASL:
			// This check must be done here unfortunately, it would be nice
			// to do this while decoding operands but it would require
			// a substantial change to the architecture of the emulator
			if (am == AM_ACC)
			{
				cpu->status.carry = cpu->regs[A] >> 7;
				cpu->regs[A] <<= 1;
				stat_nz(cpu, cpu->regs[A]);
			}
			else
			{
				cpu->status.carry = cpu->mem[scr_dirty(cpu, a.val)] >> 7;
				cpu->mem[scr_dirty(cpu, a.ptr)] <<= 1;
				stat_nz(cpu, cpu->mem[scr_dirty(cpu, a.ptr)]);
			}
			break;

		case LSR:
			if (am == AM_ACC)
			{
				cpu->status.carry = cpu->regs[A] & 1;
				cpu->regs[A] >>= 1;
				stat_nz(cpu, cpu->regs[A]);
			}
			else
			{
				cpu->status.carry = cpu->mem[scr_dirty(cpu, a.val)] & 7;
				cpu->mem[scr_dirty(cpu, a.ptr)] >>= 1;
				stat_nz(cpu, cpu->mem[scr_dirty(cpu, a.ptr)]);
			}
			break;

		case ROL:
			if (am == AM_ACC)
			{
				cpu->status.carry = cpu->regs[A] >> 7;
				cpu->regs[A] = rol(cpu->regs[A], 1);
				stat_nz(cpu, cpu->regs[A]);
			}
			else
			{
				cpu->status.carry = cpu->mem[scr_dirty(cpu, a.val)] >> 7;
				cpu->mem[scr_dirty(cpu, a.ptr)] = rol(a.val, 1);
				stat_nz(cpu, cpu->mem[scr_dirty(cpu, a.ptr)]);
			}
			break;

		case ROR:
			if (am == AM_ACC)
			{
				cpu->status.carry = cpu->regs[A] & 1;
				cpu->regs[A] = ror(cpu->regs[A], 1);
				stat_nz(cpu, cpu->regs[A]);
			}
			else
			{
				cpu->status.carry = cpu->mem[scr_dirty(cpu, a.val)] & 1;
				cpu->mem[scr_dirty(cpu, a.ptr)] = ror(a.val, 1);
				stat_nz(cpu, cpu->mem[scr_dirty(cpu, a.ptr)]);
			}
			break;

		case AND:
			cpu->regs[A] &= a.val;
			stat_nz(cpu, cpu->regs[A]);
			break;

		case ORA:
			cpu->regs[A] |= a.val;
			stat_nz(cpu, cpu->regs[A]);
			break;

		case EOR:
			cpu->regs[A] ^= a.val;
			stat_nz(cpu, cpu->regs[A]);
			break;

		case CMP:
			cmp(cpu, A, a.val);
			break;

		case CPX:
			cmp(cpu, X, a.val);
			break;

		case CPY:
			cmp(cpu, Y, a.val);
			break;

		// TODO: implement BIT here

		#define BRANCHES \
			B(BCC, carry == 0) \
			B(BCS, carry == 1) \
			B(BNE, zero == 0) \
			B(BEQ, zero == 1) \
			B(BPL, negative == 0) \
			B(BMI, negative == 1) \
			B(BVC, overflow == 0) \
			B(BVS, overflow == 1)

		#define B(i, c) \
			case i: \
				if (cpu->status . c) \
					cpu->pc = a.ptr;\
				break;

			BRANCHES

		#undef B
		#undef BRANCHES

		#define TRANSFERS \
			T(A, X) \
			T(X, A) \
			T(A, Y) \
			T(Y, A)

		#define T(a, b) \
			case T ## a ## b: \
				cpu->regs[b] = cpu->regs[a]; \
				stat_nz(cpu, cpu->regs[b]); \
				break;

			TRANSFERS

		#undef T
		#undef TRANSFERS

		case TSX:
			cpu->regs[X] = cpu->regs[SP];
			stat_nz(cpu, cpu->regs[X]);
			break;

		case TXS:
			cpu->regs[SP] = cpu->regs[X];
			stat_nz(cpu, cpu->regs[X]);
			break;

		case PHA:
			stack_push(cpu, cpu->regs[A]);
			break;

		case PLA:
			cpu->regs[A] = stack_pop(cpu);
			stat_nz(cpu, cpu->regs[A]);
			break;

		case PHP:
			stack_push(cpu, *(uint8_t *)(&cpu->status));
			break;

		case PLP:
		{
			uint8_t s = stack_pop(cpu);
			*(uint8_t *)(&cpu->status) = s;
		}

		case JMP:
			cpu->pc = a.ptr;
			break;

		case JSR:
			stack_pushle(cpu, cpu->pc);
			cpu->pc = a.ptr;
			break;

		case RTS:
			cpu->pc = stack_pople(cpu);
			break;

		// TODO: implement RTI
		// TODO: implement flag instructions
		
		case BRK:
			// TODO: trigger an interrupt
			cpu->running = false;
			break;

		case NOP:
			break;

		default:
			warn("Unsupported opcode: %x\n", op);
			THROW("Unsupported opcode");
	}
	#undef REGS
}

uint16_t fetch_le(cpu_t *cpu)
{
	uint8_t a = cpu->mem[scr_dirty(cpu, cpu->pc++)];
	uint8_t b = cpu->mem[scr_dirty(cpu, cpu->pc++)];
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
			return arg_imm(cpu->mem[scr_dirty(cpu, cpu->pc++)]);

		case AM_ABS:
			return arg_ptr(cpu, f, fetch_le(cpu));

		case AM_REL:
		{
			// Aparently, PC should will point to the NEXT opcode
			// I can't find any documentation on this unfortunately, but
			// I have discovered this through testing the output of other
			// assemblers.
			uint16_t pc = cpu->pc + 1;
			return arg_ptr(cpu, f, (int8_t)cpu->mem[scr_dirty(cpu, cpu->pc++)] + pc);
		}

		case AM_IND:
		{
			uint16_t addr = fetch_le(cpu);

			if (f & FETCH_NO_INDIRECTION)
				return arg_imm(addr);

			uint8_t low = cpu->mem[scr_dirty(cpu, addr)],
				high = cpu->mem[scr_dirty(cpu, addr + 1)];

			return arg_ptr(cpu, f, le_to_native(low, high));
		}

		case AM_AX:
			if (f & FETCH_NO_INDIRECTION)
				return arg_ptr(cpu, f, fetch_le(cpu));

			return arg_ptr(cpu, f, fetch_le(cpu) + cpu->regs[X]);

		case AM_AY:
			if (f & FETCH_NO_INDIRECTION)
				return arg_ptr(cpu, f, fetch_le(cpu));
		
			return arg_ptr(cpu, f, fetch_le(cpu) + cpu->regs[Y]);

		case AM_ZPX:
			if (f & FETCH_NO_INDIRECTION)
				return arg_ptr(cpu, f, cpu->mem[cpu->pc++]);
			return arg_ptr(cpu, f, cpu->mem[cpu->pc++] + cpu->regs[X]);

		case AM_ZPY:
			if (f & FETCH_NO_INDIRECTION)
				return arg_ptr(cpu, f, cpu->mem[cpu->pc++]);
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
			uint8_t zp = cpu->mem[scr_dirty(cpu, cpu->pc++)];

			if (f & FETCH_NO_INDIRECTION)
				return arg_imm(zp);

			uint16_t base = le_to_native(cpu->mem[zp], cpu->mem[scr_dirty(cpu, zp + 1)]);
			return arg_ptr(cpu, f, base + cpu->regs[Y]);
		}

		default:
			warn("Unknown address mode %x", am);
			THROW("Unknowng address mode");
			__builtin_unreachable();
	}
}

void step(cpu_t *cpu)
{
	cpu->screen_dirty = false;
	uint8_t pc = cpu->pc;
	uint8_t op = cpu->mem[cpu->pc++];
	switch (op)
	{
#define INST(mn, am, op) \
		case op: \
			execute(cpu, #mn, mn, fetch_addr(cpu, am, 0), am); \
			break;

		INSTRUCTIONS

#undef INST

		default:
			warn("Undefined opcode %x near %x [%x]", op, pc, cpu->mem[pc]);
			THROW("Undefined opcode");
	}
}

int dump_inst(cpu_t *cpu, char *buf, const char *mn, uint16_t addr, uint8_t am)
{
	char *end = buf;
	end += sprintf(end, "%s ", mn);

	switch (am)
	{
		case AM_IMM:
			end += sprintf(end, "#");
		case AM_REL:
		case AM_ABS:
		case AM_ZP:
			end += sprintf(end, "$%x", addr);
			break;

		case AM_IND:
			end += sprintf(end, "($%x)", addr);
			break;

		case AM_AX:
		case AM_ZPX:
			end += sprintf(end, "$%x, X", addr);
			break;

		case AM_AY:
		case AM_ZPY:
			end += sprintf(end, "$%x, Y", addr);
			break;

		case AM_ZIX:
			end += sprintf(end, "($%x, X)", addr);
			break;

		case AM_ZIY:
			end += sprintf(end, "($%x), Y", addr);
			break;
	}

	return end - buf;
}

char *disas_step(cpu_t *cpu)
{
	char *buffer = malloc(80);
	char *end = buffer;

	// end += sprintf(buffer, "$%x", cpu->pc);
	uint8_t op = cpu->mem[scr_dirty(cpu, cpu->pc++)];
	switch (op)
	{
#define INST(mn, am, op) \
		case op: \
			end += dump_inst(cpu, end, #mn, \
				fetch_addr(cpu, am, FETCH_NO_INDIRECTION).ptr, am); \
			break;

		INSTRUCTIONS

#undef INST

		default:
			end += sprintf(end, "Undefined opcode %x", op);
	}

	*end = 0;

	return buffer;
}

void disas_num(cpu_t *cpu, uint16_t num)
{
	uint16_t pc = cpu->pc;
	for (int i = 0; i < num; i++)
	{
		uint16_t last_pc = cpu->pc;
		char *line = disas_step(cpu);
		printf("$%x\t%s\n", last_pc, line);
		free(line);
	}
	cpu->pc = pc;
}

void disas(cpu_t *cpu)
{
	uint16_t pc = cpu->pc;
	// Raw binary, no way to know what's code what isn't
	while (cpu->pc < 0xFFFF)
	{
		uint16_t last_pc = cpu->pc;
		char *line = disas_step(cpu);
		printf("$%x\t%s\n", last_pc, line);
		free(line);
	}
	cpu->pc = pc;
}

void run(cpu_t *cpu)
{
	while (cpu->running)
	{
		step(cpu);
	}

	printf("CPU Halted\n");
}

void run_mq(cpu_t *cpu, mqd_t mq)
{
	char buf[MQ_BUF_LEN];
	bool running;

	while (true)
	{
		if (running)
		{
			if (cpu->running)
				step(cpu);
			else
				running = false;
		}

		ssize_t recvd = mq_receive(mq, buf, MQ_BUF_LEN * 2, NULL);

		if (recvd == -1 && errno != EAGAIN)
		{
			printf("errno = %d\n", errno);
			THROW("mq_receive returned -1");
		}

		if (recvd > 0)
		{
			if (debug_stmt(cpu, buf, &running))
				break;
		}
	}
}
