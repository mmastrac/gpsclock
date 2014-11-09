#include <avr/io.h>
#include <stdbool.h>
#include <string.h>
#include "calibrate.h"
#include "pins.h"
#include "bitbang.h"

#if 1
#define SERIAL_DEBUG(fmt, ...) {}
#else
#include <stdio.h>
#define SERIAL_DEBUG(fmt, ...) { \
	char serial_debug_buffer[32]; \
	sprintf(serial_debug_buffer, fmt, __VA_ARGS__); \
	bitbang_write_string(serial_debug_buffer); \
}
#endif

// We can bit-bang serial after calibration more quickly
bool calibrated = false;

void bitbang_write(char* buffer, uint8_t length) {
	// If the oscillator has been calibrated, we can blast bits out as fast 
	// as we want 
	uint8_t ticks = calibrated ? 10 : 20;

	uint8_t serial_value, serial_counter;

	for (int i = 0; i < length; i++) {
		serial_value = buffer[i];
		serial_counter = 0;

		// Note that we're writing really slow just in case our timing is off -- this 
		// lets a serial receiver take its time and understand what we sent
		while (serial_counter < ticks) {
			TCNT0 = 0;
			setBit(TIFR, OCF0A); // To clear it, write a one

			while (!readBit(TIFR, OCF0A));

			switch (serial_counter) {
				case 0:
					setLow(DATAOUT);
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					setState(DATAOUT, serial_value & 1);
					serial_value >>= 1;
					break;
				case 9:
					setHigh(DATAOUT);
					break;
			}
		
			serial_counter++;
		}
	}
}

void bitbang_write_string(char* buffer) {
	bitbang_write(buffer, strlen(buffer));
}

uint16_t score(uint16_t calibration) {
	if (calibration < IDEAL_CALIBRATION)
		return IDEAL_CALIBRATION - calibration;
	return calibration - IDEAL_CALIBRATION;
}

uint16_t calibrate_test(__attribute__((__unused__)) uint8_t low, __attribute__((__unused__)) uint8_t high, uint8_t guess, uint8_t* best_score, uint16_t* best_calibration) {
	uint8_t original_calibration = OSCCAL;
	OSCCAL = guess;
	uint16_t calibration = calibrate();
	OSCCAL = original_calibration;

	bitbang_write_string(".");
	SERIAL_DEBUG("OSCCAL = %x < %x < %x\r\n", low, guess, high);
	SERIAL_DEBUG("C: %u\r\n", calibration);

	if (score(calibration) < score(*best_calibration)) {
		*best_score = guess;
		*best_calibration = calibration;
	}

	return calibration;
}

void calibrate_range(uint8_t low, uint8_t high, uint8_t* best_score, uint16_t* best_calibration) {
	for (;;) {
		uint8_t guess = (high + low) >> 1;
		uint16_t calibration = calibrate_test(low, high, guess, best_score, best_calibration);

		if (calibration < IDEAL_CALIBRATION - 1) {
			low = guess + 1;
		} else if (calibration > IDEAL_CALIBRATION + 1) {
			high = guess - 1;
		} else {
			// Nailed it!
			break;
		}

		if (low >= high) {
			// On top of the binary search we'll also check the neighbors
			calibrate_test(guess - 1, guess + 1, guess - 1, best_score, best_calibration);
			calibrate_test(guess - 1, guess + 1, guess + 1, best_score, best_calibration);
			break;
		}
	}
}

void calibrate_binary(uint8_t* best_score, uint16_t* best_calibration) {
	calibrate_range(0, 0x7f, best_score, best_calibration);
	calibrate_range(0x80, 0x9f, best_score, best_calibration);
}

void calibrate_dumb(uint8_t* best_score, uint16_t* best_calibration) {
	for (uint8_t i = 0; i < 255; i++) {
		calibrate_test(i, i, i, best_score, best_calibration);
	}
}

void calibrate_serial() {
	bitbang_write_string("*\r\n*** OSCCAL serial calibration: send '~' until 'done'\r\n");
	SERIAL_DEBUG("OSCCAL = 0x%x\r\n", OSCCAL);

	uint8_t best_score = 0;
	uint16_t best_calibration = 0;

	// Calibrate low and high range
	calibrate_binary(&best_score, &best_calibration);

	SERIAL_DEBUG("Final = 0x%x, c: %u\r\n", best_score, best_calibration);

	OSCCAL = best_score;

	calibrated = true;
	bitbang_write_string("\r\n*** OSCCAL serial calibration complete.\r\n");
	bitbang_write_string("Done.\r\n\r\n");
}
