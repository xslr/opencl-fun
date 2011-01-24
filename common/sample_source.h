#ifndef AACL_SAMPLE_SOURCE_H
#define AACL_SAMPLE_SOURCE_H

(unsigned int)* getRandom(size_t count);

(unsigned int)* getAlt_1_0(size_t count);

(unsigned int)* getSineWave(unsigned int sr, float seconds, unsigned int freq);

(unsigned int)* getWhiteNoise(unsigned int sr, float seconds);

//(unsigned int)* getWAVFile(char* filename);

#endif
