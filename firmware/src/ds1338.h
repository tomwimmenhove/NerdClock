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
	inline uint8_t get_sec() const { return to_decimal(sec); }
	inline uint8_t get_min() const { return to_decimal(min); }
	inline uint8_t get_hour() const { return to_decimal(hour & (is_24() ? 0x3f : 0x1f)); }
	inline uint8_t get_weekDay() const { return week_day; }
	inline uint8_t get_date() const { return to_decimal(date); }
	inline uint8_t get_month() const { return to_decimal(month); }
	inline uint16_t get_year() const { return to_decimal(year) + 2000; }

	inline void set_sec(uint8_t sec) { this->sec = to_nibbles(sec); }
	inline void set_min(uint8_t min) { this->min = to_nibbles(min); }
	inline void set_hour(uint8_t hour, bool is_24 = true, bool pm = false)
	{
		this->hour = to_nibbles(hour);
		if (!is_24)
		{
			this->hour |= 0x40;
			if (pm)
			{
				this->hour |= 0x20;
			}
		}
	}
	inline void set_weekDay(uint8_t week_day) { this->week_day = week_day; }
	inline void set_date(uint8_t date) { this->date = to_nibbles(date); }
	inline void set_month(uint8_t month) { this->month = to_nibbles(month); }
	inline void set_year(uint16_t year) { this->year = to_nibbles(year - 2000); }

	inline bool is_24() const { return (hour & 0x40) == 0; }
	inline bool is_pm() const { return (hour & 0x20) != 0; }

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t week_day;
	uint8_t date;
	uint8_t month;
	uint8_t year;

private:
	inline uint8_t to_decimal(uint8_t nibbles) const  { return (nibbles >> 4) * 10 + (nibbles & 0xf); }
	inline uint8_t to_nibbles(uint8_t decimal) const  { return ((decimal / 10) << 4) | (decimal % 10); }
};

class DS1338
{
public:
	static void init();

	static void set(time& time);
	static void get(time& time);

	static volatile bool tick;

private:
	const static uint8_t address_read = 0xd1;
	const static uint8_t address_write = 0xd0;
};

#endif /* DS1338_H_ */
