/*
 * fft_2048.cl
 *
 * 2048 point FFT computation kernel.
 * This has been developed on an nVidia 460GTX with 1024 MB global memory and 48 KB local memory.
 * It uses 16 KB of local memory
 * It computes two 1024 point FFTs using 1024 threads and then computes two
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


// computes a 512 point radix-2 fft using N/2 threads
void fft_512( __local float *r_buf,
			  __local float *i_buf,
			  size_t lcl_id)
{
	// This method computes a 2048 point radix 2 real to complex fft.
	// Assumes that i_buf and i_buf is reset (all zeroes).

	for (ushort lcl_fft_size = 2;
				lcl_fft_size <= 512;
				lcl_fft_size <<= 1)
	{
		__private float2 twdl;
		__private ushort sample_index =
				floor((float)(lcl_id<<1) / (float)lcl_fft_size)
				* lcl_fft_size
				+ (lcl_id % (lcl_fft_size>>1));

		// twiddle factor explanation
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
// shuffles 512 data items and applies a window function
// pre processing for fft
void fft_shuffle( __local float *r,
				  __local float *i,
				  size_t lcl_id)
{
	__private ushort2 index_pair;
	// calculate initial swap indices
	index_pair.x = lcl_id;
	// lcl_id is expected to range from 0 .. 256
	index_pair.y = ((index_pair.x & 0x0555) << 1) | ((index_pair.x & 0x0aaa) >> 1);
	index_pair.y = ((index_pair.y & 0x0333) << 2) | ((index_pair.y & 0x0ccc) >> 2);
	index_pair.y = ((index_pair.y & 0x000f) << 8) | (index_pair.y & 0x00f0) | ((index_pair.y & 0x0f00) >> 8);
	
	index_pair.y >>= 3;
	
	if (index_pair.y > index_pair.x) {
		SWAP( r[index_pair.x],
			  r[index_pair.y]);
		
		SWAP( i[index_pair.x],
			  i[index_pair.y]);
	}
	
	barrier (CLK_LOCAL_MEM_FENCE);
	
	index_pair += (ushort2)(256, 1);
	if (index_pair.y > index_pair.x) {
		SWAP (r[index_pair.x],
			  r[index_pair.y]);
		
		SWAP( i[index_pair.x],
			  i[index_pair.y]);
	}
	
	barrier (CLK_LOCAL_MEM_FENCE);
}

void download_rotate_samples( __global float *in,
							  __local  float *rot,
							  size_t lid)
{
	in += get_group_id(0) << 11;

	event_t ev1, ev2;

	ev1 = async_work_group_copy( rot,
								 in + 1536,
								 (size_t)512,
								 0);

	ev2 = async_work_group_copy( rot + 512,
								 in,
								 (size_t)1536,
								 0);
	wait_group_events(1, &ev1);

	rot[lid] = -1 * rot[lid];
	rot[lid + 256] = -1 * rot[lid + 256];

	wait_group_events(1, &ev2);
}

void calc_fk( __local float *rot,
			  __local float *bufa_r,
			  __local float *bufa_i,
			  size_t lid)
{
	bufa_r[lid] = rot[lid<<1] - rot[2048 - (lid<<1) - 1];
	bufa_i[lid] = rot[1024 - (lid<<1) - 1] - rot[1024 + (lid<<1)];

	lid += 256;

	bufa_r[lid] = rot[lid<<1] - rot[2048 - (lid<<1) - 1];
	bufa_i[lid] = rot[1024 - (lid<<1) - 1] - rot[1024 + (lid<<1)];
}

// multiply complex numbers in a with twiddle factor
// and output to b
void twiddle_mul( __local float *ar,
				  __local float *ai,
				  __local float *br,
				  __local float *bi,
				  size_t lid)
{
	float temp, tr, ti;

	temp = -1 * TWO_PI * (((float)lid)+0.125) / ((float)2048);
	tr = native_cos(temp);
	ti = native_sin(temp);
	br[lid] = (ar[lid]*tr - ai[lid]*ti);
	bi[lid] = (ar[lid]*ti + ai[lid]*tr);

	lid += 256;

	temp = -1 * TWO_PI * (((float)lid)+0.125) / ((float)2048);
	tr = native_cos(temp);
	ti = native_sin(temp);
	br[lid] = (ar[lid]*tr - ai[lid]*ti);
	bi[lid] = (ar[lid]*ti + ai[lid]*tr);
}

void arrange_output( __local float *r,
					 __local float *i,
					 __local float *out,
					 size_t lid)
{
	out[lid << 1] = r[lid];
	out[1024 - (lid<<1) - 1] = -1 * i[lid];

	lid += 256;

	out[lid << 1] = r[lid];
	out[1024 - (lid<<1) - 1] = -1 * i[lid];

}

void upload_result( __local  float *in,
					__global float *out)
{
	event_t ev;

	__private size_t grp_id = get_group_id(0);

	ev = async_work_group_copy( out + (grp_id << 10),
								in,
								(size_t)1024,
								0);

	wait_group_events(1, &ev);
}

__kernel void mdct_fft( __global float *in,
						__global float *out)
{
	__local float rot[2048];
	__local float bufa_r[512];
	__local float bufa_i[512];
	__local float *bufb_r = rot + 1024;
	__local float *bufb_i = rot + 1536;

	size_t lcl_id = get_local_id(0);

	download_rotate_samples(in, rot, lcl_id);

	// rot -> bufa
	calc_fk(rot, bufa_r, bufa_i, lcl_id);
	barrier(CLK_LOCAL_MEM_FENCE);

	// bufA -> bufB
	twiddle_mul(bufa_r, bufa_i, bufb_r, bufb_i, lcl_id);
	barrier(CLK_LOCAL_MEM_FENCE);

	// bufB -> bufB
	{
		fft_shuffle(bufb_r, bufb_i, lcl_id);
		fft_512(bufb_r, bufb_i, lcl_id);
	}

	// fft and shuffle implement local memory barrier

	// bufB -> bufA
	twiddle_mul(bufb_r, bufb_i, bufa_r, bufa_i, lcl_id);
	barrier(CLK_LOCAL_MEM_FENCE);

	// bufA -> rot
	arrange_output(bufa_r, bufa_i, rot, lcl_id);
	barrier(CLK_LOCAL_MEM_FENCE);

	upload_result(rot, out);
}
