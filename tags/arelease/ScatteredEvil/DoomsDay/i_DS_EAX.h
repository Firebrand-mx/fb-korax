#ifndef __DOOMSDAY_DS3D_EAX_H__
#define __DOOMSDAY_DS3D_EAX_H__

int I2_Init();
void I2_Shutdown();
void I2_BeginSoundFrame();
void I2_EndSoundFrame();

int I_Play2DSound(void *data, int volume, int pan, int pitch);
void I_Update2DSound(int handle, int volume, int pan, int pitch);
int I_Play3DSound(void *data, sound3d_t *desc);
void I_Update3DSound(int handle, sound3d_t *desc);
void I_StopSound(int handle);
void I_UpdateListener(listener3d_t *desc);
int I_SoundIsPlaying(int handle);

#endif //__DOOMSDAY_A3D_H__