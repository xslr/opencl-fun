#include <stdlib.h>
#include <string.h>
#include <fftw3.h>
#include <math.h>

#include "sample_source.h"

#define PI 3.14159265358979324

void calc_fft_fftw(float *in_r, float *in_i,
					 float **out_r, float **out_i,
					 size_t count)
{
	fftw_complex *in, *out;
	fftw_plan plan;

	// buffer for holding the real and complex components of fft
	*out_r = malloc(count * sizeof(float));
	*out_i = malloc(count * sizeof(float));

	in  = fftw_malloc(count*sizeof(fftw_complex));
	out = fftw_malloc(count*sizeof(fftw_complex));

	size_t i;
	for (i = 0; i < count; i++)
	{
		in[i][0] = in_r[i];
		in[i][1] = in_i[i];
	}

	plan = fftw_plan_dft_1d(count, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	fftw_execute(plan);

	fftw_destroy_plan(plan);

	for (i = 0; i < count; i++)
	{
		(*out_r)[i] = out[i][0];
		(*out_i)[i] = out[i][1];
	}
}

float *mdct_fft_cpu(float *in, size_t count)
{
	float *rot = calloc(count, sizeof(float));
	float *fk = calloc(count/2, sizeof(float));
	float *fk_r = fk;
	float *fk_i = fk + (count/4);
	float *out = calloc(count/2, sizeof(float));
	float *fft_r, *fft_i;

	// rotate samples

	int i;
	float *_in = in + (3*count/4);
	for (i = 0; i < count/4; i++) {
		rot[i] = -1 * _in[i];
	}
	for (i = 0; i < 3*count/4; i++) {
		rot[i+count/4] = in[i];
	}


	totxtf(rot, 2048, ",", 2048, "\n",
				  "rot-c.csv", NULL, "MDCT");


	// calculate fk
	for (i = 0; i < count/4; i++) {
		float temp, temp_r, temp_i;
		temp = -2*PI*(((float)i)+0.125)/((float)count);

		temp_r = cos(temp);
		temp_i = sin(temp);

		/*
		fk_r[i] = temp_r;
		fk_i[i] = temp_i;
		*/

		fk_r[i] = rot[2*i] - rot[count - (2*i) - 1];
		fk_i[i] = -1*rot[(count/2) + (2*i)] + rot[(count/2) - (2*i) - 1];


		float re = (fk_r[i]*temp_r) - (fk_i[i]*temp_i);
		float im = (fk_r[i]*temp_i) + (fk_i[i]*temp_r);
		fk_r[i] = re/2;
		fk_i[i] = im/2;

	}

	totxtf(fk, 1024, ",", 512, "\n",
				  "fkprefft.csv", NULL, "MDCT");


	// calculate N/4 FFT
	calc_fft_fftw(fk_r, fk_i, &fft_r, &fft_i, count/4);


	for (i = 0; i < count/4; i++) {
		fk_r[i] = fft_r[i];
		fk_i[i] = fft_i[i];
	}


	totxtf(fk, 1024, ",", 512, "\n",
				  "fkpostfft.txt", NULL, "MDCT");

	for (i = 0; i < count/4; i++) {
		float temp, temp_r, temp_i;
		temp = -2*PI*(((float)i)+0.125)/((float)count);

		temp_r = cos(temp);
		temp_i = sin(temp);

		temp = fft_r[i]*temp_r - fft_i[i]*temp_i;
		fk_i[i] = fft_r[i]*temp_i + fft_i[i]*temp_r;
		fk_r[i] = temp;
	}

	for (i = 0; i < count/4; i++) {
		out[2*i] = fk_r[i];
		out[(count/2) - (2*i) - 1] = -1 * fk_i[i];
	}

	free(fft_r);
	free(fft_i);

	return out;
}

int main(int argc, char *argv[])
{
	float *samples;
	float *result;

	// 2 second 500Hz sine wave sampled at 2kHz
	samples = get_sine_wave(8000, 2, 500);

	//samples = get_white_noise(2000, 2);

	//samples = get_seq(1, 2048);

	result = mdct_fft_cpu(samples, 2048);

	int ret;
	ret = totxtf( samples, 2048, ",", 2048, "\n",
				  "sample_sine.txt", NULL, "MDCT");

	ret = totxtf( result, 1024, ",", 1024, "\n",
				  "mdct_fft_cpu.txt", NULL, "MDCT");


	free(samples);
	free(result);

	return EXIT_SUCCESS;
}
