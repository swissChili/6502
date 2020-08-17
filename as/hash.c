#include "hash.h"

uint32_t hash(char *str)
{
	uint32_t hash = 5381;
	char c;

	while (c = *str++)
	{
		hash = (hash << 5) + hash + c;
	}

	return hash;
}
