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
#include "ds1338.h"
#include "serial.h"

int main()
{
	serial::init();
	serial::puts("Init three-phase: ");

	ThreePhase::init();
	serial::puts("OK\n");

	serial::puts("Starting detector: ");
    Draw::start_detector();
    serial::puts("OK\n");

    serial::puts("Enabling interrupts\n");
	sei();

    uint8_t last_halfs = 255;

	Draw::speed_target = 0;

	serial::puts("Braking: ");
	/* BRAKE */
	PORTD &= ~(1 << PIND6);
	PORTB &= ~((1 << PINB1) | (1 << PINB2));
	for (;;)
	{
		Draw::moving = 0;
		for (uint16_t i = 0; i < 5000; i++)
		{
			_delay_us(100);
		}
		if (!Draw::moving)
		{
			break;
		}
	}
	serial::puts("OK\n");

	serial::puts("Enabling motor output pins\n");
	PORTD &= ~(1 << PIND6);
	PORTB &= ~((1 << PINB1) | (1 << PINB2));

	serial::puts("Init draw: ");
	Draw::init();
	serial::puts("OK\n");

	/* Force to closest pole */
	serial::puts("Forcing to closest pole: ");
	Draw::amplitude = 255;
	for (uint16_t i = 0; i < 200; i++)
	{
		_delay_us(100);
	}
	serial::puts("OK\n");

	Draw::amplitude = 128;
	Draw::speed_target = 65536;
	Draw::ramp_time = 20;

	//DS1338::init();

	serial::puts("Entering main-loop\n");
	for (;;)
	{
		uint64_t jiffies = Draw::jiffies / 625;

		/* Don't ramp down the amplitude before we have picked up a little speed.
		 * Apparently, the first bit is haaard; push at full force. PUSH MOTHERFUCKER! PUSH! */
		if (Draw::speed_actual > 8192)
		{
			uint16_t amplitude = 128 +  Draw::speed_target / 512;
//			//int16_t amplitude = 64 +  Draw::vspeed / 341;
			Draw::amplitude = amplitude <= 255 ? amplitude : 255;
		}
		else
		{
			/* Keep track of where the platters are until we're up to speed */
			Draw::offset_angle = -Draw::dot_angle - 400;
		}

		uint64_t half_secs = jiffies / 50;
		uint8_t halfs = half_secs % 200;

		if (halfs != last_halfs)
		{
			uint64_t total_secs = jiffies / 100;
			uint64_t total_mins = total_secs / 60;
			//uint64_t total_hours = (total_mins / 60);

			//uint8_t hund = jiffies % 100;
			uint8_t secs = total_secs % 60;
			uint8_t mins = total_mins % 60;
			//uint8_t hours = total_hours % 24;

			Draw::draw_digit(0, mins / 10);
			Draw::draw_digit(35, mins % 10);
			Draw::draw_digit(91, secs / 10);
			Draw::draw_digit(126, secs % 10);

			/* Blinking colon */
			if ((halfs & 1) == 0)
			{
				Draw::draw_digit(63, 10);
			}
			else
			{
				Draw::draw_digit(63, 11);
			}
		}

		last_halfs = halfs;
	}
}

