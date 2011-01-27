#ifndef AACL_SAMPLE_SOURCE_H
#define AACL_SAMPLE_SOURCE_H

float *get_random(size_t count);

float *get_alt_1_0(size_t count);

float *get_sine_wave(unsigned int sr, float seconds, unsigned int freq);

float *get_white_noise(unsigned int sr, float seconds);

int tocsvf(float *data, size_t count, const char *filename, const char *outdir,
		   const char *modname);

float *get_zero(size_t count);
#endif