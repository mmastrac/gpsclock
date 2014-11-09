#pragma once

/*
	Attiny85 pinouts:

	           +----+ 
	RESET   5 -|o   |-   VCC 
	DATAOUT 3 -|    |- 2 SIN
	DATAIN  4 -|    |- 1 GSCLK/SCLK
	GND       -|    |- 0 BLANK/XLAT
		       +----+
*/


// BLANK + XLAT
#define BLANK_PORT B
#define BLANK_PIN 0

// GSCLK + SCLK
#define GSCLK_PORT B
#define GSCLK_PIN 1

// SIN
#define SIN_PORT B
#define SIN_PIN 2

// DATAIN
#define DATAIN_PORT B
#define DATAIN_PIN 4

// DATAOUT
#define DATAOUT_PORT B
#define DATAOUT_PIN 3
