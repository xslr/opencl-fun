#include <stdlib.h>
#include <time.h>

#include <glib.h>
#include <glib/gstdio.h>

#include "sample_source.h"

char *sox_waveform_gen( unsigned int sr, float seconds, unsigned int freq, size_t sample_len,
						const char shape[], const char prefix[], size_t prefix_len);
char *sox_noise_gen( unsigned int sr, float seconds, size_t sample_len,
					 const char noise_type[], const char prefix[], size_t prefix_len);

int totxtf(float *data, size_t count, const char *separator,
		   size_t rowlen, const char *rowmarker,
		   const char *filename, const char *outdir, const char *modname)
{
	// TODO: use outdir
	FILE *fp = g_fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "%s:: Failed to open %s for writing!!\n", modname, filename);
		return -1;
	}

	size_t index;
	for (index = 0; index < count; index++) {
		if (index != 0 && index%rowlen == 0)
			fprintf(fp, "%s", rowmarker);

		if (index < (count-1))
			fprintf(fp, "%f%s", data[index], separator);
		else
			fprintf(fp, "%f", data[index]);
	}

	fclose(fp);

	return 0;
}

int totxtd(double *data, size_t count, const char *separator,
		   size_t rowlen, const char *rowmarker,
		   const char *filename, const char *outdir, const char *modname)
{
	// TODO: use outdir
	FILE *fp = g_fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "%s:: Failed to open %s for writing!!\n", modname, filename);
		return -1;
	}

	size_t index;
	for (index = 0; index < count; index++) {
		if (index != 0 && index%rowlen == 0)
			fprintf(fp, "%s", rowmarker);

		if (index < (count-1))
			fprintf(fp, "%f%s", data[index], separator);
		else
			fprintf(fp, "%f", data[index]);
	}

	fclose(fp);

	return 0;
}

float *get_zero(size_t count)
{
	float *buf = malloc(count * sizeof(float));

	while (count > 0) {
		--count;
		buf[count] = 0;
	}
	return buf;
}

float *get_seq(float delta, size_t count)
{
	float *data = malloc(count*sizeof(float));

	float val = 0.0f;
	size_t i;
	for (i = 0; i < count; i++)
	{
		data[i] = val;
		val += delta;
	}

	return data;
}

float *get_random(size_t count)
{
	float *buf = malloc(sizeof(float) * count);

	srand( time(NULL) );

	while (count > 0){
		--count;
		buf[count] = rand();
	}

	return buf;
}

float *get_alt_1_0(size_t count)
{
	float *buf = malloc(sizeof(float) * count);

	size_t float_bit_len = sizeof(float) << 3;

	unsigned int pilot = 0;

	while (float_bit_len > 0){
		pilot ^= float_bit_len % 2;
		pilot <<= 1;

		--float_bit_len;
	}

	while (count > 0){
		buf[--count] = pilot;
	}

	return buf;
}

float *get_sine_wave(unsigned int sr, float seconds, unsigned int freq)
{
	size_t sample_len = sizeof(float);
	char *filename = sox_waveform_gen( sr, seconds, freq, sample_len,
									   "sine", "aacl_sine", 10);

	// got pcm data in file. now load it into a buffer
	float *buf;
	size_t buf_size;
	GError *gerr;
	if (!g_file_get_contents( filename,
							  (char**)&buf,
							  &buf_size,
							  &gerr)) {
		fprintf(stderr, "Failed to read sound file: %s.\n", gerr->message);
		g_error_free(gerr);
		return NULL;
	}

	// TODO: remove file after using it

	// release buffers
	free(filename);

	return buf;
}

float *get_white_noise(unsigned int sr, float seconds)
{
	size_t sample_len = sizeof(float);
	char *filename = sox_noise_gen(sr, seconds, sample_len, "whitenoise", "aacl_whitenoise", 10);

	// got pcm data in file. now use it
	float *buf;
	size_t buf_size;
	GError *gerr;
	if (!g_file_get_contents( filename,
							  (char**)&buf,
							  &buf_size,
							  &gerr)) {
		fprintf(stderr, "Failed to read sound file: %s.\n", gerr->message);
		g_error_free(gerr);
		return NULL;
	}
	// TODO: remove file after using it

	// release buffers
	free(filename);

	return buf;
}

char *sox_waveform_gen(unsigned int sr, float seconds, unsigned int freq, size_t sample_len,
					   const char shape[], const char prefix[], size_t prefix_len)
{
	size_t fname_len = prefix_len + 6 + 1;
	char *filename = malloc((fname_len) * sizeof(char));

	snprintf(filename, fname_len, "%sXXXXXX", prefix);

	// buffer for storing the sox command line
	size_t cmd_len = 100 + fname_len;
	char *cmd = malloc(cmd_len * sizeof(char));

	filename = mktemp(filename);
	if (filename[0] == 0) {
		// cannot create
		return NULL;
	}

	// create raw file of signal amplitudes encoded as floats
	int ret = snprintf(cmd, cmd_len,
					   "sox -r %d -e float -%lu -n %s synth %f %s %d",
					   sr, sample_len, filename, seconds, shape, freq);
	if (ret >= cmd_len) {
		// command buffer for sox is too small
		fprintf(stderr, "Command buffer for SoX is too small.\n");
	}

	ret = system(cmd);
	if (ret != 0) {
		// failed to generate wav file
		return NULL;
	}

	free(cmd);

	return filename;
}

char *sox_noise_gen(unsigned int sr, float seconds, size_t sample_len,
					const char noise_type[], const char prefix[], size_t prefix_len)
{
	size_t fname_len = prefix_len + 6 + 1;
	char *filename = malloc((fname_len) * sizeof(char));

	snprintf(filename, fname_len, "%sXXXXXX", prefix);

	// buffer for storing the sox command line
	size_t cmd_len = 100 + fname_len;
	char *cmd = malloc(cmd_len * sizeof(char));

	filename = mktemp(filename);
	if (filename[0] == 0) {
		// cannot create
		return NULL;
	}

	// create raw file of signal amplitudes encoded as floats
	int ret = snprintf(cmd, cmd_len,
					   "sox -r %d -e float -%lu -n %s synth %f %s",
					   sr, sample_len, filename, seconds, noise_type);
	if (ret >= cmd_len) {
		// command buffer for sox is too small
		fprintf(stderr, "Command buffer for SoX is too small.\n");
	}

	ret = system(cmd);
	if (ret != 0) {
		// failed to generate wav file
		return NULL;
	}

	free(cmd);

	return filename;
}
