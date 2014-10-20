#include "tlc5940.h"

// 16 channels worth of grayscale data per TLC (12 bits)
uint8_t gs_data[24 * NUM_TLCS] = { 0 };

uint8_t* get_gs_data() {
	return gs_data;
}

uint16_t get_gs_data_size() {
	return sizeof(gs_data);
}

void set_channel_gs(int channel, uint16_t value) {
	uint8_t* base = gs_data + sizeof(gs_data) - ((channel >> 1) * 3);

	// The lower array index contains the higher bits
	if (channel & 1) {
		base[-3] = value >> 4;
		base[-2] = value << 8 | (base[-2] & 0x0f);
	} else {
		base[-2] = (value >> 8) | (base[-2] & 0xf0);
		base[-1] = value & 0xff;
	}
}

uint16_t get_channel_gs(int channel) {
	uint8_t* base = gs_data + sizeof(gs_data) - ((channel >> 1) * 3);

	if (channel & 1) {
		return (base[-3] << 4) | (base[-2] >> 4);
	} else {
		return (base[-2] << 8) | base[-1];
	}

}
