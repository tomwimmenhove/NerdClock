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

	volatile static uint8_t u;
	volatile static uint8_t v;
	volatile static uint8_t w;
};

#endif /* SRC_THREEPHASE_H_ */
