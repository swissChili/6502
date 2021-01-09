#include "as.h"
#include "../cpu.h"
#include "../instructions.h"
#include "../mnemonics.h"
#include "map.h"

#include <endian.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#ifdef VERBOSE_ASSEMBLER
#define dbgf(fmt, ...) fprintf(fmt, __VA_ARGS__)
#else
#define dbgf(fmt, ...)
#endif

enum
{
	ARG_16,						/* Absolute 16 bit argument */
	ARG_8,						/* Absolute 8 bit argument */
	ARG_8REL,					/* Relative 8 bit argument */
	ARG_REL,					/* Relative label */
	ARG_ABS,					/* Absolute label */
	ARG_IMP,					/* Implied argument */
};

#define MAX_LEN (0xFFFF - 0x600)
#define MAX_INSTS (MAX_LEN / 2)

typedef struct
{
	uint8_t opcode;
	uint8_t arg_type;
	uint16_t line;
	union
	{
		char label[32];
		uint16_t long_arg;
		uint8_t byte_arg;
		int8_t rel_arg;
	};
} inst_t;

typedef struct ll_node
{
	struct ll_node *last;
	char name[32];
	uint16_t addr;
} ll_node_t;

// Normal strtok() counts 2 seperators as one, ie asdf\n\njlk is 2 lines.
// This functions counts that as 3 lines, and will return an empty line in between.
char *strtok_fix(char *string, const char *token)
{
	static char *pos;
	if (string)
		pos = string;
	else
		string = pos;

	if (*pos == 0)
		return NULL;
	
	for (; *string; string++)
	{
		for (int i = 0; i < strlen(token); i++)
		{
			if (*string == token[i])
			{
				*string = 0;
				char *old_pos = pos;
				pos = string + 1;

				return old_pos;
			}
		}
	}
	return pos;
}

void putshort(uint16_t a, FILE *out)
{
	uint16_t le = htole16(a);
	fwrite(&le, sizeof(uint16_t), 1, out);
}

void print_inst(inst_t *arg)
{
	char *arg_types =
		"16  8   8RELREL ABS IMP ";
	
	dbgf("\033[33mInst: %.4s $%x ", arg_types + arg->arg_type * 4, arg->opcode);

	switch (arg->arg_type)
	{
	case ARG_16:
		dbgf("%x", arg->long_arg);
		break;
	case ARG_8:
		dbgf("%x", arg->byte_arg);
		break;
	case ARG_8REL:
		dbgf("%d", arg->rel_arg);
		break;
	case ARG_REL:
	case ARG_ABS:
		dbgf("%s", arg->label);
		break;
	}

	dbgf("\033[0m\n");
}

bool is_ident(char c)
{
	return c && (isalpha(c) || isdigit(c)
				 || c == '_' || c == '-'
				 || c == '$' || c == '.');
}

uint32_t skip_ws(char **code)
{
	uint32_t len = 0;

	for (; **code == ' ' || **code == '\t'; (*code)++, len++)
	{}

	return len;
}

uint32_t skip_to_eol(char **code)
{
	uint32_t len = 0;

	for (; **code && **code != '\n'; (*code)++, len++)
	{}

	if (**code)
		(*code)++;

	return len;
}

char *parse_label_name(char **code)
{
	char *start = *code;
	for (; is_ident(**code); (*code)++)
	{}

	if (start == *code)
		return false;

	**code = 0;
	return start;
}

char *parse_label(char **code)
{
	char *start = *code;

	for (; is_ident(**code); (*code)++)
	{}

	skip_ws(code);

	if (*code != start && **code == ':')
	{
		**code = 0;
		(*code)++;
		return start;
	}
	dbgf(">> lkl: **code == %c %x\n", **code, **code);

	*code = start;

	return NULL;
}

char *parse_inst(char **code)
{
	char *start = *code;

	for (; isalpha(**code); (*code)++)
	{}

	if (start == *code)
		return NULL;

	// If code is incremented when it points to \0, it will wrap to the next line
	// returned by strtok(), which causes a bug where instructions followed immediately
	// by a newline and no arguments causes the next instruction to parse the entire
	// program as its argument (not good)
	if (**code)
	{
		**code = 0;
		(*code)++;
	}

	return start;
}

bool is_eol(char c)
{
	return c == ';' ||
		c == '\n' ||
		c == '\0';
}

bool skip(char **code, const char *p)
{
	for (; *p && *p == **code; p++, (*code)++)
	{}

	if (!*p)
		return true;
	return false;
}

bool parse_num(char **code, uint64_t *num)
{
	char *start = *code;
	int base = 10;
	if (**code == '$')
	{
		base = 16;
		(*code)++;
	}
	
	skip_ws(code);

	char *endptr = *code;
	int64_t val = strtol(*code, &endptr, base);

	if (*code == endptr)
	{
		*code = start;
		return false;
	}
	*num = val;
	*code = endptr;
	return true;
}

