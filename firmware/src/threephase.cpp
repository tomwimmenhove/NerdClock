/*
 * threephase.cpp
 *
 *  Created on: 5 Jul 2021
 *      Author: tom
 */

#include <avr/interrupt.h>

#include "threephase.h"

volatile uint8_t ThreePhase::u = 128;
volatile uint8_t ThreePhase::v = 128;
volatile uint8_t ThreePhase::w = 128;

void ThreePhase::init()
{
	/* Setup mostly stolen from https://github.com/hannahvsawiuk/3-Phase-PWM */

	/* Sets Timer0 in Fast PWM mode. Clears OC0A on Compare Match, set OC0A at BOTTOM (non-inverting mode).
       Then, waveform generation is set to mode 3: Fast PWM with TOP of 0xFF*/
	TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00) | (1 << WGM01); /*page 104 and page 106*/
	TCCR0B = (1 << CS00); /*No pre-scaling (page 108)*/

	/* Sets Timer1 in Fast PWM mode. Clears OC1A/B on Compare Match, set OC1A/B at BOTTOM (non-inverting mode).
   Then, waveform generation is set to mode 3: Fast PWM with TOP of 0xFF*/
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM12) | (1 << WGM10) ; /*page 171 and 172*/
	TCCR1B = (1 << CS10); /*No pre-scaling (page 173) NOTE: must change if using an external clock*/

	/* Sets Timer2 in CTC mode mode.TOP = OCR2A, update at immediate, no pre-scaling */
	TCCR2A = (1 << WGM21); /*page 203 and 205*/
	TCCR2B = (1 << CS20);

	cli(); /*disable interrupts*/

	TIMSK0 = (1 << TOIE0);  // Enable Timer0
	TIMSK1 = (1 << TOIE1);  // Enable Timer1
	TIMSK2 = (1 << OCIE2A); // Configure Timer2 interrupts to send LUT value

	/*Note: OCR2A is set after TCCR1x initialization to avoid overwriting/reset*/
	OCR2A = 255;
}

ISR (TIMER0_OVF_vect)
{
	OCR0A = ThreePhase::u;
}

ISR (TIMER1_OVF_vect)
{
	OCR1A = ThreePhase::v;
	OCR1B = ThreePhase::w;
}
