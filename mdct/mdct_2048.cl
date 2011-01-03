/*
  512 threads.
*/

#define PI 3.14159265f

__kernel void mdct_2048
(__global float *samples, __global float *out)
{
	size_t local_id = get_local_id();
	size_t group_id = get_group_id();

	// buffer for input samples and intermediate values
	__local float xab[2048];
	__local float xp[1024];
	__local float xpp[1024];

	// sines and cosines used for computing a and b
	__local float sincos_ab[1024];
}

inline void download_samples
(__global float *samples, __local float *samples_local)
{
}

inline void calc_xp_xpp
(size_t local_id, __local float *xab, __local float *xp, __local float *xpp)
{
	size_t i = local_id;

	xp[i] = xab[i] - xab[2048 - i - 1];
	xpp[i] = xab[i] + xab[2048 - i - 1];

	i += 512;

	xp[i] = xab[i] - xab[2048 - i - 1];
	xpp[i] = xab[i] + xab[2048 - i - 1];
}

/*
inline void calc_xpp (size_t local_id, __local float *xab, __local float *xpp)
{
}
*/

inline void calc_sincos_tables
(size_t local_id, __local float *sincos_ab)
{
	float x = (PI *((local_id <<2) +1)) >> 12;

	sincos_ab[local_id] = native_sin(x);
	sincos_ab[local_id + 512] = native_cos(x);
}

inline void calc_ab
(size_t local_id, __local float *a, __local float *b, __local float *xp, __local float *xpp)
{
	float x = (PI *((local_id <<2) +1)) >> 12;

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
		float x = PI * local_id * ((n<<2) + 1);
		float sinx = native_sin(x);
		float cosx = native_cos(x);
		mul *= -1.0f;

		z[local_id] += ((a[local_id] * cosx) - (b[local_id] * sinx));
		z[local_id] += ((a[local_id] * sinx) - (b[local_id] * cosx)) * mul;
	}
}

inline void calc_coeff
(size_t local_id, __local float *c, __local float *z)
{
	size_t dest = local_id << 2;

	c[dest] = z[local_id];
	c[dest + 1] = -1 * z[1024 - local_id - 1];
}
