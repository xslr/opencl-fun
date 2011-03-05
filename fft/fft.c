
#define FFT_KERNEL_FILE "fft_2048.cl"
#define FFT_KERNEL_NAME "fft_2048"

#include <stdlib.h>

#define BLK_SIZE 1024

#include "clutil.h"

float *fft(size_t count, float *samples,
		   float *time_upload, float *time_exec, float *time_download,
		   int profiling)
{
	DevInfo info;
	aacl_kernel_ctx ctx;
	cl_int err;
	float *result;

	cl_ulong upload_start;
	cl_ulong upload_end;
	cl_ulong exec_start;
	cl_ulong exec_end;
	cl_ulong download_start;
	cl_ulong download_end;

	cl_event ev_upload;
	cl_event ev_exec;
	cl_event ev_download;

	ctx.ws_local = BLK_SIZE;
	ctx.ws_global = (count/BLK_SIZE - 1) * BLK_SIZE;

	ctx.in_count = count;
	ctx.out_count = (count/BLK_SIZE - 1) * (BLK_SIZE<<2);

	init_cl(&ctx, &info, "FFT");
	dev_diag(&info);

	buffer_create(samples, &ctx, "FFT");
	build_kernel( FFT_KERNEL_FILE,
				  FFT_KERNEL_NAME,
				  &ctx,
				  &info,
				  "FFT");


	// set kernel arguments
	err = clSetKernelArg(ctx.kernel, 0, sizeof(cl_mem), &ctx.in);
	check_cl_err(err, "FFT", "Kernel::InputArg");
	err = clSetKernelArg(ctx.kernel, 1, sizeof(cl_mem), &ctx.out);
	check_cl_err(err, "FFT", "Kernel::OutputArg");

	if (profiling != 0) {
		upload_samples(&ctx, &ev_upload, samples);
		enqueue_kernel(&ctx, &ev_exec, &info, "FFT");
		result = download_result(&ctx, &ev_download, "FFT");

		clGetEventProfilingInfo( ev_upload,
								 CL_PROFILING_COMMAND_START,
								 sizeof(cl_ulong),
								 &upload_start,
								 NULL);
		clGetEventProfilingInfo( ev_upload,
								 CL_PROFILING_COMMAND_END,
								 sizeof(cl_ulong),
								 &upload_end,
								 NULL);
		clGetEventProfilingInfo( ev_exec,
								 CL_PROFILING_COMMAND_START,
								 sizeof(cl_ulong),
								 &exec_start,
								 NULL);
		clGetEventProfilingInfo( ev_exec,
								 CL_PROFILING_COMMAND_END,
								 sizeof(cl_ulong),
								 &exec_end,
								 NULL);
		clGetEventProfilingInfo( ev_download,
								 CL_PROFILING_COMMAND_START,
								 sizeof(cl_ulong),
								 &download_start,
								 NULL);
		clGetEventProfilingInfo( ev_download,
								 CL_PROFILING_COMMAND_END,
								 sizeof(cl_ulong),
								 &download_end,
								 NULL);

		*time_upload = (float)(upload_end - upload_start) / 1000.0f;
		*time_exec = (float)(exec_end - exec_start) / 1000.0f;
		*time_download = (float)(download_end - download_start) / 1000.0f;
	} else {
		upload_samples(&ctx, NULL, samples);
		enqueue_kernel(&ctx, NULL, &info, "FFT");
		result = download_result(&ctx, NULL, "FFT");
	}

	return result;
}
