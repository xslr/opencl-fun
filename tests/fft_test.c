#include "fft.h"

#include "sample_source.h"
#include <stdlib.h>
#include <fftw3.h>

float* calc_fft_fftw(float *data, size_t count);
float *diff(float *a, float *b, size_t count);

int main(int argc, char *argv[])
{

	float *samples;
	float *result_gpu;
	float *result_fftw;

	float *diff_gpu_fftw;
	//samples = get_random(2048);
	samples = get_seq(1, 2048);
	//samples = get_sine_wave(8000, 2, 500);

	result_gpu = fft(2048, samples);

	result_fftw = calc_fft_fftw(samples, 2048);

	diff_gpu_fftw = diff(result_gpu, result_fftw, 4096);

	int ret;
	ret = totxtf( result_fftw, 4096, ",", 2048, "\n",
				  "fft_out_fftw.txt", NULL, "FFT");

	ret = totxtf( result_gpu, 4096, ",", 2048, "\n",
				  "fft_out_gpu.txt", NULL, "FFT");

	ret = totxtf( diff_gpu_fftw, 4096, ",", 2048, "\n",
				  "fft_diff_gpu_fftw.txt", NULL, "FFT");

	ret = totxtf( samples, 2048, ",", 2048, "\n",
				  "fft_in.txt", NULL, "FFT");


	free(result_gpu);
	free(result_fftw);
	free(diff_gpu_fftw);
	free(samples);

	return EXIT_SUCCESS;
}

float *diff(float *a, float *b, size_t count)
{
	float *buf = malloc(count * sizeof(float));

	size_t i;
	for (i = 0; i < count; i++)
		buf[i] = b[i] - a[i];

	return buf;
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
