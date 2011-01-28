
#define FFT_KERNEL_FILE "fft_2048.cl"
#define FFT_KERNEL_NAME "fft_2048"

#define BLK_SIZE 1024

#include <stdlib.h>

#include "clutil.h"

float *fft(size_t count, float *samples)
{
	DevInfo info;
	aacl_kernel_ctx ctx;
	cl_int err;
	float *result;

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

	upload_samples(samples, &ctx);
	enqueue_kernel(&ctx, &info, "FFT");
	result = download_result(&ctx, "FFT");

	return result;
}
