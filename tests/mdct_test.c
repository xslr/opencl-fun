
#define PI 3.14159265

#include <math.h>

#include <stdlib.h>
#include <stdio.h>

#include "mdct.h"
#include "mdct_vorbis.h"
#include "sample_source.h"

float *mdct_vorbis(float *in);
static float *mdct_forward_brute(float *in, int N);

int main(int argc, char *argv[])
{
	float *samples;
	//samples = getRandom(2048);
	//samples = getAlt_1_0(2048);
	//samples = get_seq(1, 2048);
	samples = get_sine_wave(4000, 2, 500);
	//samples = get_zero(2048);
	//samples = get_white_noise(2000, 2);

	float *result = mdct(2048, samples);
	float *result_ref = mdct_forward_brute(samples, 2048);;
	//float *result_ref = mdct_vorbis(samples);;

	totxtf( result, 1024, ",", 1024, "\n", "mdct_out_gpu_sine.txt", "out", "MDCT");

	totxtf( result_ref,
			1024, ",",              // element count and separator
			1024, "\n",             // line length and separator
			"mdct_out_brute_sine.txt",     // output file
			"out",                  // output dir
			"MDCT");                // module name

	totxtf( samples,
			2048, ",",              // element count and separator
			2048, "\n",             // line length and separator
			"samples_sine.txt",     // output file
			"out",                  // output dir
			"MDCT");                // module name

	free(samples);
	free(result);
	free(result_ref);

	return EXIT_SUCCESS;
}

static float *mdct_forward_brute(float *in, int N)
{
	// Very slow implementation for brute force testing

	float *out = malloc(N * sizeof(float)/2);

	int k, n;
	float temp;

	for (k = 0; k < N/2; k++) {
		temp = 0;
		for (n = 0; n < N; n++) {
			temp +=	in[n] * cos(
								((2*n) + 1 + (N/2))
								* ((2*k) + 1)
								* (PI/(2*N)));
		}
		out[k] = temp;
	}
	
	return out;
}

float *mdct_vorbis(float *in)
{
	mdct_lookup lookup;

	float *result = malloc(2048 * sizeof(float));

	mdct_init(&lookup, 2048);
	mdct_forward(&lookup, in, result);

	return result;
}
