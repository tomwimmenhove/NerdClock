#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main()
{
	double x0 = 100.0;
	double y0 = 100.0;

	double r = 55.5 + 1.65 / 2.0;

	int n = 8;

	for (int i = 0; i < n; i++)
	{
		double a = 2 * M_PI * (double) (i + .5) / (double) n;

		double x = sin(a) * r + x0;
		double y = y0 - cos(a) * r;

		printf("D%d: %.3f , %.3f at %.3f\n", i * 2 + 8, x, y, -a * 180.0 / M_PI);
	}

	return 0;
}
