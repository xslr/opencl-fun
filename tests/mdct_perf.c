
#define PI 3.14159265

#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "mdct.h"
#include "mdct_vorbis.h"
#include "sample_source.h"

// size of input blocks
#define BLK_SIZE 1024

static float *mdct_forward_brute(float *in, int N, int num_blk,
								 double *time_us)
{
	// Very slow implementation for brute force testing
	struct timespec start, end;

	float *out = malloc(N * sizeof(float)/2);

	int k, n;
	float temp;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
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
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	*time_us = (double)(end.tv_sec - start.tv_sec) * (double)1000000.0f;
	*time_us += (double)(end.tv_nsec - start.tv_nsec) / (double)1000.0f;

	return out;
}

float *mdct_vorbis(float *in, int N, int num_blk,
				   double *time_init, double *time_exec)
{
	struct timespec init_start, init_end,
		exec_start, exec_end;
	mdct_lookup lookup;

	float *result = malloc(2048 * sizeof(float));

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &init_start);
	mdct_init(&lookup, 2048);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &init_end);

	*time_init = (double)(init_end.tv_sec - init_start.tv_sec) * (double)1000000.0;
	*time_init += (double)(init_end.tv_nsec - init_start.tv_nsec) / (double)1000.0;

	int i;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &exec_start);
	for (i = 0; i < num_blk; i++) {
		mdct_forward(&lookup, in, result);
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &exec_end);

	*time_exec = (double)(exec_end.tv_sec - exec_start.tv_sec) * (double)1000000.0;
	*time_exec += (double)(exec_end.tv_nsec - exec_start.tv_nsec) / (double)1000.0;
	
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
				   double time_vorbis_init,
				   double time_vorbis_exec,
				   double time_vorbis_total,
				   double time_brute)
{
	printf("GPU (us): %f\n\tUL:%f\tEX:%f\tDL:%f\n",
		   time_gpu_total, time_gpu_upload, time_gpu_exec, time_gpu_download);
	printf("VORBIS (us): %f\n\tPLAN:%f\tEX:%f\n",
		   time_vorbis_total, time_vorbis_init, time_vorbis_exec);
	printf("BRUTE (us): %f\n",
		   time_brute);
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
	float *result_vorbis;

	double time_gpu_upload,
		time_gpu_download,
		time_gpu_exec,
		time_gpu,
		time_vorbis_init,
		time_vorbis_exec,
		time_vorbis,
		time_brute;

	count = getSignal(argc, argv, &sample);

	result = mdct(count, sample,
				  &time_gpu_upload, &time_gpu_exec, &time_gpu_download,
				  1);
	result_ref = mdct_forward_brute(sample, count, count/2048,
									&time_brute);
	result_vorbis = mdct_vorbis(sample, count, count/2048,
								&time_vorbis_init, &time_vorbis_exec);

	free(sample);
	free(result);
	free(result_ref);
	free(result_vorbis);

	time_gpu = time_gpu_upload + time_gpu_exec + time_gpu_download;
	time_vorbis = time_vorbis_init + time_vorbis_exec;

	print_timings(time_gpu_upload, time_gpu_download, time_gpu_exec, time_gpu,
				  time_vorbis_init, time_vorbis_exec, time_vorbis,
				  time_brute);

	return EXIT_SUCCESS;
}
