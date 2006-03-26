#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "binary.h"

/* limit.c: limit a waveform's dynamic range, roughly */

#ifndef RATE
#define RATE 88200
#endif

#define ATTACK  (RATE * 10/1000)
#define RELEASE (RATE * 30/1000)

#define CLAMP(min, n, max) ((n)<(min)?(min):((n)>(max)?(max):(n)))
#define MIN(x, y) ((x)<(y)?(x):(y))
static void limit(float max);

int main(int argc, char *argv[])
{
	float max = 32768.0f;
	int i;

	SET_BINARY_MODE
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-amp") && ++i < argc)
			max = atof(argv[i]) * 32768.0f;
	}

	max = CLAMP(256.0f, max, 65536.0f);

	limit(max);
	return 0;
}

/* This needs some work, but it now sort of compresses a little.
   To do: Read ATTACK new values in advance and calculate expected based
   on how the sample ATTACK samples forward is expected to actually turn out.
 */
static void limit(float max)
{
	float volume = 1.0f;
	float goal = 1.0f;
	float change = 0.0f;
	float lookout;
	float f;
	float in;
	float expected;
	float oldvalues[ATTACK];
	float maxvalue = 0.0f;
	int start = 0;
	char *fw;
	int c, i, odd = 0;

	fw = (char *)&f;
	lookout = max * 0.9f;

	for (i = 0; i < ATTACK; i++)
		oldvalues[i] = 0.0f;

	for (;;)
	{
		int maxchanged = 0;

		odd = !odd;

		for (i = 0; i < (int)sizeof (float); i++)
		{
			c = getchar();
			if (c == EOF)
				return;
			fw[i] = c;
		}

		in = f;

		/* if what was the max is dropping off, then find a new max */
		if (oldvalues[start] == maxvalue)
		{
			oldvalues[start] = 0.0f;
			maxvalue = 0.0f;
			for (i = 0; i < ATTACK; i++)
				if (oldvalues[i] > maxvalue)
					maxvalue = oldvalues[i];
			maxchanged = 1;
		}

		oldvalues[start] = in;
		start++;
		if (start == ATTACK)
			start = 0;

		if (in > maxvalue)
		{
			maxvalue = in;
			maxchanged = 1;
		}

		f *= volume;

		for (i = 0; i < (int)sizeof (float); i++)
			putchar(fw[i]);

		if (maxchanged)
		{
			expected = maxvalue * goal;
			goal = MIN(lookout / fabs(expected), 1.0f);
		}

		if (odd)
			change = (goal - volume)
				/ (goal > volume ? RELEASE : ATTACK);
		else
		{
			change += (goal - volume)
				/ (goal > volume ? RELEASE : ATTACK);
			change /= 2;
			volume += change;
		}
	}
}
