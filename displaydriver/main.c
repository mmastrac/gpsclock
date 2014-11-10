#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/wdt.h>

#include "bitbang.h"
#include "sin_cycle.h"
#include "font.h"
#include "tlc5940.h"
#include "ticks.h"
#include "serial.h"
#include "pins.h"
#include "hex_digits.h"

uint8_t mode = 0;
uint8_t display[4] = { 'v', '1', '0', '0' };

uint8_t scroll_buffer[16] = { 0 };
uint8_t scrolling_counter = 0;
uint8_t scrolling_length = 0;

bool lockup = false;

void soft_reset() {
	do {
	    wdt_enable(WDTO_15MS);  
	    for(;;) {}
	} while(0);
}

void process_serial() {
	// A packet is minimum three bytes (sync, length, cmd)
	while (serial_available() >= 3) {
		uint8_t sync_byte = serial_peek(0);
		if (sync_byte != 0xff) {
			if (sync_byte == '~') {
				soft_reset();
			}

			display[0] = 'E';
			display[1] = sync_byte;
			break;
		}

		uint8_t packet_length = serial_peek(1);

		// Do we even have enough data to read the packet?
		if (serial_available() < packet_length + 2) {
			// Nope, leave the data there for the next cycle
			break;
		}

		// We're committed now
		serial_drop(2);

		uint8_t cmd = serial_read();
		packet_length--;

		switch (cmd) {
			// Set the display explictly
			case 'S': {
				for (uint8_t i = 0; i < sizeof(display); i++) {
					if (packet_length) {
						display[i] = serial_read();
						packet_length--;
					}
					else {
						display[i] = ' ';
					}
				}
				break;
			}
			// Set the scrolling buffer
			case 'B': {
				for (uint8_t i = 0; i < packet_length; i++) {
					scroll_buffer[i] = serial_read();
				}
				scrolling_length = packet_length;
				scrolling_counter = 0;
				mode = 1;
				break;
			}
			default: {
				display[0] = 'C';
				display[1] = cmd;
				break;
			}
		}
	}
}

void write_gs() {
	int gs_data_start = GS_TICKS / 8 - get_gs_data_size();
	uint8_t* gs_data = get_gs_data();

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
			// _delay_loop_1(2);
			setHigh(GSCLK);
			// _delay_loop_1(2);
			setLow(GSCLK);
		}
	}
}

int main(void) {
    MCUSR = 0;
    wdt_disable();

	setOutput(BLANK);
	setOutput(GSCLK);
	setOutput(SIN);
	setInput(DATAIN);
	setOutput(DATAOUT);

	setHigh(BLANK);
	setLow(GSCLK);
	setLow(SIN);

	initialize_ticks();
	initialize_serial();

	sei();

	// for (;;) {
	// 	if (serial_available()) {
	// 		uint8_t c = serial_read();
	// 		if (c == '\r') {
	// 			serial_write_string("\r\n");
	// 		} else {
	// 			serial_write(pgm_read_word(&hex[c >> 4]));
	// 			serial_write(pgm_read_word(&hex[c & 0xf]));
	// 		}
	// 	}

	// 	if (tick_epoch() != current_tick_epoch) {
	// 		// serial_write('.');
	// 		current_tick_epoch = tick_epoch();
	// 	}
	// }

	int current_cycle = 0;
	uint8_t current_tick_epoch = 0;

	for (;;) {
		_delay_us(10);

		setHigh(BLANK);
		ticks_t now = ticks();
		uint8_t tick_epoch_cycle = now.epoch;
		uint16_t current_brightness = sin_cycle(now.ticks);

		current_cycle++;

		if (!lockup) {
			if (tick_epoch_cycle != current_tick_epoch) {
				current_tick_epoch = tick_epoch_cycle;
				if (mode == 1) {
					scrolling_counter++;
					if (scrolling_counter == scrolling_length - 1) {
						mode = 0;
					}
				}
			}

			process_serial();
		}

		// display[0] = hex_hi(now.epoch);
		// display[1] = hex_lo(now.epoch);

		for (uint8_t i = 0; i < sizeof(display); i++) {
			int glyph = font((mode == 1) ? scroll_buffer[scrolling_counter + i] : display[i]);
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

		write_gs();
	}

	return 0;
}
