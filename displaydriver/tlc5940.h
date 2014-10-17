#pragma once

#define setOutput(pin) setOutputPin(pin ## _DDR, pin ## _PIN)
#define setLow(pin) setLowPin(pin ## _PORT, pin ## _PIN)
#define setHigh(pin) setHighPin(pin ## _PORT, pin ## _PIN)
#define pulse(pin) pulsePin(pin ## _PORT, pin ## _PIN)

#define setOutputPin(ddr, pin) ((ddr) |= (1 << (pin)))
#define setLowPin(port, pin) ((port) &=  ~(1 << (pin)))
#define setHighPin(port, pin) ((port) |= (1 << (pin)))
#define pulsePin(port, pin) do { \
                            setHighPin((port), (pin)); \
                            setLowPin((port), (pin)); \
                         } while (0)
#define outputStatePin(port, pin) ((port) & (1 << (pin)))
