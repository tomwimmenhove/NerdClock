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

static uint8_t buffer[128];
static volatile uint32_t index = 0;

volatile uint8_t Draw::ramp_time = 0;
volatile uint32_t Draw::speed_target = 0;
volatile uint32_t Draw::speed_actual = 0;
volatile uint8_t Draw::amplitude = 0;
volatile uint64_t Draw::jiffies;
volatile uint16_t Draw::current_angle = 0;
volatile uint16_t Draw::dot_angle = 0;
volatile uint8_t Draw::moving = 0;
volatile uint16_t Draw::offset_angle;

void Draw::init()
{
	DDRB |= (1 << PINB3);

	TCCR2A = 0;
	//TCCR2A = (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (1 << CS20);

	TIMSK2 = (1 << OCIE2A); // Configure Timer2 interrupts to send LUT value

	clear();
}

void Draw::start_detector()
{
    EICRA |= (1 << ISC11);  // INT1 on falling edge of PD3
    EIMSK |= (1 << INT1);   // Turns on INT1
}

void Draw::clear()
{
	for (uint8_t i = 0; i < sizeof(buffer); i++)
	{
		buffer[i] = 0;
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

void Draw::set_angle(uint16_t angle, uint8_t enable)
{
	if (enable)
	{
		buffer[(angle >> 3) & 127] |= 1 << (angle & 7);
	}
	else
	{
		buffer[(angle >> 3) & 127] &= ~(1 << (angle & 7));
	}
}

void Draw::draw_segment(uint16_t angle, int segment, uint8_t enable)
{
	angle += segment_angle(segment);

	set_angle(angle, enable);
}

void Draw::draw_segments(uint16_t angle, uint8_t segment_mask)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		draw_segment(angle, i, segment_mask & 1);

		segment_mask >>= 1;
	}
}

void Draw::draw_digit(int16_t angle, int digit)
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
			0b00000000,
	};

	draw_segments(angle, digits[digit]);
}

ISR(TIMER2_COMPA_vect)
{
	//static int16_t jiffie_timer;
	static int8_t ramp_timer;

	/* DDS: Set the UVW angle of the motor */
	Draw::current_angle = index >> 16;
	ThreePhase::set_angle(Draw::current_angle, Draw::amplitude);

	index += Draw::speed_actual;

	/* Ramp up to target speed */
	if (++ramp_timer == Draw::ramp_time)
	{
		if (Draw::speed_actual < Draw::speed_target)
		{
			Draw::speed_actual++;
		}
		ramp_timer = 0;
	}

	/* Check if light is on at this angle */
	uint16_t x = Draw::current_angle + Draw::offset_angle;
	if (buffer[(x >> 3) & 127] & 1 << (x & 7))
	{
		PORTB |= (1 << PINB3);
		//OCR2A = Draw::brightness;
	}
	else
	{
		PORTB &= ~(1 << PINB3);
		//OCR2A = 0;
	}

	Draw::jiffies++;
}

ISR (INT1_vect)
{
	Draw::dot_angle = Draw::current_angle & 1023;
	Draw::moving = 1;
}
