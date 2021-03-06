#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"
#include "rate.inc"

/* env.c: apply or generate an envelope */

typedef struct
{
	int len;
	float end;
} envpoint_t;

static void envelope(float start, envpoint_t *envs, int numenvs, int apply);

#define MAXPTS 400

int main(int argc, char *argv[])
{
	float start = 1.0f;
	int apply;
	envpoint_t envs[MAXPTS];
	int numenvs = 0;
	int i;
	int startset = 0;

	get_rate();

	/* by default apply env if stdin is not tty */
	apply = !isatty(STDIN_FILENO);

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-generate")) /* generate envelope */
			apply = 0;
		else if (!strcmp(argv[i], "-help"))
		{
			fprintf(stderr, "options: -generate\n");
			fprintf(stderr, "args: start len level "
				"[len level]... -- len is in seconds\n");
			exit(0);
		}
		else if (!startset)
		{
			startset = 1;
			start = atof(argv[i]);
		}
		else if (numenvs == MAXPTS)
		{
			fprintf(stderr, "warning: too many points "
				"(hit limit %d), ignoring rest\n", MAXPTS);
		}
		else if (i + 1 < argc)
		{
			envs[numenvs].len = (int)(RATE * atof(argv[i]));
			i++;
			envs[numenvs].end = atof(argv[i]);
			numenvs++;
		}
	}

	SET_BINARY_MODE
	envelope(start, envs, numenvs, apply);
	return 0;
}

static void envelope(float start, envpoint_t *envs, int numenvs, int apply)
{
	float amp = start, d_amp, end = start;
	float f[2] = {1.0f, 1.0f};
	int stage, pos, len;

	for (stage = 0; stage < numenvs; stage++)
	{
		amp = end; /* prevent it from getting a little off */
		end = envs[stage].end;
		len = envs[stage].len;
		d_amp = (end - amp) / len;
		for (pos = 0; pos < len; pos++)
		{
			if (!apply)
				f[0] = f[1] = amp;
			else if (fread(f, sizeof f[0], 2, stdin) < 2)
				return;
			else
			{
				f[0] *= amp;
				f[1] *= amp;
			}

			amp += d_amp;

			if (fwrite(f, sizeof f[0], 2, stdout) < 2)
				return;
		}
	}
}
