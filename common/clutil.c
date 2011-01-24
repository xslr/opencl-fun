// clutil.c

void init_cl(cl_context *ctx, cl_device_id *dev_id)
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
	checkCLErr(err, "Error in clGetPlatformIDs");

	err = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_GPU, MAX_CL_DEVS,
			gpu_dev_id, &num_gpu_devs);
	if( false == checkCLErr(err, "Error in clGetDeviceIDs for GPU") )
		gpu_sel = -1;

	err = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_CPU, MAX_CL_DEVS,
			cpu_dev_id, &num_cpu_devs);
	if( false == checkCLErr(err, "Error in clGetDeviceIDs for CPU") )
		cpu_sel = -1;

	// try to use a GPU, fall back to CPU
	if( gpu_sel ==  -1 )
	{
		fprintf(stderr, "No OpenCL GPUs were found. Looking for OpenCL CPUs.");
		if( cpu_sel == -1 )
		{
			fprintf(stderr, "No OpenCL devices were found. Exiting.");
			
			dev_id = NULL;
		}
		else
		{
			*dev_id = cpu_dev_id[cpu_sel];
		}
	}
	else
	{
		*dev_id = gpu_dev_id[gpu_sel];
	}

	cl_context_properties ctx_props[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id[0],
		0
	};
	*ctx = clCreateContext(ctx_props, 1, dev_id, &ctx_err, NULL, &err);
	if( false == checkCLErr(err, "Context creation") )
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
