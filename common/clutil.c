
// clutil.c

#include <glib.h>
#include <glib/gstdio.h>

#include "clutil.h"

void init_cl(aacl_kernel_ctx *ctx, DevInfo *info, const char *modname)
{
	cl_int err;
	
	cl_platform_id platform_id[MAX_CL_PLATFORMS];
	cl_uint num_platforms;

	cl_device_id gpu_dev_id[MAX_CL_DEVS],
				 cpu_dev_id[MAX_CL_DEVS];
	int cpu_sel = 0,
		gpu_sel = 0;
	cl_uint num_gpu_devs;
	cl_uint num_cpu_devs;

	err = clGetPlatformIDs(MAX_CL_PLATFORMS, platform_id, &num_platforms);
	check_cl_err(err, modname, "init_cl::clGetPlatformIDs");

	err = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_GPU, MAX_CL_DEVS,
			gpu_dev_id, &num_gpu_devs);
	if( 0 != check_cl_err(err, modname, "init_cl::clGetDeviceIDs for GPU") )
		gpu_sel = -1;

	err = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_CPU, MAX_CL_DEVS,
			cpu_dev_id, &num_cpu_devs);
	if( 0 != check_cl_err(err, modname, "init_cl::clGetDeviceIDs for CPU") )
		cpu_sel = -1;

	// try to use a GPU, fall back to CPU
	if( gpu_sel ==  -1 )
	{
		fprintf(stderr, "%s: No OpenCL GPUs were found. Looking for OpenCL CPUs.\n",
				modname);
		if( cpu_sel == -1 )
		{
			fprintf(stderr, "%sNo OpenCL devices were found. Exiting.\n", modname);
			info->dev_id = NULL;
		}
		else
		{
			info->dev_id = cpu_dev_id[cpu_sel];
		}
	}
	else
	{
		info->dev_id = gpu_dev_id[gpu_sel];
	}

	cl_context_properties ctx_props[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id[0],
		0
	};
	ctx->cl_ctx = clCreateContext(ctx_props, 1, &info->dev_id, &ctx_err, NULL, &err);
	if( 0 != check_cl_err(err, modname, "Context creation") )
	{
		ctx = NULL;
	}

	fprintf(stderr, "Got %i platforms.\n", num_platforms);
	fprintf(stderr, "Got %i GPU and %i CPU devices.\n", num_gpu_devs, num_cpu_devs);
}

void close_cl(cl_context *ctx)
{
	clReleaseContext(*ctx);
}

void dev_diag(DevInfo *dev_info)
{
	size_t parm_val_size_ret;

	clGetDeviceInfo( dev_info->dev_id,
					 CL_DEVICE_TYPE,
					 sizeof(cl_device_type),
					 (void*)&(dev_info->dev_type),
					 &parm_val_size_ret);

	clGetDeviceInfo( dev_info->dev_id,
					 CL_DEVICE_MAX_WORK_GROUP_SIZE,
					 sizeof(size_t),
					 (void*)&(dev_info->max_work_group_size),
					 &parm_val_size_ret);

	clGetDeviceInfo( dev_info->dev_id,
					 CL_DEVICE_LOCAL_MEM_SIZE,
					 sizeof(cl_ulong),
					 (void*)&(dev_info->local_mem_size),
					 &parm_val_size_ret);

	clGetDeviceInfo( dev_info->dev_id,
					 CL_DEVICE_LOCAL_MEM_TYPE,
					 sizeof(cl_device_local_mem_type),
					 (void*)&(dev_info->local_mem_type),
					 &parm_val_size_ret);
}

void ctx_err(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
	fprintf(stderr, "(EE): Context error\n");
}

