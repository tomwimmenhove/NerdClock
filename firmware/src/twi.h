/*
 * twi.h
 *
 *  Created on: 13 Jul 2021
 *      Author: tom
 */

#ifndef TWI_H_
#define TWI_H_

#include <stdint.h>

class Twi {
public:
	static void init();
	static void start(uint8_t address);
	static void stop(void);
	static void write(uint8_t u8data);
	static uint8_t read_ack(void);
	static uint8_t read_nack(void);
	static void write8(uint8_t address, uint8_t reg, uint8_t data);
	static uint8_t read8(uint8_t address, uint8_t reg);
};

#endif /* TWI_H_ */
