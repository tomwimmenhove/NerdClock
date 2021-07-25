/*
 * main.cpp
 *
 *  Created on: Jul 5, 2021
 *      Author: Tom Wimmenhove
 */

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

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

	ThreePhase::set_amplitude(255);

	/* Force to closest pole */
	serial::puts("Forcing to closest pole: ");
	for (uint16_t i = 0; i < 1000; i++)
	{
		_delay_us(100);
	}
	serial::puts("OK\n");

	Draw::speed_target = 65536;
	Draw::ramp_time = 6;

	serial::puts("Initializing RTC: ");
	DS1338::init();
	serial::puts("OK\n");

	// Switches
	PORTC |= ((1 << PINC2) | (1 << PINC3));
	PORTD |= (1 << PIND4);

	int8_t button_state = 0;
	uint16_t correction = eeprom_read_word((uint16_t*) 0);
	int8_t wait_save_correction = 0;

	//ThreePhase::set_amplitude(255);

	uint8_t wait_stable = 0;
	bool ramp_down = false;

	//bool colon_on = false;
	serial::puts("Entering main-loop\n");
	bool done = false;
	for (;;)
	{
		if (done)
		{
			if (!(PIND & (1 << PIND4)))
			{
				correction++;
				Draw::offset_angle++;
				serial::puts("SW1\n");
				wait_save_correction = 3;

				serial::puts("Inc correction: ");
				serial::puti(correction);
				serial::putc('\n');
			}

			if (!(PINC & (1 << PINC2)))
			{
				correction--;
				Draw::offset_angle--;
				serial::puts("SW2\n");
				wait_save_correction = 3;

				serial::puts("Dec correction: ");
				serial::puti(correction);
				serial::putc('\n');
			}

			if ((!(PIND & (1 << PIND4)) && (button_state & (1 << 4))))
			{
				serial::puts("SW1\n");
			}

			if ((!(PINC & (1 << PINC2)) && (button_state & (1 << 2))))
			{
				serial::puts("SW2\n");
			}

			if ((!(PINC & (1 << PINC3)) && (button_state & (1 << 3))))
			{
				serial::puts("SW3\n");
			}
		}

		button_state = (PINC & ((1 << PINC2) | (1 << PINC3))) | (PIND & (1 << PIND4));

		/* Don't ramp down the amplitude before we have picked up a little speed.
		 * Apparently, the first bit is haaard; push at full force. PUSH MOTHERFUCKER! PUSH! */
		if (!done)
		{
			if (Draw::speed_actual == Draw::speed_target)
			{
				serial::puts("Reached target speed\n");
				done = true;
				wait_stable = 2;
				ramp_down = true;
			}
			else
			{
				/* Keep track of where the platters are until we're up to speed */
				Draw::offset_angle = -Draw::dot_angle - 512 + correction;
			}
		}

		if (ramp_down && wait_stable == 0)
		{
			uint8_t amplitude = ThreePhase::get_amplitude();

			if (amplitude > 160)
			{
				ThreePhase::set_amplitude(amplitude - 1);
			}
			else
			{
				ramp_down = false;
			}
		}

		if (!DS1338::tick)
		{
			continue;
		}
		DS1338::tick = false;

		if (wait_save_correction)
		{
			wait_save_correction--;
			if (wait_save_correction == 0)
			{
				serial::puts("Saving correction to EEPROM\n");
				eeprom_write_word((uint16_t*) 0, correction);
			}
		}

		if (wait_stable)
		{
			wait_stable--;
		}

		serial::puts("tick\n");

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

