#ifndef __H2MUSMIDI_H__
#define __H2MUSMIDI_H__

void MUS_PauseSong();
void MUS_ResumeSong();
void MUS_SetMasterVolume(int vol);
int MUS_RegisterSong(void *data, int length);
void MUS_UnregisterSong(void);
int MUS_IsSongPlaying();
void MUS_StopSong();
void MUS_PlaySong(int looping);
int MUS_Init();
void MUS_Shutdown();
void MUS_Disable(int really);

void MUS_SongPlayer();

#endif