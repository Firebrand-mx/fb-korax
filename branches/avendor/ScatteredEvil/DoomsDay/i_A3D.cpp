//
// i_A3D.c
// Playing 3D sounds with A3D 3.0 for the Doomsday engine.
//
// Author: Jaakko Keränen
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <objbase.h>
#include <initguid.h>
#include <ia3dapi.h>
#include <ia3dutil.h>
#include <stdlib.h>
#include <math.h>
#include "dd_def.h"
#include "i_sound.h"
#include "i_win32.h"
#include "settings.h"

// MACROS ------------------------------------------------------------------

#define MAX_SND_DIST    2025

#define SBUFPTR(handle)		(buffers+handle-1)
#define SBUFHANDLE(sbptr)	((sbptr-buffers)+1)

#define SBF_UNOWNED		0x1	// The buffer is not owned by anyone.
#define SBF_3D			0x2 // The buffer is a 3D sound.

// TYPES -------------------------------------------------------------------

typedef struct
{
	IA3dSource2	*source;	// Pointer to the source.
	int			flags;
	int			startTime;	// I_GetTime().
} sndbuffer_t;

// PRIVATE FUNCTIONS --------------------------------------------------------

void I3_DestroyBuffers();
int I3_PlaySound(void *data, boolean play3d, sound3d_t *desc, int pan);

// EXTERNAL DATA ------------------------------------------------------------

extern HWND hWndMain;
extern int snd_SfxVolume;	// 0..255

// PRIVATE DATA -------------------------------------------------------------

static boolean initOk = false;		// A3D has been initialized successfully?

static IA3d5			*a3d;
static IA3dReverb		*a3dReverb = NULL;
static IA3dListener		*a3dListener;
static A3DCAPS_HARDWARE	hwCaps;
static HRESULT			hr;

static sndbuffer_t		*buffers = 0;
static int				numBuffers = 0;


// CODE ---------------------------------------------------------------------

// Returns true if the initialization succeeds.
int I3_Init()
{
	if(initOk) return true;

	if(FAILED(A3dRegister())) return false;
	if(FAILED(A3dInitialize())) return false;
	if(FAILED(hr = A3dCreate(NULL, (void**) &a3d, NULL, A3D_REVERB)))
	{
		ST_Message( "I3_Init: Couldn't create a3d (result = %d).\n", hr&0xffff);
		return false;
	}
	// Set the cooperative level.
	if(FAILED(hr = IA3d5_SetCooperativeLevel(a3d, hWndMain, A3D_CL_NORMAL)))
	{
		ST_Message( "I3_Init: Couldn't set cooperative level to normal (result = %d).\n", hr&0xffff);
		return false;
	}
	// Give me the listener!
	if(FAILED(hr = IA3d5_QueryInterface(a3d, IID_IA3dListener, (void**) &a3dListener)))
	{
		ST_Message( "I3_Init: Couldn't get a listener (result = %d).\n", hr&0xffff);
		return false;
	}
	IA3d5_SetCoordinateSystem(a3d, A3D_LEFT_HANDED_CS);
	IA3d5_SetUnitsPerMeter(a3d, 36); // Based on the player sprite.
	IA3d5_SetDopplerScale(a3d, 1.5);
	hwCaps.dwSize = sizeof(hwCaps);
	IA3d5_GetHardwareCaps(a3d, &hwCaps);
	ST_Message( "I3_Init: Number of 3D buffers: %d\n", hwCaps.dwMax3DBuffers);

	// Everything is OK.
	initOk = true;
	return true;
}

void I3_Shutdown()
{
	if(!initOk) return;

	// Destroy all audio buffers.
	I3_DestroyBuffers();

	if(a3d) IA3d5_Shutdown(a3d); // This'll get rid of everything else.
	a3d = NULL;
	a3dListener = NULL;
	a3dReverb = NULL;
	A3dUninitialize();
	initOk = false;
}

