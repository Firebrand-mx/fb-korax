#ifndef __JAKETOOLS_SOUND_H__
#define __JAKETOOLS_SOUND_H__

#ifdef __cplusplus
extern "C" {
#endif

int jtSoundInit(HWND hWnd);
void jtSoundShutdown();

void jtSoundSetChannels(int numChannels);

// Returns the handle of the sample.
int jtSoundPlay(void *sampleData, int length, int bits, int stereo, int freq, 
				int volume, int pan, int pitch);
int jtSoundIsPlaying(int handle);
void jtSoundStop(int handle);

// Volume is in range from 0 to 255.
void jtSoundSetVolume(int handle, int vol);

// Pan is in range from 0 to 1000, 500 being in the middle.
void jtSoundSetPan(int handle, int pan);

// Relative frequency, 1000 corresponds the original one.
void jtSoundSetFreq(int handle, int freq);


#ifdef __cplusplus
}
#endif

#endif // __JAKETOOLS_SOUND_H__