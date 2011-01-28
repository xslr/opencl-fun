#include <stdlib.h>
#include <stdio.h>

#include "mdct.h"
#include "sample_source.h"

int main(int argc, char *argv[])
{
	//float *samples = getRandom(2048);
	//float *samples = getAlt_1_0(2048);
	float *samples;
	samples = get_zero(2048);

	float *result = mdct(2048, samples);
	free(samples);

	totxtf(result, 1024, ",", 1024, "\n", "data.txt", "out", "MDCT");
	free(result);

	return EXIT_SUCCESS;
}
