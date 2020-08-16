#include "as.h"
#include "../cpu.h"
#include "../instructions.h"
#include "../mnemonics.h"

#include <collect/map.h>
#include <collect/vector.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

enum
{
	ARG_16,						/* Absolute 16 bit argument */
	ARG_8,						/* Absolute 8 bit argument */
	ARG_8REL,					/* Relative 8 bit argument */
	ARG_REL,					/* Relative label */
	ARG_ABS,					/* Absolute label */
	ARG_IMP,					/* Implied argument */
};

typedef struct
{
	uint8_t opcode;
	uint8_t arg_type;
	union
	{
		char label[32];
		uint16_t long_arg;
		uint8_t byte_arg;
		int8_t rel_arg;
	};
} inst_t;

void print_inst(inst_t *arg)
{
	char *arg_types =
		"16  8   8RELREL ABS IMP ";
	
	printf("\033[33mInst: %.4s $%x ", arg_types + arg->arg_type * 4, arg->opcode);

	switch (arg->arg_type)
	{
	case ARG_16:
		printf("%x", arg->long_arg);
		break;
	case ARG_8:
		printf("%x", arg->byte_arg);
		break;
	case ARG_8REL:
		printf("%d", arg->rel_arg);
		break;
	case ARG_REL:
	case ARG_ABS:
		printf("%s", arg->label);
		break;
	}

	printf("\033[0m\n");
}

bool is_ident(char c)
{
	return c && (isalpha(c) || isdigit(c));
}

uint32_t skip_ws(char **code)
{
	uint32_t len = 0;

	for (; isspace(**code); (*code)++, len++)
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

	if (**code == ':')
	{
		**code = 0;
		(*code)++;
		return start;
	}

	*code = start;

	return NULL;
}

char *parse_inst(char **code)
{
	char *start = *code;

	for (; isalpha(**code); (*code)++)
	{}

	**code = 0;

	if (start == *code)
		return NULL;

	(*code)++;
	return start;
}

bool is_eol(char c)
{
	return c == ';' ||
		c == '\n' ||
		c == '\r' ||
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
		printf("Trying AM_IMP on '%.8s'\n", code);
		skip_ws(&code);
		if (is_eol(*code))
		{
			inst->arg_type = ARG_IMP;
			return ws_end(&code);
		}
		break;

	case AM_IMM:
		printf("Trying AM_IMM on '%.8s'\n", code);
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
			return true;
		}
		else if ((lbl = parse_label_name(&code)))
		{
			inst->arg_type = ARG_ABS;
			strncpy(inst->label, lbl, 32);
			return true;
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
		return true;

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
		if (!skip(&code, ","))
			return false;

		skip_ws(&code);

		if (tolower(*code) != (am == AM_AY || am == AM_ZPY ? 'y' : 'x'))
			return false;

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

uint32_t assemble(char *code, FILE *out)
{
	uintptr_t num_insts = 0;
	uint32_t line_no = 1;
	map *labels = new_map();
	vector *insts = new_vector();
	char *line;

	printf("Assembling File\n");
	printf("%s\n", code);

	line = strtok(code, "\r\n");
	
	while (line)
	{
		skip_ws(&line);

		printf("\033[36m%.9s\033[0m\n", line);
		
		char *label = parse_label(&line),
			*mn = parse_inst(&line);
		int32_t mnemonic = -1;

		if (label)
		{
			map_set(labels, label, (void *)num_insts);
			printf("Set label %s at %lu\n", label, num_insts);
		}

		if (mn)
		{
#define MN(a) if (!strcasecmp(mn, #a)) \
				mnemonic = a;		   \
			else

			MNEMONICS;
#undef MN

			printf("Got instruction %s %d\n", mn, mnemonic);

			inst_t arg;
			// printf("Parsing '%s'\n", line);
#define INST(_mn, am, op, len) \
			if (mnemonic == _mn && parse_arg(line, am, &arg)) \
			{												  \
				arg.opcode = op;							  \
				print_inst(&arg);							  \
			}												  \
			else

			INSTRUCTIONS
			{
				printf("\033[31mCould not be parsed: %s '%s'\033[0m\n", mn, line);
			}
#undef INST
		}

		num_insts++;
		line = strtok(NULL, "\r\n");
	}

	free_map(labels);

	return num_insts;
}
