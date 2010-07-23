#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG_RIFF_CHUNK 0x46464952
#define TAG_WAVE_CHUNK 0x45564157
#define TAG_FMT_CHUNK 0x20746D66
#define TAG_DATA_CHUNK 0x61746164

#define MAX_PATH_LEN 1024

int print_wav_info(char* wavfilepath);
void get_file_path(const char* src_filename, char** file_path);

int main(int argc, char** argv)
{
	char* filename_src = "wav-list.txt";
	char wav_path[MAX_PATH_LEN];

	get_file_path(filename_src, &wav_path);

	if(wav_path == NULL)
	{
		fprintf(stderr, "Cannot find a path to a wav file in %s.", filename_src);
		return EXIT_FAILURE;
	}

	print_wav_info(wav_path);
}

void get_file_path(const char* src_filename, char** file_path)
{
	FILE* pathfile = fopen(src_filename, "rt");

	if(pathfile == NULL)
	{
		return NULL;
	}

	fgets(file_path, MAX_PATH_LEN - 2, pathfile);

	fclose(src_filename);
}

int print_wav_info(char* wavfilepath)
{
	void* buf_hdr = malloc(4);

	long szFile;
	long szPCMData;
	unsigned int szSubChunk1;
	unsigned int audioFormat = 0;
	unsigned int sampleRate;
	unsigned int bitsPerSample = 0;
	unsigned int numChannels = 0;

	FILE* wavfilehandle;

	wavfilehandle = fopen(wavfilepath, "rb");

	if(wavfilehandle == NULL)
	{
		fprintf(stderr, "error opening file:%s\n", wavfilepath);
		return EXIT_FAILURE;
	}
	else
	{
		fprintf(stdout, "Reading file %s\n", wavfilepath);
	}

	fread(&buf_hdr, 1, 4, wavfilehandle);

	if(buf_hdr != TAG_RIFF_CHUNK)
	{
		fprintf(stderr, "Cannot find a 'RIFF' chunk.\n");
		return EXIT_FAILURE;
	}
	
	fread(&szFile, 1, 4, wavfilehandle);
	szFile += 8;

	fread(&buf_hdr, 1, 4, wavfilehandle);
	if(buf_hdr != TAG_WAVE_CHUNK)
	{
		fprintf(stderr, "Cannot find a 'WAVE' chunk.\n");
		return EXIT_FAILURE;
	}

	fread(&buf_hdr, 1, 4, wavfilehandle);
	if(buf_hdr != TAG_FMT_CHUNK)
	{
		fprintf(stdout, "Cannot find a 'fmt' subchunk.\n");
		return EXIT_FAILURE;
	}
	
	// first sub-chunk size
	fread(&szSubChunk1, 1, 4, wavfilehandle);
	if(szSubChunk1 != 16)
	{
		fprintf(stderr, "Size of subchunk 1 is %u. This is not PCM.\n", szSubChunk1);
		return EXIT_FAILURE;
	}

	// audio format
	// 0x0001 LPCM
	// 0x0006 A-law log PCM
	// 0x0007 mu-law log PCM
	fread(&audioFormat, 1, 2, wavfilehandle);

	// mono / stereo
	fread(&numChannels, 1, 2, wavfilehandle);

	// sample rate
	fread(&sampleRate, 1, 4, wavfilehandle);

	// byte rate -- not used
	fread(&buf_hdr, 1, 2, wavfilehandle);

	// block align -- not used
	fread(&buf_hdr, 1, 4, wavfilehandle);

	// bits per sample
	fread(&bitsPerSample, 1, 2, wavfilehandle);

	fread(&buf_hdr, 1, 4, wavfilehandle);
	if(buf_hdr != TAG_DATA_CHUNK)
	{
		fprintf(stderr, "Not found a 'data' chunk.");
		return EXIT_FAILURE;
	}

	fread(&szPCMData, 1, 4, wavfilehandle);

	fprintf(stdout, "Filesize (bytes): %i\n", szFile);
	fprintf(stdout, "PCM data (bytes): %i\n", szPCMData);
	fprintf(stdout, "Channels: %u\n", numChannels);
	fprintf(stdout, "Sample rate: %u\n", sampleRate);
	fprintf(stdout, "Bits per sample: %u\n", bitsPerSample);

	fclose(wavfilehandle);

	getchar();
	
	return EXIT_SUCCESS;
}