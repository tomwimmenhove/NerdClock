/*
 * ds1338.h
 *
 *  Created on: 13 Jul 2021
 *      Author: tom
 */

#ifndef DS1338_H_
#define DS1338_H_

#include <stdint.h>

#include "twi.h"

struct time
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t weekDay;
	uint8_t date;
	uint8_t month;
	uint8_t year;
};

class DS1338
{
public:
	static void init();

	static void set(time& time);
	static void get(time& time);

	static volatile bool tick;

private:
	const static uint8_t address_read = 0xD1u;
	const static uint8_t address_write = 0xD0u;
};

#endif /* DS1338_H_ */
