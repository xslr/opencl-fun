
#include "fft.h"

#include "sample_source.h"
#include <stdlib.h>
#include <fftw3.h>
#include <time.h>

void calc_fft_fftw(float *data, size_t count, int num_blk,
				   float *time_plan_us, float *time_exec_us)
{
	fftw_complex *in, *out;
	fftw_plan plan;

	struct timespec plan_start,
		plan_end,
		exec_start,
		exec_end;

	// buffer for holding the real and complex components of fft
	in  = fftw_malloc(2*BLK_SIZE*sizeof(fftw_complex));
	out = fftw_malloc(2*BLK_SIZE*sizeof(fftw_complex));

	size_t i;
	for (i = 0; i < BLK_SIZE*2; i++)
	{
		in[i][0] = data[i];
		in[i][1] = 0;
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &plan_start);
	plan = fftw_plan_dft_1d(BLK_SIZE*2, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &plan_end);

	// data elements are not important since we are only perf testing
	// not necessary to updata data
	// execute DFT num_blk times
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &exec_start);
	for (i = 0; i < num_blk; i++) {
		fftw_execute(plan);
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &exec_end);

	fftw_destroy_plan(plan);

	*time_plan_us = (plan_end.tv_sec - plan_start.tv_sec) * 1000000;
	*time_plan_us += (float)(plan_end.tv_nsec - plan_start.tv_nsec) / 1000.0f;
	*time_exec_us = (exec_end.tv_sec - exec_start.tv_sec) * 1000000;
	*time_exec_us += (float)(exec_end.tv_nsec - exec_start.tv_nsec) / 1000.0f;
}

int getSignal(int argc, char *argv[], float **buf)
{
	int count;
	count = BLK_SIZE*(atoi(argv[1]) + 1);

	*buf = get_zero(count);

	return count;
}

void print_timings(int count,
				   float time_gpu_upload,
				   float time_gpu_download,
				   float time_gpu_exec,
				   float time_gpu_total,
				   float time_fftw_plan,
				   float time_fftw_exec,
				   float time_fftw_total)
{
	// count, GPU: total, upload, execute, download
	printf("%d,%f,%f,%f,%f",
		   count,
		   time_gpu_total, time_gpu_upload, time_gpu_exec, time_gpu_download);
	// FFTW: total, plan, execute
	printf(",%f,%f,%f\n",
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

	float time_gpu_upload,
		time_gpu_download,
		time_gpu_exec,
		time_gpu_total,
		time_fftw_plan,
		time_fftw_exec,
		time_fftw_total;

	count = getSignal(argc, argv, &sample);

	fprintf(stderr,"count:%d\n", count);

	fft(count, sample,
		&time_gpu_upload, &time_gpu_exec, &time_gpu_download,
		1);

	// TODO adjust FFT length
	calc_fft_fftw(sample, count, count/2048,
				  &time_fftw_plan, &time_fftw_exec);

	free(sample);

	time_gpu_total = time_gpu_upload + time_gpu_exec + time_gpu_download;
	time_fftw_total = time_fftw_plan + time_fftw_exec;

	print_timings(count/1024 - 1,
				  time_gpu_upload, time_gpu_download, time_gpu_exec, time_gpu_total,
				  time_fftw_plan, time_fftw_exec, time_fftw_total);

	return EXIT_SUCCESS;
}
