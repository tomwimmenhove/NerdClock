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

	serial::puts("Start-up\n");

#if 0
	DS1338::init();

	static time time;
	time.set_year(2021);
	time.set_date(23);
	time.set_month(07);
	time.set_hour(21, true);
	time.set_min(12);
	time.set_sec(20);

	DS1338::set(time);
#endif

	serial::puts("Enabling motor output pins\n");
	// Motor enable
	DDRC |= (1 << PINC0);
	PORTC |= (1 << PINC0);

	// Motor reset
	DDRD |= (1 << PIND5);
	PORTD |= (1 << PIND5);

	// Fault input
	PORTB |= (1 << PINB0);

	serial::puts("Starting detector: ");
    Draw::start_detector();
    serial::puts("OK\n");

    serial::puts("Enabling interrupts\n");
	sei();

	serial::puts("Braking");
	DDRD |= (1 << PIND6);
	DDRB |= ((1 << PINB1) | (1 << PINB2));
	PORTD |= ~(1 << PIND6);
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
			serial::puts(" OK\n;");
			break;
		}

		serial::putc('.');
	}
	serial::puts("OK\n");

	serial::puts("Init three-phase: ");
	ThreePhase::init();
	serial::puts("OK\n");

	Draw::speed_target = 0;

	serial::puts("Init draw: ");
	Draw::init();
	serial::puts("OK\n");

	/* Force to closest pole */
	serial::puts("Forcing to closest pole: ");
	Draw::amplitude = 128;
	for (uint16_t i = 0; i < 20; i++)
	{
		_delay_us(100);
	}
	serial::puts("OK\n");

	Draw::amplitude = 128;
	Draw::speed_target = 65536;
	Draw::ramp_time = 4;

	serial::puts("Initializing RTC: ");
	DS1338::init();
	serial::puts("OK\n");

	//bool colon_on = false;
	serial::puts("Entering main-loop\n");
	for (;;)
	{
		/* Don't ramp down the amplitude before we have picked up a little speed.
		 * Apparently, the first bit is haaard; push at full force. PUSH MOTHERFUCKER! PUSH! */
		if (Draw::speed_actual > 8192)
		{
			if (Draw::speed_actual == Draw::speed_target)
			{
				Draw::amplitude = 128;
			}
			else
			{
				uint16_t amplitude = 128 +  Draw::speed_target / 512;
				//int16_t amplitude = 64 +  Draw::speed_target / 341 / 2;
				Draw::amplitude = amplitude <= 255 ? amplitude : 255;
			}
		}
		else
		{
			/* Keep track of where the platters are until we're up to speed */
			Draw::offset_angle = -Draw::dot_angle - 512 + 63;
		}

//		if (colon_on && Draw::jiffies > 31250)
//		{
//			Draw::draw_digit(63, 11);
//			colon_on = false;
//		}

//		if (!DS1338::tick)
//		{
//			continue;
//		}
//		DS1338::tick = false;

		serial::puts("tick\n");

		//Draw::jiffies = 0;
		//colon_on = true;
		struct time time;
		DS1338::get(time);

#if 1
		Draw::draw_digit(0, time.hour >> 4);
		Draw::draw_digit(35, time.hour & 0xf);
		Draw::draw_digit(91, time.min >> 4);
		Draw::draw_digit(126, time.min & 0xf);
#else
		Draw::draw_digit(0, time.min >> 4);
		Draw::draw_digit(35, time.min & 0xf);
		Draw::draw_digit(91, time.sec >> 4);
		Draw::draw_digit(126, time.sec & 0xf);
#endif

		/* Blinking colon */
		if ((time.sec & 1) == 0)
		{
			Draw::draw_digit(63, 10);
		}
		else
		{
			Draw::draw_digit(63, 11);
		}
	}
}