void I3_BeginSoundFrame()
{
	if(!initOk) return;
	IA3d5_Clear(a3d);
}

void I3_EndSoundFrame()
{
	if(!initOk) return;
	IA3d5_Flush(a3d);
}


int I3_IsBufferPlaying(sndbuffer_t *buf)
{
	DWORD status;
	IA3dSource2_GetStatus(buf->source, &status);
	// Status == zero if buffer is *not* playing.
	return(status != 0);
}

// Returns a free buffer.
sndbuffer_t *I3_GetFreeBuffer(boolean play3d)
{
	int				i;
	unsigned int	num3D = 0;
	sndbuffer_t		*suitable = NULL, *oldest = NULL;

	if(numBuffers) oldest = buffers;
	// We are not going to have software 3D sounds, of all things.
	// Release stopped sources and count the number of playing 3D sources.
	for(i=0; i<numBuffers; i++)
	{
		if(!buffers[i].source) 
		{
			if(!suitable) suitable = buffers + i;
			continue;
		}
		// See if this is the oldest buffer.
		if(buffers[i].startTime < oldest->startTime)
		{
			// This buffer will be stopped if need be.
			oldest = buffers + i;
		}
		if(I3_IsBufferPlaying(buffers + i))
		{
			if(buffers[i].flags & SBF_3D) num3D++;						
		}
		else
		{
			// This buffer is not playing.
			if(!suitable) suitable = buffers + i;
		}
	}
	if(play3d && num3D >= hwCaps.dwMax3DBuffers && oldest)
	{
		// There are as many 3D buffers as there can be.
		// Stop the oldest sound.		
		if(oldest->source)
		{
			IA3dSource2_Stop(oldest->source);
			IA3dSource2_Release(oldest->source);
			oldest->source = NULL;
		}
		return oldest;		
	}
	if(suitable) return suitable;

	// Ah well. We need to allocate a new one.
	buffers = (sndbuffer_t *)realloc(buffers, sizeof(sndbuffer_t) * ++numBuffers);
	// -JL- Paranoia
	if (!buffers)
		I_Error("I3_GetFreeBuffer: realloc faiiled");
	// Clear it.
	memset(buffers + numBuffers-1, 0, sizeof(sndbuffer_t));
	// Return a pointer to it.
	return buffers + numBuffers-1;
}

void I3_DestroyBuffers()
{
	int		i;

	for(i=0; i<numBuffers; i++)
		if(buffers[i].source) IA3dSource2_Release(buffers[i].source);

	free(buffers);
	buffers = NULL;
	numBuffers = 0;
}

// vol is from 0 to 1000.
void I3_SetGlobalVolume(int vol)
{
	if(!initOk) return;
	
	IA3d5_SetOutputGain(a3d, vol/1000.0f);
}

// Volume is linear, from 0 to 1.
void I3_SetVolume(sndbuffer_t *buf, float volume)
{
	//float gain = 0;
	//if(volume > 0) gain = pow(.5, (100*(1-volume))/6);
	IA3dSource2_SetGain(buf->source, volume);
}

void I3_SetPitch(sndbuffer_t *buf, float pitch)
{
	IA3dSource2_SetPitch(buf->source, pitch);
}

// Pan is linear, from 0 to 1. 0.5 is in the middle.
void I3_SetPan(sndbuffer_t *buf, float pan)
{
	float	gains[2];
	
	if(pan < 0) pan = 0;
	if(pan > 1) pan = 1;

	if(pan == 0.5)
	{
		gains[0] = gains[1] = 1;	// In the center.
	}
	else if(pan < 0.5) // On the left?
	{
		gains[0] = 1;
		//gains[1] = pow(.5, (100*(1 - 2*pan))/6);
		gains[1] = 2*pan;
	}
	else if(pan > 0.5) // On the right?
	{
		//gains[0] = pow(.5, (100*(2*(pan-0.5)))/6);
		gains[0] = 1 - 2*(pan-0.5);
		gains[1] = 1;
	}
	IA3dSource2_SetPanValues(buf->source, 2, gains);
}

