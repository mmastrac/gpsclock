#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include "ticks.h"
#include "bitbang.h"

/*

The timer is a 1024-bit counter that wraps once a second, combined with an "epoch" that is 
incremented each time the timer wraps. This yields an effective 18-bit timer that can be 
used for various purposes.

Timer layout:

T0 T1 T2 T3 Ea Eb
^ TPTR

On every interrupt overflow, we increment the timer that TPTR is pointing at, then increment TPTR
itself, wrapping from T3 to T0. If T3 is going to wrap, we first increment Ea, then T3, then Eb.

The timer runs infrequenty enough that we can guarantee that it will interrupt a read process at
most once. This will affect how we read the timer, as described below.

Reading the timer is simple. First, a copy of T0, T1, T2, E1, T3 and E2 is made, in that order. 
During the copy process, it is possible that a timer has modified one to three of the values (more 
than one is possible only in the case where the timer wraps). Because the timer runs atomically,
this means that from left-to-right, the left-most values will be pre-tick and the right-most values
will be post-tick.

If some of the Tx values we copied are zero and the rest are FF, the timer has wrapped. The 
zeros are treated as 256 and 

00 00 00 00 00 00 = 0
01 00 00 00 00 00 = 1
01 01 00 00 00 00 = 2
...
FF FF FF FE 00 00 = 1019
FF FF FF FF 00 00 = 1020
00 FF FF FF 00 00 = 1021
00 00 FF FF 00 00 = 1022
00 00 00 FF 00 00 = 1023

00 00 00 FF 01 00 = 1024 \__ These are equivalent
00 00 00 00 01 00 = 1024  |
00 00 00 00 01 01 = 1024 /

01 00 00 00 01 01 = 1025

*/

// Tick counter
volatile uint8_t ticks_shards[4] = { 0, 0, 0, 0 };
volatile uint8_t epoch_a = 0;
volatile uint8_t epoch_b = 0;

uint8_t tick_ptr = 0;

ISR(TIMER1_OVF_vect) {
	if (tick_ptr == 3) {
		if (ticks_shards[3] == 0xff) {
			epoch_a++;
			ticks_shards[3] = 0;
			epoch_b++;
		} else {
			ticks_shards[3]++;
		}
		tick_ptr = 0;
	} else {
		ticks_shards[tick_ptr]++;
		tick_ptr++;
	}
}

ticks_t ticks() {
	uint8_t tick_shards_copy[4];
	uint8_t epoch_a_copy, epoch_b_copy;

	// Make a copy in the specified order (T0, T1, T2, Ea, T3, Eb)
	tick_shards_copy[0] = ticks_shards[0];
	tick_shards_copy[1] = ticks_shards[1];
	tick_shards_copy[2] = ticks_shards[2];
	epoch_a_copy = epoch_a;
	tick_shards_copy[3] = ticks_shards[3];
	epoch_b_copy = epoch_b;

	ticks_t result;
	result.epoch = epoch_a_copy;
	result.ticks = tick_shards_copy[0] + tick_shards_copy[1] 
		+ tick_shards_copy[2] + tick_shards_copy[3];

	uint8_t zero_count = (tick_shards_copy[0] ? 0 : 1) + (tick_shards_copy[1] ? 0 : 1) +
		(tick_shards_copy[2] ? 0 : 1) + (tick_shards_copy[3] ? 0 : 1);

	// If we have zeros, we need to check to see if we are wrapping
	if (zero_count) {
		if (epoch_a_copy != epoch_b_copy || zero_count == 4) {
			// Ea != Eb, so that means epoch and timer has rolled but hasn't updated
			result.ticks = 0;
		} else {
			if (tick_shards_copy[0] == 0xff || tick_shards_copy[1] == 0xff || 
				tick_shards_copy[2] == 0xff || tick_shards_copy[3] == 0xff) {
				// If any of the tick counters are FF, we count zeros as 256
				result.ticks += 256 * zero_count;
			}
		}
	}

	return result;
}

void initialize_ticks() {
	// Tick timer
	// Use timer1, since it's a bizarre timer that doesn't match other AVRs

	// Clock is 8000000Mhz

	// Prescale by 32 so that 1024*245 timer ticks is ~1 sec
	// I thought we'd need to set CTC1 here, but it doesn't seem to be so 
	// (and the timer doesn't hit OVF if that's the case)
	TCCR1 = _BV(PWM1A) | _BV(CS12) | _BV(CS11);
	OCR1C = 245;

	// Restart/reset the timer
	GTCCR = _BV(PSR1);
	TCNT1 = 0;

	setBit(TIMSK, TOIE1);
}
