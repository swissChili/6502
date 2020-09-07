#pragma once

#include "map.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define ERR "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

char *strtok_fix(char *string, const char *token);
uint32_t skip_ws(char **code);
char *parse_label_name(char **code);
bool ws_end(char **code);

/**
 * @returns 0 on success, error code otherwise
 */
int preproc(char *code, FILE *out, map_t *macros, int flags);

/**
 * @returns NULL on failure, printing info to stderr
 */
uint32_t assemble(char *code, FILE *out);
