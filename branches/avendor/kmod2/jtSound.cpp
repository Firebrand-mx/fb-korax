// Jake Tools: Sound effects using DirectSound
// Aagh, sounddebug is making this a big messy...

#define DIRECTSOUND_VERSION 0x0300

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <math.h>
#include <stdio.h>
#include "jtSound.h"

extern "C" int sounddebug;

// Private data.
static int initOk = 0;
static HRESULT hres;
// The DirectSound objects.
static LPDIRECTSOUND dSound;

// A sound channel.
typedef struct
{
	int		sampleFreq;			// The original frequency of the sample.
	LPDIRECTSOUNDBUFFER dsBuf;	// Pointer to the sound buffer.
} jtsoundch_t;

// The channels.
static int numChan = 0;
static jtsoundch_t *chan = 0;

// Function prototypes.
void jtSoundKillChannels();
int jtSoundChanInit(int chnum, void *data, int length, int bits, int channels, int freq);
void jtSoundChanStart(int chnum);

static inline jtsoundch_t *getChan(int handle) 
{ 
	if(sounddebug) printf( "getChan(%d out of %d)\n", handle, numChan);	
	if(!chan || !initOk || handle <= 0) return NULL;
	return chan + handle-1; 
}


int jtSoundInit(HWND hWnd)
{
	if(initOk) return 0;	// We're not going to initialize again!

	if(sounddebug) printf( "sound init\n");

	// First create a DirectSound object.
	if(FAILED(hres=DirectSoundCreate(NULL, &dSound, NULL)))
	{
		if(sounddebug) printf( "DirectSoundCreate failed\n");
		return -1;
	}

	// I'm the only one who gets to use it, haha.
	if(FAILED(hres=dSound->SetCooperativeLevel(hWnd, DSSCL_EXCLUSIVE)))
	{
		if(sounddebug) printf( "SetCooperativeLevel failed\n");
		return -1;
	}
	// The initialization was a success.
	initOk = 1;

	// By default, there are eight channels.
	jtSoundSetChannels(16);
	return 0;
}

#define SAFE_RELEASE(x) if(x) {(x)->Release(); (x)=0;}

void jtSoundShutdown()
{
	if(!initOk) return;

	if(sounddebug) printf( "sound shutdown\n");

	jtSoundKillChannels();
	SAFE_RELEASE(dSound);
	initOk = 0;
}

void jtSoundKillChannels()
{
	if(!chan || !initOk) return;

	if(sounddebug) printf( "sound kill channels\n");

	for(int i=0; i<numChan; i++) jtSoundStop(i);

	free(chan);
	chan = 0;
	numChan = 0;
}

// Set the number of channels.
void jtSoundSetChannels(int numChannels)
{
	if(numChannels <= 0 || !initOk) return;

	if(sounddebug) printf( "sound set channels\n");

	// Get rid of the old channels.
	jtSoundKillChannels(); 

	numChan = numChannels;
	chan = (jtsoundch_t*) malloc(sizeof(jtsoundch_t) * numChan);
	// -JL- Paranoia
	if (!chan)
		I_Error("jtSoundSetChannels: malloc failed");
	memset(chan, 0, sizeof(jtsoundch_t) * numChan);
}

// Returns the channel playing the sample (the handle).
int jtSoundPlay(void *sampleData, int length, int bits, int stereo, int freq, 
				int volume, int pan, int pitch)
{
	// First find a free channel. If there are none, use the first 
	// channel.
	int		i;

	if(!initOk) return 0;	// Can't play sounds!

	if(sounddebug) printf( "sound play:\n");

	for(i=0; i<numChan; i++) 
	{
		// Get rid of sounds that have stopped.
		if(!jtSoundIsPlaying(i+1)) 
		{
			if(sounddebug) printf( "- destroying stopped sound\n");
			jtSoundStop(i+1);
		}
		if(!chan[i].dsBuf) break;
	}
	if(i==numChan)
	{
		if(sounddebug) printf( "- no free channels\n");
		// No free channels were found.
		jtSoundStop(i=1);
	}
	else i++;

	if(sounddebug) printf( "- init channel\n");
	// Now we have a free channel at i. Initialize it.
	jtSoundChanInit(i, sampleData, length, bits, stereo? 2 : 1, freq);
	// Set the channel properties.
	jtSoundSetVolume(i, volume);
	jtSoundSetPan(i, pan);
	jtSoundSetFreq(i, pitch);
	// Start playing the channel.
	jtSoundChanStart(i);
	return i;
}

