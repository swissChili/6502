#pragma once

#include <stdint.h>

enum // Registers
{
	A, X, Y, SR, SP
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
	LDA,
	LDX,
	LDY,
	STA,
	STX,
	STY,
	ADC,
	SBC,
	INC,
	INX,
	INY,
	DEC,
	DEX,
	DEY,
	ASL,
	LSR,
	ROL,
	ROR,
	AND,
	ORA,
	EOR,
	CMP,
	CPX,
	CPY,
	BIT,
	BCC,
	BCS,
	BNE,
	BEQ,
	BPL,
	BMI,
	BVC,
	BVS,
	TAX,
	TXA,
	TAY,
	TYA,
	TSX,
	TXS,
	PHA,
	PLA,
	PHP,
	PLP,
	JMP,
	JSR,
	RTS,
	RTI,
	CLC,
	SEC,
	CLD,
	SED,
	CLI,
	SEI,
	CLV,
	BRK,
	NOP,
};

// Emulator instance, create with new_cpu()
typedef struct
{
	uint8_t regs[5]; // A, X, Y, SP and SR registers
	uint16_t pc;
	uint8_t *mem;
} cpu_t;

// Argument type, includes both pointer and its value
typedef struct
{
	uint16_t val; // Value at pointer (used by most instructions)
	uint16_t ptr; // Pointer (used by jumps, etc)
} arg_t;

cpu_t new_cpu();
void free_cpu(cpu_t *cpu);
void die(const char *message);
void disas(cpu_t *cpu);
