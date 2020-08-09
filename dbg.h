#pragma once

#include "cpu.h"

#include <stdbool.h>
#include <mqueue.h>

bool debug_stmt(cpu_t *cpu, char *input, bool *running);
// void debug_prompt(mqd_t mq, cpu_t *cpu);
pthread_t start_debug_prompt(mqd_t mq, cpu_t *cpu);