
#include "fft.h"

#include "sample_source.h"
#include <stdlib.h>
#include <fftw3.h>
#include <sys/time.h>

float *calc_fft_fftw(float *data, size_t count,
					 double *time_plan_us, double *time_exec_us)
{
	float *result;
	fftw_complex *in, *out;
	fftw_plan plan;

	struct timeval plan_start,
		plan_end,
		exec_start,
		exec_end;

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

	gettimeofday(&plan_start, NULL);
	plan = fftw_plan_dft_1d(count, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	gettimeofday(&plan_end, NULL);

	gettimeofday(&exec_start, NULL);
	fftw_execute(plan);
	gettimeofday(&exec_end, NULL);

	fftw_destroy_plan(plan);

	for (i = 0; i < count; i++)
	{
		result[i] = out[i][0];
		result[i+2048] = out[i][1];
	}

	*time_plan_us = (plan_end.tv_sec - plan_start.tv_sec) * 1000000;
	*time_plan_us += (plan_end.tv_usec - plan_start.tv_usec);
	*time_exec_us = (exec_end.tv_sec - exec_start.tv_sec) * 1000000;
	*time_exec_us += (exec_end.tv_usec - exec_start.tv_usec);

	return result;
}

int getSignal(int argc, char *argv[], float **buf)
{
	int count;
	count = BLK_SIZE*(atoi(argv[1]) + 1);

	*buf = get_zero(count);

	return count;
}

void print_timings(double time_gpu_upload,
				   double time_gpu_download,
				   double time_gpu_exec,
				   double time_gpu_total,
				   double time_fftw_plan,
				   double time_fftw_exec,
				   double time_fftw_total)
{
	printf("GPU (us): %f\n\tUL:%f\tEX:%f\tDL:%f\n",
		   time_gpu_total, time_gpu_upload, time_gpu_exec, time_gpu_download);
	printf("FFTW (us): %f\n\tPLAN:%f\tEX:%f\n",
		   time_fftw_total, time_fftw_plan, time_fftw_exec);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Invalid argument.\n\tUsage: fftperf num_blocks\n");
		return -1;
	}

	int count;

	float *sample;
	float *result;
	float *result_ref;

	double time_gpu_upload,
		time_gpu_download,
		time_gpu_exec,
		time_gpu_total,
		time_fftw_plan,
		time_fftw_exec,
		time_fftw_total;

	count = getSignal(argc, argv, &sample);

	fprintf(stderr,"count:%d\n", count);

	result = fft(count, sample,
				 &time_gpu_upload, &time_gpu_exec, &time_gpu_download,
				 1);

	// TODO adjust FFT length
	result_ref = calc_fft_fftw(sample, count,
							   &time_fftw_plan, &time_fftw_exec);

	free(result);
	free(result_ref);
	free(sample);

	time_gpu_total = time_gpu_upload + time_gpu_exec + time_gpu_download;
	time_fftw_total = time_fftw_plan + time_fftw_exec;

	print_timings(time_gpu_upload, time_gpu_download, time_gpu_exec, time_gpu_total,
				  time_fftw_plan, time_fftw_exec, time_fftw_total);

	return EXIT_SUCCESS;
}
