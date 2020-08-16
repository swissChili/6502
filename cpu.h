#pragma once

#include "mnemonics.h"

#include <stdint.h>
#include <stdbool.h>
#include <mqueue.h>

#define REGISTERS R(A) R(X) R(Y) R(SP)
#define CPU_FB_ADDR 0x200
#define CPU_FB_W 32
#define CPU_FB_H 32

#define MQ_BUF_LEN 512
#define MQ_NAME "/6502-to-cpu"

enum // Registers
{
	A, X, Y, SP
};

enum // Flags
{
	CARRY	= 1 << 0,
	ZERO	= 1 << 1,
	NO_INT	= 1 << 2,
	DECIMAL	= 1 << 3,
	BREAK	= 1 << 4,
	UNUSED	= 1 << 5,
	OVERFLW	= 1 << 6,
	NEGATIV	= 1 << 7,
};

enum // Address Modes
{
	AM_ACC,	// Accumulator implied as operand
	AM_IMP,	// Implied operand
	AM_IMM,	// Immediate operand (8 bit)
	AM_ABS,	// Absolute operand (16 bit)
	AM_ZP,	// Zero-page operand
	AM_REL,	// Relative operand (signed 8 bit)
	AM_IND,	// Absolute indirect operand (16 bit address of 16 bit address)
	AM_AX,	// Absolute indexed with X
	AM_AY,	// Absolute indexed with Y
	AM_ZPX,	// Zero-page indexed with X
	AM_ZPY,	// Zero-page indexed with Y
	AM_ZIX,	// Zero-page indexed indirect (zp,x)
	AM_ZIY,	// Zero-page indirect indexed (zp),y
};

enum // Opcodes
{
#define MN(name) name,
	MNEMONICS
#undef MN
};

enum // Fetch flags
{
	FETCH_NO_INDIRECTION = 1, // Do not follow indirection (used for disassembly)
};

// Status register
typedef struct __attribute__((packed))
{
	bool negative	: 1;
	bool overflow	: 1;
	bool unused		: 1;
	bool break_		: 1;
	bool decimal	: 1;
	bool no_int		: 1;
	bool zero		: 1;
	bool carry		: 1;
} status_t;

// Emulator instance, create with new_cpu()
typedef struct
{
	uint8_t regs[4]; // A, X, Y, SP registers
	uint16_t pc;
	status_t status;
	uint8_t *mem;
	bool running;
	bool screen_dirty;
} cpu_t;

// Argument type, includes both pointer and its value
typedef struct
{
	uint16_t val; // Value at pointer (used by most instructions)
	uint16_t ptr; // Pointer (used by jumps, etc)
} arg_t;

cpu_t new_cpu();
void step(cpu_t *cpu);
void free_cpu(cpu_t *cpu);
void die(const char *message);
void reset(cpu_t *cpu);
void disas(cpu_t *cpu);
void disas_num(cpu_t *cpu, uint16_t num);
// Buffer must be freed by user
char *disas_step(cpu_t *cpu, uint16_t *pc);
void run(cpu_t *cpu);
void run_mq(cpu_t *cpu, mqd_t mq);
