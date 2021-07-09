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

static volatile uint8_t buffer0[128];
static volatile uint8_t buffer1[128];
static volatile uint8_t* read_buffer = buffer0;
static volatile uint8_t* write_buffer = buffer1;
static volatile uint32_t index = 0;

volatile uint8_t Draw::ramp_time = 0;
volatile uint32_t Draw::speed_target = 0;
volatile uint32_t Draw::speed_actual = 0;
volatile uint8_t Draw::amplitude = 0;
volatile uint64_t Draw::jiffies;
volatile uint16_t Draw::current_angle = 0;
volatile uint16_t Draw::dot_angle = 0;
volatile uint8_t Draw::moving = 0;

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

void Draw::start_detector()
{
    EICRA |= (1 << ISC11);  // INT1 on falling edge of PD3
    EIMSK |= (1 << INT1);   // Turns on INT1
}

void Draw::clear()
{
	for (uint8_t i = 0; i < sizeof(buffer0); i++)
	{
		write_buffer[i] = 0;
	}
}

void Draw::flip()
{
	volatile uint8_t* tmp = read_buffer;
	read_buffer = write_buffer;
	write_buffer = tmp;
}

int16_t Draw::segment_angle(int segment)
{
	const int16_t x = 11;

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
	while (angle > 1024) angle -= 1024;
	while (angle <    0) angle += 1024;

	if (enable)
	{
		write_buffer[angle / 8] |= 1 << (angle & 7);
	}
	else
	{
		write_buffer[angle / 8] &= ~(1 << (angle & 7));
	}
}

void Draw::draw_segment(int16_t angle, int segment, uint8_t enable)
{
	angle += segment_angle(segment);

	set_angle(angle, enable);
}

void Draw::draw_segments(int16_t angle, uint8_t segment_mask, uint8_t enable)
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

void Draw::draw_digit(int16_t angle, int digit, uint8_t enable)
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
	static int16_t jiffie_timer;
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
	uint16_t x = Draw::current_angle;// - Draw::dot_angle;
	if (read_buffer[(x >> 3) & 127] & 1 << (x & 7))
	{
		PORTB |= (1 << PINB3);
	}
	else
	{
		PORTB &= ~(1 << PINB3);
	}

	if (++jiffie_timer == 625)
	{
		jiffie_timer = 0;

		Draw::jiffies++;
	}
}

ISR (INT1_vect)
{
	Draw::dot_angle = Draw::current_angle & 1023;
	Draw::moving = 1;
}
