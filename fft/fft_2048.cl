/*
 * fft_2048.cl
 *
 * 2048 point FFT computation kernel.
 * This has been developed on an nVidia 8800GT with 512MB global memory and 16KB.
 * It computes two 1024 point FFTs using 512 threads and then computes two
 * 2048 point partial FFTs in 'fft_2048_interleave' that together compute a 2048
 * point FFT.
 *
 * History: 24/8/2010 // Subrat Meher // Initial working version
 *
 */

#define TWO_PI 6.28318531

//{{{ function macros

// swap two values using a temporary value
#define SWAP(a, b) ({ float tmp; tmp=a; a=b; b=tmp; })

// calculate twiddle factor
#define TWDL_CALC(twdl) ({ twdl.y = 0 - native_sin(twdl.x); twdl.x = native_cos(twdl.x); })

#define TWDL_MUL(twdl, r, i, idx) ({ float2 a = (float2)(r[idx], i[idx]); r[idx] = (a.x*twdl.x) - (a.y*twdl.y); i[idx] = (a.y*twdl.x) + (a.x*twdl.y); })

#define DFT2(r, i, i1, i2) ({ float2 a = (float2)(r[i1], i[i1]); float2 b = (float2)(r[i2], i[i2]); a = a+b; b=a-(2*b); r[i1] = a.x; i[i1] = a.y; r[i2] = b.x; i[i2] = b.y; })

// TWDL_MUL_ALT and DFT_2_ALT are function macros for special addressing of i_buf used while computing 2048 point fft
#define TWDL_MUL_ALT(twdl, r, i, idx_re, idx_im) ({ float2 a = (float2)(r[idx_re], i[idx_im]); r[idx_re] = (a.x*twdl.x) - (a.y*twdl.y); i[idx_im] = (a.y*twdl.x) + (a.x*twdl.y); })

#define DFT2_ALT(r, i, idx1_re, idx2_re, idx1_im, idx2_im) ({ float2 a = (float2)(r[idx1_re], i[idx1_im]); float2 b = (float2)(r[idx2_re], i[idx2_im]); a = a+b; b=a-(2*b); r[idx1_re] = a.x; i[idx1_im] = a.y; r[idx2_re] = b.x; i[idx2_im] = b.y; })
//}}}

//{{{ prototypes

void fft_1024( __local float *r_buf,
			   __local float *i_buf);

void fft_2048_interleave( __local  float *r_buf,
						  __local  float *i_buf,
						  __private ushort offset);

void shuff_and_window( __global float *in,
					   __global float *out,
					   __local  float *r_buf,
					   __local  float *i_buf);
//}}}

__kernel void fft_2048
	( __global float *in,
	  __global float *out)
{

	__local float r_buf[2048],
				  i_buf[1024];

	shuff_and_window(in, out, r_buf, i_buf);

	fft_1024( r_buf,
			  i_buf);

	//{{{ upload results first 1024 points' FFT and reset imaginary local buffer
	{
		__private event_t ev;
		__global  float  *i_out = out + ((get_num_groups(0))<<11);
		__private size_t grp_id = get_group_id(0);
		
		// copy imaginary components to global mem
		async_work_group_copy( i_out + (grp_id << 11),
							   &i_buf[0],
							   (size_t)1024,
							   ev);
		
		// reset local imaginary buffer for fft computation of second half of data
		__private size_t lcl_id = get_local_id(0);
		i_buf[lcl_id] = 0;
		i_buf[lcl_id + 512] = 0;

		wait_group_events(1, &ev);
	}
	//}}}

	fft_1024( r_buf + 1024,
			  i_buf);

	//{{{ upload results and download imaginary components for next fft
	{
		__private event_t ev;
		__global  float  *i_out = out + ((get_num_groups(0))<<11);
		__private size_t grp_id = get_group_id(0);

		// 2048 real components always preserved in local mem

		// copy imaginary components from 1024 point FFT to global mem
		async_work_group_copy( i_out + (grp_id << 11) + 1024,
							   &i_buf[0],
							   (size_t)1024,
							   ev);

		// copy imaginary component chunks for interleaved 2048 point FFT
		async_work_group_copy( &i_buf[0],
							   i_out + (grp_id << 11),
							   (size_t)512,
							   ev);

		// offset imaginary global ptr by half of 2048 = 1024
		async_work_group_copy( &i_buf[0] + 512,
							   i_out + (grp_id << 11) + 1024,
							   (size_t)512,
							   ev);

		wait_group_events(3, &ev);
	}
	//}}}

	fft_2048_interleave( r_buf,
						 i_buf,
						 0);

	//{{{ upload imaginary components of previous computation and download future ones
	{
		__private event_t ev;
		__global  float  *i_out = out + ((get_num_groups(0))<<11);
		__private size_t grp_id = get_group_id(0);

		// copy imaginary components to global mem
		async_work_group_copy( i_out + (grp_id << 11),
							   &i_buf[0],
							   (size_t)512,
							   ev);
		async_work_group_copy( i_out + (grp_id << 11) + 1024,
							   &i_buf[0] + 512,
							   (size_t)512,
							   ev);

		// copy imaginary components to local mem
		
		// offset imaginary global ptr by 512 since that is the interleaving stride
		async_work_group_copy( &i_buf[0],
							   i_out + (grp_id << 11) + 512,
							   (size_t)512,
							   ev);

		// offset imaginary global ptr by 512 + 1024 for interleaving stride + N/2
		async_work_group_copy( &i_buf[0] + 512,
							   i_out + (grp_id << 11) + 512 + 1024,
							   (size_t)512,
							   ev);

		wait_group_events(4, &ev);
	}
	//}}}

	fft_2048_interleave( r_buf + 512,
						 i_buf,
						 512);

	//{{{ upload imaginary and real components
	{
		__private event_t ev;
		__private size_t  grp_id = get_group_id(0);
		__global float *i_out = out + ((get_num_groups(0))<<11);

		// copy imaginary components to global mem
		async_work_group_copy( i_out + (grp_id << 11) + 512,
							   &i_buf[0],
							   (size_t)512,
							   ev);

		async_work_group_copy( i_out +  (grp_id << 11) + 512 + 1024,
							   i_buf + 512,
							   (size_t)512,
							   ev);

		// copy real components to global mem
		async_work_group_copy( out + (grp_id << 11),
							   &r_buf[0],
							   (size_t)2048,
							   ev);
		wait_group_events(1, &ev);
	}
	//}}}

}

