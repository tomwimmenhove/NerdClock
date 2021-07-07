/*
 * Draw.h
 *
 *  Created on: 5 Jul 2021
 *      Author: tom
 */

#ifndef SRC_DRAW_H_
#define SRC_DRAW_H_

#include <stdint.h>

class Draw {
public:
	static void init();
	static void clear();

	static void draw_digit(double angle, int digit, uint8_t enable);

	// RPS = F_CPU / OCR2A / sizeof(sinewaveLUT) / 65536 / N_POLES * vspeed
	// RPS = vspeed / 1073.742
	static volatile uint32_t speed_target;
	static volatile uint32_t speed_actual;
	static volatile uint8_t amplitude;
	static volatile uint64_t jiffies;
	static volatile uint8_t ramp_time;


private:
	static uint16_t segment_angle(int segment);
	static void set_angle(int16_t angle, uint8_t enable);
	static void draw_segment(uint16_t angle, int segment, uint8_t enable);
	static void draw_segments(uint16_t angle, uint8_t segment_mask, uint8_t enable);
};

#endif /* SRC_DRAW_H_ */