// Updates the parameters of the source.
void I3_UpdateBuffer(sndbuffer_t *buf, sound3d_t *desc)
{
	int		i;
	float	temp[3];

	if(desc->flags & DDSOUNDF_VOLUME)
	{
		I3_SetVolume(buf, desc->volume/1000.0f);
	}
	if(desc->flags & DDSOUNDF_PITCH)
	{
		I3_SetPitch(buf, desc->pitch/1000.0f);
	}
	if(desc->flags & DDSOUNDF_POS)
	{
		for(i=0; i<3; i++) temp[i] = FIX2FLT(desc->pos[i]);
		IA3dSource2_SetPosition3fv(buf->source, temp);
	}
	if(desc->flags & DDSOUNDF_MOV)
	{
		for(i=0; i<3; i++) temp[i] = FIX2FLT(desc->mov[i]);
		IA3dSource2_SetVelocity3fv(buf->source, temp);
	}
}

int I3_PlaySound2D(void *data, int volume, int pan, int pitch)
{
	sound3d_t desc;

	desc.flags = DDSOUNDF_VOLUME | DDSOUNDF_PITCH;
	desc.volume = volume;
	desc.pitch = pitch;
	return I3_PlaySound(data, false, &desc, pan);
}

int I3_PlaySound3D(void *data, sound3d_t *desc)
{
	return I3_PlaySound(data, true, desc, 0);
}

void I3_UpdateSound2D(int handle, int volume, int pan, int pitch)
{
	sndbuffer_t *buf;

	if(!initOk || handle < 1 || handle > numBuffers) return;
	buf = SBUFPTR(handle);
	if(buf->source == NULL) return;
	if(!I3_IsBufferPlaying(buf)) IA3dSource2_Play(buf->source, A3D_SINGLE);
	I3_SetVolume(buf, volume/1000.0f);
	I3_SetPan(buf, pan/1000.0f);
	I3_SetPitch(buf, pitch/1000.0f);
}

void I3_UpdateSound3D(int handle, sound3d_t *desc)
{
	sndbuffer_t *buf;

	if(!initOk || handle < 1 || handle > numBuffers) return;
	buf = SBUFPTR(handle);
	if(buf->source == NULL) return;
	if(!I3_IsBufferPlaying(buf)) IA3dSource2_Play(buf->source, A3D_SINGLE);
	I3_UpdateBuffer(SBUFPTR(handle), desc);
}

