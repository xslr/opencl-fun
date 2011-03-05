
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

static void mdct_forward_brute(float *in, int N, int num_blk,
								 float *time_us)
{
	// Very slow implementation for brute force testing
	struct timespec start, end;

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
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	*time_us = (end.tv_sec - start.tv_sec) * 1000000;
	*time_us += (float)(end.tv_nsec - start.tv_nsec) / 1000.0f;
}

void mdct_vorbis(float *in, int N, int num_blk,
				 float *time_init, float *time_exec)
{
	struct timespec init_start, init_end,
		exec_start, exec_end;
	mdct_lookup lookup;

	float *result = malloc(2048 * sizeof(float));

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &init_start);
	mdct_init(&lookup, 2048);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &init_end);

	*time_init = (float)(init_end.tv_sec - init_start.tv_sec) * 1000000.0f;
	*time_init += (float)(init_end.tv_nsec - init_start.tv_nsec) / 1000.0f;

	int i;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &exec_start);
	for (i = 0; i < num_blk; i++) {
		mdct_forward(&lookup, in, result);
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &exec_end);

	*time_exec = (exec_end.tv_sec - exec_start.tv_sec) * 1000000;
	*time_exec += (float)(exec_end.tv_nsec - exec_start.tv_nsec) / 1000.0f;

	free(result);
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
				   float time_vorbis_init,
				   float time_vorbis_exec,
				   float time_vorbis_total,
				   float time_brute)
{
	// GPU: total, upload, execute, download
	printf("%d,%f,%f,%f,%f",
		   count,
		   time_gpu_total, time_gpu_upload, time_gpu_exec, time_gpu_download);
	// VORBIS: total, init, execute
	printf(",%f,%f,%f",
		   time_vorbis_total, time_vorbis_init, time_vorbis_exec);
	// BRUTE: total
	printf(",%f",
		   time_brute);

	printf("\n");
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
		time_gpu,
		time_vorbis_init,
		time_vorbis_exec,
		time_vorbis,
		time_brute;

	count = getSignal(argc, argv, &sample);

	mdct(count, sample,
				  &time_gpu_upload, &time_gpu_exec, &time_gpu_download,
				  1);
	//	mdct_forward_brute(sample, count, count/2048,
	//								&time_brute);
	mdct_vorbis(sample, count, count/2048,
								&time_vorbis_init, &time_vorbis_exec);

	free(sample);

	time_gpu = time_gpu_upload + time_gpu_exec + time_gpu_download;
	time_vorbis = time_vorbis_init + time_vorbis_exec;

	print_timings(count/1024 - 1,
				  time_gpu_upload, time_gpu_download, time_gpu_exec, time_gpu,
				  time_vorbis_init, time_vorbis_exec, time_vorbis,
				  time_brute);

	return EXIT_SUCCESS;
}
