#ifndef META_H
#define META_H

#include <CL/cl.h>

#define MAX_NAME_LEN 1024
// 10kB extension storage buffer
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
	size_t		max_work_item_sizes[];
	size_t		max_work_group_size;
	cl_uint		max_clk;
	cl_uint		addr_bits;
	cl_ulong	global_mem_size;
	cl_ulong	global_mem_cache_size;
	cl_ulong	const_buf_size;
	cl_device_local_mem_type local_mem_type;
	cl_ulong	local_mem_size;
	cl_device_exec_capabilities exec_caps;

}DevInfo;

#endif