//{{{ shuff_and_window
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
	i_buf[lcl_id + 512] = 0;

	// wait for data to arrive before shuffling it
	wait_group_events(1, &ev);

	{
		__private ushort2 index_pair;
		//{{{ calculate initialswap indices
		index_pair.x = lcl_id;
		// lcl_id is expected to range from 0 .. 512
		index_pair.y = ((index_pair.x & 0x5555) << 1) | ((index_pair.x & 0xaaaa) >> 1);
		index_pair.y = ((index_pair.y & 0x3333) << 2) | ((index_pair.y & 0xcccc) >> 2);
		index_pair.y = ((index_pair.y & 0x000f) << 8) | (index_pair.y & 0x00f0) | ((index_pair.y & 0x0f00) >> 8);
		
		index_pair.y >>= 1;
		//}}}

		//{{{ swap components
//		{
			SWAP( r_buf[index_pair.x],
				  r_buf[index_pair.y]);

			barrier(CLK_LOCAL_MEM_FENCE);

			index_pair += (ushort2)(512, 2);
			if(index_pair.y > index_pair.x) {
				SWAP( r_buf[index_pair.x],
					  r_buf[index_pair.y]);
			}

			barrier(CLK_LOCAL_MEM_FENCE);

			index_pair += (ushort2)(512, 0-1);
			if(index_pair.y > index_pair.x) {
				SWAP( r_buf[index_pair.x],
					  r_buf[index_pair.y]);
			}

			barrier(CLK_LOCAL_MEM_FENCE);

			index_pair += (ushort2)(512, 2);
			if(index_pair.y > index_pair.x) {
				SWAP( r_buf[index_pair.x],
					  r_buf[index_pair.y]);
	
			}

			barrier(CLK_LOCAL_MEM_FENCE);
//		}
		//}}}
	}
}
//}}}

//{{{ fft_2048_interleave
void fft_2048_interleave( __local float *r_buf,
						  __local float *i_buf,
						  __private ushort offset)
{
	__private event_t ev;
	__private size_t lcl_id = get_local_id(0);
	__private size_t num_groups = get_num_groups(0);
	__private size_t grp_id = get_group_id(0);
	__private float2 twdl;
	__private ushort sample_index = floor((float)(lcl_id<<1) / (float)2048)
								* 2048
								+ (lcl_id % (1024));


	// compute fft
	// this is a simplification for FFT of 2048 points case of the expression
	// in fft_1024
	twdl.x = (float)((lcl_id+offset) % 1024) * (float)TWO_PI / (float)2048 ;

	// calculate twiddle factor
	TWDL_CALC(twdl);
	
	// apply twiddle factor
	TWDL_MUL_ALT(twdl, r_buf, i_buf, sample_index + 1024, sample_index + 512);

	// calculate two point DFT
	DFT2_ALT(r_buf, i_buf, sample_index, sample_index + 1024, sample_index, sample_index + 512);

	barrier(CLK_LOCAL_MEM_FENCE);
}
//}}}

//{{{ fft_1024
//	computes a 1024 point radix-2 fft
void fft_1024( __local float *r_buf,
			   __local float *i_buf)
{
	//{{{ description
	// this method computes a 1024 point radix 2 fft and uploads the result
	// to global memory.
	// assumes the 1024 source data points are located starting at r_buf and i_buf
	// in local memory and results are to be uploaded to global memory at
	// out (for real results) and out + (num_groups * 2048) for imaginary results
	//}}}

	//{{{ assumptions
	// r_buf always contains the 2048 real data items being worked on
	// i_buf contains the 1024 data items that are useful to the current function
	//}}}

	for( ushort lcl_fft_size = 2;
				lcl_fft_size <= 1024;
				lcl_fft_size = lcl_fft_size << 1)
	{
		
		__private size_t lcl_id = get_local_id(0);
		__private float2 twdl;
		__private ushort sample_index =
				floor((float)(lcl_id<<1) / (float)lcl_fft_size)
				* lcl_fft_size
				+ (lcl_id % (lcl_fft_size>>1));

		//{{{ twiddle factor derivation explanation
		// twiddle factor is given by exp(-2i*PI*k*n/N)
		//		= cos(k/N) - isin(k/N)
		// k for a work item is given by (thread_id % (local_fft_size/2)) * stride
		//		stride = final_fft_length / local_fft_size
		//			   = N / local_fft_size
		//		so k/N simplifies to (thread_id % local_fft_size/2) * (N/local_fft_size) / N
		//			   = (thread_id  % (local_fft_size/2)) / local_fft_size
		//}}}
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

		barrier(CLK_LOCAL_MEM_FENCE);
	}
}
//}}} 

