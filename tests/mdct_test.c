#include <stdlib.h>
#include <stdio.h>

#include "mdct.h"
#include "sample_source.h"

int main(int argc, char *argv[])
{
	float *samples;
	//samples = getRandom(2048);
	//samples = getAlt_1_0(2048);
	//samples = get_seq(1, 2048);
	//samples = get_sine_wave(8000, 2, 500);
	samples = get_zero(2048);

	float *result = mdct(2048, samples);
	free(samples);

	totxtf(result, 1024, ",", 1024, "\n", "mdct_out_gpu_zero.txt", "out", "MDCT");
	free(result);

	return EXIT_SUCCESS;
}
