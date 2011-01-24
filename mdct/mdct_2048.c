#include <stdlib.h>
#include <CL/cl.h>

#include <glib.h>
#include <glib/gstdio.h>

#define MDCT_KERNEL_SRC "mdct.cl"

#include "common/clutil.h"

int mdct(size_t count, float *samples)
{
	DevInfo dev_info;
	cl_context ctx;

	initCL(&ctx, &dev_info.dev_id);
	dev_diag(&dev_info);



	return 0;
}

// upload samples 
float *prep_kernel(cl_context *ctx, DevInfo *info, float *samples, size_t count)
{
	cl_int err;
	cl_mem in, out;

	in = clCreateBuffer( *ctx,
						 CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
						 count * sizeof(float),
						 samples,
						 &err);
	check_cl_err(err, "Create input buffer on CL device");

	out = clCreateBuffer( *ctx,
						  CL_MEM_WRITE_ONLY,
						  count * sizeof(float) >> 1,
						  0,
						  &err);
	check_cl_err(err, "Create output buffer on CL device");

	char *mdct_kernel_source;
	size_t mdct_kernel_size;
	GError *gerr;
	if (!g_file_get_contents( MDCT_KERNEL_SRC,
							  &mdct_kernel_source,
							  &mdct_kernel_size,
							  &gerr)) {
		fprintf(stderr, "Error reading kernel source, %s.\n", gerr->message);
		g_error_free(gerr);
		return NULL;
	}
}
