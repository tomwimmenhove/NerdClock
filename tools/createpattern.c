#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include "SDL2/SDL2_gfxPrimitives.h"

void rotate(double* x, double* y, double a)
{
	double x2 = cos(a) * *x - sin(a) * *y;
	double y2 = sin(a) * *x + cos(a) * *y;

	*x = x2;
	*y = y2;
}

void rotate_poly(double* x, double* y, int n, double midx, double midy, double a)
{
	double c = cos(a);
	double s = sin(a);

	for(int i = 0; i < n; i++)
	{
		double x1 = x[i] - midx;
		double y1 = y[i] - midy;

		double x2 = c * x1 - s * y1;
		double y2 = s * x1 + c * y1;

		x[i] = x2 + midx;
		y[i] = y2 + midy;
	}
}

double deg(double deg)
{
	return deg * M_PI / 180.0;
}

void mk_point(double x, double y, double midx, double midy, double* o_x, double* o_y, double a)
{
	rotate(&x, &y, a);
	*o_x = midx + x;
	*o_y = midy + y;
}

void vert_segment(double top, double straight, double tip, double a,
	       double* x, double* y)
{
	mk_point(0, top,
			0.5, 0.5, &x[0], &y[0], 0);
	mk_point(0, top - tip, 0.5,
			0.5, &x[1], &y[1], a/2.0);
	mk_point(0, top - straight - tip,
			0.5, 0.5, &x[2], &y[2], a/2.0);
	mk_point(0, top - straight - tip * 2,
			0.5, 0.5, &x[3], &y[3], 0);
	mk_point(0, top - straight - tip,
			0.5, 0.5, &x[4], &y[4], -a/2.0);
	mk_point(0, top - tip, 0.5, 0.5,
			&x[5], &y[5], -a/2.0);
}

void hor_segment(double center_y, double straight_a, double tip_a, double height,
	       double* x, double* y)
{
	mk_point(0, center_y + height / 2,
			0.5, 0.5, &x[0], &y[0], straight_a / 2.0);
	mk_point(0, center_y,
			0.5, 0.5, &x[1], &y[1], straight_a / 2.0 + tip_a);
	mk_point(0, center_y - height / 2,
			0.5, 0.5, &x[2], &y[2], straight_a / 2.0);
	mk_point(0, center_y - height / 2,
			0.5, 0.5, &x[3], &y[3], -straight_a / 2.0);
	mk_point(0, center_y,
			0.5, 0.5, &x[4], &y[4], -straight_a / 2.0 - tip_a);
	mk_point(0, center_y + height / 2,
			0.5, 0.5, &x[5], &y[5], -straight_a / 2.0);
}

void draw_polygon(SDL_Renderer* renderer, double* x, double* y, int n, uint32_t c)
{
	int16_t ix[n];
	int16_t iy[n];
	int w, h;

	SDL_GetRendererOutputSize(renderer, &w, &h);

	for (int i = 0; i < n; i++)
	{
		ix[i] = (int16_t) round(x[i] * w);
		iy[i] = (int16_t) round(y[i] * h);
	}

	filledPolygonColor(renderer, ix, iy, n, c);	
}

void draw_cirle(SDL_Renderer* renderer, double x, double y, double r, uint32_t c)
{
	int w, h;

	SDL_GetRendererOutputSize(renderer, &w, &h);
	filledCircleColor(renderer, (int) round(x * w), (int) round(y * h), (int) round(r * w), c);
}

