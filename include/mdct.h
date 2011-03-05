#ifndef AACL_MDCT_H
#define AACL_MDCT_H

extern float *mdct(size_t count, float *samples,
				   float *time_upload, float *time_exec, float *time_download,
				   int profiling);
#endif
