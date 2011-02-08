#include "fft.h"

#include "sample_source.h"
#include <stdlib.h>
#include <fftw3.h>

void set_fname_csv( enum sigtype st,
					char **f_result,
					char **f_result_ref,
					char **f_sample)
{
	switch (st) {
	case ZERO:
		*f_result = "out_fft_ZERO.csv";
		*f_sample = "spl_fft_ZERO.csv";
		*f_result_ref = "refout_fft_ZERO.csv";
		break;
	case SINE:
		*f_result = "out_fft_SINE.csv";
		*f_sample = "spl_fft_SINE.csv";
		*f_result_ref = "refout_fft_SINE.csv";
		break;
	case SEQ:
		*f_result = "out_fft_SEQ.csv";
		*f_sample = "spl_fft_SEQ.csv";
		*f_result_ref = "refout_fft_SEQ.csv";
		break;
	case RANDOM:
		*f_result = "out_fft_RND.csv";
		*f_sample = "spl_fft_RND.csv";
		*f_result_ref = "refout_fft_RND.csv";
		break;
	case WHITE_NOISE:
		*f_result = "out_fft_WHITENOISE.csv";
		*f_sample = "spl_fft_WHITENOISE.csv";
		*f_result_ref = "refout_fft_WHITENOISE.csv";
		break;
	case ALT10:
		*f_result = "out_fft_ALT10.csv";
		*f_sample = "spl_fft_ALT10.csv";
		*f_result_ref = "refout_fft_ALT10.csv";
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
		*f_result = "out_fft_ZERO.bin";
		*f_sample = "spl_fft_ZERO.bin";
		*f_result_ref = "refout_fft_ZERO.bin";
		break;
	case SINE:
		*f_result = "out_fft_SINE.bin";
		*f_sample = "spl_fft_SINE.bin";
		*f_result_ref = "refout_fft_SINE.bin";
		break;
	case SEQ:
		*f_result = "out_fft_SEQ.bin";
		*f_sample = "spl_fft_SEQ.bin";
		*f_result_ref = "refout_fft_SEQ.bin";
		break;
	case RANDOM:
		*f_result = "out_fft_RND.bin";
		*f_sample = "spl_fft_RND.bin";
		*f_result_ref = "refout_fft_RND.bin";
		break;
	case WHITE_NOISE:
		*f_result = "out_fft_WHITENOISE.bin";
		*f_sample = "spl_fft_WHITENOISE.bin";
		*f_result_ref = "refout_fft_WHITENOISE.bin";
		break;
	case ALT10:
		*f_result = "out_fft_ALT10.bin";
		*f_sample = "spl_fft_ALT10.bin";
		*f_result_ref = "refout_fft_ALT10.bin";
		break;

	case UNKNOWN:
		
	default:
		*f_result = NULL;
		*f_sample = NULL;
		*f_result_ref = NULL;
	}
}

float *calc_fft_fftw(float *data, size_t count)
{
	float *result;
	fftw_complex *in, *out;
	fftw_plan plan;

	// buffer for holding the real and complex components of fft
	result = malloc(count * 2 * sizeof(float));

	in  = fftw_malloc(count*sizeof(fftw_complex));
	out = fftw_malloc(count*sizeof(fftw_complex));

	size_t i;
	for (i = 0; i < count; i++)
	{
		in[i][0] = data[i];
		in[i][1] = 0;
	}

	plan = fftw_plan_dft_1d(count, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	fftw_execute(plan);

	fftw_destroy_plan(plan);

	for (i = 0; i < count; i++)
	{
		result[i] = out[i][0];
		result[i+2048] = out[i][1];
	}

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

	result = fft(2048, sample);
	result_ref = calc_fft_fftw(sample, 2048);

	totxtf( result,
			4096, ",",
			2048, "\n",
			f_result,
			"out",
			"FFT");

	totxtf( result_ref,
			4096, ",",
			2048, "\n",
			f_result_ref,
			"out",
			"FFT");

	totxtf( sample,
			2048, ",",
			2048, "\n",
			f_sample,
			"out",
			"FFT");

	set_fname_bin(st, &f_result, &f_result_ref, &f_sample);
	// binary output
	tobinf( result,
			4096,
			f_result,
			"out",
			"FFT");

	tobinf( result_ref,
			4096,
			f_result_ref,
			"out",
			"FFT");

	tobinf( sample,
			2048,
			f_sample,
			"out",
			"FFT");


	free(result);
	free(result_ref);
	free(sample);

	return EXIT_SUCCESS;
}
