/*
 * fft_test_fftw.c
 *
 * Compute fft of generated data using fftw. Used for testing results of OpenCL computed FFT
 * 
 * History: 24-8-2010 / Subrat Meher / Initial (and hopefully final:) version
 */

#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#define SIZE 2048
#define MAXSIZE 2048

double* gen_data(size_t count);
double* calc_fft(double *data, size_t count);
void disp_data(double *data, size_t count);

int main()
{
	double *data = gen_data(SIZE);

	double *result = calc_fft(data, SIZE);

	disp_data(result, SIZE);

	return EXIT_SUCCESS;
}

double* gen_data(size_t count)
{
	double *data = (double*)malloc(count*sizeof(double));

	size_t i;
	for(i=0; i<count; i++)
	{
		data[i] = (i*(2048/SIZE));
	}

	return data;
}

void disp_data(double *data, size_t count)
{
	size_t i;
	for(i=0; i<count; i++)
	{
		printf("%lu:\t%f\t%f\n", i, data[i*2], data[i*2+1]);
	}
}

double* calc_fft(double *data, size_t count)
{
	double *result;
	fftw_complex *in, *out;
	fftw_plan plan;

	result = (double*)malloc(count * 2 * sizeof(double));

	in  = (fftw_complex*)fftw_malloc(count*sizeof(fftw_complex));
	out = (fftw_complex*)fftw_malloc(count*sizeof(fftw_complex));

	size_t i;
	for(i=0; i<count; i++)
	{
		in[i][0] = data[i];
		in[i][1] = 0;
	}

	plan = fftw_plan_dft_1d(count, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	fftw_execute(plan);

	fftw_destroy_plan(plan);

	for(i=0; i<count; i++)
	{
		result[i*2] = out[i][0];
		result[i*2+1] = out[i][1];
	}

	return result;
}
