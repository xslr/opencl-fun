/*
 * main.cpp
 *
 * Driver program for testing fft_2048.cl
 *
 * History: 24/8/2010 // Subrat Meher - Initial version of working fft_2048.cl
 *
 */

#include <cstdlib>
#include <cstdio>

#include <CL/cl.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <fftw3.h>

#include "meta.h"

#define SIZE_DATA_BLK    1024
#define NUM_DATA_BLK     2
#define MAX_CL_PLATFORMS 4
#define MAX_CL_DEVS      4
#define KRNL_SRC_FFT4    "fft_2048.cl"
#define KRNL_NAM_FFT4    "fft_2048"

//{{{ prototypes
float* genData  (unsigned int num_blk, unsigned int blk_size);
void disp_data (float* data, unsigned int num_blk, unsigned int blk_size);
float* fft2048 (cl_context *ctx, cl_device_id *dev_id, int count, float data[]);
void initCL(cl_context *ctx, cl_device_id *dev_id);
bool checkCLErr(cl_int err, const char message[]);
void ctx_err(const char *errinfo, const void *private_info, size_t cb, void *user_data);
void closeCL(cl_context *ctx);
void devDiag(cl_device_id *dev_id, DevInfo *devInfo);
void fft16_fftw();
void disp_fft(float *data, size_t count);

//}}}

int main(int argc, char *argv[])
{
	float *data;
	float *fft_result;
	float *debug;
	cl_context ctx;
	cl_device_id dev_id;
	DevInfo devInfo;

	data = genData(NUM_DATA_BLK, SIZE_DATA_BLK);

	initCL(&ctx, &dev_id);
	if( NULL == ctx )
	{
		fprintf(stderr, "Cannot create an OpenCL context.\n");
		return EXIT_FAILURE;
	}

	devDiag(&dev_id, &devInfo);

	fprintf(stderr, "local mem size: %lu\n", devInfo.local_mem_size);

	fft_result = fft2048(&ctx, &dev_id, NUM_DATA_BLK, data);

	closeCL(&ctx);

	disp_fft(fft_result, NUM_DATA_BLK*SIZE_DATA_BLK);

	free(fft_result);
	free(data);

	return EXIT_SUCCESS;
}

//{{{ disp_fft
void disp_fft(float *data, size_t count)
{
	size_t i;
	for(i=0; i<count; i++)
	{
//		if(i%2 == 0)
			printf("%lu:\t%f\t%f\n", i, data[i], data[i+count]);
/*		else {
			float diff = data[i]-data[i-1];
			if(diff != 1024)
				printf("%lu:\t%f\t%f\t%f  ***%lu\n", i, data[i], data[i+count], diff, i);
			else
				printf("%lu:\t%f\t%f\t%f\n", i, data[i], data[i+count], diff);
		}
*/	}
}
//}}}

//{{{ void dispData(int count, float buf[][])
void disp_data(float *data, unsigned int num_blk, unsigned int blk_size)
{
	for(unsigned int blk = 0; blk < num_blk; blk++)
	{
		for(unsigned int blk_idx = 0; blk_idx < blk_size; blk_idx++)
		{
			unsigned int index = blk_idx + (blk_size*blk);

			printf("%u:\t%f\n", index, data[index]);
		}
	}
}
//}}}

//{{{ void genData(int count, float buf[])
float* genData(unsigned int num_blk, unsigned int blk_size)
{
	float *data;

	data = (float*)malloc(num_blk * blk_size * sizeof(float));

	for(unsigned int blk = 0; blk < num_blk; blk++)
	{
		for(unsigned int blk_idx = 0; blk_idx < blk_size; blk_idx++)
		{
			unsigned int index = blk_idx + (blk*blk_size);
			//buf[index] = (float)rand()/1000;
			data[index] = (float)(index);
		}
	}

	return data;
}
//}}}

