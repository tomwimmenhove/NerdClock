/*
 * main.cpp
 *
 *  Created on: Jul 5, 2021
 *      Author: Tom Wimmenhove
 */

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "sinetable.h"  // LUT file
#include "draw.h"
#include "threephase.h"

volatile uint8_t beep = 0;
volatile uint8_t moving = 0;

volatile uint16_t dot_angle = 0;

int main()
{
	DDRD |= (1 << PIND6);
	DDRB |= (1 << PINB1) | (1 << PINB2);

    EICRA |= (1 << ISC11);  // INT1 on falling edge of PD3
    EIMSK |= (1 << INT1);   // Turns on INT1
	sei();

    int16_t angle_correction = 0;
    uint8_t last_halfs = 255;

	uint8_t digits[4];

	Draw::speed_target = 0;

	/* BRAKE */
	PORTD &= ~(1 << PIND6);
	PORTB &= ~((1 << PINB1) | (1 << PINB2));
	for (;;)
	{
		moving = 0;
		for (uint16_t i = 0; i < 5000; i++)
		{
			_delay_us(100);
		}
		if (!moving)
		{
			break;
		}
	}

	ThreePhase::init();
	Draw::init();

	/* Force to closest pole */
	Draw::amplitude = 255;
	for (uint16_t i = 0; i < 100; i++)
	{
		_delay_us(100);
	}

	Draw::amplitude = 128;
	Draw::speed_target = 65536;
	Draw::ramp_time = 20;

	for (uint16_t i = 0;;i++)
	{
		uint64_t jiffies = Draw::jiffies;

		/* Don't ramp down the amplitude before we have picked up a little speed.
		 * Apparently, the first bit is haaard; push at full force. PUSH MOTHERFUCKER! PUSH! */
		if (Draw::speed_actual > 8192)
		{
			uint16_t amplitude = 128 +  Draw::speed_target / 512;
			//int16_t amplitude = 64 +  Draw::vspeed / 341;
			Draw::amplitude = amplitude <= 255 ? amplitude : 255;
		}

		/* Keep track of where the platters are until we're up to speed */
		if (Draw::speed_actual < Draw::speed_target)
		{
			angle_correction = dot_angle - 256;
		}
//		else
//		{
//			EIMSK &= ~(1 << INT1);
//		}

		uint64_t half_secs = jiffies / 50;
		uint64_t total_secs = jiffies / 100;
		uint64_t total_mins = total_secs / 60;
		//uint64_t total_hours = (total_mins / 60);

		//uint8_t hund = jiffies % 100;
		uint8_t halfs = half_secs % 200;
		uint8_t secs = total_secs % 60;
		uint8_t mins = total_mins % 60;
		//uint8_t hours = total_hours % 24;

		if (halfs != last_halfs)
		{
#if 1
			digits[0] = mins / 10;
			digits[1] = mins % 10;
			digits[2] = secs / 10;
			digits[3] = secs % 10;
#else
			digits[0] = 0;
			digits[1] = (Draw::amplitude / 100) % 10;
			digits[2] = (Draw::amplitude /  10) % 10;
			digits[3] = (Draw::amplitude /   1) % 10;
#endif

			Draw::clear();

			int16_t angle = angle_correction;
			for (uint8_t j = 0; j < 4; j++)
			{
				uint8_t digit = digits[j];
				Draw::draw_digit(angle, digit, 1);
				angle += 35;
				if (j == 1) /* Skip the colon */
				{
					angle += 28;
				}
			}

			/* Blinking colon */
			if ((halfs & 1) == 0)
			{
				Draw::draw_digit(70 + angle_correction, 10, 1);
			}

			Draw::flip();
		}

		last_halfs = halfs;
	}
}

ISR (INT1_vect)
{
	dot_angle = Draw::current_angle & 1023;
	moving = 1;
}

