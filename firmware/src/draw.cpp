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

static volatile uint16_t jiffie_tracker = 0;
static volatile uint16_t next_jiffie = 625;

volatile uint32_t Draw::vspeed = 0;
volatile uint8_t Draw::jiffies = 0;

void Draw::init()
{
	DDRB |= (1 << PINB3);

	/* Sets Timer2 in CTC mode mode.TOP = OCR2A, update at immediate, no pre-scaling */
	TCCR2A = (1 << WGM21); /*page 203 and 205*/
	TCCR2B = (1 << CS20);

	TIMSK2 = (1 << OCIE2A); // Configure Timer2 interrupts to send LUT value

	/*Note: OCR2A is set after TCCR1x initialization to avoid overwriting/reset*/
	OCR2A = 255;

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
	ThreePhase::w = pgm_read_byte(&sinetable[i & 0xff]);
	ThreePhase::v = pgm_read_byte(&sinetable[(i + 85) & 0xff]);
	ThreePhase::u = pgm_read_byte(&sinetable[(i + 170) & 0xff]);

	index += Draw::vspeed;

	/* Check if light is on at this angle */
	if (on_at[(i >> 3) & 127] & 1 << (i & 7) && Draw::vspeed > 2048)
	{
		PORTB |= (1 << PINB3);
	}
	else
	{
		PORTB &= ~(1 << PINB3);
	}

	/* Jiffies */
	jiffie_tracker++;
	if (jiffie_tracker == next_jiffie)
	{
		Draw::jiffies++;
		next_jiffie += 625;

		if (Draw::jiffies == 100)
		{
			Draw::jiffies = 0;
		}
	}
}
