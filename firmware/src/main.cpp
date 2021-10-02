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

static uint16_t correction;
static bool time_set = false;
static bool force_update = false;
static uint8_t button_state = 0;
static bool blink_hours = false;
static bool blink_minutes = false;

uint8_t get_button_event()
{
	static int8_t buttons = 0;
	static int8_t buttons_old = 0;
	static bool hold = false;
	static uint8_t repeat_timer = 0;

	int8_t button_event = 0;

	buttons = ((PINC & (1 << PINC2)) ? 0 : 0x02) |
			  ((PINC & (1 << PINC3)) ? 0 : 0x04) |
			  ((PIND & (1 << PIND4)) ? 0 : 0x01);

	/* No more than one button at a time */
	if (buttons & (buttons - 1))
	{
		return 0;
	}

	button_event = 0;
	if (buttons != buttons_old && buttons)
	{
		/* A button has been pressed */
		serial::puts("Pressed");
		repeat_timer = 0;
		button_event = buttons;
	}
	else if (buttons == buttons_old && buttons)
	{
		/* A button is being held */
		if (((Draw::current_angle / 256) & 0x07) == 0)
		{
			if (!hold)
			{
				if (repeat_timer < 8)
				{
					repeat_timer++;
				}
				else
				{
					button_event = buttons;
				}
				hold = true;
			}
		}
		else
		{
			hold = false;
		}
	}

	buttons_old = buttons;

	return button_event;
}

void handle_buttons()
{
	static int8_t wait_save_correction = 0;

	int8_t button_event = get_button_event();

	if (!button_event)
	{
		return;
	}

	/* SW1 changes the state */
	if (button_event & 0x01)
	{
		if (++button_state > 2)
		{
			button_state = 0;
			force_update = true;
		}
	}

	switch(button_state)
	{
	case 0:
		time_set = false;
		blink_hours = blink_minutes = false;
		if (button_event & 0x04)
		{
			correction+=3;
			Draw::offset_angle+=3;
			wait_save_correction = 3;
		}

		if (button_event & 0x02)
		{
			correction-=3;
			Draw::offset_angle-=3;
			wait_save_correction = 3;
		}
		break;

	case 1:
	case 2:
		time_set = true;
		blink_hours = button_state == 1;
		blink_minutes = button_state == 2;

		/* SW2 or 3 */
		if (button_event & 0x06)
		{
			struct time time;
			DS1338::get(time);
			uint8_t hours = time.get_hour();
			uint8_t mins = time.get_min();

			if ((button_state == 1) && (button_event & 0x02)) hours--;
			if ((button_state == 1) && (button_event & 0x04)) hours++;
			if ((button_state == 2) && (button_event & 0x02)) mins--;
			if ((button_state == 2) && (button_event & 0x04)) mins++;

			if (mins > 127) mins = 59;
			if (mins > 59) mins = 0;
			if (hours > 127) hours = 23;
			if (hours > 23) hours = 0;

			time.set_hour(hours);
			time.set_min(mins);
			DS1338::set(time);
		}
		break;
	}

	if (!DS1338::tick)
	{
		return;
	}

	if (wait_save_correction)
	{
		wait_save_correction--;
		if (wait_save_correction == 0)
		{
			serial::puts("Saving correction to EEPROM\n");
			eeprom_write_word((uint16_t*) 0, correction);
		}
	}
}

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

	//ThreePhase::set_amplitude(255);

	uint8_t wait_stable = 0;
	bool ramp_down = false;

	correction = eeprom_read_word((uint16_t*) 0);

	//bool colon_on = false;
	serial::puts("Entering main-loop\n");
	bool done = false;
	for (;;)
	{
		if (done)
		{
			handle_buttons();
		}

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

		if (!DS1338::tick && !time_set && !force_update)
		{
			continue;
		}

		DS1338::tick = false;
		force_update = false;

		if (wait_stable) wait_stable--;

		//serial::puts("tick\n");

		struct time time;
		DS1338::get(time);

		if (!blink_hours || (Draw::current_angle / 8192) & 1)
		{
			Draw::draw_digit(0, time.hour >> 4);
			Draw::draw_digit(35, time.hour & 0xf);
		}
		else
		{
			Draw::draw_digit(0, 11);
			Draw::draw_digit(35, 11);
		}
		if (!blink_minutes || (Draw::current_angle / 8192) & 1)
		{
			Draw::draw_digit(91, time.min >> 4);
			Draw::draw_digit(126, time.min & 0xf);
		}
		else
		{
			Draw::draw_digit(91, 11);
			Draw::draw_digit(126, 11);
		}

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

