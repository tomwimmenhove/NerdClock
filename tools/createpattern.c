#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include "SDL2/SDL2_gfxPrimitives.h"

void rotate(int* x, int* y, double a)
{
	double x2 = cos(a) * *x - sin(a) * *y;
	double y2 = sin(a) * *x + cos(a) * *y;

	*x = (int) round(x2);
	*y = (int) round(y2);
}

void rotate_poly(int16_t* x, int16_t* y, int n, int midx, int midy, double a)
{
	double c = cos(a);
	double s = sin(a);

	for(int i = 0; i < n; i++)
	{
		int16_t x1 = x[i] - midx;
		int16_t y1 = y[i] - midy;

		double x2 = c * x1 - s * y1;
		double y2 = s * x1 + c * y1;

		x[i] = ((int16_t) round(x2)) + midx;
		y[i] = ((int16_t) round(y2)) + midy;
	}
}

double deg(double deg)
{
	return deg * M_PI / 180.0;
}

void mk_point(int x, int y, int midx, int midy, int16_t* o_x, int16_t* o_y, double a)
{
	rotate(&x, &y, a);
	*o_x = midx + x;
	*o_y = midy + y;
}

void vert_segment(int top, int straight, int tip, double a,
	       int midx, int midy, int16_t* x, int16_t* y)
{
	mk_point(0, top,
			midx, midy, &x[0], &y[0], 0);
	mk_point(0, top - tip, midx,
			midy, &x[1], &y[1], a/2.0);
	mk_point(0, top - straight - tip,
			midx, midy, &x[2], &y[2], a/2.0);
	mk_point(0, top - straight - tip * 2,
			midx, midy, &x[3], &y[3], 0);
	mk_point(0, top - straight - tip,
			midx, midy, &x[4], &y[4], -a/2.0);
	mk_point(0, top - tip, midx, midy,
			&x[5], &y[5], -a/2.0);
}

void hor_segment(int center_y, double straight_a, double tip_a, int height,
	       int midx, int midy, int16_t* x, int16_t* y)
{
	//void mk_point(int x, int y, int midx, int midy, int16_t* o_x, int16_t* o_y, double a)
	mk_point(0, center_y + height / 2,
			midx, midy, &x[0], &y[0], straight_a / 2.0);
	mk_point(0, center_y,
			midx, midy, &x[1], &y[1], straight_a / 2.0 + tip_a);
	mk_point(0, center_y - height / 2,
			midx, midy, &x[2], &y[2], straight_a / 2.0);
	mk_point(0, center_y - height / 2,
			midx, midy, &x[3], &y[3], -straight_a / 2.0);
	mk_point(0, center_y,
			midx, midy, &x[4], &y[4], -straight_a / 2.0 - tip_a);
	mk_point(0, center_y + height / 2,
			midx, midy, &x[5], &y[5], -straight_a / 2.0);
}

void draw_eight(SDL_Renderer* renderer, int midx, int midy, int r, double a)
{
	int top_space = 20;
	int vert_digit_straight = 50;
	int vert_digit_tip = 3;
	int vert_digit_space = 3;
	double vert_digit_width_deg = 1;
	double hor_digit_width = deg(5);
	double hor_digit_tip_width = deg(.5);

	int16_t digit_xpoints[6];
	int16_t digit_ypoints[6];
	int vert_digit_bottom = -r + top_space + vert_digit_straight + 2 * vert_digit_tip;

	vert_segment(vert_digit_bottom, vert_digit_straight, vert_digit_tip, deg(vert_digit_width_deg),
			midx, midy, digit_xpoints, digit_ypoints);
	rotate_poly(digit_xpoints, digit_ypoints, 6, midx, midy, a + deg(60));
	filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

	vert_segment(vert_digit_bottom + vert_digit_straight + 2 * vert_digit_tip + vert_digit_space,
			vert_digit_straight, vert_digit_tip, deg(vert_digit_width_deg),
			midx, midy, digit_xpoints, digit_ypoints);
	rotate_poly(digit_xpoints, digit_ypoints, 6, midx, midy, a + deg(120));
	filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	


	int center_y = -r + top_space - vert_digit_space / 2;
	int hor_digit_height = (int) round(vert_digit_width_deg / 360.0 * 2 * M_PI * (-center_y));
	hor_segment(center_y, hor_digit_width, hor_digit_tip_width, hor_digit_height,
			midx, midy, digit_xpoints, digit_ypoints);
	rotate_poly(digit_xpoints, digit_ypoints, 6, midx, midy, a + deg(180));
	filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

	center_y += vert_digit_straight + 2 * vert_digit_tip + vert_digit_space;
	hor_digit_height = (int) round(vert_digit_width_deg / 360.0 * 2 * M_PI * (-center_y));
	hor_segment(center_y, hor_digit_width, hor_digit_tip_width, hor_digit_height,
			midx, midy, digit_xpoints, digit_ypoints);
	rotate_poly(digit_xpoints, digit_ypoints, 6, midx, midy, a + deg(240));
	filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

	center_y += vert_digit_straight + 2 * vert_digit_tip + vert_digit_space;
	hor_digit_height = (int) round(vert_digit_width_deg / 360.0 * 2 * M_PI * (-center_y));
	hor_segment(center_y, hor_digit_width, hor_digit_tip_width, hor_digit_height,
			midx, midy, digit_xpoints, digit_ypoints);
	rotate_poly(digit_xpoints, digit_ypoints, 6, midx, midy, a + deg(300));
	filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);

	int x = 0;
	int y = - r + top_space + vert_digit_space + (vert_digit_straight + vert_digit_tip * 2) / 2;
	rotate(&x, &y, a);
	filledCircleColor(renderer, midx + x, midy + y, 15, 0xffffffff);

	x = 0;
	y = - r + top_space + vert_digit_space + (vert_digit_straight + vert_digit_tip * 2) / 2
		+ vert_digit_straight + vert_digit_tip * 2 + vert_digit_space;
	rotate(&x, &y, a);
	filledCircleColor(renderer, midx + x, midy + y, 15, 0xffffffff);
}

