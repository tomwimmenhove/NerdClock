/*
 * twi.cpp
 *
 *  Created on: 13 Jul 2021
 *      Author: tom
 */

#include <avr/io.h>

#include "twi.h"

void Twi::init()
{
	//set SCL to 1MHz
	//(Fc)/[16+2(TWBR)*(PrescalerValue)]
	TWSR = 0x00;
	TWBR = 72; //100kHz
	//enable TWI
	TWCR = (1<<TWEN);
}

void Twi::write(uint8_t u8data)	 //twi write command
{
	TWDR = u8data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	volatile uint8_t status = TWSR & 0xF8;
	(void) status;
}

uint8_t Twi::read_ack(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}

uint8_t Twi::read_nack(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}

void Twi::start(uint8_t address) //twi start command
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));

	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	//volatile uint8_t status= TWSR & 0xF8;
}

void Twi::stop(void) ///TWI stop command
{
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

void Twi::write8(uint8_t address, uint8_t reg, uint8_t data)
{
	start(address);
	write(reg);
	write(data);
	stop();
}

uint8_t Twi::read8(uint8_t address, uint8_t reg)	  //read 8 bits from speciific register
{
	uint8_t temp;
	start(address+1);
	write(reg);
	temp = read_nack();
	stop();
	return temp;
}


