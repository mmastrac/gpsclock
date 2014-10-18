#pragma once

// Need to use a level of indirection so we can expand args
#define CONCAT__(a, b) a ## b
#define CONCAT_(a, b) CONCAT__(a, b)

// These reference the _PIN and _PORT #defines for a given variable
#define PORTVAR(pin) CONCAT_(pin, _PORT)
#define PINVAR(pin) CONCAT_(pin, _PIN)

#define DDRID(pin) CONCAT_(DDR, PORTVAR(pin))
#define PINID(pin) CONCAT_(PIN, PORTVAR(pin))
#define PORTID(pin) CONCAT_(PORT, PORTVAR(pin))

#define setInput(pin) clearBit(DDRID(pin), PINVAR(pin))
#define setOutput(pin) setBit(DDRID(pin), PINVAR(pin))
#define setLow(pin) clearBit(PORTID(pin), PINVAR(pin))
#define setHigh(pin) setBit(PORTID(pin), PINVAR(pin))
#define pulse(pin) do { setHigh(pin); setLow(pin); } while (0)
#define outputState(pin) readBit(PORTID(pin), PINVAR(pin))
#define readState(pin) readBit(PORTID(pin), PINVAR(pin))

#define setBit(reg, pin) ((reg) |= (1 << (pin)))
#define clearBit(reg, pin) ((reg) &= ~(1 << (pin)))
#define readBit(reg, pin) ((reg) & (1 << (pin)))
