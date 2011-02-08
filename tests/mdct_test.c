
#define PI 3.14159265

#include <math.h>

#include <stdlib.h>
#include <stdio.h>

#include "mdct.h"
#include "mdct_vorbis.h"
#include "sample_source.h"

void set_fname_csv( enum sigtype st,
					char **f_result,
					char **f_result_ref,
					char **f_sample)
{
	switch (st) {
	case ZERO:
		*f_result = "out_mdct_ZERO.csv";
		*f_sample = "spl_mdct_ZERO.csv";
		*f_result_ref = "refout_mdct_ZERO.csv";
		break;
	case SINE:
		*f_result = "out_mdct_SINE.csv";
		*f_sample = "spl_mdct_SINE.csv";
		*f_result_ref = "refout_mdct_SINE.csv";
		break;
	case SEQ:
		*f_result = "out_mdct_SEQ.csv";
		*f_sample = "spl_mdct_SEQ.csv";
		*f_result_ref = "refout_mdct_SEQ.csv";
		break;
	case RANDOM:
		*f_result = "out_mdct_RND.csv";
		*f_sample = "spl_mdct_RND.csv";
		*f_result_ref = "refout_mdct_RND.csv";
		break;
	case WHITE_NOISE:
		*f_result = "out_mdct_WHITENOISE.csv";
		*f_sample = "spl_mdct_WHITENOISE.csv";
		*f_result_ref = "refout_mdct_WHITENOISE.csv";
		break;
	case ALT10:
		*f_result = "out_mdct_ALT10.csv";
		*f_sample = "spl_mdct_ALT10.csv";
		*f_result_ref = "refout_mdct_ALT10.csv";
		break;

	case UNKNOWN:
		
	default:
		*f_result = NULL;
		*f_sample = NULL;
		*f_result_ref = NULL;
	}
}

void set_fname_bin( enum sigtype st,
					char **f_result,
					char **f_result_ref,
					char **f_sample)
{
	switch (st) {
	case ZERO:
		*f_result = "out_mdct_ZERO.bin";
		*f_sample = "spl_mdct_ZERO.bin";
		*f_result_ref = "refout_mdct_ZERO.bin";
		break;
	case SINE:
		*f_result = "out_mdct_SINE.bin";
		*f_sample = "spl_mdct_SINE.bin";
		*f_result_ref = "refout_mdct_SINE.bin";
		break;
	case SEQ:
		*f_result = "out_mdct_SEQ.bin";
		*f_sample = "spl_mdct_SEQ.bin";
		*f_result_ref = "refout_mdct_SEQ.bin";
		break;
	case RANDOM:
		*f_result = "out_mdct_RND.bin";
		*f_sample = "spl_mdct_RND.bin";
		*f_result_ref = "refout_mdct_RND.bin";
		break;
	case WHITE_NOISE:
		*f_result = "out_mdct_WHITENOISE.bin";
		*f_sample = "spl_mdct_WHITENOISE.bin";
		*f_result_ref = "refout_mdct_WHITENOISE.bin";
		break;
	case ALT10:
		*f_result = "out_mdct_ALT10.bin";
		*f_sample = "spl_mdct_ALT10.bin";
		*f_result_ref = "refout_mdct_ALT10.bin";
		break;

	case UNKNOWN:
		
	default:
		*f_result = NULL;
		*f_sample = NULL;
		*f_result_ref = NULL;
	}
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
								* (PI/(2*N))
								);
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

int main(int argc, char *argv[])
{
	float *sample;
	float *result;
	float *result_ref;

	char *f_sample;
	char *f_result;
	char *f_result_ref;

	enum sigtype st;

	st = get_signal(argc, argv, &sample);

	if (st == UNKNOWN) {
		fprintf(stderr, "Invalid or no signal type selected.\n");
		return -1;
	}

	set_fname_csv(st, &f_result, &f_result_ref, &f_sample);

	result = mdct(2048, sample);
	result_ref = mdct_forward_brute(sample, 2048);

	totxtf( result,
			1024, ",",
			1024, "\n",
			f_result,
			"out",
			"MDCT");

	totxtf( result_ref,
			1024, ",",              // element count and separator
			1024, "\n",             // line length and separator
			f_result_ref,           // output file
			"out",                  // output dir
			"MDCT");                // module name

	totxtf( sample,
			2048, ",",              // element count and separator
			2048, "\n",             // line length and separator
			f_sample,               // output file
			"out",                  // output dir
			"MDCT");                // module name


	set_fname_bin(st, &f_result, &f_result_ref, &f_sample);
	// binary output
	tobinf( result,
			4096,
			f_result,
			"out",
			"MDCT");

	tobinf( result_ref,
			4096,
			f_result_ref,
			"out",
			"MDCT");

	tobinf( sample,
			2048,
			f_sample,
			"out",
			"MDCT");


	free(sample);
	free(result);
	free(result_ref);

	return EXIT_SUCCESS;
}