int check_cl_err(cl_int err, const char *modname, const char *message)
{
	switch(err)
	{
		//{{{ B
		case CL_BUILD_PROGRAM_FAILURE:
			fprintf(stderr, "(EE %i) CL_BUILD_PROGRAM_FAILURE: ", err);
			break;
		//}}}

		//{{{ C
		case CL_COMPILER_NOT_AVAILABLE:
			fprintf(stderr, "(EE %i) CL_COMPILER_NOT_AVAILABLE: ", err);
			break;
		//}}}

		//{{{ D
		case CL_DEVICE_NOT_FOUND:
			fprintf(stderr, "(EE %i) CL_DEVICE_NOT_FOUND: ", err);
			break;
		//}}}

		//{{{ INVALID_A
		case CL_INVALID_ARG_INDEX:
			fprintf(stderr, "(EE %i) CL_INVALID_ARG_INDEX: ", err);
			break;
		case CL_INVALID_ARG_SIZE:
			fprintf(stderr, "(EE %i) CL_INVALID_ARG_SIZE: ", err);
			break;
		case CL_INVALID_ARG_VALUE:
			fprintf(stderr, "(EE %i) CL_INVALID_ARG_VALUE: ", err);
			break;
		//}}}
		
		//{{{ INVALID_B
		case CL_INVALID_BINARY:
			fprintf(stderr, "(EE %i) CL_INVALID_BINARY: ", err);
			break;
		case CL_INVALID_BUILD_OPTIONS:
			fprintf(stderr, "(EE %i) CL_INVALID_BUILD_OPTIONS: ", err);
			break;
		//}}}
		
		//{{{ INVALID_C
		case CL_INVALID_COMMAND_QUEUE:
			fprintf(stderr, "(EE %i) CL_INVALID_COMMAND_QUEUE: ", err);
			break;
		case CL_INVALID_CONTEXT:
			fprintf(stderr, "(EE %i) CL_INVALID_CONTEXT: ", err);
			break;
		//}}}
			
		//{{{ INVALID_D
		case CL_INVALID_DEVICE:
			fprintf(stderr, "(EE %i) CL_INVALID_DEVICE: ", err);
			break;
		case CL_INVALID_DEVICE_TYPE:
			fprintf(stderr, "(EE %i) CL_INVALID_DEVICE_TYPE: ", err);
			break;
		//}}}

		//{{{ INVALID_E
		case CL_INVALID_EVENT_WAIT_LIST:
			fprintf(stderr, "(EE %i) CL_INVALID_EVENT_WAIT_LIST: ", err);
			break;
		case CL_INVALID_EVENT:
			fprintf(stderr, "(EE %i) CL_INVALID_EVENT: ", err);
			break;
		//}}}

		//{{{ INVALID_G
		case CL_INVALID_GLOBAL_OFFSET:
			fprintf(stderr, "(EE %i) CL_INVALID_GLOBAL_OFFSET: ", err);
			break;
		case CL_INVALID_GLOBAL_WORK_SIZE:
			fprintf(stderr, "(EE %i) CL_INVALID_GLOBAL_WORK_SIZE: ", err);
			break;
		//}}}

		//{{{ INVALID_K
		case CL_INVALID_KERNEL:
			fprintf(stderr, "(EE %i) CL_INVALID_KERNEL: ", err);
			break;
		case CL_INVALID_KERNEL_ARGS:
			fprintf(stderr, "(EE %i) CL_INVALID_KERNEL_ARGS: ", err);
			break;
		case CL_INVALID_KERNEL_DEFINITION:
			fprintf(stderr, "(EE %i) CL_INVALID_KERNEL_DEFINITION: ", err);
			break;
		case CL_INVALID_KERNEL_NAME:
			fprintf(stderr, "(EE %i) CL_INVALID_KERNEL_NAME: ", err);
			break;
		//}}}

		//{{{ INVALID_M
		case CL_INVALID_MEM_OBJECT:
			fprintf(stderr, "(EE %i) CL_INVALID_MEM_OBJECT:: ", err);
			break;
		//}}}

		//{{{ INVALID_O
		case CL_INVALID_OPERATION:
			fprintf(stderr, "(EE %i) CL_INVALID_OPERATION:: ", err);
			break;
		//}}}

		//{{{ INVALID_P
		case CL_INVALID_PLATFORM:
			fprintf(stderr, "(EE %i) CL_INVALID_PLATFORM: ", err);
			break;
		case CL_INVALID_PROGRAM_EXECUTABLE:
			fprintf(stderr, "(EE %i) : CL_INVALID_PROGRAM_EXECUTABLE: ", err);
			break;
		//}}}

		//{{{ INVALID_S
		case CL_INVALID_SAMPLER:
			fprintf(stderr, "(EE %i) CL_INVALID_SAMPLER: ", err);
			break;
		//}}}

		//{{{ INVALID V
		case CL_INVALID_VALUE:
			fprintf(stderr, "(EE %i) CL_INVALID_VALUE: ", err);
			break;
		//}}}

		//{{{ INVALID_W
		case CL_INVALID_WORK_DIMENSION:
			fprintf(stderr, "(EE %i) CL_INVALID_WORK_DIMENSION: ", err);
			break;
		case CL_INVALID_WORK_GROUP_SIZE:
			fprintf(stderr, "(EE %i) CL_INVALID_WORK_GROUP_SIZE: ", err);
			break;
		case CL_INVALID_WORK_ITEM_SIZE:
			fprintf(stderr, "(EE %i) CL_INVALID_WORK_ITEM_SIZE: ", err);
			break;
		//}}}

		//{{{ M
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			fprintf(stderr, "(EE %i) CL_MEM_OBJECT_ALLOCATION_FAILURE: ", err);
			break;
		//}}}

		//{{{ O
		case CL_OUT_OF_RESOURCES:
			fprintf(stderr, "(EE %i) CL_OUT_OF_RESOURCES: ", err);
			break;
		case CL_OUT_OF_HOST_MEMORY:
			fprintf(stderr, "(EE %i) CL_OUT_OF_HOST_MEMORY: ", err);
			break;
		//}}}
	
		case CL_PROFILING_INFO_NOT_AVAILABLE:
			fprintf(stderr, "(EE %i) CL_PROFILING_INFO_NOT_AVAILABLE: ", err);
			break;

		case CL_SUCCESS:
			break;

		default:
			fprintf(stderr, "(EE %i) Unknown error: ", err);
	}
	
	if( CL_SUCCESS != err )
	{
		if( NULL != message)
			fprintf(stderr, "%s::%s.\n", modname, message);
		else
			fprintf(stderr, ".\n");

		return 1;
	}

	return 0;
}

