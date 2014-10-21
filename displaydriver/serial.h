#pragma once
#include <stdint.h>
#include <stdbool.h>

void initialize_serial();
uint8_t serial_available();
uint8_t serial_read();
uint8_t serial_peek(uint8_t where);
void serial_drop(uint8_t count);


