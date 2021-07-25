/*
 * threephase.h
 *
 *  Created on: 5 Jul 2021
 *      Author: tom
 */

#ifndef SRC_THREEPHASE_H_
#define SRC_THREEPHASE_H_

#include <stdint.h>

class ThreePhase {
public:
	static void init();

	static inline void set_uvw(uint8_t u, uint8_t v, uint8_t w) { OCR0A = u; OCR1A = v; OCR1B = w; }
	static void set_angle(uint16_t angle);
	static void set_amplitude(uint8_t amplitude);
	static uint8_t get_amplitude();
};

#endif /* SRC_THREEPHASE_H_ */