// Here the DirectSound buffer gets created.
int jtSoundChanInit(int chnum, void *data, int length, int bits, int channels, int freq)
{
	HRESULT hr;
	jtsoundch_t *ch = getChan(chnum);
	WAVEFORMATEX form;
	DSBUFFERDESC desc;
	void *writePtr, *writePtr2=NULL;
	DWORD writeBytes, writeBytes2, dataBytes = length * bits/8 * channels;

	if(!initOk) return FALSE;

	if(sounddebug) printf( "sound chan init\n");

	ch->sampleFreq = freq;

	// The format.
	memset(&form, 0, sizeof(form));
	form.wFormatTag = WAVE_FORMAT_PCM;
	form.nChannels = channels;
	form.nSamplesPerSec = freq;
	form.nBlockAlign = channels * bits/8;
	form.nAvgBytesPerSec = form.nSamplesPerSec * form.nBlockAlign;
	form.wBitsPerSample = bits;

	// The buffer description.
	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_CTRLDEFAULT | DSBCAPS_STATIC;
	desc.dwBufferBytes = dataBytes;
	desc.lpwfxFormat = &form;

	if(sounddebug) printf( "attempting to create a buffer\n");	

	// Let's create the buffer.
	if(FAILED(hres=dSound->CreateSoundBuffer(&desc, &ch->dsBuf, NULL)))
	{
		if(sounddebug) printf( "creation of buffer failed (code: %x)\n", hres);	
		ch->dsBuf = NULL;	// Let's take no chances.
		return FALSE;
	}

	if(sounddebug) printf( "attempting to lock buffer\n");	

	// Copy the data to the buffer.
	hr = ch->dsBuf->Lock(0, dataBytes, &writePtr, &writeBytes, &writePtr2, &writeBytes2, 0);
	if(hr == DSERR_BUFFERLOST)
	{
		if(sounddebug) printf( "buffer lost, restoring\n");	
		ch->dsBuf->Restore();
		// Try to lock again.
		if(sounddebug) printf( "attempting a new lock\n");	
		hr = ch->dsBuf->Lock(0, dataBytes, &writePtr, &writeBytes, &writePtr2, &writeBytes2, 0);
	}
	if(hr == DS_OK)
	{
		if(sounddebug) printf( "writing sample to buffer\n   (ptr1:%p, bytes1:%d, ptr2:%p, bytes2:%d)\n",
			writePtr, writeBytes, writePtr2, writeBytes2);

		memcpy(writePtr, data, writeBytes);
		if(writePtr2) memcpy(writePtr2, (char*)data + writeBytes, writeBytes2);

		// The Houdini trick!
		if(sounddebug) printf( "unlocking buffer\n");			
		hres = ch->dsBuf->Unlock(writePtr, dataBytes, writePtr2, writeBytes2);	
		if(FAILED(hres))
		{
			printf( "Unlock buffer: ");
			if(hres == DSERR_INVALIDPARAM)
				printf( "INVALID PARAM\n");
			if(hres == DSERR_INVALIDCALL)
				printf( "INVALID CALL\n");
			if(hres == DSERR_PRIOLEVELNEEDED)
				printf( "PRIOLEVELNEEDED\n");
		}
		if(sounddebug) printf( "- result: %x (failed: %d)\n", hres, FAILED(hres));	
	}
	else
		if(sounddebug) printf( "buffer lock failed (code: %x)\n", hr);	
	
	// A success!
	return TRUE;
}

