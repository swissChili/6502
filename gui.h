#pragma once

#include "cpu.h"

// void gui(cpu_t *cpu);
void start_gui(mqd_t mq_to_cpu, cpu_t *cpu);
