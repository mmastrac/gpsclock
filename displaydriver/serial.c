#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/cpufunc.h>
#include <string.h>
#include <stdbool.h>
#include "serial.h"
#include "bitbang.h"
#include "pins.h"
#include "calibrate_serial.h"
#include "serial_timing.h"

// Serial 

#define SERIAL_SIZE 32

// Input/output buffer
typedef struct serial_buffer_t {
	uint8_t uart_start;
	volatile uint8_t uart_end;
	volatile uint8_t uart_buffer[SERIAL_SIZE];
} serial_buffer_t;

// The serial value we are clocking in
uint8_t receive_value = 0;
uint8_t receive_counter = 0;
uint8_t send_value = 0;
uint8_t send_counter = 0;

serial_buffer_t uart_input = { 0 };
serial_buffer_t uart_output = { 0 };

inline void _serial_push(serial_buffer_t* uart, uint8_t value);
uint8_t _serial_available(serial_buffer_t* buffer);
uint8_t _serial_read(serial_buffer_t* buffer);
void _serial_push(serial_buffer_t* buffer, uint8_t value);
inline void receive_bit();
inline void send_bit();

typedef enum receive_flag_t { RECEIVE_IDLE = 0, RECEIVE_HALF = 1, RECEIVE_FULL = 2 } receive_flag_t;
typedef enum send_flag_t { SEND_IDLE = 0, SEND_ACTIVE = 1 } send_flag_t;

receive_flag_t receive_flag = RECEIVE_IDLE;
send_flag_t send_flag = SEND_IDLE;

ISR(PCINT0_vect) {
	int dataIn = readState(DATAIN);

	// Serial start bit is a zero, so we'll see this as a pin-change interrupt 
	// with a read of zero
	if (!dataIn) {
		// Start bit detection

		// Disable the pin-change interrupt as we transition to timers
		clearBit(PCMSK, PCINT4);

		receive_value = 0;
		receive_counter = 0;

		uint8_t current_timer = TCNT0;

		if (current_timer < SERIAL_TIMER_COMPARE_HALF_THRESHOLD || current_timer >= SERIAL_TIMER_COMPARE_FULL_THRESHOLD) {
			_serial_push(&uart_input, 'h');
			// Trigger serial receive on the half timer
			receive_flag = RECEIVE_HALF;
		} else {
			_serial_push(&uart_input, 'f');
			// Trigger serial receive on the full timer
			receive_flag = RECEIVE_FULL;
		}
	}
}

ISR(TIMER0_COMPA_vect) {
	// Receive can be on the full-timer interrupt
	if (receive_flag == RECEIVE_FULL) {
		receive_bit();
	}

	// Sending is always on the full-timer interrupt
	send_bit();
}

ISR(TIMER0_COMPB_vect) {
	// Receive can be on the half-timer interrupt
	if (receive_flag == RECEIVE_HALF) {
		receive_bit();
	}
}

void send_bit() {
	if (send_flag == SEND_IDLE) {
		if (_serial_available(&uart_output)) {
			send_flag = SEND_ACTIVE;
			send_counter = 0;
			send_value = _serial_read(&uart_output);
		}
	}

	if (send_flag == SEND_ACTIVE) {
		switch (send_counter) {
			case 0:
				// Start
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
				setState(DATAOUT, send_value & 1);
				send_value >>= 1;
				break;
			case 9:
				// Stop bit
				setHigh(DATAOUT);

				// Queue up next character (if applicable)
				if (_serial_available(&uart_output)) {
					send_counter = -1;
					send_value = _serial_read(&uart_output);
				} else {
					send_flag = SEND_IDLE;
				}
				break;
		}

		send_counter++;
	}
}

void receive_bit() {
	switch (receive_counter) {
		case 0:
			// Start bit (zero)
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			// Clock in the serial bit
			receive_value |= (readState(DATAIN) ? 1 : 0) << (receive_counter - 1);
			break;
		case 9:
			// Stop bit (one)
			_serial_push(&uart_input, receive_value);

			// Re-enable pin-change interrupt
			setBit(PCMSK, PCINT4); 

			receive_flag = RECEIVE_IDLE;
			break;
	}

	receive_counter++;
}

void _serial_push(serial_buffer_t* buffer, uint8_t value) {
	buffer->uart_buffer[buffer->uart_end] = value;
	buffer->uart_end++;
	if (buffer->uart_end >= SERIAL_SIZE)
		buffer->uart_end = 0;
}

uint8_t _serial_available(serial_buffer_t* buffer) {
	cli();
	uint8_t end = buffer->uart_end;
	sei();

	if (end == buffer->uart_start)
		return 0;

	if (end > buffer->uart_start)
		return end - buffer->uart_start;

	return SERIAL_SIZE - (buffer->uart_start - end);
}

void _serial_drop(serial_buffer_t* buffer, uint8_t count) {
	buffer->uart_start = (buffer->uart_start + count) % SERIAL_SIZE;
}

uint8_t _serial_peek(serial_buffer_t* buffer, uint8_t where) {
	return buffer->uart_buffer[(buffer->uart_start + where) % SERIAL_SIZE];
}

uint8_t _serial_read(serial_buffer_t* buffer) {
	uint8_t ret = buffer->uart_buffer[buffer->uart_start];
	buffer->uart_start++;
	if (buffer->uart_start >= SERIAL_SIZE)
		buffer->uart_start = 0;

	return ret;
}

uint8_t serial_available() {
	return _serial_available(&uart_input);
}

uint8_t serial_read() {
	return _serial_read(&uart_input);
}

uint8_t serial_peek(uint8_t where) {
	return _serial_peek(&uart_input, where);
}

void serial_drop(uint8_t count) {
	_serial_drop(&uart_input, count);
}

void serial_write(uint8_t c) {
	_serial_push(&uart_output, c);
}

void serial_write_buffer(uint8_t* buffer, uint8_t length) {
	for (uint8_t i = 0; i < length; i++) {
		_serial_push(&uart_output, buffer[i]);
	}
}

void serial_write_string(char* buffer) {
	serial_write_buffer((uint8_t*)buffer, strlen(buffer));
}

void initialize_serial() {
	// Serial timer (see comments above)

	// CTC mode, OCRA is top
	TCCR0A = _BV(WGM01);
	
	// Timer interrupts for each serial bit
	OCR0A = SERIAL_TIMER_COMPARE;
	OCR0B = SERIAL_TIMER_COMPARE_HALF;

	// Enable the timer for bit-bang serial
	TCCR0B = SERIAL_CLOCK_DIVISOR;

	calibrate_serial();

	// Enable timer interrupts -- overflow for timer0 and timer1
	TCNT0 = 0;
	TIMSK = _BV(OCIE0A) | _BV(OCIE0B) | _BV(TOIE1);

	// Turn on pin-change interrupt for port 4
	setBit(PCMSK, PCINT4);
	setBit(GIMSK, PCIE);
}
