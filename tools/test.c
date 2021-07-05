#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>

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

int main(int argc, char ** argv)
{
	int quit = 0;
	SDL_Event event;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("SDL2 Displaying Image",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1920, 0);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Surface* image = SDL_LoadBMP("digitdisk.bmp");
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);

	while (!quit)
	{
		SDL_WaitEvent(&event);

		switch (event.type)
		{
			case SDL_QUIT:
				quit = 1;
				break;
		}

		time_t rawtime;
		struct tm * timeinfo;

		time ( &rawtime );
		timeinfo = localtime ( &rawtime );

		double angle_space = 10.5;
		double angle = - angle_space * 2.0;

		SDL_RenderClear(renderer);

		int first = timeinfo->tm_min - 50;
		int second = timeinfo->tm_sec;

		draw_digit(renderer, texture, angle, first / 10);
		angle += angle_space;
		draw_digit(renderer, texture, angle, first % 10);
		angle += angle_space;
		if ((timeinfo->tm_sec % 2) == 0)
		{
			draw_digit(renderer, texture, angle, 10);
		}
		angle += angle_space;
		draw_digit(renderer, texture, angle, second / 10);
		angle += angle_space;
		draw_digit(renderer, texture, angle, second % 10);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(image);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
