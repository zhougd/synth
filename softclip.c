#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "synth.h"

/* softclip.c: soft clip a waveform */

static void softclip(float range, float gradation);

int main(int argc, char *argv[])
{
	float range = 1.0f;
	float gradation = 1.0f;
	int i;

	/* read options */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-range") && i+1 < argc)
			range = atof(argv[++i]);
		else if (!strcmp(argv[i], "-gradation") && i+1 < argc)
			gradation = atof(argv[++i]);
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -range arg, -gradation "
				"arg\n");
			exit(0);
		}
	}

	/* check options */
	range     = CLAMP(-2.0f, range, 2.0f);
	gradation = CLAMP(0.1f, gradation, 15.0f);

	SET_BINARY_MODE
	softclip(range, gradation);
	return 0;
}

static void softclip(float range, float gradation)
{
	float f;
	float gradinv;

	gradinv = 1.0f/gradation;

	while (fread(&f, sizeof f, 1, stdin) == 1)
	{
		f /= range;

		f = gradinv * atan(f * gradation);

		f *= range;

		if (fwrite(&f, sizeof f, 1, stdout) < 1)
			return;
	}
}
