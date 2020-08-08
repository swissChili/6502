#pragma once

#include <stdint.h>
#include <string.h>

#define __FILENAME__ \
	(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 \
		: strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)


#define ASSERT(message, body) \
	{ \
		if (!body) \
			THROW("Assert failed: " message " [" #body "]"); \
	}

#define MAX_CATCH_LEN 32

typedef void (* handle_t)(intptr_t);

typedef struct
{
	handle_t fn;
	intptr_t arg;
} catch_t;

extern catch_t g_catch[MAX_CATCH_LEN];
extern unsigned g_catch_len;

void throw_(const char *msg, const char *file, unsigned int line);
void catch_(handle_t hdl, intptr_t arg);

#define THROW(msg) throw_(msg, __FILENAME__, __LINE__)
#define CATCH(fn, arg) catch_((handle_t) fn, (intptr_t) arg)
