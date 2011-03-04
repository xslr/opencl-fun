#ifndef AACL_MDCT_H
#define AACL_MDCT_H

extern float *mdct(size_t count, float *samples,
				   double *time_upload, double *time_exec, double *time_download,
				   int profiling);
#endif
