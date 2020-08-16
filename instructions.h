#pragma once

// AUTO GENERATED FILE, DO NOT EDIT BY HAND
#define INSTRUCTIONS \
	INST(ADC, AM_IMM, 0x69, 2) \
	INST(ADC, AM_ZP, 0x65, 2) \
	INST(ADC, AM_ZPX, 0x75, 2) \
	INST(ADC, AM_ABS, 0x6d, 3) \
	INST(ADC, AM_AX, 0x7d, 3) \
	INST(ADC, AM_AY, 0x79, 3) \
	INST(ADC, AM_ZIX, 0x61, 2) \
	INST(ADC, AM_ZIY, 0x71, 2) \
	INST(AND, AM_IMM, 0x29, 2) \
	INST(AND, AM_ZP, 0x25, 2) \
	INST(AND, AM_ZPX, 0x35, 2) \
	INST(AND, AM_ABS, 0x2d, 3) \
	INST(AND, AM_AX, 0x3d, 3) \
	INST(AND, AM_AY, 0x39, 3) \
	INST(AND, AM_ZIX, 0x21, 2) \
	INST(AND, AM_ZIY, 0x31, 2) \
	INST(ASL, AM_ACC, 0x0a, 1) \
	INST(ASL, AM_ZP, 0x06, 2) \
	INST(ASL, AM_ZPX, 0x16, 2) \
	INST(ASL, AM_ABS, 0x0e, 3) \
	INST(ASL, AM_AX, 0x1e, 3) \
	INST(BCC, AM_REL, 0x90, 2) \
	INST(BCS, AM_REL, 0xB0, 2) \
	INST(BEQ, AM_REL, 0xF0, 2) \
	INST(BMI, AM_REL, 0x30, 2) \
	INST(BNE, AM_REL, 0xD0, 2) \
	INST(BPL, AM_REL, 0x10, 2) \
	INST(BVC, AM_REL, 0x50, 2) \
	INST(BVS, AM_REL, 0x70, 2) \
	INST(BIT, AM_ZP, 0x24, 2) \
	INST(BIT, AM_ABS, 0x2c, 3) \
	INST(BIT, AM_IMM, 0x89, 2) \
	INST(BIT, AM_ZPX, 0x34, 2) \
	INST(BIT, AM_AX, 0x3c, 3) \
	INST(BRK, AM_IMP, 0x00, 1) \
	INST(CLC, AM_IMP, 0x18, 1) \
	INST(CLD, AM_IMP, 0xd8, 1) \
	INST(CLI, AM_IMP, 0x58, 1) \
	INST(CLV, AM_IMP, 0xb8, 1) \
	INST(NOP, AM_IMP, 0xea, 1) \
	INST(PHA, AM_IMP, 0x48, 1) \
	INST(PLA, AM_IMP, 0x68, 1) \
	INST(PHP, AM_IMP, 0x08, 1) \
	INST(PLP, AM_IMP, 0x28, 1) \
	INST(RTI, AM_IMP, 0x40, 1) \
	INST(RTS, AM_IMP, 0x60, 1) \
	INST(SEC, AM_IMP, 0x38, 1) \
	INST(SED, AM_IMP, 0xf8, 1) \
	INST(SEI, AM_IMP, 0x78, 1) \
	INST(TAX, AM_IMP, 0xaa, 1) \
	INST(TXA, AM_IMP, 0x8a, 1) \
	INST(TAY, AM_IMP, 0xa8, 1) \
	INST(TYA, AM_IMP, 0x98, 1) \
	INST(TSX, AM_IMP, 0xba, 1) \
	INST(TXS, AM_IMP, 0x9a, 1) \
	INST(CMP, AM_IMM, 0xc9, 2) \
	INST(CMP, AM_ZP, 0xc5, 2) \
	INST(CMP, AM_ZPX, 0xd5, 2) \
	INST(CMP, AM_ABS, 0xcd, 3) \
	INST(CMP, AM_AX, 0xdd, 3) \
	INST(CMP, AM_AY, 0xd9, 3) \
	INST(CMP, AM_ZIX, 0xc1, 2) \
	INST(CMP, AM_ZIY, 0xd1, 2) \
	INST(CPX, AM_IMM, 0xe0, 2) \
	INST(CPX, AM_ZP, 0xe4, 2) \
	INST(CPX, AM_ABS, 0xec, 3) \
	INST(CPY, AM_IMM, 0xc0, 2) \
	INST(CPY, AM_ZP, 0xc4, 2) \
	INST(CPY, AM_ABS, 0xcc, 3) \
	INST(DEC, AM_ZP, 0xc6, 2) \
	INST(DEC, AM_ZPX, 0xd6, 2) \
	INST(DEC, AM_ABS, 0xce, 3) \
	INST(DEC, AM_AX, 0xde, 3) \
	INST(DEC, AM_ACC, 0x3a, 1) \
	INST(DEX, AM_IMP, 0xca, 1) \
	INST(DEY, AM_IMP, 0x88, 1) \
	INST(INX, AM_IMP, 0xe8, 1) \
	INST(INY, AM_IMP, 0xc8, 1) \
	INST(EOR, AM_IMM, 0x49, 2) \
	INST(EOR, AM_ZP, 0x45, 2) \
	INST(EOR, AM_ZPX, 0x55, 2) \
	INST(EOR, AM_ABS, 0x4d, 3) \
	INST(EOR, AM_AX, 0x5d, 3) \
	INST(EOR, AM_AY, 0x59, 3) \
	INST(EOR, AM_ZIX, 0x41, 2) \
	INST(EOR, AM_ZIY, 0x51, 2) \
	INST(INC, AM_ZP, 0xe6, 2) \
	INST(INC, AM_ZPX, 0xf6, 2) \
	INST(INC, AM_ABS, 0xee, 3) \
	INST(INC, AM_AX, 0xfe, 3) \
	INST(INC, AM_ACC, 0x1a, 1) \
	INST(JMP, AM_ABS, 0x4c, 3) \
	INST(JMP, AM_IND, 0x6c, 3) \
	INST(JMP, AM_AX, 0x7c, 3) \
	INST(JSR, AM_ABS, 0x20, 3) \
	INST(LDA, AM_IMM, 0xa9, 2) \
	INST(LDA, AM_ZP, 0xa5, 2) \
	INST(LDA, AM_ZPX, 0xb5, 2) \
	INST(LDA, AM_ABS, 0xad, 3) \
	INST(LDA, AM_AX, 0xbd, 3) \
	INST(LDA, AM_AY, 0xb9, 3) \
	INST(LDA, AM_ZIX, 0xa1, 2) \
	INST(LDA, AM_ZIY, 0xb1, 2) \
	INST(LDX, AM_IMM, 0xa2, 2) \
	INST(LDX, AM_ZP, 0xa6, 2) \
	INST(LDX, AM_ZPY, 0xb6, 2) \
	INST(LDX, AM_ABS, 0xae, 3) \
	INST(LDX, AM_AY, 0xbe, 3) \
	INST(LDY, AM_IMM, 0xa0, 2) \
	INST(LDY, AM_ZP, 0xa4, 2) \
	INST(LDY, AM_ZPX, 0xb4, 2) \
	INST(LDY, AM_ABS, 0xac, 3) \
	INST(LDY, AM_AX, 0xbc, 3) \
	INST(LSR, AM_ACC, 0x4a, 1) \
	INST(LSR, AM_ZP, 0x46, 2) \
	INST(LSR, AM_ZPX, 0x56, 2) \
	INST(LSR, AM_ABS, 0x4e, 3) \
	INST(LSR, AM_AX, 0x5e, 3) \
	INST(ORA, AM_IMM, 0x09, 2) \
	INST(ORA, AM_ZP, 0x05, 2) \
	INST(ORA, AM_ZPX, 0x15, 2) \
	INST(ORA, AM_ABS, 0x0d, 3) \
	INST(ORA, AM_AX, 0x1d, 3) \
	INST(ORA, AM_AY, 0x19, 3) \
	INST(ORA, AM_ZIX, 0x01, 2) \
	INST(ORA, AM_ZIY, 0x11, 2) \
	INST(ROL, AM_ACC, 0x2a, 1) \
	INST(ROL, AM_ZP, 0x26, 2) \
	INST(ROL, AM_ZPX, 0x36, 2) \
	INST(ROL, AM_ABS, 0x2e, 3) \
	INST(ROL, AM_AX, 0x3e, 3) \
	INST(ROR, AM_ACC, 0x6a, 1) \
	INST(ROR, AM_ZP, 0x66, 2) \
	INST(ROR, AM_ZPX, 0x76, 2) \
	INST(ROR, AM_ABS, 0x7e, 3) \
	INST(ROR, AM_AX, 0x6e, 3) \
	INST(SBC, AM_IMM, 0xe9, 2) \
	INST(SBC, AM_ZP, 0xe5, 2) \
	INST(SBC, AM_ZPX, 0xf5, 2) \
	INST(SBC, AM_ABS, 0xed, 3) \
	INST(SBC, AM_AX, 0xfd, 3) \
	INST(SBC, AM_AY, 0xf9, 3) \
	INST(SBC, AM_ZIX, 0xe1, 2) \
	INST(SBC, AM_ZIY, 0xf1, 2) \
	INST(STA, AM_ZP, 0x85, 2) \
	INST(STA, AM_ZPX, 0x95, 2) \
	INST(STA, AM_ABS, 0x8d, 3) \
	INST(STA, AM_AX, 0x9d, 3) \
	INST(STA, AM_AY, 0x99, 3) \
	INST(STA, AM_ZIX, 0x81, 2) \
	INST(STA, AM_ZIY, 0x91, 2) \
	INST(STX, AM_ZP, 0x86, 2) \
	INST(STX, AM_ZPY, 0x96, 2) \
	INST(STX, AM_ABS, 0x8e, 3) \
	INST(STY, AM_ZP, 0x84, 2) \
	INST(STY, AM_ZPX, 0x94, 2) \
	INST(STY, AM_ABS, 0x8c, 3) \
