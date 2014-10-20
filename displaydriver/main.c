#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

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
#define DATAIN_PIN 4

#define BIT_SET(reg, bit) reg |= _BV(bit)
#define BIT_CLEAR(reg, bit) reg &= ~_BV(bit)

char* hex = "0123456789ABCDEF";

// 16 channels worth of grayscale data (12 bits)
uint8_t gs_data[24 * NUM_TLCS] = { 0 };

volatile uint8_t display[2] = { 'z', '2' };

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

// Tick counter

volatile uint32_t ticks_ = 0;

ISR(TIMER1_OVF_vect) {
	ticks_++;
}

uint16_t ticks() {
	cli();
	uint16_t tmp = ticks_ & 1023;
	sei();
	return tmp;
}

// Serial 

volatile uint8_t uart_buffer[32] = { 0 };
volatile uint8_t* uart_start = &uart_buffer[0];
volatile uint8_t* uart_end = &uart_buffer[0];
volatile uint8_t serial_value = 0;
volatile uint8_t serial_counter = 0;

ISR(PCINT0_vect) {
	int dataIn = readState(DATAIN);

	// Serial start bit is a zero, so we'll see this as a pin-change interrupt 
	// with a read of zero
	if (!dataIn) {
		// Start bit detection

		// Disable the pin-change interrupt as we transition to timers
		BIT_CLEAR(PCMSK, PCINT4);

		// Reset the timer
		TCNT0 = 0;

		// Start the timer in CTC mode
		TCCR0B = _BV(CS01);

		serial_value = 0;
		serial_counter = 0;
	}
}

ISR(TIMER0_COMPA_vect) {
	// Restart the timer
	TCNT0 = 0;

	if (serial_counter == 8) {
		*uart_end = serial_value;
		uart_end++;
		if (uart_end > uart_buffer + sizeof(uart_buffer))
			uart_end = uart_buffer;

		display[0] = display[1];
		display[1] = serial_value & 0x7f;
		// display[0] = hex[serial_value >> 4 & 0xf];
		// display[1] = hex[serial_value & 0xf];

		// Stop bit is a one

		// Disable timer
		TCCR0B = 0;
		// Re-enable pin-change interrupt
		BIT_CLEAR(GIFR, PCIF);
		BIT_SET(PCMSK, PCINT4);
	} else {
		// Clock in the serial bit
		serial_value |= (readState(DATAIN) ? 1 : 0) << serial_counter;
	}

	serial_counter++;
}

bool serial_available() {
	cli();
	bool ret = uart_end != uart_start;
	sei();

	return ret;
}

uint8_t serial_read() {
	cli();
	uint8_t ret = *uart_start;
	uart_start++;
	if (uart_start > uart_buffer + sizeof(uart_buffer))
		uart_start = uart_buffer;
	sei();

	return ret;
}

int main(void) {
	setOutput(BLANK);
	setOutput(GSCLK);
	setOutput(SIN);
	setInput(DATAIN);

	setHigh(BLANK);

	// Tick timer
	// Use timer1, since it's a bizarre timer that doesn't match other AVRs

	// Prescale by 64 so that 1024*256 timer ticks is ~1 sec
	// I thought we'd need to set CTC1 here, but it doesn't seem to be so 
	// (and the timer doesn't hit OVF if that's the case)
	TCCR1 = _BV(PWM1A) | _BV(CS12) | _BV(CS11) | _BV(CS10);
	OCR1C = 244;

	// Restart/reset the timer
	GTCCR = _BV(PSR1);
	TCNT1 = 0;

	// Serial timer
	// CPU clock is 16,000,000Hz, period is 62.5ns
	// Baud rate is 9600, period is 104166.67ns (104us)
	
	// We're going to set the timer divisor to 8 which gives us a timer period of 500ns 
	// and use OCR0A to trigger the appropriate intervals

	// CTC mode, OCRA is top
	TCCR0A = _BV(WGM01);
	// Timer interrupts every 104000ns (slightly more often than the 104166.7ns we need)
	OCR0A = 208;
	// Disable the timer off the bat
	TCCR0B = 0;

	// Enable timer interrupts -- overflow for timer0 and timer1
	TIMSK = _BV(OCIE0A) | _BV(TOIE1);

	// Turn on pin-change interrupt for port 4
	PCMSK = _BV(PCINT4);
	GIMSK = _BV(PCIE);

	sei();

	int current_cycle = 0;
	int gs_data_start = 4096 / 8 - sizeof(gs_data);

	for (;;) {
		_delay_us(10);

		setHigh(BLANK);
		uint16_t tick_cycle = ticks();
		uint16_t current_brightness = pgm_read_word(&SIN_CYCLE[tick_cycle]);

		current_cycle++;

		// uint16_t display_value = input_count;
		// display[0] = hex[(display_value >> 4) & 0xf];
		// display[1] = hex[display_value & 0xf];

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
