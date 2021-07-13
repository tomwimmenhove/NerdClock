/*
 * serial.cpp
 *
 *  Created on: Jun 14, 2021
 *      Author: Tom Wimmenhove
 */

#include <avr/io.h>
#include <stdlib.h>

#include "serial.h"

void serial::init()
{   
	UCSR0A = 1 << U2X0;

	uint16_t ubrr_value = (F_CPU / 4 / 115200 - 1) / 2;

	UBRR0L = ubrr_value;                            //0b01100111
	UBRR0H = (ubrr_value>>8);

	UCSR0B = 0b00011000;                            //(1 << RXEN0)|(1 << TXEN0);
	//Set Frame Format; Asynchronous mode, No Parity, 1 StopBit, char size 8
	UCSR0C = 0b00000110;
}

void serial::putc(char c)
{
	if (c == '\n')
	{
		putc('\r');
	}

	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1 << UDRE0)))
	{ }

	UCSR0A |= 1 << TXC0; // Clear TXC0
	UDR0 = c;
}

void serial::puts(const char* s)
{   
	while(*s)
	{   
		putc(*s);

		s++;
	}
}

void serial::puti(int32_t x)
{   
	char s[11]; 
	puts((const char*) itoa(x, s, 10));
}

void serial::flush()
{
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1<<UDRE0)))
	{ }

	/* Wait for transmit complete */
	while (!(UCSR0A & (1<<TXC0)))
	{ }
}

