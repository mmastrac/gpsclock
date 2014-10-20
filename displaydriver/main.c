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
uint8_t display[2] = { 'x', 'x' };

int main(void) {
	setOutput(BLANK);
	setOutput(GSCLK);
	setOutput(SIN);
	setInput(DATAIN);

	setHigh(BLANK);
	setLow(GSCLK);
	setLow(SIN);

	initialize_ticks();
	initialize_serial();

	sei();

	int current_cycle = 0;
	int gs_data_start = GS_TICKS / 8 - get_gs_data_size();
	uint8_t* gs_data = get_gs_data();

	bool lockup = false;

	uint16_t input_count = 0;

	for (;;) {
		_delay_us(10);

		setHigh(BLANK);
		uint16_t tick_cycle = ticks();
		uint16_t current_brightness = sin_cycle(tick_cycle);

		current_cycle++;

		// Protocol: S 'char' 'char'
		if (!lockup) {
			while (serial_available() >= 3) {
				uint8_t cmd = serial_read();
				if (cmd == 'S') {
					input_count++;
					if (input_count > 99)
						input_count = 0;
					display[1] = '0' + (input_count % 10);
					display[0] = '0' + (input_count / 10);
					serial_read();
					serial_read();

					if (serial_available() > 0) {
						display[0] = 'O';
						display[1] = '0' + serial_available();
						lockup = true;
						break;
					}
				} else {
					display[0] = 'E';
					display[1] = cmd;
					lockup = true;
					break;
				}
			}
		}

		for (int i = 0; i < sizeof(display); i++) {
			int glyph = font(display[i]);
			for (int j = 0; j < 16; j++) {
				if (glyph & (1 << j)) {
					set_channel_gs(j + i * 16, GS_MAX);
				} else {
					set_channel_gs(j + i * 16, 0);
				}
			}
		}

		set_channel_gs(16, current_brightness);
		setLow(BLANK);

		_delay_us(10);

		for (int i = 0; i < GS_TICKS / 8; i++) {
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
	}

	return 0;
}
