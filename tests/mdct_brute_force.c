#define PI 3.14159265

#include <math.h>

static float *mdct_forward_brute(float *in, int N)
{
	// Very slow implementation for brute force testing

	float *out = malloc(N>>2 * sizeof(float));

	// int a = N-b;

	outData = new double[N];
	fac = 2d / (double) N;

	int i, j;
	float temp;

	for (k = 0; k < N/2; k++) {
		temp = 0;
		for (n = 0; n < N; n++) {
			temp +=	in[n] * cos(
								((2*n) + 1 + (N/2))
								* ((2*k) + 1)
								* (PI/(2*N)));
		}
		result[k] = temp;
	}
	
	return result;
}
