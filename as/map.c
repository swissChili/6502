#include "map.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef MAP_DEBUG
#define MAP_DEBUG_PRINTF printf
#else
#define MAP_DEBUG_PRINTF // nothing
#endif

map_t *new_map_sized(uint64_t size)
{
	map_t *m = malloc(sizeof(map_t));

	/*
	 * Calloc zeroes the memory, which is what we want.
	 */

	map_node empty = EMPTY_NODE;

	m->items = calloc(size, sizeof(map_node));
	m->len = MAP_ALLOC_SIZE;
	m->full = 0;

	for (int i = 0; i < m->len; i++)
	{
		m->items[i] = empty;
	}

	return m;
}

map_t *new_map()
{
	return new_map_sized(MAP_ALLOC_SIZE);
}

void free_map(map_t *m)
{
	free(m->items);
	free(m);
}

void free_map_items(map_t *m)
{
	for (int i = 0; i < m->len; i++)
	{
		if (m->items[i].key != NULL)
		{
			free(m->items[i].key);
			free(m->items[i].val);
		}
	}
	free(m->items);
	free(m);
}

void map_set(map_t *m, char *k, void *v)
{
	uint64_t h = hash(k);
	uint32_t i = h % m->len;

	MAP_DEBUG_PRINTF("hash %u i %u\n", h, i);

	m->count++;

	if (m->full++ < m->len)
	{
		map_node val =
				{
						malloc(strlen(k)),
						v,
						1,
						h,
				};

		strcpy(val.key, k);

		uint32_t current = i;

		while (MAP_USED_AT(m->items, current))
		{
			if (current == m->len - 1)
			{
				current = 0;
			}
			else
			{
				current++;
			}
		}

		MAP_DEBUG_PRINTF("Current %d\n", current);

		m->items[current] = val;

		MAP_DEBUG_PRINTF("val %d\n", *(int *) m->items[current].val);
	}
	else
	{
		/* resize map */
		map_t *n_m = new_map_sized(m->len * 2);

		for (uint64_t j = 0; j < m->len; j++)
		{
			if (!MAP_USED_AT(m->items, j))
			{
				continue;
			}

			uint32_t current = m->items[j].h;

			while (MAP_USED_AT(m->items, current))
			{
				if (current == m->len - 1)
				{
					current = 0;
				}
				else
				{
					current++;
				}
			}

			n_m->items[current] = m->items[j];
		}

		free_map(m);
		m = n_m;
	}
}

int map_exists(map_t *m, char *k)
{
	uint64_t h = hash(k);
	uint32_t i = h % m->len;

	uint32_t current = i;

	while (m->items[current].used)
	{
		if (strcmp(m->items[current].key, k) == 0)
		{
			return 1;
		}

		if (current >= m->len - 1)
		{
			current = 0;
			MAP_DEBUG_PRINTF("Current reset to 0\n");
		}
		else
		{
			current++;
			MAP_DEBUG_PRINTF("Incrementing current\n");
		}
	}

	return 0;
}

void *map_get(map_t *m, char *k)
{
	uint64_t h = hash(k);
	uint32_t i = h % m->len;

	uint32_t current = i;

	MAP_DEBUG_PRINTF("%s should be %s\n", m->items[current].key, k);
	MAP_DEBUG_PRINTF("streq %d\n", strcmp(m->items[current].key, k));

	MAP_DEBUG_PRINTF("Current get %d\n", current);

	while (strcmp(m->items[current].key, k) != 0)
	{
		MAP_DEBUG_PRINTF("While at %d\n", current);
		if (current >= m->len - 1)
		{
			current = 0;
			MAP_DEBUG_PRINTF("Current reset to 0\n");
		}
		else
		{
			current++;
			MAP_DEBUG_PRINTF("Incrementing current\n");
		}
	}

	MAP_DEBUG_PRINTF("val %d\n", *(int *) m->items[current].val);

	return m->items[current].val;

	//return 1;
}

void map_debug(map_t *m)
{
	for (int i = 0; i < m->len; i++)
	{
		if (m->items[i].val != NULL)
		{
			MAP_DEBUG_PRINTF("i = %d, k = %s, v = %d\n",
							 i, m->items[i].key, *(int *) m->items[i].val);
		}
	}
}
