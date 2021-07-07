/*
 * threephase.cpp
 *
 *  Created on: 5 Jul 2021
 *      Author: tom
 */

#include <avr/interrupt.h>

#include "threephase.h"

void ThreePhase::init()
{
	DDRD |= (1 << PIND6);
	DDRB |= (1 << PINB1) | (1 << PINB2);

	/* Setup partly stolen from https://github.com/hannahvsawiuk/3-Phase-PWM */

	GTCCR = (1<<TSM)|(1<<PSRASY)|(1<<PSRSYNC); // halt all timers

	/* Sets Timer0 in Fast PWM mode. Clears OC0A on Compare Match, set OC0A at BOTTOM (non-inverting mode).
       Then, waveform generation is set to mode 3: Fast PWM with TOP of 0xFF*/
	TCCR0A = (1 << COM0A1) | (1 << WGM00) | (1 << WGM01); /*page 104 and page 106*/
	TCCR0B = (1 << CS00); /*No pre-scaling (page 108)*/
	//TCCR0B = (1 << CS01);

	/* Sets Timer1 in Fast PWM mode. Clears OC1A/B on Compare Match, set OC1A/B at BOTTOM (non-inverting mode).
       Then, waveform generation is set to mode 3: Fast PWM with TOP of 0xFF*/
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM12) | (1 << WGM10) ; /*page 171 and 172*/
	TCCR1B = (1 << CS10); /*No pre-scaling (page 173) NOTE: must change if using an external clock*/
	//TCCR1B = (1 << CS11);

	// Release all timers synchronized
	GTCCR = 0;
}

