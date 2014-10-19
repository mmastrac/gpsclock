#pragma once

#define c (1 << 1)
#define l (1 << 2)
#define d1 (1 << 3)
#define d2 0
#define m (1 << 4)
#define n (1 << 5)
#define e (1 << 6)
#define g1 (1 << 7)
#define f (1 << 8)
#define h (1 << 9)
#define a1 (1 << 10)
#define j (1 << 11)
#define k (1 << 12)
#define a2 (1 << 13)
#define b (1 << 14)
#define g2 (1 << 15)

const uint16_t FONT[128] PROGMEM = {
	a1|a2|b|c|d1|d2|e|f|g1|g2|h|j|k|l|m|n,
	a1|a2|b|c|d1|d2|e|f|g1|g2|h|j|l|m|n,
	a1|a2|b|c|d1|d2|e|f|g1|h|j|k|l|m|n,
	a1|a2|b|c|d1|d2|e|f|g1|g2|h|j|k|m|n,
	a1|a2|b|c|d1|d2|e|f|g1|g2|h|j|k|l|n,
	a1|a2|b|c|d1|d2|e|f|g1|g2|h|j|k|l|m,
	a1|a2|b|c|d1|d2|e|f|g2|h|j|k|l|m|n,
	a1|a2|b|c|d1|d2|e|f|g1|g2|j|k|l|m|n,
	a1|a2|b|c|d1|d2|e|f|g1|g2|h|k|l|m|n,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	g1|g2|h|j|k|l|m|n,
	g1|g2|h|j|l|m|n,
	g1|h|j|k|l|m|n,
	g1|g2|h|j|k|m|n,
	g1|g2|h|j|k|l|n,
	g1|g2|h|j|k|l|m,
	g2|h|j|k|l|m|n,
	g1|g2|j|k|l|m|n,
	g1|g2|h|k|l|m|n,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	j|m,
	b|j,
	b|c|d1|d2|g1|g2|j|m,
	a1|a2|c|d1|d2|f|g1|g2|j|m,
	c|f|k|n,
	a1|d1|d2|e|f|g1|g2|j|l|n,
	k,
	k|l,
	h|n,
	g1|g2|h|k|l|n,
	g1|g2|j|m,
	n,
	g1|g2,
	c|d1|d2|e|g1|g2|l|m|n,
	k|n,
	a1|a2|b|c|d1|d2|e|f,
	a1|d1|d2|j|m,
	a1|a2|b|d1|d2|e|g1|g2,
	a1|a2|b|c|d1|d2|g2,
	b|c|f|g1|g2,
	a1|a2|c|d1|d2|f|g1|g2,
	a1|a2|c|d1|d2|e|f|g1|g2,
	a1|a2|b|c,
	a1|a2|b|c|d1|d2|e|f|g1|g2,
	a1|a2|b|c|d1|d2|f|g1|g2,
	a1|a2|b|f|g1|g2|h|j|k,
	a1|c|d1|d2|e|f|g1|g2|h|j|l|m|n,
	d1|d2|k|n,
	d1|d2|g1|g2,
	d1|d2|h|l,
	a1|a2|b|g2|m,
	a1|a2|b|c|d1|d2|e|g1|m,
	a1|a2|b|c|e|f|g1|g2,
	a1|a2|b|c|d1|d2|g2|j|m,
	a1|a2|d1|d2|e|f,
	a1|a2|b|c|d1|d2|j|m,
	a1|a2|d1|d2|e|f|g1,
	a1|a2|e|f|g1,
	a1|a2|c|d1|d2|e|f|g2,
	b|c|e|f|g1|g2,
	a1|a2|d1|d2|j|m,
	a2|b|c|d1|d2|e,
	e|f|g1|k|l,
	d1|d2|e|f,
	a1|a2|b|c|e|f|j,
	b|c|e|f|h|l,
	a1|a2|b|c|d1|d2|e|f,
	a1|a2|b|e|f|g1|g2,
	a1|a2|b|c|d1|d2|e|f|l,
	a1|a2|b|e|f|g1|g2|l,
	a1|a2|c|d1|d2|f|g1|g2,
	a1|a2|j|m,
	b|c|d1|d2|e|f,
	e|f|k|n,
	b|c|d1|d2|e|f|m,
	h|k|l|n,
	b|f|g1|g2|m,
	a1|a2|d1|d2|k|n,
	a1|a2|d1|d2|e|f,
	h|l,
	a1|a2|b|c|d1|d2,
	f|h,
	d1|d2,
	h,
	d1|d2|e|g1|m,
	c|d1|d2|e|f|g1|g2,
	d1|d2|e|g1|g2,
	b|c|d1|d2|e|g1|g2,
	d1|d2|e|g1|n,
	a2|g2|j|m,
	a2|b|c|d1|d2|g2|j,
	e|f|g1|m,
	m,
	b|c|d1|d2,
	j|k|l|m,
	j|m,
	c|e|g1|g2|m,
	e|g1|m,
	c|d1|d2|e|g1|g2,
	a1|e|f|g1|j,
	a2|b|c|g2|j,
	e|g1,
	d1|d2|g2|l,
	g1|g2|j|m,
	c|d1|d2|e,
	c|l,
	c|d1|d2|e|m,
	d1|d2|g1|g2|m,
	h|k|n,
	d1|d2|g1|n,
	a1|a2|d1|d2|g1|h|n,
	k|n,
	a1|a2|d1|d2|g2|k|l,
	f|h|k,
	a2|b|c|d1|d2|e|g1|g2|j|k|l|m|n,
};

#undef a1
#undef a2
#undef b
#undef c
#undef d1
#undef d2
#undef e
#undef f
#undef g1
#undef g2
#undef h
#undef i
#undef j
#undef k
#undef l
#undef m
#undef n
