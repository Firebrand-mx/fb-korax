// i_A3D.h

#ifndef __DOOMSDAY_A3D_H__
#define __DOOMSDAY_A3D_H__

int I3_Init();
void I3_Shutdown();
void I3_BeginSoundFrame();
void I3_EndSoundFrame();
void I3_SetGlobalVolume(int volume); // Volume is 0..1000

int I3_PlaySound2D(void *data, int volume, int pan, int pitch);
void I3_UpdateSound2D(int handle, int volume, int pan, int pitch);
int I3_PlaySound3D(void *data, sound3d_t *desc);
void I3_UpdateSound3D(int handle, sound3d_t *desc);
void I3_StopSound(int handle);
void I3_UpdateListener(listener3d_t *desc);
int I3_IsSoundPlaying(int handle);

#endif //__DOOMSDAY_A3D_H__