void buffer_create(float *samples, aacl_kernel_ctx *ctx, const char *modname)
{
	cl_int err;
	ctx->in = clCreateBuffer( ctx->cl_ctx,
							  CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
							  ctx->in_count * sizeof(float),
							  samples,
							  &err);
	check_cl_err(err, modname, "Create input buffer on CL device");

	ctx->out = clCreateBuffer( ctx->cl_ctx,
							   CL_MEM_WRITE_ONLY,
							   ctx->out_count * sizeof(float),
							   0,
							   &err);
	check_cl_err(err, modname, "Create output buffer on CL device");
}

int build_kernel(char* kernel_path, const char *kernel_name, aacl_kernel_ctx *ctx,
				  DevInfo *info, const char *modname)
{
	GError *gerr;
	cl_int err;
	char *kernel_source;
	size_t kernel_size;

	size_t build_log_size;
	char* build_log = malloc(MAX_BUILD_LOG_SIZE);

	if (!g_file_get_contents( kernel_path,
							  &kernel_source,
							  &kernel_size,
							  &gerr)) {
		fprintf(stderr, "%s::Kernel::Read, %s.\n", modname, gerr->message);
		g_error_free(gerr);
		return -1;
	}

	ctx->prog = clCreateProgramWithSource( ctx->cl_ctx,
										   1,
										   (const char**)&kernel_source,
										   &kernel_size,
										   &err);
	check_cl_err(err, modname, "Program::Create");

	err = clBuildProgram( ctx->prog,
						  1,
						  &info->dev_id,
						  NULL,
						  NULL, NULL);
	check_cl_err(err, modname, "Program::Build");
	g_free(kernel_source);

	// set ptr to NULL to indicate that it is invalid
	kernel_source = NULL;

	clGetProgramBuildInfo( ctx->prog,
						   info->dev_id,
						   CL_PROGRAM_BUILD_LOG,
						   MAX_BUILD_LOG_SIZE,
						   build_log,
						   &build_log_size);
	if (0 != check_cl_err(err, modname, "Program::Build")) {
		fprintf(stderr, "Build Log:\n%s\n", build_log);
		fflush(stderr);
	}

	free(build_log);

	ctx->kernel = clCreateKernel(ctx->prog, kernel_name, &err);
	check_cl_err(err, modname, "Kernel::Execute");

	size_t wg_size_suggest;
	err = clGetKernelWorkGroupInfo( ctx->kernel,
									info->dev_id,
									CL_KERNEL_WORK_GROUP_SIZE,
									sizeof(size_t),
									&wg_size_suggest,
									NULL);

	cl_ulong local_mem_usage;
	err = clGetKernelWorkGroupInfo( ctx->kernel,
									info->dev_id,
									CL_KERNEL_LOCAL_MEM_SIZE,
									sizeof(cl_ulong),
									&local_mem_usage,
									NULL);


	fprintf(stderr, "%s: Suggested WG Size: %lu\n", modname, wg_size_suggest);
	fprintf(stderr, "%s: Est. Local Mem Usage: %lu\n", modname, local_mem_usage);

	return 0;
}

void enqueue_kernel(aacl_kernel_ctx *ctx,
					DevInfo *info, const char *modname)
{
	cl_int err;

	ctx->cmd_queue = clCreateCommandQueue( ctx->cl_ctx,
										   info->dev_id,
										   CL_QUEUE_PROFILING_ENABLE,
										   &err);
	check_cl_err(err, modname, "CommandQueue::Create");

	cl_event kernel_exec_event;
	err = clEnqueueNDRangeKernel( ctx->cmd_queue,
								  ctx->kernel,
								  1,
								  NULL,
								  &ctx->ws_global,
								  &ctx->ws_local,
								  0, NULL, &kernel_exec_event);
	check_cl_err(err, modname, "Kernel::Enqueue");
}

float *download_result(aacl_kernel_ctx *ctx,
						const char *modname)
{
	cl_int err;
	size_t result_size = ctx->out_count * sizeof(float);
	float *result = malloc(result_size);

	fprintf(stderr, "%s::Downloading %lu bytes of results...", modname, result_size);

	err = clEnqueueReadBuffer( ctx->cmd_queue,
							   ctx->out,
							   CL_TRUE,     // blocking read
							   0,
							   result_size,
							   result,
							   0, NULL,
							   NULL);
	check_cl_err(err, modname, "Results::Read");

	clFinish(ctx->cmd_queue);
	fprintf(stderr, "done!\n");

	return result;
}

void *upload_samples(float *samples, aacl_kernel_ctx *ctx)
{
	clEnqueueWriteBuffer( ctx->cmd_queue,
						 ctx->in,
						 CL_TRUE,
						 0,
						 ctx->in_count*sizeof(float),
						 samples,
						 0, NULL,
						 NULL);
}

void dump_aacl_kernel_ctx(aacl_kernel_ctx *ctx)
{
	// TODO
}