double segment_angle(int segment)
{
	const double x = 3.5;

	switch(segment)
	{
		case 0: return 180.0;
		case 1: return 300.0 + x;
		case 2: return 240.0 + x;
		case 3: return 60.0;
		case 4: return 240.0 - x;
		case 5: return 300.0 -x;
		case 6: return 120.0;
		case 7: return 0.0;
	}
}

void draw_segment(SDL_Renderer* renderer, int midx, int midy, int r, double angle, int segment)
{
	angle += deg(segment_angle(segment));

	draw_eight(renderer, midx, midy, r, angle);
}

void draw_segments(SDL_Renderer* renderer, int midx, int midy, int r, double angle, uint8_t segment_mask)
{
	for (int i = 0; i < 8; i++)
	{
		if (segment_mask & 1)
		{
			draw_segment(renderer, midx, midy, r, angle, i);
		}

		segment_mask >>= 1;
	}
}

void draw_digit(SDL_Renderer* renderer, int midx, int midy, int r, double angle, int digit)
{
	static uint8_t digits[] =
	{
		0b00111111,
		0b00000110,
		0b01011011,
		0b01001111,
		0b01100110,
		0b01101101,
		0b01111101,
		0b00000111,
		0b01111111,
		0b01101111,
		0b10000000,
	};

	draw_segments(renderer, midx, midy, r, angle, digits[digit]);
}


int main(int argc, char ** argv)
{
	int width = 1920;
	int height = 1920;
	int midx = width / 2;
	int midy = height / 2;

	int r = 800;

	int quit = 0;
	SDL_Event event;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("Digits and shit",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1920, 0);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	while (!quit)
	{
		SDL_WaitEvent(&event);

		switch (event.type)
		{
			case SDL_QUIT:
				quit = 1;
				break;
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);//0xFF, 0xFF);
		SDL_RenderClear(renderer);
		//thickLineColor(renderer, 0, 0, 1000, 1000, 20, 0xFF00FFFF) ;
		//thickLineColor(renderer, 0, 1000, 1000, 0, 20, 0xFF00FFFF) ;

		circleColor (renderer, midx, midy, r, 0xffffffff);

		//draw_eight(renderer, midx, midy, r, deg(0));
		
		draw_digit(renderer, midx, midy, r, deg(-22.1), 1);
		draw_digit(renderer, midx, midy, r, deg(-9.8), 2);
		draw_digit(renderer, midx, midy, r, 0, 10);
		draw_digit(renderer, midx, midy, r, deg(9.8), 3);
		draw_digit(renderer, midx, midy, r, deg(22.1), 4);

		/*
		draw_eight(renderer, midx, midy, r, deg(-15));
		draw_eight(renderer, midx, midy, r, deg(-5));
		draw_eight(renderer, midx, midy, r, deg(5));
		draw_eight(renderer, midx, midy, r, deg(15));
		*/

		SDL_RenderPresent(renderer);
		SDL_Delay(10);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
