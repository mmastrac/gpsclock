This is the AVR code that runs the display driver

The attiny85 can be programmed like so, note the lfuse settings for the external clock:

make clean && make && avrdude -p t85 -c avrispv2 -P /dev/cu.usbmodem00028541 -U flash:w:main.hex -U lfuse:w:0xe2:m
