#pragma once

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
#define SERIAL_TIMER_COMPARE 		208

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
#define SERIAL_TIMER_COMPARE 		104

#else
#error Unknown FCPU
#endif

// Half a serial period, since our timer is running 2x as fast
#define SERIAL_TIMER_COMPARE_HALF 				(SERIAL_TIMER_COMPARE >> 1)

#define SERIAL_TIMER_THRESHOLD 					(SERIAL_TIMER_COMPARE / 20)

// The distance we need to be from the half timer to trigger receive on it
#define SERIAL_TIMER_COMPARE_HALF_THRESHOLD 	(SERIAL_TIMER_COMPARE_HALF - SERIAL_TIMER_THRESHOLD)

// The distance we need to be from the full timer to trigger receive on it
#define SERIAL_TIMER_COMPARE_FULL_THRESHOLD 	(SERIAL_TIMER_COMPARE - SERIAL_TIMER_THRESHOLD)
