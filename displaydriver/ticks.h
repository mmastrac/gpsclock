#pragma once
#include <stdint.h>

typedef struct ticks_t {
	uint16_t ticks;
	uint8_t epoch;
} ticks_t;

ticks_t ticks();
void initialize_ticks();
