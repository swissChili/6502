#pragma once

#include <stdio.h>
#include <stdint.h>

/*
 * @returns NULL on failure, printing info to stderr
 */
uint32_t assemble(char *code, FILE *out);