void jtSoundChanStart(int chnum)
{
	jtsoundch_t *ch = getChan(chnum);

	if(!ch || !ch->dsBuf || !initOk) return;

	if(sounddebug) printf( "sound chan start\n");	

	hres = ch->dsBuf->Play(0, 0, 0);

	if(sounddebug) printf( "buffer play result: %x\n", hres);	
}

void jtSoundStop(int chnum)
{
	jtsoundch_t *ch = getChan(chnum);

	if(!ch || !ch->dsBuf || !initOk) return;

	if(sounddebug) printf( "sound stop\n");	
	
	ch->dsBuf->Stop();
	
	if(sounddebug) printf( "- stopped, releasing\n");	
	
	ch->dsBuf->Release();

	if(sounddebug) printf( "- ok\n");	
	
	ch->dsBuf = NULL;
}

int jtSoundIsPlaying(int chnum)
{
	jtsoundch_t *ch = getChan(chnum);
	DWORD status;

	if(!ch || !ch->dsBuf || !initOk) return 0;

	if(sounddebug) printf( "sound is playing? (ch %d)\n", chnum);	

	// Get the status of the buffer.
	hres = ch->dsBuf->GetStatus(&status);

	if(sounddebug) printf( "- buffer status received, result: %x\n", hres);	

	if(status & DSBSTATUS_PLAYING) return 1;
	return 0;
}

// Volume is in range from 0 to 255.
void jtSoundSetVolume(int chnum, int volume)
{
	jtsoundch_t *ch = getChan(chnum);
	int ds_vol;
	float fvol = (float) volume;

	if(!ch || !ch->dsBuf || !initOk) return;

	if(sounddebug) printf( "sound set volume (ch %d, vol %d)\n", chnum, volume);	

	if(volume > 0)
	{
		int friendlyvol = volume;
		if(friendlyvol < 1) friendlyvol = 1;
		if(friendlyvol > 255) friendlyvol = 255;
		fvol = float(106.0 * log10(friendlyvol));
	}
	else fvol = 0;
	ds_vol = DSBVOLUME_MIN + int(fvol*(DSBVOLUME_MAX-DSBVOLUME_MIN)/255.0);
	//if(sounddebug) printf( "- ds vol: %d\n", ds_vol);
	hres = ch->dsBuf->SetVolume(ds_vol);	
	if(sounddebug) printf( "- result: %x\n", hres);	
}

// Pan is in range from 0 to 1000, 500 being in the middle.
void jtSoundSetPan(int chnum, int pan)
{
	// The logarithm should be used here, too, to counter 
	// the effects of dB attenuation.
	jtsoundch_t *ch = getChan(chnum);
	int ds_pan = DSBPAN_LEFT + pan*(DSBPAN_RIGHT-DSBPAN_LEFT)/1000;

	if(!ch || !ch->dsBuf || !initOk) return;

	if(sounddebug) printf( "sound set pan (ch %d, pan %d)\n", chnum, pan);	

	hres = ch->dsBuf->SetPan(ds_pan);

	if(sounddebug) printf( "- result: %x\n", hres);	
}

// Relative frequency, 1000 corresponds the original one.
void jtSoundSetFreq(int chnum, int pitch)
{
	jtsoundch_t *ch = getChan(chnum);

	if(!ch || !ch->dsBuf || !initOk) return;

	if(sounddebug) printf( "sound set freq (ch %d, pitch %d)\n", chnum, pitch);	

	if(pitch == 1000) 
		hres = ch->dsBuf->SetFrequency(DSBFREQUENCY_ORIGINAL);
	else
		hres = ch->dsBuf->SetFrequency( (ch->sampleFreq*pitch) / 1000 );

	if(sounddebug) printf( "- result: %x\n", hres);	
}
