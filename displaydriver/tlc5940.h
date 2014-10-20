#pragma once
#include <stdint.h>

#define NUM_TLCS 2

uint8_t* get_gs_data();
uint16_t get_gs_data_size();

void set_channel_gs(int channel, uint16_t value);
uint16_t get_channel_gs(int channel);
