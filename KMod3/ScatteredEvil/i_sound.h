#ifndef __SOUND__
#define __SOUND__

#include "i_a3d.h"	
#include "i_ds_eax.h"

#define SND_TICRATE		140             // tic rate for updating sound
#define SND_MAXSONGS    40              // max number of songs in game
#define SND_SAMPLERATE  11025			// sample rate of sound effects

#define MAX_SND_DIST    2025
#define MAX_CHANNELS    16


void I_PauseSong(void);
void I_ResumeSong(void);
void I_SetMusicVolume(int volume);
void I_SetSfxVolume(int volume);
int I_RegisterSong(void *data);
int I_QrySongPlaying(void);
void I_StopSong(void);
int I_PlaySong(void *data, int length, int looping);


void I_StartupSound (void);
void I_ShutdownSound (void);
void I_SetChannels(int channels);

void I_UpdateCDMusic(void);

byte *I_Resample8bitSound(byte *data, int len, int freq, int resampleFactor, 
						  boolean make16bits, int *outlen);

#endif // __SOUND__

#ifndef __ICDMUS__
#define __ICDMUS__

extern int cd_Error;

int I_CDMusInit(void);
int I_CDMusPlay(int track, int loop);
int I_CDMusStop(void);
int I_CDMusResume(void);
int I_CDMusGetVolume();
int I_CDMusSetVolume(int volume);
int I_CDMusFirstTrack(void);
int I_CDMusLastTrack(void);
int I_CDMusTrackLength(int track);

void I_UpdateMusicDevice();

#endif