// Returns the handle to the new sound.
int I3_PlaySound(void *data, boolean play3d, sound3d_t *desc, int pan)
{
	unsigned short	*ptr = (unsigned short *)data;		// Easier access to the header.
	int				length, bits = 8, channels = 1, freq;
	int				hr;
	sndbuffer_t		*sndbuf;
	WAVEFORMATEX	form;
	void			*writePtr1=NULL, *writePtr2=NULL;
	DWORD			writeBytes1, writeBytes2, dataBytes;

	// Can we play sounds?
	if(!initOk || data == NULL) return 0;	// Sorry...

	if(snd_Resample != 1 && snd_Resample != 2 && snd_Resample != 4)
	{
		ST_Message( "I2_PlaySound: invalid resample factor.\n");
		snd_Resample = 1;
	}

	length = ptr[2];
	freq = ptr[1];
	ptr += 4; // Go to the beginning of the sample data.

	// Do we need to resample the data?
	if(snd_Resample != 1 || snd_16bits)
	{
		ptr = (unsigned short*) I_Resample8bitSound( (byte*) ptr, length, freq, 
			snd_Resample, snd_16bits, NULL);
		length *= snd_Resample;
		freq *= snd_Resample;
		if(snd_16bits) bits = 16;
	}
	dataBytes = length * bits/8 * channels;

	//ST_Message( "Playing: len:%d freq:%d bytes:%d vol:%d pitch:%d\n", length, freq, dataBytes, desc->volume, desc->pitch);

	// Get a buffer that's doing nothing.
	sndbuf = I3_GetFreeBuffer(play3d);
	sndbuf->flags = play3d? SBF_3D : 0;
	sndbuf->startTime = I_GetTime();

	// Create a new source, if necessary.
	if(sndbuf->source)
	{
		IA3dSource2_Release(sndbuf->source);
	}
	if(FAILED(hr = IA3d5_NewSource(a3d, play3d? A3DSOURCE_TYPEDEFAULT 
		: A3DSOURCE_INITIAL_RENDERMODE_NATIVE, &sndbuf->source)))
	{
		I_Error("I3_PlaySound: couldn't create a new source (result = %d).\n", hr & 0xffff);
		return 0;
	}

	// We need to load the wave data into the source.
	// Prepare the format description.
	memset(&form, 0, sizeof(form));
	form.wFormatTag = WAVE_FORMAT_PCM;
	form.nChannels = channels;
	form.nSamplesPerSec = freq;
	form.nBlockAlign = channels * bits/8;
	form.nAvgBytesPerSec = form.nSamplesPerSec * form.nBlockAlign;
	form.wBitsPerSample = bits;

	if(FAILED(hr = IA3dSource2_SetAudioFormat(sndbuf->source, (void*) &form)))
	{
		I_Error("I3_PlaySound: couldn't set wave format (result = %d).\n", hr & 0xffff);
		return 0;
	}

	// Allocate the resources.
	if(FAILED(hr = IA3dSource2_AllocateAudioData(sndbuf->source, dataBytes)))
	{
		I_Error("I3_PlaySound: couldn't allocate wave data (result = %d).\n", hr & 0xffff);
		return 0;
	}

	// Lock the buffer.
	if(FAILED(hr = IA3dSource2_Lock(sndbuf->source, 0, dataBytes, 
		&writePtr1, &writeBytes1, &writePtr2, &writeBytes2,	A3D_ENTIREBUFFER)))
	{
		I_Error("I3_PlaySound: couldn't lock source (result = %d).\n", hr & 0xffff);
		return 0;
	}

	// Copy the data over.
	memcpy(writePtr1, ptr, writeBytes1);
	if(writePtr2) memcpy(writePtr2, (char*) ptr + writeBytes1, writeBytes2);

	// Unlock the buffer.
	if(FAILED(hr = IA3dSource2_Unlock(sndbuf->source, writePtr1, writeBytes1, 
		writePtr2, writeBytes2)))
	{
		I_Error("I3_PlaySound: couldn't unlock source (result = %d).\n", hr & 0xffff);
		return 0;
	}
	if(play3d)
	{
		// Set the 3D parameters of the source.
		if(desc->flags & DDSOUNDF_VERY_LOUD)
		{
			// You can hear this from very far away (e.g. thunderclap).
			IA3dSource2_SetMinMaxDistance(sndbuf->source, 10000, 20000, A3D_MUTE);
		}
		else
		{
			IA3dSource2_SetMinMaxDistance(sndbuf->source, 256, MAX_SND_DIST, A3D_MUTE);
		}
		/*if(desc->flags & DDSOUNDF_LOCAL)
			IDirectSound3DBuffer_SetMode(sndbuf->source3D, DS3DMODE_DISABLE, DS3D_DEFERRED);*/
	}
	else
	{
		// If playing in 2D mode, set the pan values.
		I3_SetPan(sndbuf, pan/1000.0f);
	}
	I3_UpdateBuffer(sndbuf, desc);
	// Start playing the buffer.
	if(FAILED(hr = IA3dSource2_Play(sndbuf->source, A3D_SINGLE)))
	{
		I_Error("I3_PlaySound: couldn't start source (result = %d).\n", hr & 0xffff);
		return 0;
	}
	// Return the handle to the sound source.
	return SBUFHANDLE(sndbuf);
}

