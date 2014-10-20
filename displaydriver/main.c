#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bitbang.h"
#include "sin_cycle.h"
#include "font.h"
#include "tlc5940.h"
#include "ticks.h"
#include "serial.h"
#include "pins.h"

char* hex = "0123456789ABCDEF";

volatile uint8_t display[2] = { ' ', ' ' };

int main(void) {
	setOutput(BLANK);
	setOutput(GSCLK);
	setOutput(SIN);
	setInput(DATAIN);

	setHigh(BLANK);

	initialize_ticks();
	initialize_serial();

	sei();

	int current_cycle = 0;
	int gs_data_start = 4096 / 8 - get_gs_data_size();
	uint8_t* gs_data = get_gs_data();

	for (;;) {
		_delay_us(10);

		setHigh(BLANK);
		uint16_t tick_cycle = ticks();
		uint16_t current_brightness = sin_cycle(tick_cycle);

		current_cycle++;

		// uint16_t display_value = input_count;
		// display[0] = hex[(display_value >> 4) & 0xf];
		// display[1] = hex[display_value & 0xf];

		for (int i = 0; i < sizeof(display); i++) {
			int glyph = font(display[i]);
			for (int j = 0; j < 16; j++) {
				if (glyph & (1 << j)) {
					set_channel_gs(j + i * 16, 4095);
				} else {
					set_channel_gs(j + i * 16, 0);
				}
			}
		}

		set_channel_gs(16, current_brightness);
		setLow(BLANK);

		_delay_us(10);

		for (int i = 0; i < 4096 / 8; i++) {
			int byte;
			if (i >= gs_data_start) {
				byte = gs_data[i - gs_data_start];
			} else {
				byte = 0;
			}

			for (int j = 0; j < 8; j++) {
				int bit = byte & (1 << 7);
				byte <<= 1;

				setState(SIN, bit);
				_delay_loop_1(2);
				setHigh(GSCLK);
				_delay_loop_1(2);
				setLow(GSCLK);
			}
		}

		// for (;;) {}
	}

	return 0;
}
