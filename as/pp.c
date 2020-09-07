#include "as.h"
#include "map.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>

enum /* State */
{
	NORMAL,			/* Default State */
	MACRO_DEF		/* In Macro Definition */
};

enum /* Flags */
{
	PRESERVE_MACRO_ARGS,
};

int preproc(char *in, FILE *out, map_t *macros, int flags)
{
	int line_no = 1,
		err = 0,
		state = NORMAL;
	char *line = strtok_fix(in, "\n"),
		*macro_name = NULL;

	while (line)
	{
		skip_ws(&line);

		if (*line == '%')
		{
			// Line is preprocessor directive
			
			char *directive = parse_label_name(&line);
			if (!directive)
			{
				fprintf(stderr, ERR "Expected preprocessor directive on line %d\n" RESET,
						line_no);
				err = EINVAL;
				goto cleanup;
			}
			else
			{
				if (!strcasecmp(directive, "macro"))
				{
					skip_ws(&line);
					macro_name = parse_label_name(&line);
					if (!macro_name)
					{
						fprintf(stderr, ERR "Expected name after %%macro on line %d\n" RESET,
								line_no);
						err = EINVAL;
						goto cleanup;
					}

					if (!ws_end(&line))
					{
						fprintf(stderr, ERR "Expected new line after macro definition\n" RESET);
						err = EINVAL;
						goto cleanup;
					}

					state = MACRO_DEF;
				}
			}
		}

		fprintf(out, "%s\n", line);
		
		line = strtok_fix(NULL, "\n");
		line_no++;
	}
	
cleanup:
	return err;
}
