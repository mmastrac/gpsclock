#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include "tlc5940.h"

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
	// No prescaling, effectively 8000000Mhz timer
	TCCR0B = (1<<CS00);
	// Enable timer interrupt
	TIMSK |= (1<<TOIE0);

	sei();

	int current_cycle = 1;

	int gs_data_start = 4096 / 8 - sizeof(gs_data);

	for (;;) {
		setHigh(BLANK);
		int current_brightness = sin(ticks() / 10000.0) * 2047.0 + 2048.0;
		current_cycle++;
		setLow(BLANK);

		for (int i = 0; i < 32; i += 2) {
			set_channel_gs(i + 0, current_brightness);
			set_channel_gs(i + 1, current_brightness);
		}
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

				if (bit) {
					setHigh(SIN);
				} else {
					setLow(SIN);
				}

				pulse(GSCLK);
			}
		}

		// for (;;) {}
	}

	return 0;
}
