#pragma once


#include <stdint.h>
#include <stdlib.h>
#include "hash.h"

/* 1kb seems reasonable doesn't it? */
#define MAP_ALLOC_SIZE (1 * 1024)

struct map_node
{
    char *key;
    void *val;
    int used : 1;
    uint32_t h;
};

typedef struct map_node map_node;

#define EMPTY_NODE { NULL, NULL, 0, 0 }

/**
 * @brief A simple hashmap.
 *
 * Allocate map with new_map() and free it with free_map(). Items can be added
 * with map_set() or the MAP_SET() macro. map_exists() can be used to check if
 * an item exists in the map, map_get() and MAP_GET() can be used to retrieve
 * items.
 */
struct map
{
    uint64_t len;
    map_node *items;
    uint64_t full;
    uint64_t count;
};

typedef struct map map_t;

/**
 * @deprecated Used map_exists instead
 */
#define MAP_USED_AT(m, i) (m[i].used)

/**
 * @brief Allocate a new map on the heap
 * @return A pointer to the allocated map
 */
map_t *new_map();

/**
 * @brief Allocate a map on the heap with a specific capacity. Map will be
 *        reallocated if this capacity is exceeded.
 * @return A pointer to the allocated map
 */
map_t *new_map_sized(uint64_t);

/**
 * @brief Free a map on the heap
 * @param m The map to free
 */
void free_map(map_t *m);

/**
 * @brief Free a map and its items
 * @param m The map to free
 */
void free_map_items(map_t *m);

/**
 * @brief Set a certain value in the map
 * @param m The map
 * @param k The key whose value should be changed
 * @param v The new value
 */
void map_set(map_t *m, char *k, void *v);

/**
 * @brief Get the value in a map by it's key
 * @param m The map
 * @param k The key
 * @return The value at k
 */
void *map_get(map_t *m, char *k);

/**
 * @brief Check if a key is used in a map
 * @param m The map
 * @param k The key
 * @return 1 if the key is used, 0 otherwise
 */
int map_exists(map_t *m, char *k);

/**
 * @brief Print a map's contents to stdout
 * @warning Should not be used in production
 * @param m The map
 */
void map_debug(map_t *m);

#define MAP_GET(t, m, k) *(t *)map_get(m, k)

/**
 * @brief Helper macro to allocate memory for a value on the heap and store it
 *		  at a given key
 * @warning This is unsafe and should be used with caution, especially for more
 *			complex types
 * @param m The map
 * @param k The key at which to insert the value
 * @param v The value
 */
#define MAP_SET(m, k, v)										 \
	{															 \
		__typeof__(v) *__map_temp = malloc(sizeof(__typeof(v))); \
		*__map_temp = v;										 \
		map_set(m, k, __map_temp);								 \
	}

