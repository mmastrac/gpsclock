#include <avr/interrupt.h>
#include <avr/io.h>
#include "serial.h"
#include "bitbang.h"
#include "pins.h"

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
		clearBit(PCMSK, PCINT4);

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
		// Stop bit is a one
		*uart_end = serial_value;
		uart_end++;
		if (uart_end > uart_buffer + sizeof(uart_buffer))
			uart_end = uart_buffer;

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

void initialize_serial() {
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
}