void draw_pattern(SDL_Renderer* renderer, double a)
{
	double top_space = .01883239171374764595;
	double vert_digit_straight = .05649717514124293785;
	double vert_digit_tip = .00564971751412429378;
	double vert_digit_space = .00941619585687382297;
	double vert_digit_width_deg = 1.8;
	double hor_digit_width = deg(7);
	double hor_digit_tip_width = deg(.5);

	double digit_xpoints[6];
	double digit_ypoints[6];
	double vert_digit_bottom = -.5 + top_space + vert_digit_straight + 2 * vert_digit_tip;

	vert_segment(vert_digit_bottom, vert_digit_straight, vert_digit_tip, deg(vert_digit_width_deg),
			digit_xpoints, digit_ypoints);
	rotate_poly(digit_xpoints, digit_ypoints, 6, 0.5, 0.5, a + deg(60));
	draw_polygon(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

	vert_segment(vert_digit_bottom + vert_digit_straight + 2 * vert_digit_tip + vert_digit_space,
			vert_digit_straight, vert_digit_tip, deg(vert_digit_width_deg),
			digit_xpoints, digit_ypoints);
	rotate_poly(digit_xpoints, digit_ypoints, 6, 0.5, 0.5, a + deg(120));
	draw_polygon(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

	double center_y = -.5 + top_space - vert_digit_space / 2;
	double hor_digit_height = vert_digit_width_deg / 360.0 * 2 * M_PI * (-center_y);
	hor_segment(center_y, hor_digit_width, hor_digit_tip_width, hor_digit_height,
			digit_xpoints, digit_ypoints);
	rotate_poly(digit_xpoints, digit_ypoints, 6, 0.5, 0.5, a + deg(180));
	draw_polygon(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

	center_y += vert_digit_straight + 2 * vert_digit_tip + vert_digit_space;
	// Cheat
	//hor_digit_height = (int) round(vert_digit_width_deg / 360.0 * 2 * M_PI * (-center_y));
	hor_segment(center_y, hor_digit_width, hor_digit_tip_width, hor_digit_height,
			digit_xpoints, digit_ypoints);
	rotate_poly(digit_xpoints, digit_ypoints, 6, 0.5, 0.5, a + deg(240));
	draw_polygon(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);	

	center_y += vert_digit_straight + 2 * vert_digit_tip + vert_digit_space;
	// Cheat
	//hor_digit_height = (int) round(vert_digit_width_deg / 360.0 * 2 * M_PI * (-center_y));
	hor_segment(center_y, hor_digit_width, hor_digit_tip_width, hor_digit_height,
			digit_xpoints, digit_ypoints);
	rotate_poly(digit_xpoints, digit_ypoints, 6, 0.5, 0.5, a + deg(300));
	draw_polygon(renderer, digit_xpoints, digit_ypoints, 6, 0xffffffff);

	double cr = .01;
	double x = 0;
	double y = - .5 + top_space + vert_digit_space + (vert_digit_straight + vert_digit_tip * 2) / 2;
	rotate(&x, &y, a);
	draw_cirle(renderer, 0.5 + x, 0.5 + y, cr, 0xffffffff);

	x = 0;
	y = - .5 + top_space + vert_digit_space + (vert_digit_straight + vert_digit_tip * 2) / 2
		+ vert_digit_straight + vert_digit_tip * 2 + vert_digit_space;
	rotate(&x, &y, a);
	draw_cirle(renderer, 0.5 + x, 0.5 + y, cr, 0xffffffff);
}

double segment_angle(int segment)
{
	const double x = 3.9;

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

void draw_segment(SDL_Renderer* renderer, double angle, int segment)
{
	angle += deg(segment_angle(segment));

	draw_pattern(renderer, angle);
}

void draw_segments(SDL_Renderer* renderer, double angle, uint8_t segment_mask)
{
	for (int i = 0; i < 8; i++)
	{
		if (segment_mask & 1)
		{
			draw_segment(renderer, angle, i);
		}

		segment_mask >>= 1;
	}
}

void draw_digit(SDL_Renderer* renderer, double angle, int digit)
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

	draw_segments(renderer, angle, digits[digit]);
}


int main(int argc, char ** argv)
{
	int r = 531;
	//int r = 2000;
	int width = r * 2;
	int height = width;

	int quit = 0;

	SDL_Event event;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("Digits and shit",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);

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

		circleColor (renderer, width / 2, height / 2, r, 0xffffffff);

		if (0)
		{
			draw_digit(renderer, deg(-22.1), 1);
			draw_digit(renderer, deg(-9.8), 2);
			draw_digit(renderer, 0, 10);
			draw_digit(renderer, deg(9.8), 8);
			draw_digit(renderer, deg(22.1), 4);
		}
		else
		{
			draw_pattern(renderer, 0);
			SDL_Surface* infoSurface = SDL_GetWindowSurface(window);
			unsigned char * pixels = malloc(infoSurface->w * infoSurface->h *
				       	infoSurface->format->BytesPerPixel);
			SDL_RenderReadPixels(renderer, &infoSurface->clip_rect,
				       	infoSurface->format->format, pixels,
				       	infoSurface->w * infoSurface->format->BytesPerPixel);

			SDL_Surface* saveSurface = SDL_CreateRGBSurfaceFrom(pixels, infoSurface->w, infoSurface->h, infoSurface->format->BitsPerPixel, infoSurface->w * infoSurface->format->BytesPerPixel, infoSurface->format->Rmask, infoSurface->format->Gmask, infoSurface->format->Bmask, infoSurface->format->Amask);

			SDL_SaveBMP(saveSurface, "pattern.bmp");
			SDL_FreeSurface(saveSurface);

			free(pixels);



		}

		SDL_RenderPresent(renderer);
		SDL_Delay(10);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
