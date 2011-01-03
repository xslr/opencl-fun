/*
 * fft_2048.cl
 *
 * 2048 point FFT computation kernel.
 * This has been developed on an nVidia 460GTX with 1024 MB global memory and 48 KB local memory.
 * It uses 16 KB of local memory
 * It computes two 1024 point FFTs using 512 threads and then computes two
 * 2048 point partial FFTs in 'fft_2048_interleave' that together compute a 2048
 * point FFT.
 *
 * History: 24/8/2010 // Subrat Meher: Initial working version
 * 			07/10/2010 // Subrat Meher: Optimized for Fermi class GPUs.
 *										Dropped support for previous architectures.
 *
 */

#define TWO_PI 6.28318531

// swap two values using a temporary value
#define SWAP(a, b) ({ float tmp; tmp=a; a=b; b=tmp; })

// calculate twiddle factor
#define TWDL_CALC(twdl) ({ twdl.y = 0 - native_sin(twdl.x); twdl.x = native_cos(twdl.x); })

#define TWDL_MUL(twdl, r, i, idx) ({ float2 a = (float2)(r[idx], i[idx]); r[idx] = (a.x*twdl.x) - (a.y*twdl.y); i[idx] = (a.y*twdl.x) + (a.x*twdl.y); })

#define DFT2(r, i, i1, i2) ({ float2 a = (float2)(r[i1], i[i1]); float2 b = (float2)(r[i2], i[i2]); a = a+b; b=a-(2*b); r[i1] = a.x; i[i1] = a.y; r[i2] = b.x; i[i2] = b.y; })

// TWDL_MUL_ALT and DFT_2_ALT are function macros for special addressing of i_buf used while computing 2048 point fft
#define TWDL_MUL_ALT(twdl, r, i, idx_re, idx_im) ({ float2 a = (float2)(r[idx_re], i[idx_im]); r[idx_re] = (a.x*twdl.x) - (a.y*twdl.y); i[idx_im] = (a.y*twdl.x) + (a.x*twdl.y); })

#define DFT2_ALT(r, i, idx1_re, idx2_re, idx1_im, idx2_im) ({ float2 a = (float2)(r[idx1_re], i[idx1_im]); float2 b = (float2)(r[idx2_re], i[idx2_im]); a = a+b; b=a-(2*b); r[idx1_re] = a.x; i[idx1_im] = a.y; r[idx2_re] = b.x; i[idx2_im] = b.y; })

// prototypes
void fft_2048_do( __local float *r_buf,
				  __local float *i_buf);
void shuff_and_window( __global float *in,
					   __global float *out,
					   __local  float *r_buf,
					   __local  float *i_buf);

__kernel void fft_2048( __global float *in,
						__global float *out)
{

	__local float r_buf[2048],
				  i_buf[2048];

	shuff_and_window(in, out, r_buf, i_buf);

	fft_2048_do(r_buf, i_buf);

	// upload imaginary and real components
	{
		__private event_t ev;
		__private size_t  grp_id = get_group_id(0);
		__global float *i_out = out + ((get_num_groups(0))<<11);

		// copy imaginary components to global mem
		async_work_group_copy (i_out + (grp_id << 11),
							   &i_buf[0],
							   (size_t)2048,
							   ev);

		// copy real components to global mem
		async_work_group_copy (out + (grp_id << 11),
							   &r_buf[0],
							   (size_t)2048,
							   ev);
		wait_group_events (1, &ev);
	}

}

// fft_2048_do
// computes a 2048 point radix-2 fft
void fft_2048_do( __local float *r_buf,
				  __local float *i_buf)
{
	// This method computes a 2048 point radix 2 real to complex fft.
	// Assumes that i_buf and i_buf is reset (all zeroes).

	for (ushort lcl_fft_size = 2;
				lcl_fft_size <= 2048;
				lcl_fft_size = lcl_fft_size << 1)
	{
		
		__private size_t lcl_id = get_local_id(0);
		__private float2 twdl;
		__private ushort sample_index =
				floor((float)(lcl_id<<1) / (float)lcl_fft_size)
				* lcl_fft_size
				+ (lcl_id % (lcl_fft_size>>1));

		// twiddle factor derivation explanation
		// twiddle factor is given by exp(-2i*PI*k*n/N)
		//		= cos(k/N) - isin(k/N)
		// k for a work item is given by (thread_id % (local_fft_size/2)) * stride
		//		stride = final_fft_length / local_fft_size
		//			   = N / local_fft_size
		//		so k/N simplifies to (thread_id % local_fft_size/2) * (N/local_fft_size) / N
		//			   = (thread_id  % (local_fft_size/2)) / local_fft_size
		twdl.x = (float)(lcl_id % (lcl_fft_size>>1)) * (float)TWO_PI / (float)lcl_fft_size;

		// calculate twiddle factor
		// (cos twdl.x - isin twdl.x)
		TWDL_CALC(twdl);
	
		// multiply twiddle factor
		// accepts the twiddle factor, real and imaginary buffers
		// and the sample index of the 'second term' of fft calculation
		TWDL_MUL(twdl, r_buf, i_buf, sample_index + (lcl_fft_size>>1));

		// calculate two point DFT
		// sample_index and (lcl_fft_size>>1) are the index and index difference
		// of two numbers whose dft is calculated.
		// indices are valid within r_buf and i_buf
		DFT2(r_buf, i_buf, sample_index, sample_index + (lcl_fft_size>>1));

		barrier (CLK_LOCAL_MEM_FENCE);
	}
}

// shuff_and_window
// shuffles 2048 data items and applies a window function
// pre processing for fft
void shuff_and_window( __global float *in,
					   __global float *out,
					   __local  float *r_buf,
					   __local  float *i_buf)
{
	event_t ev;
	__private size_t lcl_id = get_local_id(0);
	__private size_t grp_id = get_group_id(0);
	__private size_t num_grp = get_num_groups(0);

	async_work_group_copy( &r_buf[0],
						   in + (grp_id<<10),
						   (size_t)2048,
						   ev);

	// initialize imaginary data buffer
	// while copying data from global mem
	i_buf[lcl_id] = 0;
	i_buf[lcl_id + 1024] = 0;

	// wait for data to arrive before shuffling it
	wait_group_events(1, &ev);

	{
		__private ushort2 index_pair;
		// calculate initial swap indices
		index_pair.x = lcl_id;
		// lcl_id is expected to range from 0 .. 1023
		index_pair.y = ((index_pair.x & 0x5555) << 1) | ((index_pair.x & 0xaaaa) >> 1);
		index_pair.y = ((index_pair.y & 0x3333) << 2) | ((index_pair.y & 0xcccc) >> 2);
		index_pair.y = ((index_pair.y & 0x000f) << 8) | (index_pair.y & 0x00f0) | ((index_pair.y & 0x0f00) >> 8);
		
		index_pair.y >>= 1;

		if (index_pair.y > index_pair.x)
		{
			SWAP( r_buf[index_pair.x],
				  r_buf[index_pair.y]);
		}

		barrier (CLK_LOCAL_MEM_FENCE);

		index_pair += (ushort2)(1024, 1);
		if (index_pair.y > index_pair.x)
		{
			SWAP (r_buf[index_pair.x],
				  r_buf[index_pair.y]);
		}

		barrier (CLK_LOCAL_MEM_FENCE);
	}
}