//{{{ fft2048(cl_context *ctx, int count, float data[][SIZE_DATA_BLK])
float* fft2048(cl_context *ctx, cl_device_id *dev_id, int count, float data[])
{
	GError *gerr;
	cl_int err;
	cl_command_queue cmd_qu;
	char *krn_fft_rdx2_src;
	size_t len_fft_rdx2_src;
	cl_kernel krn_fft_rdx2;
	cl_program prg_fft_rdx2;

	size_t krn_wg_size;
	cl_ulong krn_loc_size;

	size_t max_sz_bld_log = 16 * 1024;
	size_t sz_bld_log;
	char* log_bld_fft_rdx2 = (char*)malloc(max_sz_bld_log);

	// size of overlapped output blocks is twice as input, and has one block less
	size_t out_size = ((count-1) * (SIZE_DATA_BLK<<1) * sizeof(float)) <<1;
	size_t in_size = count * SIZE_DATA_BLK * sizeof(float);
	size_t local_work_size = 1024;
	size_t global_work_size = (count-1) * local_work_size;
	ushort blk_size = SIZE_DATA_BLK;
	ushort out_buf_size = (ushort)out_size;

	fprintf(stderr, "output buffer size is %lu\n", out_size);
	fprintf(stderr, "global work size is %lu\n", global_work_size);
	fprintf(stderr, "local work size is %lu\n", local_work_size);

	float* out_host_buf = (float*)malloc(out_size);
	cl_mem in, out;

	//{{{ allocate device memory
	in = clCreateBuffer( *ctx,
						 CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
						 in_size,
						 data,
						 &err);
	checkCLErr(err, "fft2048: Creating input buf on CL dev");

	out = clCreateBuffer(*ctx,
						 CL_MEM_WRITE_ONLY,
						 out_size,
						 0,
						 &err);
	checkCLErr(err, "fft2048: Creating output buf on CL dev");
	//}}}

	//{{{ create program object
	if(!g_file_get_contents(KRNL_SRC_FFT4, &krn_fft_rdx2_src, &len_fft_rdx2_src, &gerr))
	{
		fprintf(stderr, "Error reading the kernel, %s\n", gerr->message);
		g_error_free(gerr);
		return NULL;
	}
	prg_fft_rdx2 = clCreateProgramWithSource( *ctx,
											  1,
											  (const char**)&krn_fft_rdx2_src,
											  &len_fft_rdx2_src,
											  &err);
	checkCLErr(err, "fft2048: Create program");

	err = clBuildProgram( prg_fft_rdx2,
						  1,
						  dev_id,
						  NULL,
						  NULL, NULL);
	g_free(krn_fft_rdx2_src);

	clGetProgramBuildInfo( prg_fft_rdx2,
						   *dev_id,
						   CL_PROGRAM_BUILD_LOG,
						   max_sz_bld_log,
						   log_bld_fft_rdx2,
						   &sz_bld_log);
	if(!checkCLErr(err, "fft2048: Build program"))
	{
		fprintf(stderr, "Program build error.\n");
		fprintf(stderr, "Build Log (Truncated to 16k):\n%s\n", log_bld_fft_rdx2);
	}
	else
	{
		fprintf(stderr, "Program built successfully.\n");
	}
	//}}}
	free(build_log);

	//{{{ create kernel object
	krn_fft_rdx2 = clCreateKernel(prg_fft_rdx2, KRNL_NAM_FFT4, &err);
	checkCLErr(err, "fft2048: Create kernel");

	err = clGetKernelWorkGroupInfo( krn_fft_rdx2,
									*dev_id,
									CL_KERNEL_WORK_GROUP_SIZE,
									sizeof(size_t),
									&krn_wg_size,
									NULL);

	err = clGetKernelWorkGroupInfo( krn_fft_rdx2,
									*dev_id,
									CL_KERNEL_LOCAL_MEM_SIZE,
									sizeof(cl_ulong),
									&krn_loc_size,
									NULL);

	fprintf(stderr, "Suggested Work Group Size: %lu\n", krn_wg_size);
	fprintf(stderr, "Estimated Local Mem Usage: %lu\n", krn_loc_size);

	err = clSetKernelArg(krn_fft_rdx2, 0, sizeof(cl_mem), &in);
	checkCLErr(err, "fft2048: Set kernel args in");
	err = clSetKernelArg(krn_fft_rdx2, 1, sizeof(cl_mem), &out);
	checkCLErr(err, "fft2048: Set kernel args out");
	//}}}

	//{{{ execute kernel
	cmd_qu = clCreateCommandQueue(*ctx, *dev_id, CL_QUEUE_PROFILING_ENABLE, &err );
	checkCLErr(err, "fft2048: Create command queue");

	cl_event ev_krn_exec;
	err = clEnqueueNDRangeKernel( cmd_qu,
								  krn_fft_rdx2,
								  1,
								  NULL,
								  &global_work_size,
								  &local_work_size,
								  0, NULL, &ev_krn_exec);
	checkCLErr(err, "fft2048: Enqueue kernel");
	//}}}

	//{{{ profiling fft computation
#ifdef PROFILING
	clWaitForEvents(1, &ev_krn_exec);

	cl_ulong krn_exec_start,
			 krn_exec_end,
			 krn_exec_time;

	err = clGetEventProfilingInfo( ev_krn_exec,
							 CL_PROFILING_COMMAND_START,
							 sizeof(cl_ulong),
							 &krn_exec_start,
							 NULL);
	checkCLErr(err, "fft2048: Read kernel start time");

	err = clGetEventProfilingInfo( ev_krn_exec,
							 CL_PROFILING_COMMAND_END,
							 sizeof(cl_ulong),
							 &krn_exec_end,
							 NULL);
	checkCLErr(err, "fft2048: Read kernel end time");

	krn_exec_time = krn_exec_end - krn_exec_start;

	fprintf(stderr, "FFT computed in %lu us (%lu ns)\n", krn_exec_time/1000, krn_exec_time);
#endif
	//}}}

	//{{{ read back results
	clEnqueueReadBuffer(cmd_qu,
						out,
						CL_TRUE,
						0,
						out_size,
						out_host_buf,
						0, NULL, NULL);
	checkCLErr(err, "fft2048: Read data from CL dev");
	//}}}

	clFinish(cmd_qu);

	return out_host_buf;
}
//}}}

