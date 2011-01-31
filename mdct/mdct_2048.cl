/*
  512 threads.
*/

#define PI 3.14159265f

/*
	Calculation flow is as follows:
		samples[N]
		xp[N/2], xpp[N/2]
		a[N/4], b[N/4]
		z[N/4 + N/4]
		c[N/4 + N/4]
*/

inline void download_samples
(__global float *samples, __local float *samples_local)
{
	event_t download_ev;
	download_ev = async_work_group_copy( samples_local,
										 samples,
										 2048,
										 0);

	wait_group_events(1, &download_ev);
}

inline void upload_samples
(__global float *out, __local float *coeffs)
{
	event_t upload_ev;
	upload_ev = async_work_group_copy( out,
									   coeffs,
									   1024,
									   0);

	wait_group_events(2, &upload_ev);
}

inline void calc_xp_xpp
(size_t local_id, __local float *xab, __local float *xp, __local float *xpp)
{
	xp[local_id] = xab[local_id] - xab[2048 - local_id - 1];
	xpp[local_id] = xab[local_id] + xab[2048 - local_id - 1];

	local_id += 512;

	xp[local_id] = xab[local_id] - xab[2048 - local_id - 1];
	xpp[local_id] = xab[local_id] + xab[2048 - local_id - 1];
}

inline void calc_ab
(size_t local_id, __local float *a, __local float *b, __local float *xp, __local float *xpp)
{
	float x = (PI *((local_id *2) +1)) / 4096;

	float sinx = native_sin(x);
	float cosx = native_cos(x);

	a[local_id] =
		((xp[local_id] - xpp[1024 - local_id - 1]) * cosx) -
		((xpp[local_id] - xp[1024 - local_id - 1]) * sinx);

	b[local_id] =
		((xp[local_id] - xpp[1024 - local_id - 1]) * sinx) +
		((xpp[local_id] - xp[1024 - local_id - 1]) * cosx);
}

// TODO: optimize summation using prefix sums
inline void calc_z
(size_t local_id, __local float *z, __local float *a, __local float *b)
{
	float mul = 1.0f;

	z[local_id] = 0;
	z[local_id + 512] = 0;

	for (ushort n = 0; n < 512; n++) {
		float x = PI * local_id * ((n*2) + 1)/1024;
		float sinx = native_sin(x);
		float cosx = native_cos(x);
		mul *= -1.0f;

		z[local_id] 
			+= (a[local_id] * cosx - b[local_id] * sinx);

		z[local_id + 512]
			+= (a[local_id] * sinx - b[local_id] * cosx) * mul;
	}
	mul = native_rsqrt(2.0f);
	if (local_id%2 == 1)
		mul *= -1.0f;

	z[local_id] *= mul;
	z[local_id + 512] *= mul;
}

inline void calc_coeff
(size_t local_id, __local float *c, __local float *z)
{
	size_t dest = local_id << 1;

	c[dest] = z[local_id];
	c[dest + 1] = -1 * z[1024 - local_id - 1];
}

__kernel void mdct_2048
(__global float *samples, __global float *out)
{
	size_t local_id = get_local_id(0);
	size_t group_id = get_group_id(0);

	// xab[] is used for storing three kinds of values
	// 1. x[N]		 	 	input samples
	// 2. a[N/4] and b[N/4]	intermediate values
	// 3. z[N/4 + N/4]		intermediate values
	__local float xab[2048];
	__local float xp[1024];
	__local float xpp[1024];

	download_samples(samples, xab);

	calc_xp_xpp( local_id,
				 xab,
				 xp,
				 xpp);

	barrier(CLK_LOCAL_MEM_FENCE);

	calc_ab( local_id,
			 &xab[0],        // a: xab[0]
			 &xab[512],      // b: xan[512]
			 xp,
			 xpp);

	barrier(CLK_LOCAL_MEM_FENCE);

	calc_z( local_id,
			&xab[1024],      // z: xab[1024]
			&xab[0],         // a: xab[0]
			&xab[512]);      // b: xab[512]

	barrier(CLK_LOCAL_MEM_FENCE);

	calc_coeff( local_id,
				&xab[0],     // c: xab[0]
				&xab[1024]); // z: xab[1024]

	barrier(CLK_LOCAL_MEM_FENCE);

	upload_samples(out, &xab[0]);
}
