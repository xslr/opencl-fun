#include "fft.h"

#include "sample_source.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{

	float *samples;

	samples = get_random(2048);

	float *result = fft(2048, samples);

	int ret = tocsvf( result, 4096,
					  "fft_out.txt", NULL, "FFT");

	ret = tocsvf( samples, 2048,
				  "fft_in.txt", NULL, "FFT");

	free(samples);
	free(result);

	return EXIT_SUCCESS;
}
