#pragma once

#if F_CPU == 8000000UL

/*
 * 8 MHz
 */

// CPU clock is 8,000,000Hz, period is 125ns
// Baud rate is 9600, period is 104166.67ns (104us)

// We're going to set the timer divisor to 8 which gives us a timer period of 1000ns 
// and use OCR0A to trigger the appropriate intervals
#define SERIAL_CLOCK_DIVISOR 	_BV(CS01)

// Timer interrupts every 104000ns (slightly more often than the 104166.7ns we need)
#define SERIAL_TIMER_COMPARE 		104U

#else
#error Unknown FCPU
#endif

#define SERIAL_TIMER_COMPARE_HALF 				(SERIAL_TIMER_COMPARE >> 1)
#define SERIAL_TIMER_COMPARE_QUARTER			(SERIAL_TIMER_COMPARE >> 2)
