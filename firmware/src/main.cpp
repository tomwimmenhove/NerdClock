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
	//Draw::vspeed = 512;

	uint8_t old_digits[4];
	uint8_t digits[4];

	Draw::vspeed = 0;
	Draw::amplitude = 32;

	//for (;;);

	for (uint16_t i = 0;;i++)
	{
		if (Draw::vspeed < 65536)
		{
			_delay_us(10);

			Draw::vspeed++;

			int16_t amplitude = 128 +  Draw::vspeed / 512;//256;
			Draw::amplitude = amplitude <= 255 ? amplitude : 255;
		}

		uint16_t mod = i % 5000;

		if (mod == 0)
		{
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

				digits[0] = hours / 10;
				digits[1] = hours % 10;
				digits[2] = mins / 10;
				digits[3] = mins % 10;

				uint8_t angle = 0;
				for (uint8_t j = 0; j < 4; j++)
				{
					uint8_t digit = digits[j];
					Draw::draw_digit(angle, old_digits[j], 0);
					Draw::draw_digit(angle, digit, 1);
					old_digits[j] = digit;
					angle += 35;
					if (j == 1) /* Skip the colon */
					{
						angle += 28;
					}
				}
			}

		/* Blinking colon */
		if (mod < 2500)
		{
			Draw::draw_digit(63, 10, 1);
		}
		else
		{
			Draw::draw_digit(63, 10, 0);
		}
	}
}
