/*
 * main.cpp
 *
 *  Created on: Jul 5, 2021
 *      Author: Tom Wimmenhove
 */

#include <util/delay.h>

#include "sinetable.h"  // LUT file
#include "draw.h"
#include "threephase.h"

int main()
{
	uint8_t hours = 12;
	uint8_t mins = 0;

	ThreePhase::init();
	Draw::init();
	Draw::vspeed = 512;

	uint8_t last_jiffie = Draw::jiffies;

	for (uint16_t i = 0;;i++)
	{
		if (Draw::vspeed < 25600)
		{
			_delay_us(20);

			Draw::vspeed++;
		}

		uint8_t jiffies = Draw::jiffies;
		if (jiffies != last_jiffie)
		{
			/* A second past */
			if (jiffies == 0)
			{
				Draw::draw_digit(0,hours / 10, 0);
				Draw::draw_digit(32, hours % 10, 0);
				Draw::draw_digit(96, mins / 10, 0);
				Draw::draw_digit(128, mins % 10, 0);

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

				Draw::draw_digit(0,hours / 10, 1);
				Draw::draw_digit(32, hours % 10, 1);
				Draw::draw_digit(96, mins / 10, 1);
				Draw::draw_digit(128, mins % 10, 1);
			}

			last_jiffie = jiffies;
		}

		/* Blinking colon */
		if (jiffies < 50)
		{
			Draw::draw_digit(64, 10, 1);
		}
		else
		{
			Draw::draw_digit(64, 10, 0);
		}
	}
}
