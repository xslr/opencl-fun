#ifndef AACL_FFT_H
#define AACL_FFT_H

#include <stdlib.h>

#define BLK_SIZE 1024

extern float *fft(size_t count, float *samples,
				  double *time_upload, double *time_exec, double *time_download,
				  int profiling);

#endif
