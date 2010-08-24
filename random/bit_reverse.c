/*
 * bit_reverse.c
 *
 * Test program to test sample shuffling using bit reversal.
 *
 */

#include <stdio.h>
#include <stdlib.h>

int main()
{
	int original;
	printf("sizeof(unsigned short) is %lu bits\n", 8*sizeof(unsigned short));

	for(original = 0; original < 2048; original++)
	{
		unsigned short rev = original;
				
		{
			rev = ((rev & 0x5555) << 1) | ((rev & 0xaaaa) >> 1);
			rev = ((rev & 0x3333) << 2) | ((rev & 0xcccc) >> 2);
			rev = ((rev & 0x000f) << 8) | (rev & 0x00f0) | ((rev & 0x0f00) >> 8);
			
			rev >>= 1;
		}
		
		printf("%u\t%u\n", original, rev);
	}

	return EXIT_SUCCESS;
}
