/*
 * ds1338.cpp
 *
 *  Created on: 13 Jul 2021
 *      Author: tom
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "ds1338.h"
#include "twi.h"

volatile bool DS1338::tick = false;

void DS1338::init()
{
    EICRA |= (1 << ISC01);  // INT0 on falling edge of PD2
    EIMSK |= (1 << INT0);   // Turns on INT0

	Twi::start();
	Twi::write(address_write);
	Twi::write(7); // Select control register
	Twi::write(0x10); // Enable 1PPS square wave
    Twi::stop();
}

void DS1338::set(time& time)
{
	Twi::start();

	Twi::write(address_write);
	Twi::write(0);

	Twi::write(time.sec);
	Twi::write(time.min);
	Twi::write(time.hour);
	Twi::write(time.weekDay);
	Twi::write(time.date);
	Twi::write(time.month);
	Twi::write(time.year);

    Twi::stop();
}

void DS1338::get(time& time)
{
	Twi::start();                            // Start I2C communication

    Twi::write(address_write);        		// connect to DS1307 by sending its ID on I2c Bus
    Twi::write(0); 							// Request Sec RAM address at 00H

    Twi::stop();                            // Stop I2C communication after selecting Sec Register

    Twi::start();                           // Start I2C communication
    Twi::write(address_read);            	// connect to DS1307(Read mode) by sending its ID

    time.sec     = Twi::read_ack();         // read second and return Positive ACK
    time.min     = Twi::read_ack();         // read minute and return Positive ACK
    time.hour    = Twi::read_ack();         // read hour and return Negative/No ACK
    time.weekDay = Twi::read_ack();         // read weekDay and return Positive ACK
    time.date    = Twi::read_ack();         // read Date and return Positive ACK
    time.month   = Twi::read_ack();         // read Month and return Positive ACK
    time.year    = Twi::read_nack();        // read Year and return Negative/No ACK

    Twi::stop();
}

ISR (INT0_vect)
{
	DS1338::tick = true;
}
