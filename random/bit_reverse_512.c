/*
 * bit_reverse_512.c
 *
 * Test program to test sample shuffling using bit reversal.
 *
 */

#include <stdio.h>
#include <stdlib.h>

int main()
{
	unsigned short original;
	printf("sizeof(unsigned short) is %lu bits\n", 8*sizeof(unsigned short));

	for (original = 0; original < 512; original++)
	{
		unsigned short rev = original;

		rev = ((rev & 0x0555) << 1) | ((rev & 0x0aaa) >> 1);
		rev = ((rev & 0x0333) << 2) | ((rev & 0x0ccc) >> 2);
		rev = ((rev & 0x000f) << 8) | ((rev & 0x0f00) >> 8) | (rev & 0x00f0);

		rev >>= 3;

		printf("%i\t%i\n", original, rev);
	}

	return EXIT_SUCCESS;
}
