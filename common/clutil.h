#ifndef AACL_CLUTIL_H
#define AACL_CLUTIL_H

#include <CL/cl.h>

// assume that extension names are less than 1024 bytes
#define MAX_NAME_LEN 1024
#define MAX_CL_PLATFORMS 4
#define MAX_CL_DEVS 4
#define MAX_BUILD_LOG_SIZE 1024 << 4

// 10kB buffer for storing list of extensions
#define MAX_EXT_LEN 1024*10
typedef struct
{
	cl_device_id	dev_id;
	cl_platform_id	platform_id;
	char		dev_name[MAX_NAME_LEN];
	char		dev_vendor[MAX_NAME_LEN];
	char		driver_ver[MAX_NAME_LEN];
	char		dev_ver[MAX_NAME_LEN];
	char		dev_ext[MAX_EXT_LEN];
	cl_device_type	dev_type;
	cl_uint		max_compute_units;
	cl_uint		max_work_dims;
	size_t		max_work_group_size;
	cl_uint		max_clk;
	cl_uint		addr_bits;
	cl_ulong	global_mem_size;
	cl_ulong	global_mem_cache_size;
	cl_ulong	const_buf_size;
	cl_device_local_mem_type local_mem_type;
	cl_ulong	local_mem_size;
	cl_device_exec_capabilities exec_caps;
	size_t		max_work_item_sizes[];
}DevInfo;

typedef struct {
	cl_context cl_ctx;
	cl_program prog;
	cl_kernel kernel;
	cl_command_queue cmd_queue;

	size_t ws_local;
	size_t ws_global;

	cl_mem in;
	size_t in_count;
	cl_mem out;
	size_t out_count;
} aacl_kernel_ctx;

void init_cl(aacl_kernel_ctx *ctx,
			 DevInfo *info, const char *modname);

void close_cl(cl_context *ctx);

void dev_diag(DevInfo *dev_info);

void ctx_err(const char *errinfo, const void *private_info,
			 size_t cb, void *user_data);

int check_cl_err(cl_int err, const char *modname, const char *message);

void buffer_create(float *samples, aacl_kernel_ctx *ctx, const char *modname);

int build_kernel(char* kernel_path, const char *kernel_name,
				  aacl_kernel_ctx *ctx, DevInfo *info, const char *modname);

void enqueue_kernel(aacl_kernel_ctx *ctx, cl_event *ev,
					DevInfo *info, const char *modname);

float *download_result(aacl_kernel_ctx *ctx, cl_event *ev,
					   const char *modname);

void upload_samples(aacl_kernel_ctx *ctx, cl_event *ev,
					 float *samples);

void dump_aacl_kernel_ctx(aacl_kernel_ctx *ctx);

#endif
