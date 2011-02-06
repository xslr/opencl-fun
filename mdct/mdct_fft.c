
#define MDCT_KERNEL_FILE "mdct_fft.cl"
#define MDCT_KERNEL_NAME "mdct_fft"

#include <stdlib.h>

#include "clutil.h"

float *mdct(size_t count, float *samples)
{
	DevInfo dev_info;
	aacl_kernel_ctx ctx;
	cl_int err;
	float *result;

	ctx.ws_local = 256;
	ctx.ws_global = count>>3;

	ctx.in_count = count;
	ctx.out_count = count/2;

	init_cl(&ctx, &dev_info, "MDCT");
	dev_diag(&dev_info);

	buffer_create(samples, &ctx, "MDCT");
	build_kernel(MDCT_KERNEL_FILE, MDCT_KERNEL_NAME, &ctx, &dev_info, "MDCT");

	// set kernel arguments
	err = clSetKernelArg(ctx.kernel, 0, sizeof(cl_mem), &ctx.in);
	check_cl_err(err, "MDCT", "Kernel::InputArg");
	err = clSetKernelArg(ctx.kernel, 1, sizeof(cl_mem), &ctx.out);
	check_cl_err(err, "MDCT", "Kernel::OutputArg");

	upload_samples(samples, &ctx);

	enqueue_kernel(&ctx, &dev_info, "MDCT");
	result = download_result(&ctx, "MDCT");

	return result;
}