//{{{ checkCLErr(cl_int err, const char message[])
bool checkCLErr(cl_int err, const char message[])
{
	switch(err)
	{
		//{{{ B
		case CL_BUILD_PROGRAM_FAILURE:
			fprintf(stderr, "(EE %i) CL_BUILD_PROGRAM_FAILURE:: ", err);
			break;
		//}}}

		//{{{ C
		case CL_COMPILER_NOT_AVAILABLE:
			fprintf(stderr, "(EE %i) CL_COMPILER_NOT_AVAILABLE:: ", err);
			break;
		//}}}

		//{{{ D
		case CL_DEVICE_NOT_FOUND:
			fprintf(stderr, "(EE %i) CL_DEVICE_NOT_FOUND: ", err);
			break;
		//}}}

		//{{{ INVALID_A
		case CL_INVALID_ARG_INDEX:
			fprintf(stderr, "(EE %i) CL_INVALID_ARG_INDEX:: ", err);
			break;
		case CL_INVALID_ARG_SIZE:
			fprintf(stderr, "(EE %i) CL_INVALID_ARG_SIZE:: ", err);
			break;
		case CL_INVALID_ARG_VALUE:
			fprintf(stderr, "(EE %i) CL_INVALID_ARG_VALUE:: ", err);
			break;
		//}}}
		
		//{{{ INVALID_B
		case CL_INVALID_BINARY:
			fprintf(stderr, "(EE %i) CL_INVALID_BINARY:: ", err);
			break;
		case CL_INVALID_BUILD_OPTIONS:
			fprintf(stderr, "(EE %i) CL_INVALID_BUILD_OPTIONS:: ", err);
			break;
		//}}}
		
		//{{{ INVALID_C
		case CL_INVALID_COMMAND_QUEUE:
			fprintf(stderr, "(EE %i) : CL_INVALID_COMMAND_QUEUE: ", err);
			break;
		case CL_INVALID_CONTEXT:
			fprintf(stderr, "(EE %i) : CL_INVALID_CONTEXT: ", err);
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
			fprintf(stderr, "(EE %i) : CL_INVALID_EVENT_WAIT_LIST: ", err);
			break;
		case CL_INVALID_EVENT:
			fprintf(stderr, "(EE %i) : CL_INVALID_EVENT: ", err);
			break;
		//}}}

		//{{{ INVALID_G
		case CL_INVALID_GLOBAL_OFFSET:
			fprintf(stderr, "(EE %i) : CL_INVALID_GLOBAL_OFFSET: ", err);
			break;
		case CL_INVALID_GLOBAL_WORK_SIZE:
			fprintf(stderr, "(EE %i) : CL_INVALID_GLOBAL_WORK_SIZE: ", err);
			break;
		//}}}

		//{{{ INVALID_K
		case CL_INVALID_KERNEL:
			fprintf(stderr, "(EE %i) : CL_INVALID_KERNEL: ", err);
			break;
		case CL_INVALID_KERNEL_ARGS:
			fprintf(stderr, "(EE %i) : CL_INVALID_KERNEL_ARGS: ", err);
			break;
		case CL_INVALID_KERNEL_DEFINITION:
			fprintf(stderr, "(EE %i) : CL_INVALID_KERNEL_DEFINITION: ", err);
			break;
		case CL_INVALID_KERNEL_NAME:
			fprintf(stderr, "(EE %i) : CL_INVALID_KERNEL_NAME: ", err);
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
			fprintf(stderr, "(EE %i) CL_INVALID_SAMPLER:: ", err);
			break;
		//}}}

		//{{{ INVALID V
		case CL_INVALID_VALUE:
			fprintf(stderr, "(EE %i) CL_INVALID_VALUE: ", err);
			break;
		//}}}

		//{{{ INVALID_W
		case CL_INVALID_WORK_DIMENSION:
			fprintf(stderr, "(EE %i) : CL_INVALID_WORK_DIMENSION: ", err);
			break;
		case CL_INVALID_WORK_GROUP_SIZE:
			fprintf(stderr, "(EE %i) : CL_INVALID_WORK_GROUP_SIZE: ", err);
			break;
		case CL_INVALID_WORK_ITEM_SIZE:
			fprintf(stderr, "(EE %i) : CL_INVALID_WORK_ITEM_SIZE: ", err);
			break;
		//}}}

		//{{{ M
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			fprintf(stderr, "(EE %i) : CL_MEM_OBJECT_ALLOCATION_FAILURE: ", err);
			break;
		//}}}

		//{{{ O
		case CL_OUT_OF_RESOURCES:
			fprintf(stderr, "(EE %i) : CL_OUT_OF_RESOURCES: ", err);
			break;
		case CL_OUT_OF_HOST_MEMORY:
			fprintf(stderr, "(EE %i) : CL_OUT_OF_HOST_MEMORY: ", err);
			break;
		//}}}
	
		case CL_PROFILING_INFO_NOT_AVAILABLE:
			fprintf(stderr, "(EE %i) : CL_PROFILING_INFO_NOT_AVAILABLE: ", err);
			break;

		case CL_SUCCESS:
			break;

		default:
			fprintf(stderr, "(EE %i) Unknown error: ", err);
	}
	
	if( CL_SUCCESS != err )
	{
		if( NULL != message)
			fprintf(stderr, "%s.\n", message);
		else
			fprintf(stderr, ".\n");

		return false;
	}

	return true;
}
//}}}
