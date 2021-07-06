/*
 * Draw.cpp
 *
 *  Created on: 5 Jul 2021
 *      Author: tom
 */

#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "draw.h"
#include "threephase.h"
#include "sinetable.h"

static volatile uint8_t on_at[128];
static volatile uint32_t index = 0;

volatile uint32_t Draw::vspeed = 0;
volatile uint8_t Draw::amplitude = 0;

void Draw::init()
{
	DDRB |= (1 << PINB3);

	TCCR2A = (1 << WGM21); /*page 203 and 205*/
	TCCR2B = (1 << CS21);

	TIMSK2 = (1 << OCIE2A); // Configure Timer2 interrupts to send LUT value

	/*Note: OCR2A is set after TCCR1x initialization to avoid overwriting/reset*/
	OCR2A = 31;

	clear();
}

void Draw::clear()
{
	for (uint8_t i = 0; i < sizeof(on_at); i++)
	{
		on_at[i] = 0;
	}
}

uint16_t Draw::segment_angle(int segment)
{
	const uint16_t x = 11;

	switch(segment)
	{
	case 0: return 512;
	case 1: return 853 + x;
	case 2: return 683 + x;
	case 3: return 171;
	case 4: return 683 - x;
	case 5: return 853 - x;
	case 6: return 341;
	}

	return 0;
}

void Draw::set_angle(int16_t angle, uint8_t enable)
{
	if (enable)
	{
		on_at[angle / 8] |= 1 << (angle & 7);
	}
	else
	{
		on_at[angle / 8] &= ~(1 << (angle & 7));
	}
}

void Draw::draw_segment(uint16_t angle, int segment, uint8_t enable)
{
	angle += segment_angle(segment);

	set_angle(angle, enable);
}

void Draw::draw_segments(uint16_t angle, uint8_t segment_mask, uint8_t enable)
{
	for (int i = 0; i < 8; i++)
	{
		if (segment_mask & 1)
		{
			draw_segment(angle, i, enable);
		}

		segment_mask >>= 1;
	}
}

void Draw::draw_digit(double angle, int digit, uint8_t enable)
{
	static uint8_t digits[] =
	{
			0b00111111,
			0b00000110,
			0b01011011,
			0b01001111,
			0b01100110,
			0b01101101,
			0b01111101,
			0b00000111,
			0b01111111,
			0b01101111,
			0b10000000,
	};

	draw_segments(angle, digits[digit], enable);
}

ISR(TIMER2_COMPA_vect)
{
	/* DDS */
	uint16_t i = index >> 16;
	int16_t u = (int8_t) pgm_read_byte(&sinetable[i & 0xff]);
	int16_t v = (int8_t) pgm_read_byte(&sinetable[(i + 85) & 0xff]);
	//int16_t w = (int8_t) pgm_read_byte(&sinetable[(i + 170) & 0xff]);

	u *= Draw::amplitude;
	u >>= 8;
	v *= Draw::amplitude;
	v >>= 8;
	//w *= amplitude;
	//w >>= 8;

	int16_t w = - (u + v);
	if (w > 127) w = 127;
	if (w < -128) w = -128;

	ThreePhase::u = u + 0x80;
	ThreePhase::v = v + 0x80;
	ThreePhase::w = w + 0x80;

	index += Draw::vspeed;

	/* Check if light is on at this angle */
	if (on_at[(i >> 3) & 127] & 1 << (i & 7))
	{
		PORTB |= (1 << PINB3);
	}
	else
	{
		PORTB &= ~(1 << PINB3);
	}
}
