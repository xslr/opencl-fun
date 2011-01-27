#include <stdlib.h>
#include <CL/cl.h>

#include <glib.h>
#include <glib/gstdio.h>

#define MDCT_KERNEL_SRC "mdct_2048.cl"

#include "clutil.h"

int mdct(size_t count, float *samples)
{
	DevInfo dev_info;
	aacl_kernel_ctx ctx;
	cl_int err;
	float *result;

	ctx.ws_local = 512;
	ctx.ws_global = count/4;
	ctx.in_count = count;
	ctx.out_count = count/2;

	init_cl(&ctx, &dev_info, "MDCT");
	dev_diag(&dev_info);

	buffer_create(samples, &ctx, "MDCT");
	build_kernel(MDCT_KERNEL_SRC, "mdct_2048", &ctx, &dev_info, "MDCT");

	// set kernel arguments
	err = clSetKernelArg(ctx.kernel, 0, sizeof(cl_mem), &ctx.in);
	check_cl_err(err, "MDCT", "Kernel::InputArgs");
	err = clSetKernelArg(ctx.kernel, 1, sizeof(cl_mem), &ctx.out);
	check_cl_err(err, "MDCT", "Kernel::OutputArgs");

	upload_samples(samples, &ctx);

	enqueue_kernel(&ctx, &dev_info, "MDCT");
	result = download_result(&ctx, "MDCT");

	// do not free result buffer
	//free(result);

	return result;
}