void I3_StopSound(int handle)
{
	sndbuffer_t *buf;

	if(!initOk || handle < 1 || handle > numBuffers) return;
	buf = SBUFPTR(handle);
	if(buf->source)
	{
		IA3dSource2_Stop(buf->source);
		IA3dSource2_Rewind(buf->source);
		// Free the source.
		//IA3dSource2_Release(buf->source);
		//buf->source = NULL;
	}
	//buf->flags = 0;
}

void I3_UpdateListener(listener3d_t *desc)
{
	float	temp[3], val; 
	int		i;

	if(!initOk || !desc) return;
	if(desc->flags & DDLISTENERF_POS)
	{
		for(i=0; i<3; i++) temp[i] = FIX2FLT(desc->pos[i]);
		IA3dListener_SetPosition3fv(a3dListener, temp);
	}	
	if(desc->flags & DDLISTENERF_MOV)
	{
		for(i=0; i<3; i++) temp[i] = FIX2FLT(desc->mov[i]);
		IA3dListener_SetVelocity3fv(a3dListener, temp);
	}
	if(desc->flags & DDLISTENERF_YAW || desc->flags & DDLISTENERF_PITCH)
	{
		float y, p, dummy;
		// Get the missing values.
		if(!(desc->flags && DDLISTENERF_YAW))
			IA3dListener_GetOrientationAngles3f(a3dListener, &y, &dummy, &dummy);
		else
			y = desc->yaw;
		if(!(desc->flags && DDLISTENERF_PITCH))
			IA3dListener_GetOrientationAngles3f(a3dListener, &dummy, &p, &dummy);
		else
			p = desc->pitch;
		// Set the values.
		IA3dListener_SetOrientationAngles3f(a3dListener, y, p, 0);
	}
	if(desc->flags & DDLISTENERF_SET_REVERB)
	{
		A3DREVERB_PROPERTIES rp;
		A3DREVERB_PRESET *pre = &rp.uval.preset;
		if(a3dReverb == NULL)
		{
			// We need to create it.
			if(FAILED(hr = IA3d5_NewReverb(a3d, &a3dReverb)))
				return; // Silently go away.
			// Bind it as the current one.
			IA3d5_BindReverb(a3d, a3dReverb);
		}
		memset(&rp, 0, sizeof(rp));
		rp.dwSize = sizeof(rp);
		rp.dwType = A3DREVERB_TYPE_PRESET;
		pre->dwSize = sizeof(A3DREVERB_PRESET);

		val = desc->reverb.space;
		if(desc->reverb.decay > .5)
		{
			// This much decay needs at least the Generic environment.
			if(val < .2) val = .2f;
		}
		pre->dwEnvPreset = val >= 1? A3DREVERB_PRESET_PLAIN
			: val >= .8? A3DREVERB_PRESET_CONCERTHALL
			: val >= .6? A3DREVERB_PRESET_AUDITORIUM
			: val >= .4? A3DREVERB_PRESET_CAVE
			: val >= .2? A3DREVERB_PRESET_GENERIC
			: A3DREVERB_PRESET_ROOM;
		pre->fVolume = desc->reverb.volume;
		pre->fDecayTime = (desc->reverb.decay - .5) + .55f;
		pre->fDamping = desc->reverb.damping;
		IA3dReverb_SetAllProperties(a3dReverb, &rp);
	}
	if(desc->flags & DDLISTENERF_DISABLE_REVERB)
	{
		IA3d5_BindReverb(a3d, NULL);
		IA3dReverb_Release(a3dReverb);
		a3dReverb = NULL;
	}
}

int I3_IsSoundPlaying(int handle)
{
	if(!initOk || handle < 1 || handle > numBuffers) return false;
	if(SBUFPTR(handle)->source == NULL) return false;
	return I3_IsBufferPlaying(SBUFPTR(handle));
}