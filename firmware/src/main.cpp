/*
 * main.cpp
 *
 *  Created on: Jul 5, 2021
 *      Author: Tom Wimmenhove
 */

#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>  // pgm_read_byte and PROGMEM
#include <util/delay.h>

#include "sinetable.h"  // LUT file
#include "draw.h"
#include "threephase.h"

volatile uint32_t index; //index of LUT

// Rotations per second
// RPS = F_CPU / OCR2A / sizeof(sinewaveLUT) / 65536 / N_POLES * vspeed
// RPS = vspeed / 1073.742

uint8_t on_at[128] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

volatile uint32_t vspeed = 512;
ISR(TIMER2_COMPA_vect)
{
	uint16_t i = index >> 16;
	ThreePhase::w = pgm_read_byte(&sinewaveLUT[i & 0xff]);
	ThreePhase::v = pgm_read_byte(&sinewaveLUT[(i + 85) & 0xff]);
	ThreePhase::u = pgm_read_byte(&sinewaveLUT[(i + 170) & 0xff]);

	if (on_at[(i >> 3) & 127] & 1 << (i & 7))
	{
		PORTB |= (1 << PINB3);
	}
	else
	{
		PORTB &= ~(1 << PINB3);
	}

	index += vspeed;
}

uint16_t segment_angle(int segment)
{
	const uint16_t x = 11;

	switch(segment)
	{
	case 0: return 512;
	case 1: return 853 + x;// - 5;
	case 2: return 683 + x;// - 5;
	case 3: return 171;
	case 4: return 683 - x;// - 11;
	case 5: return 853 - x;
	case 6: return 341;
	}

	return 0;
}

void set_angle(int16_t angle, uint8_t enable)
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

void draw_segment(uint16_t angle, int segment, uint8_t enable)
{
	angle += segment_angle(segment);

	set_angle(angle, enable);
}

void draw_segments(uint16_t angle, uint8_t segment_mask, uint8_t enable)
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

void draw_digit(double angle, int digit, uint8_t enable)
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

/******************************/
/*        System Setup        */
/******************************/
void setup (void)
{
	DDRD |= (1 << PIND6);
	DDRB |= (1 << PINB1) | (1 << PINB2);

	ThreePhase::init();

	index = 0; // reset index

	sei();
}

/******************************/
/*      Main System Loop      */
/******************************/
static uint16_t i = 0;
static uint8_t hours = 12;
static uint8_t mins = 0;

void loop (void)
{
	if (vspeed < 25600)
	{
		_delay_us(30);

		vspeed++;
	}

	if (vspeed > 10240)
	{
		DDRB |= (1 << PINB3);
	}

	uint16_t mod = i % 1000;

	if (mod == 0)
	{
		draw_digit(0,hours / 10, 0);
		draw_digit(30, hours % 10, 0);
		draw_digit(90, mins / 10, 0);
		draw_digit(120, mins % 10, 0);

		mins++;
		if (mins == 60)
		{
			mins = 0;

			hours++;
			if (hours == 24)
			{
				hours = 0;
			}
		}

		draw_digit(0,hours / 10, 1);
		draw_digit(30, hours % 10, 1);
		draw_digit(90, mins / 10, 1);
		draw_digit(120, mins % 10, 1);
	}

	if (mod < 500)
	{
		draw_digit(60, 10, 1);
	}
	else
	{
		draw_digit(60, 10, 0);
	}

	i++;
}


int main()
{
	setup();

	for (;;)
		loop();
}
