#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include "tlc5940.h"

#define BLANK_DDR DDRB
#define BLANK_PORT PORTB
#define BLANK_PIN 0

#define GSCLK_DDR DDRB
#define GSCLK_PORT PORTB
#define GSCLK_PIN 1

#define SIN_DDR DDRB
#define SIN_PORT PORTB
#define SIN_PIN 2

int main(void) {
	setOutput(BLANK);
	setOutput(GSCLK);
	setOutput(SIN);

	setHigh(BLANK);

	int current_cycle = 1;

	for (;;) {
		int current_brightness = sin(current_cycle / 20.0) * 2047.0 + 2048.0;
		current_cycle++;

		pulse(BLANK);

		for (int i = 0; i < 4096 - 12; i++) {
			pulse(GSCLK);
		}
		int brightness = current_brightness;
		for (int i = 0; i < 12; i++) {
			if (brightness & (1 << 11))
				setHigh(SIN);
			else
				setLow(SIN);
			brightness <<= 1;
			_delay_us(10);
			pulse(GSCLK);
		}

		// for (;;) {}
	}

	return 0;
}
