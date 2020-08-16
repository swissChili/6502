#include "as.h"

#include <stdio.h>
#include <stdlib.h>
#include <bits/getopt_core.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char c;
	FILE *in = stdin,
		*out = stdout;

	while ((c = getopt(argc, argv, "i:o:")) != -1)
	{
		switch (c)
		{
		case 'i':
			in = fopen(optarg, "r");
			break;
		case 'o':
			out = fopen(optarg, "w");
			break;
		case 'h':
		case '?':
			printf("6502 assembler\n"
				   "Usage:\n"
				   "    -i <input> set input file (default stdin)\n"
				   "    -o <output> set output file (default stdout)\n");
		}
	}

	fseek(in, 0, SEEK_END);
	ssize_t len = ftell(in);
	fseek(in, 0, SEEK_SET);

	char *text = malloc(len + 1);
	fread(text, len, 1, in);
	text[len] = 0;

	uint32_t built = assemble(text, out);

	free(text);
}
