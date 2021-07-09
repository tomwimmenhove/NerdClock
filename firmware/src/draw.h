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
	static void start_detector();

	static void clear();
	static void flip();

	static void draw_digit(int16_t angle, int digit, uint8_t enable);

	// RPS = F_CPU / OCR2A / sizeof(sinewaveLUT) / 65536 / N_POLES * vspeed
	// RPS = vspeed / 1073.742
	static volatile uint32_t speed_target;
	static volatile uint32_t speed_actual;
	static volatile uint8_t amplitude;
	static volatile uint64_t jiffies;
	static volatile uint8_t ramp_time;
	static volatile uint16_t current_angle;
	static volatile uint16_t dot_angle;
	static volatile uint8_t moving;


private:
	static int16_t segment_angle(int segment);
	static void set_angle(int16_t angle, uint8_t enable);
	static void draw_segment(int16_t angle, int segment, uint8_t enable);
	static void draw_segments(int16_t angle, uint8_t segment_mask, uint8_t enable);
};

#endif /* SRC_DRAW_H_ */
