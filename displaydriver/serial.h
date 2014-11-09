#pragma once
#include <stdint.h>
#include <stdbool.h>

void initialize_serial();
uint8_t serial_available();
uint8_t serial_read();
uint8_t serial_peek(uint8_t where);
void serial_drop(uint8_t count);
void serial_write(uint8_t c);
void serial_write_buffer(uint8_t* buffer, uint8_t length);
void serial_write_string(char* buffer);