bool parse_num_max(char **code, uint64_t *num, uint64_t max)
{
	uint64_t n;
	if (parse_num(code, &n))
	{
		if (n > max)
			return false;

		*num = n;
		return true;
	}
	else return false;
}

bool parse_u8(char **code, uint8_t *num)
{
	uint64_t n;
	if (!parse_num_max(code, &n, 0xFF))
		return false;

	*num = n & 0xFF;
	return true;
}

bool parse_u16(char **code, uint16_t *num)
{
	uint64_t n;
	if (!parse_num_max(code, &n, 0xFFFF))
		return false;

	*num = n & 0xFFFF;
	return true;
}

bool ws_end(char **code)
{
	skip_ws(code);
	return is_eol(**code);
}

bool parse_arg(char *code, int am, inst_t *inst)
{
	skip_ws(&code);

	uint16_t num;
	uint8_t num8;
	char *lbl;
	
	switch (am)
	{
	case AM_ACC:
	case AM_IMP:
		dbgf("Trying AM_IMP on '%.8s'\n", code);
		skip_ws(&code);
		if (is_eol(*code))
		{
			inst->arg_type = ARG_IMP;
			return ws_end(&code);
		}
		break;

	case AM_IMM:
		dbgf("Trying AM_IMM on '%.8s'\n", code);
		if (!skip(&code, "#"))
			return false;
		skip_ws(&code);
	case AM_ZP:
		if (parse_u8(&code, &num8))
		{
			inst->arg_type = ARG_8;
			inst->byte_arg = num8;

			return ws_end(&code);
		}
		break;

	case AM_ABS:
		if (parse_u16(&code, &num))
		{
			inst->arg_type = ARG_16;
			inst->long_arg = num;
			return ws_end(&code);
		}
		else if ((lbl = parse_label_name(&code)))
		{
			inst->arg_type = ARG_ABS;
			strncpy(inst->label, lbl, 32);
			return ws_end(&code);
		}
		break;

	case AM_REL:
		if (parse_u8(&code, &num8))
		{
			inst->arg_type = ARG_8REL;
			inst->rel_arg = num;
			return ws_end(&code);
		}
		else if ((lbl = parse_label_name(&code)))
		{
			inst->arg_type = ARG_REL;
			strncpy(inst->label, lbl, 32);
			return ws_end(&code);
		}
		break;

	case AM_IND:
		if (!skip(&code,"("))
			return false;
		
		if (!parse_u16(&code, &num))
			return false;

		if (!skip(&code, ")"))
			return false;

		inst->arg_type = ARG_16;
		inst->long_arg = num;
		return ws_end(&code);

	case AM_AX:
	case AM_ZPX:
	case AM_AY:
	case AM_ZPY:
		if (am == AM_AX || am == AM_AY)
		{
			if (!parse_u16(&code, &num))
				return false;
			inst->arg_type = ARG_16;
			inst->long_arg = num;
		}
		else
		{
			if (!parse_u8(&code, &num8))
				return false;
			inst->arg_type = ARG_8;
			inst->byte_arg = num8;
		}
		dbgf("Parsing AM_* worked, now parsing ,\n");
		if (!skip(&code, ","))
			return false;

		skip_ws(&code);
		dbgf(", worked yup\n");
		char reg = (am == AM_AY || am == AM_ZPY ? 'y' : 'x');
		dbgf("reg is %c, *code is %c %x\n", reg, tolower(*code), tolower(*code));
		if (tolower(*code) != reg)
			return false;
		(code)++;
		return ws_end(&code);

	case AM_ZIX:
		if (!skip(&code, "("))
			break;
		skip_ws(&code);
		if (!parse_u8(&code, &num8))
			break;
		skip_ws(&code);
		if (!skip(&code, ","))
			break;
		skip_ws(&code);
		if (tolower(*code) != 'x')
			return false;
		skip_ws(&code);

		if (!skip(&code, ")"))
			break;

		inst->arg_type = ARG_8;
		inst->byte_arg = num8;
		return ws_end(&code);

	case AM_ZIY:
		if (!skip(&code, "("))
			break;
		skip_ws(&code);
		if (!parse_u8(&code, &num8))
			break;
		skip_ws(&code);
		if (!skip(&code, ")"))
			break;
		skip_ws(&code);
		if (!skip(&code, ","))
			break;
		skip_ws(&code);
		if (tolower(*code) != 'x')
			break;

		inst->arg_type = ARG_8;
		inst->byte_arg = num8;
		return ws_end(&code);
	}
	return false;
}

// Since program counter can never be < $600, return 0 on failure
uint16_t ll_find(ll_node_t *head, char *name)
{
	ll_node_t *last = head;
	while (last)
	{
		head = last;
		if (!strcasecmp(head->name, name))
			return head->addr;
		last = head->last;
	}
	return 0;
}

