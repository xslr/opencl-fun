#ifndef AACL_FFT_H
#define AACL_FFT_H

#include <stdlib.h>

#define BLK_SIZE 1024

extern float *fft(size_t count, float *samples,
				  float *time_upload, float *time_exec, float *time_download,
				  int profiling);

#endif
