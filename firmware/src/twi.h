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
	static void init(void);
	static void start(void);
	static void stop(void);
	static void write(uint8_t u8data);
	static uint8_t read_ack(void);
	static uint8_t read_nack(void);
	static uint8_t get_status(void);
};

#endif /* TWI_H_ */
