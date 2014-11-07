# Assumes interrupts are disabled
# Returns the number of ticks required to process a tilde (character 126, 0b01111110)

# What we want to do here is approximate the number of ticks required to
# take up 6 * 104.116us (729.167us), the length of a pulse for a 9600 baud stream.

# We ideally want to see this come out as 2380, through experimental observation
# of the acceptable range for an example USB<->serial interface.

.global calibrate

calibrate:
	# Temporary counters
	ldi     r18, 0x00
	ldi     r19, 0x00
	ldi     r20, 0x00
	ldi     r21, 0x00
	ldi     r22, 0x00
	ldi     r23, 0x00
	ldi     r26, 0x00
	ldi     r27, 0x00
	ldi     r30, 0x00
	ldi     r31, 0x00
	mov		r0, r1
	ldi		r24, 0x00

	# Wait for start bit (zero)
	wait1:
	sbic    0x16, 4
	rjmp 	wait1

	# Wait for data bits (ones)
	wait2:
	sbis    0x16, 4
	rjmp 	wait2

	# Count fast
	wait3:
	inc 	r18			
	sbic 	0x16, 4	
	inc 	r19
	sbic 	0x16, 4
	inc 	r20
	sbic 	0x16, 4
	inc 	r21
	sbic 	0x16, 4
	inc 	r22
	sbic 	0x16, 4
	inc 	r23
	sbic 	0x16, 4
	inc 	r26
	sbic 	0x16, 4
	inc 	r27
	sbic 	0x16, 4
	inc 	r30
	sbic 	0x16, 4
	inc 	r31
	sbic 	0x16, 4
	inc 	r0
	sbic 	0x16, 4
	inc 	r24
	sbic 	0x16, 4
	rjmp	wait3

	# Return value is the sum of all the temporary registers
	ldi     r25, 0
	add		r24, r0
	adc		r25, r1

	add		r24, r19
	adc		r25, r1
	add		r24, r20
	adc		r25, r1
	add		r24, r21
	adc		r25, r1
	add		r24, r22
	adc		r25, r1
	add		r24, r23
	adc		r25, r1
	add		r24, r26
	adc		r25, r1
	add		r24, r27
	adc		r25, r1
	add		r24, r30
	adc		r25, r1
	add		r24, r31
	adc		r25, r1

	ret