uint32_t assemble(char *code, FILE *out)
{
	uint16_t num_insts = 0;
	uint16_t pc = 0x600;
	uint32_t line_no = 1;
	ll_node_t *last_node = NULL;
	char *line,
		*orig_line,
		*line_start;
	inst_t **insts = calloc(sizeof(inst_t), MAX_INSTS);

	dbgf("Assembling File\n");
	dbgf("%s\n", code);

	orig_line = strtok_fix(code, "\n");

	while (orig_line)
	{
		line = strdup(orig_line);
		line_start = line;
		
		if (*line == 0)
			goto end_of_line;
		
		skip_ws(&line);
		
		dbgf("line %d: \033[36m%.12s\033[0m\n", line_no, line);
		
		if (is_eol(*line))
		{
			dbgf("skip_ws() brought us to EOL\n");
			goto end_of_line;
		}
		
		char *label = parse_label(&line);
		dbgf(">> label == %.5s %p\n", label, label);
		skip_ws(&code);
		//if (is_eol(*line))
		//	goto end_of_line;
		char *mn = parse_inst(&line);
		
		bool no_argument = false;
		if (is_eol(*line))
		{
			no_argument = true;
		}
		int32_t mnemonic = -1;

		if (label)
		{
			dbgf("Storing label %s\n", label);
			ll_node_t *head = malloc(sizeof(ll_node_t));
			head->last = last_node;
			strncpy(head->name, label, 32);
			// strncpy won't zero the last byte if its over 32 bytes long
			head->name[31] = 0;
			head->addr = pc;
			last_node = head;
			dbgf("Set label %s at $%x\n", label, pc);
		}

		if (mn)
		{
#define MN(a) if (!strcasecmp(mn, #a)) \
			{						   \
				mnemonic = a;		   \
			}						   \
			else

			MNEMONICS
			{
				dbgf(ERR "Could not parse instruction on line %d\n%s\n" RESET, line_no, orig_line);
				free(line_start);
				goto cleanup;
			}
#undef MN

			dbgf("Got instruction %s %d\n", mn, mnemonic);

			inst_t *arg = malloc(sizeof(inst_t));
			arg->line = line_no;
			// dbgf("Parsing '%s'\n", line);
#define INST(_mn, am, op, len) \
			if ((no_argument && (_mn == AM_IMP || _mn == AM_ACC))		\
				 || (mnemonic == _mn && parse_arg(line, am, arg)))		\
			{															\
				dbgf(GREEN "AM_ succeeded: %s at pc=$%x\n" RESET,		\
					   #am, pc);										\
				arg->opcode = op;										\
				pc += len;												\
				print_inst(arg);										\
			}															\
			else

			INSTRUCTIONS
			{
				dbgf("\033[31mCould not be parsed: %s '%s'\033[0m\n", mn, line);
				free(line_start);
				goto cleanup;
			}
#undef INST

			insts[num_insts++] = arg;
		}
	end_of_line:
		line_no++;
		orig_line = strtok_fix(NULL, "\n");
		free(line_start);
	}

	// Generate machine code
	for (int i = 0, curr_pc = 0x600; insts[i]; i++)
	{
		putc(insts[i]->opcode, out);

		switch (insts[i]->arg_type)
		{
		case ARG_8:
			putc(insts[i]->byte_arg, out);
			curr_pc += 2;
			break;
		case ARG_16:
			putshort(insts[i]->long_arg, out);
			curr_pc += 3;
			break;
		case ARG_8REL:
			putc(insts[i]->rel_arg, out);
			curr_pc += 2;
			break;
		case ARG_ABS:
		{
			uint16_t lbl;
			if (!(lbl = ll_find(last_node, insts[i]->label)))
			{
				dbgf(ERR "Error on line %d: label '%s' is not defined" RESET "\n",
					   insts[i]->line, insts[i]->label);
				goto cleanup;
			}
			curr_pc += 3;

			putshort(lbl, out);
			break;
		}
		case ARG_REL:
		{
			uint16_t lbl;
			if (!(lbl = ll_find(last_node, insts[i]->label)))
			{
				dbgf(ERR "Error on line %d: label '%s' is not defined" RESET "\n",
					   insts[i]->line, insts[i]->label);
				goto cleanup;
			}
			curr_pc += 2;
			int16_t diff = lbl - curr_pc;
			dbgf("ARG_REL, pc (after) == %x, diff = %hx\n", curr_pc, (uint8_t) diff);
			if ((diff < 0 ? -diff : diff) > 0xFF)
			{
				dbgf(ERR "Error on line %d: label '%s' is too far away for a relative jump" RESET "\n",
					   insts[i]->line, insts[i]->label);
				dbgf("pc == %hx, label is at %hx\n", curr_pc, lbl);
				goto cleanup;
			}
			putc((uint8_t) diff, out);
			break;
		}
		default:
			curr_pc++;
		}
	}

cleanup:
	dbgf("-----\n");
	dbgf("At end, there are %d instructions\n", num_insts);
	while (last_node)
	{
		ll_node_t *curr_node = last_node;
		last_node = curr_node->last;
		free(curr_node);
	}
	for (int i = 0; insts[i]; i++)
		free(insts[i]);
	free(insts);
	fflush(out);

	return num_insts;
}
