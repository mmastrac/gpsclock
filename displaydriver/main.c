#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include "tlc5940.h"

#define NUM_TLCS 4

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

void set_channel_gs(int channel, int value) {
	// The lower array index contains the higher bits
	if (channel & 1) {
		gs_data[24 * NUM_TLCS - channel - 1] = (gs_data[24 * NUM_TLCS - channel - 1] & 0x0f) | ((value & 0xf) << 4);
		gs_data[24 * NUM_TLCS - channel - 2] = value >> 4;
	} else {
		gs_data[24 * NUM_TLCS - channel - 1] = value & 0xff;
		gs_data[24 * NUM_TLCS - channel - 2] = (gs_data[24 * NUM_TLCS - channel - 2] & 0xf0) | (value >> 8);
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

	int gs_data_start = 4096 - sizeof(gs_data) * 8;

	for (;;) {
		setHigh(BLANK);
		int current_brightness = sin(ticks() / 10000.0) * 2047.0 + 2048.0;
		current_cycle++;
		setLow(BLANK);

		set_channel_gs(0, current_brightness);

		for (int i = 0; i < 4096; i++) {
			if (readState(DATAIN)) {

			}

			int bit;
			if (i > gs_data_start) {
				int gs_ofs = i - gs_data_start;
				int byte = gs_data[gs_ofs >> 3];
				bit = byte & (1 << (7 - (gs_ofs & 7)));
			} else {
				bit = 0;
			}

			if (bit) {
				setHigh(SIN);
			} else {
				setLow(SIN);
			}

			pulse(GSCLK);
		}

		// for (;;) {}
	}

	return 0;
}
