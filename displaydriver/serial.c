#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/cpufunc.h>
#include "serial.h"
#include "bitbang.h"
#include "pins.h"
#include "calibrate.h"

#include <stdio.h>
#include <string.h>

// Serial 

#define SERIAL_SIZE 32

#if F_CPU == 16000000UL

/*
 * 16 MHz
 */

// CPU clock is 16,000,000Hz, period is 62.5ns
// Baud rate is 9600, period is 104166.67ns (104us)

// We're going to set the timer divisor to 8 which gives us a timer period of 500ns 
// and use OCR0A to trigger the appropriate intervals
#define SERIAL_CLOCK_DIVISOR 	_BV(CS01)

// Timer interrupts every 104000ns (slightly more often than the 104166.7ns we need)
#define SERIAL_TIMER_COMPARE 	208

#elif F_CPU == 8000000UL

/*
 * 8 MHz
 */

// CPU clock is 8,000,000Hz, period is 125ns
// Baud rate is 9600, period is 104166.67ns (104us)

// We're going to set the timer divisor to 8 which gives us a timer period of 1000ns 
// and use OCR0A to trigger the appropriate intervals
#define SERIAL_CLOCK_DIVISOR 	_BV(CS01)

// Timer interrupts every 104000ns (slightly more often than the 104166.7ns we need)
#define SERIAL_TIMER_COMPARE 	104

#else

#error Unknown FCPU

#endif

volatile uint8_t uart_buffer[SERIAL_SIZE] = { 0 };
uint8_t uart_start = 0;
volatile uint8_t uart_end = 0;

// The serial value we are clocking in
volatile uint8_t serial_value = 0;
volatile uint8_t serial_counter = 0;

inline void _serial_push(uint8_t value);

ISR(PCINT0_vect) {
	int dataIn = readState(DATAIN);

	// Serial start bit is a zero, so we'll see this as a pin-change interrupt 
	// with a read of zero
	if (!dataIn) {
		// Start bit detection

		// Disable the pin-change interrupt as we transition to timers
		clearBit(PCMSK, PCINT4);

		// Reset the timer
		TCNT0 = 0;

		// Start the timer in CTC mode
		TCCR0B = SERIAL_CLOCK_DIVISOR;

		serial_value = 0;
		serial_counter = 0;
	}
}

ISR(TIMER0_COMPA_vect) {
	// Restart the timer
	TCNT0 = 0;

	if (serial_counter == 8) {
		// Stop bit is a one
		_serial_push(serial_value);

		// Disable timer
		TCCR0B = 0;
		// Re-enable pin-change interrupt
		clearBit(GIFR, PCIF);
		setBit(PCMSK, PCINT4);
	} else {
		// Clock in the serial bit
		serial_value |= (readState(DATAIN) ? 1 : 0) << serial_counter;
	}

	serial_counter++;
}

ISR(TIMER0_COMPB_vect) {
}

void _serial_push(uint8_t value) {
	uart_buffer[uart_end] = value;
	uart_end++;
	if (uart_end >= SERIAL_SIZE)
		uart_end = 0;
}

uint8_t serial_available() {
	cli();
	uint8_t end = uart_end;
	sei();

	if (end == uart_start)
		return 0;

	if (end > uart_start)
		return end - uart_start;

	return SERIAL_SIZE - (uart_start - end);
}

void serial_drop(uint8_t count) {
	uart_start = (uart_start + count) % SERIAL_SIZE;
}

uint8_t serial_peek(uint8_t where) {
	return uart_buffer[(uart_start + where) % SERIAL_SIZE];
}

uint8_t serial_read() {
	uint8_t ret = uart_buffer[uart_start];
	uart_start++;
	if (uart_start >= SERIAL_SIZE)
		uart_start = 0;

	return ret;
}

void bitbang_write(char* buffer, uint8_t length) {
	for (int i = 0; i < length; i++) {
		serial_value = buffer[i];
		serial_counter = 0;

		// Note that we're writing really slow just in case our timing is off -- this 
		// lets a serial receiver take its time and understand what we sent
		while (serial_counter < 20) {
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

uint16_t calibrate_test(uint8_t low, uint8_t high, uint8_t guess, uint8_t* best_score, uint16_t* best_calibration) {
	char buffer[32];

	uint8_t original_calibration = OSCCAL;
	OSCCAL = guess;
	uint16_t calibration = calibrate();
	OSCCAL = original_calibration;

	sprintf(buffer, "OSCCAL = %x < %x < %x\r\n", low, guess, high);
	bitbang_write_string(buffer);

	sprintf(buffer, "C: %u\r\n", calibration);
	bitbang_write_string(buffer);

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

void initialize_serial() {
	char buffer[32];

	// Serial timer (see comments above)

	// CTC mode, OCRA is top
	TCCR0A = _BV(WGM01);
	// Timer interrupts for each serial bit
	OCR0A = SERIAL_TIMER_COMPARE;
	// Enable the timer for bit-bang serial
	TCCR0B = SERIAL_CLOCK_DIVISOR;

	bitbang_write_string("*\r\n*** OSCCAL serial calibration: send '~' until 'Done.'\r\n");
	sprintf(buffer, "OSCCAL = 0x%x\r\n", OSCCAL);
	bitbang_write_string(buffer);

	uint8_t best_score = 0;
	uint16_t best_calibration = 0;

	// Calibrate low and high range
	calibrate_binary(&best_score, &best_calibration);

	sprintf(buffer, "Final = 0x%x, c: %u\r\n", best_score, best_calibration);
	bitbang_write_string(buffer);

	OSCCAL = best_score;

	bitbang_write_string("Done.\r\n");

	// Disable the timer now
	TCCR0B = 0;
	// Enable timer interrupts -- overflow for timer0 and timer1
	TIMSK = _BV(OCIE0A) | _BV(TOIE1);

	// Turn on pin-change interrupt for port 4
	PCMSK = _BV(PCINT4);
	GIMSK = _BV(PCIE);	
}
