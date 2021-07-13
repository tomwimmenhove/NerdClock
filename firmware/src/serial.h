/*
 * serial.h
 *
 *  Created on: Jun 14, 2021
 *      Author: Tom Wimmenhove
 */

#ifndef SERIAL_H
#define SERIAL_H

class serial
{
public:
	static void init();

	static void putc(char c);
	static void puts(const char* s);
	static void puti(int32_t x);
	static void flush();
};

#endif /* SERIAL_H */
