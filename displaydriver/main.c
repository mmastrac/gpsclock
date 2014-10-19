#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include "tlc5940.h"
#include "sin_cycle.h"
#include "font.h"

#define NUM_TLCS 2

// BLANK + XLAT
#define BLANK_PORT B
#define BLANK_PIN 0

// GSCLK + SCLK
#define GSCLK_PORT B
#define GSCLK_PIN 1

// SIN
#define SIN_PORT B
#define SIN_PIN 2

// DATAIN
#define DATAIN_PORT B
#define DATAIN_PIN 3

// 16 channels worth of grayscale data (12 bits)
uint8_t gs_data[24 * NUM_TLCS] = { 0 };

uint8_t display[2] = { '1', '2' };

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

volatile uint32_t ticks_ = 0;

ISR(TIMER0_OVF_vect) {
	ticks_++;
}

uint32_t ticks() {
	cli();
	uint32_t tmp = ticks_;
	sei();
	return tmp;
}

int main(void) {
	setOutput(BLANK);
	setOutput(GSCLK);
	setOutput(SIN);
	setInput(DATAIN);

	setHigh(BLANK);

	// Normal (wrap) mode
	TCCR0A = 0;
	// Prescale by 1024, effectively 8000Hz timer
	// TCCR0B = (1<<CS02) | (1<<CS00);
	// Prescale by 64, effectively 125kHz timer
	TCCR0B = (1<<CS01) | (1<<CS00);
	// No prescaling, effectively 8000000Mhz timer
	// TCCR0B = (1<<CS00);
	// Enable timer interrupt
	TIMSK |= (1<<TOIE0);

	sei();

	int current_cycle = 0;
	int gs_data_start = 4096 / 8 - sizeof(gs_data);

	for (;;) {
		_delay_us(10);

		setHigh(BLANK);
		uint32_t ticks_now = ticks();
		uint16_t tick_cycle = abs(ticks_now % 1024);
		uint16_t current_brightness = pgm_read_word(&SIN_CYCLE[tick_cycle]);
		setLow(BLANK);

		int a = pgm_read_word(&FONT['A']);
		int b = pgm_read_word(&FONT['B']);

		for (int i = 0; i < sizeof(display); i++) {
			int glyph = pgm_read_word(&FONT[display[i]]);
			for (int j = 0; j < 16; j++) {
				if (glyph & (1 << j)) {
					set_channel_gs(j + i * 16, 4095);
				} else {
					set_channel_gs(j + i * 16, 0);
				}
			}
		}

		set_channel_gs(0, current_brightness);

		_delay_us(10);

		for (int i = 0; i < 4096 / 8; i++) {
			int byte;
			if (i >= gs_data_start) {
				byte = gs_data[i - gs_data_start];
			} else {
				byte = 0;
			}

			for (int j = 0; j < 8; j++) {
				if (readState(DATAIN)) {
					// This is where the 1-wire code goes
				}

				int bit = byte & (1 << 7);
				byte <<= 1;

				setState(SIN, bit);
				pulse(GSCLK);
			}
		}

		// for (;;) {}
	}

	return 0;
}
