#include "as.h"
#include "../cpu.h"
#include "../instructions.h"
#include "../mnemonics.h"
#include "map.h"

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

#define ERR "\033[31m"
#define RESET "\033[0m"

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
	uintptr_t num_insts = 0,
		pc = 0x600;
	uint32_t line_no = 1;
	map_t *labels = new_map();
	char *line,
		*orig_line,
		*line_start;

	printf("Assembling File\n");
	printf("%s\n", code);

	orig_line = strtok_fix(code, "\n");

	while (orig_line)
	{
		line = strdup(orig_line);
		line_start = line;
		
		if (*line == 0)
			goto end_of_line;
		printf("line %d: \033[36m%.12s\033[0m\n", line_no, line);
		
		skip_ws(&line);

		if (is_eol(*line))
		{
			printf("skip_ws() brought us to EOL\n");
			goto end_of_line;
		}
		
		char *label = parse_label(&line);
		skip_ws(&code);
		if (is_eol(*line))
			goto end_of_line;
		char *mn = parse_inst(&line);
		printf(" skipping %d ", skip_ws(&line));
		//printf("\033[33m%s\033[0m\n", line);
		
		bool no_argument = false;
		printf("eol is %c ($%x)\n", *line, *line);
		if (is_eol(*line))
		{
			no_argument = true;
			printf("... no argument\n");
		}
		int32_t mnemonic = -1;

		if (label)
		{
			map_set(labels, label, (void *)pc);
			printf("Set label %s at $%lx\n", label, pc);
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
				printf(ERR "Could not parse instruction on line %d\n%s\n" RESET, line_no, orig_line);
				goto cleanup;
			}
#undef MN

			printf("Got instruction %s %d\n", mn, mnemonic);

			inst_t arg;
			// printf("Parsing '%s'\n", line);
#define INST(_mn, am, op, len) \
			if ((no_argument && (_mn == AM_IMP || _mn == AM_ACC))		\
				 || (mnemonic == _mn && parse_arg(line, am, &arg)))		\
			{															\
				arg.opcode = op;										\
				pc += len;												\
				print_inst(&arg);										\
			}															\
			else

			INSTRUCTIONS
			{
				printf("\033[31mCould not be parsed: %s '%s'\033[0m\n", mn, line);
			}
#undef INST
		}
	end_of_line:
		line_no++;
		printf("Line is %d\n", line_no);
		orig_line = strtok_fix(NULL, "\n");
		free(line_start);
	}

cleanup:
	free_map(labels);

	return num_insts;
}
