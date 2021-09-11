#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include "SDL2/SDL2_gfxPrimitives.h"


double segment_angle(int segment)
{
	const double x = 4;

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

void draw_segment(SDL_Renderer* renderer, SDL_Texture* texture, double angle, int segment)
{
	angle += segment_angle(segment);

	SDL_RenderCopyEx( renderer, texture, NULL, NULL, angle, NULL, SDL_FLIP_NONE);
}

void draw_segments(SDL_Renderer* renderer, SDL_Texture* texture, double angle, uint8_t segment_mask)
{
	for (int i = 0; i < 8; i++)
	{
		if (segment_mask & 1)
		{
			draw_segment(renderer, texture, angle, i);
		}

		segment_mask >>= 1;
	}
}

void draw_digit(SDL_Renderer* renderer, SDL_Texture* texture, double angle, int digit)
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

	draw_segments(renderer, texture, angle, digits[digit]);
}

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

void hor_segment(int top, double straight_a, double tip_a, int height,
	       int midx, int midy, int16_t* x, int16_t* y)
{
	//void mk_point(int x, int y, int midx, int midy, int16_t* o_x, int16_t* o_y, double a)
	mk_point(0, top,
			midx, midy, &x[0], &y[0], straight_a / 2.0);
	mk_point(0, top - height / 2,
			midx, midy, &x[1], &y[1], straight_a / 2.0 + tip_a);
	mk_point(0, top - height,
			midx, midy, &x[2], &y[2], straight_a / 2.0);
	mk_point(0, top - height,
			midx, midy, &x[3], &y[3], -straight_a / 2.0);
	mk_point(0, top - height / 2,
			midx, midy, &x[4], &y[4], -straight_a / 2.0 - tip_a);
	mk_point(0, top,
			midx, midy, &x[5], &y[5], -straight_a / 2.0);
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
		
		int x, y;
		int16_t digit_xpoints[6];
		int16_t digit_ypoints[6];

		int digit_top = -r + 10 + 120;
		int digit_straight = 100;
		int digit_tip = 10;
		double digit_width_deg = 2;

		vert_segment(digit_top, digit_straight, digit_tip, deg(digit_width_deg),
				               midx, midy, digit_xpoints, digit_ypoints);
		rotate_poly(digit_xpoints, digit_ypoints, 6, midx, midy, deg(7));
		filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

		vert_segment(digit_top + 130, digit_straight, digit_tip, deg(digit_width_deg),
				               midx, midy, digit_xpoints, digit_ypoints);
		rotate_poly(digit_xpoints, digit_ypoints, 6, midx, midy, deg(7));
		filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	


		vert_segment(digit_top, digit_straight, digit_tip, deg(digit_width_deg),
				               midx, midy, digit_xpoints, digit_ypoints);
		rotate_poly(digit_xpoints, digit_ypoints, 6, midx, midy, deg(-7));
		filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

		vert_segment(digit_top + 130, digit_straight, digit_tip, deg(digit_width_deg),
				               midx, midy, digit_xpoints, digit_ypoints);
		rotate_poly(digit_xpoints, digit_ypoints, 6, midx, midy, deg(-7));
		filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	



		hor_segment(-r + 20, deg(10), deg(1), 20,
				midx, midy, digit_xpoints, digit_ypoints);
		filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

		hor_segment(-r + 145, deg(10), deg(1), 20,
				midx, midy, digit_xpoints, digit_ypoints);
		filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

		hor_segment(-r + 270, deg(10), deg(1), 20,
				midx, midy, digit_xpoints, digit_ypoints);
		filledPolygonColor(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	


		SDL_RenderPresent(renderer);
		SDL_Delay(10);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
