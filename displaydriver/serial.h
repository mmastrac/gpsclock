#pragma once
#include <stdint.h>
#include <stdbool.h>

void initialize_serial();
uint8_t serial_available();
uint8_t serial_read();
