#include <stdlib.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>

#include <sox.h>

#include "sample_source.h"

unsigned int *getRandom(size_t count)
{
	unsigned int *buf = malloc(sizeof(unsigned int) * count);

	srand( time(NULL) );

	while (count > 0){
		--count;
		buf[count] = rand();
	}
}

unsigned int *getAlt_1_0(size_t count)
{
	unsigned int *buf = malloc(sizeof(unsigned int) * count);

	size_t uint_bit_len = sizeof(unsigned int) << 3;

	unsigned int pilot = 0;

	while (uint_bit_len > 0){
		pilot ^= uint_bit_len % 2;
		pilot <<= 1;

		--uint_bit_len;
	}

	while (count > 0){
		buf[--count] = pilot;
	}
}

unsigned int *getSineWave(unsigned int sr, float seconds, unsigned int freq, size_t sample_len)
{
	char *filename = sox_cmd_wrap(sr, seconds, freq, sample_len);

	// got pcm data in file. now use it

	struct stat info;
	stat(filename, &info);

	unsigned int *samples = malloc(info.st_size);
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		// file open failed
		free(samples);
		return NULL;
	}
	fread(samples, 1, info.st_size, fp);
	fclose(fp);

	// remove file after using it
	snprintf(cmd, cmd_len, "rm %s.raw", filename);
	system(cmd);

	// release buffers
	free(filename);
}

char *sox_cmd_wrap_sine(unsigned int sr, float seconds, unsigned int freq, size_t sample_len,
				   (const char) *shape, (const char) *prefix, size_t prefix_len)
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
					   "sox -r %d -e float -%d -n %s synth %f %s %d",
					   sr, slen, filename, seconds, shape, freq);
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

char *sox_cmd_wrap_noise(unsigned int sr, float seconds, size_t sample_len,
				   (const char) *noise_type, (const char) *prefix, size_t prefix_len)
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
					   "sox -r %d -e float -%d -n %s synth %f %s",
					   sr, slen, filename, seconds, noise_type);
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

unsigned int *getWhiteNoise(unsigned int sr, float seconds, size_t sample_len)
{
	char *filename = sox_cmd_wrap(sr, seconds, sample_len, "whitenoise", "whitenoise", 10);

	// got pcm data in file. now use it
	struct stat info;
	stat(filename, &info);

	unsigned int *samples = malloc(info.st_size);
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		// file open failed
		free(samples);
		return NULL;
	}
	fread(samples, 1, info.st_size, fp);
	fclose(fp);

	// remove file after using it
	snprintf(cmd, cmd_len, "rm %s.raw", filename);
	system(cmd);

	// release buffers
	free(filename);
}
