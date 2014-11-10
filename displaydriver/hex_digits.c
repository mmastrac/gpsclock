#include <avr/pgmspace.h>

const uint8_t hex_digits[] PROGMEM = "0123456789ABCDEF";

uint8_t hex(uint8_t n) {
	return pgm_read_word(&hex_digits[n]);
}

uint8_t hex_hi(uint8_t n) {
	return hex(n >> 4);
}

uint8_t hex_lo(uint8_t n) {
	return hex(n & 0xf);
}
