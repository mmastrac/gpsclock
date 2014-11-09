#include <avr/interrupt.h>
#include <avr/io.h>
#include "ticks.h"
#include "bitbang.h"

// Tick counter
volatile uint16_t ticks_ = 0;

ISR(TIMER1_OVF_vect) {
	ticks_++;
}

uint16_t ticks() {
	cli();
	uint16_t tmp = ticks_ & 1023;
	sei();
	return tmp;
}

uint16_t tick_epoch() {
	cli();
	uint16_t tmp = ticks_ >> 10;
	sei();
	return tmp;
}

void initialize_ticks() {
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

	setBit(TIMSK, TOIE1);
}
