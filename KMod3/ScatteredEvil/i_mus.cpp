
//**************************************************************************
//**
//** I_MUS.C
//**
//** Version:		1.0
//** Last Build:	-?-
//** Author:		jk
//**
//** Playing MUS songs using Windows MIDI output.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <math.h>
#include "st_start.h"
#include "i_mus.h"
#include "i_timer.h"

// MACROS ------------------------------------------------------------------

#define MAX_BUFFER_LEN		65535
#define MAX_BUFFERS			8
#define BUFFER_ALLOC		4096	// Allocate in 4K chunks.

// TYPES -------------------------------------------------------------------

typedef unsigned char		byte;
typedef	unsigned short int	UWORD;

typedef struct {
	char    ID[4];          // identifier "MUS" 0x1A
    UWORD	scoreLen;
    UWORD	scoreStart;
	UWORD	channels;		// number of primary channels
	UWORD	sec_channels;	// number of secondary channels
	UWORD	instrCnt;
	UWORD	dummy;
	// The instrument list begins here.
} MUSHeader_t;

typedef struct {
	unsigned char	channel : 4;
	unsigned char	event : 3;
	unsigned char	last : 1;
} MUSEventDesc_t;

enum // MUS event types.
{
	MUS_EV_RELEASE_NOTE,
	MUS_EV_PLAY_NOTE,
	MUS_EV_PITCH_WHEEL,
	MUS_EV_SYSTEM,			// Valueless controller.
	MUS_EV_CONTROLLER,		
	MUS_EV_FIVE,			// ?
	MUS_EV_SCORE_END,
	MUS_EV_SEVEN			// ?
};

enum // MUS controllers.
{
	MUS_CTRL_INSTRUMENT,
	MUS_CTRL_BANK,
	MUS_CTRL_MODULATION,
	MUS_CTRL_VOLUME,
	MUS_CTRL_PAN,
	MUS_CTRL_EXPRESSION,
	MUS_CTRL_REVERB,
	MUS_CTRL_CHORUS,
	MUS_CTRL_SUSTAIN_PEDAL,
	MUS_CTRL_SOFT_PEDAL,
	// The valueless controllers.
	MUS_CTRL_SOUNDS_OFF,
	MUS_CTRL_NOTES_OFF,
	MUS_CTRL_MONO,
	MUS_CTRL_POLY,
	MUS_CTRL_RESET_ALL,
	NUM_MUS_CTRLS
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void I_Error (char *error, ...);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static unsigned char *MUS_SongStartAddress();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static MIDIHDR midiBuffers[MAX_BUFFERS];
static MIDIHDR *loopBuffer;
static int registered;
static byte *readPos;
static int readTime;			// In ticks.

static int midiAvail = 0, disabled = 0;
static UINT devId;
static HMIDISTRM midiStr;
static int origVol;				// The original MIDI volume.
//static MUSHeader_t *song = 0;	// The song;
//static unsigned char *playPos;	// The current play position.
static int playing = 0;//, looping = 0;	// The song is playing/looping.
//static int waitTicks = 0;		
static unsigned char chanVols[16];	// Last volume for each channel.

static char ctrl_mus2midi[NUM_MUS_CTRLS] =
{
	0,		// Not used.
	0,		// Bank select.
	1,		// Modulation.
	7,		// Volume.
	10,		// Pan.
	11,		// Expression.
	91,		// Reverb.
	93,		// Chorus.
	64,		// Sustain pedal.
	67,		// Soft pedal.
	// The valueless controllers.
	120,	// All sounds off.
	123,	// All notes off.
	126,	// Mono.
	127,	// Poly.
	121		// Reset all controllers.
};

// CODE --------------------------------------------------------------------

/*static void MUS_CheckResult(MMRESULT mmres,char *inWhichFunc)
{
	if(mmres == MMSYSERR_NOERROR) return;	// No error.

	I_Error("Bad MIDI result: %s returns %d (0x%x).\n",inWhichFunc,mmres,mmres);
}*/

//==========================================================================
//
// MUS_PlayerThread
//
// This is a thread function that calls the song player as often as 
// is necessary.
//
//==========================================================================
/*
void MUS_PlayerThread(void *dummy)
{
	int first = TRUE;
	double lastTime, nowTime;
	int i, tics;

	while(midiAvail)
	{
		// Wait for a tic to play.
		for(; midiAvail;) 
		{
			nowTime = I_GetSeconds();
			if(first)
			{
				first = FALSE;
				lastTime = nowTime;
			}
			// How many tics to play?
			tics = (int) ((nowTime - lastTime) * 140);
			if(!tics)
			{
				Sleep(2);
				continue;
			}
			lastTime = nowTime;
			break;
		}
		if(!midiAvail) break;

		// We need to have access to the song data before we can 
		// play anything.
		MUS_WaitForAccess();
		// Play them.
		for(i=0; i<tics; i++) MUS_SongPlayer();			
		// Let the other routines change the song data.
		MUS_ReleaseAccess();
	}
}
*/
//==========================================================================
//
// MUS_SongPlayer
//
// Called at 140 Hz, interprets the MUS data to MIDI events.
//
//==========================================================================

//void MUS_SongPlayer()

static void MUS_InitSongReader(MUSHeader_t *song)
{
	readPos = (byte*) song + song->scoreStart;
	readTime = 0;
}

// Returns false when the score ends.
int MUS_GetNextEvent(MIDIEVENT *mev)
{
	MUSEventDesc_t	*evDesc;
	unsigned char	midiStatus, midiChan, midiParm1, midiParm2;
	int				scoreEnd = 0, i;

	// Get immediately out of here if we have nothing to do.
	//if(disabled || !midiAvail || !song || !playing) return;	

	// This is the wait counter.
	//if(--waitTicks > 0) return;
	
	mev->dwDeltaTime = readTime;
	readTime = 0;

	// We assume playPos is OK. We'll keep playing events until the 
	// 'last' bit is set, and then the waitTicks is set.
//	for(;;)	// Infinite loop in an interrupt handler. Good call.
//	{

	evDesc = (MUSEventDesc_t*) readPos++;
	midiStatus = midiChan = midiParm1 = midiParm2 = 0;
	
	// Construct the MIDI event.
	switch(evDesc->event)
	{
	case MUS_EV_RELEASE_NOTE:
		midiStatus = 0x80;
		// Which note?
		midiParm1 = *readPos++;
		break;
		
	case MUS_EV_PLAY_NOTE:
		midiStatus = 0x90;
		// Which note?
		midiParm1 = *readPos++;
		// Is the volume there, too?
		if(midiParm1 & 0x80)	
			chanVols[evDesc->channel] = *readPos++;
		midiParm1 &= 0x7f;
		midiParm2 = chanVols[evDesc->channel];
		//ST_Message( "time: %i note: p1:%i p2:%i\n", mev->dwDeltaTime, midiParm1, midiParm2);
		break;				
		
	case MUS_EV_CONTROLLER:
		midiStatus = 0xb0;
		midiParm1 = *readPos++;
		midiParm2 = *readPos++;
		// The instrument control is mapped to another kind of MIDI event.
		if(midiParm1 == MUS_CTRL_INSTRUMENT)
		{
			midiStatus = 0xc0;
			midiParm1 = midiParm2;
			midiParm2 = 0;
		}
		else
		{
			// Use the conversion table.
			midiParm1 = ctrl_mus2midi[midiParm1];
		}
		break;
	
	// 2 bytes, 14 bit value. 0x2000 is the center. 
	// First seven bits go to parm1, the rest to parm2.
	case MUS_EV_PITCH_WHEEL:
		midiStatus = 0xe0;
		i = *readPos++ << 6;
		midiParm1 = i & 0x7f;
		midiParm2 = i >> 7;
		//ST_Message( "pitch wheel: ch %d (%x %x = %x)\n", evDesc->channel, midiParm1, midiParm2, midiParm1 | (midiParm2 << 7));
		break;
		
	case MUS_EV_SYSTEM:
		midiStatus = 0xb0;
		midiParm1 = ctrl_mus2midi[*readPos++];
		break;
		
	case MUS_EV_SCORE_END:
		// We're done.
		return FALSE;
		//scoreEnd = 1;
		
	default:
		I_Error("MUS_SongPlayer: Unhandled MUS event %d.\n",evDesc->event);
	}
	/*if(scoreEnd) 
	{
		// No more events.
		return FALSE;
	}*/
	// Choose the channel.
	midiChan = evDesc->channel;
	// Redirect MUS channel 16 to MIDI channel 10 (percussion).
	if(midiChan == 15) midiChan = 9; else if(midiChan == 9) midiChan = 15;
	//ST_Message("MIDI event/%d: %x %d %d\n",evDesc->channel,midiStatus,midiParm1,midiParm2);

	// Send out the MIDI event.
	//midiOutShortMsg(midiOut, midiChan | midiStatus | (midiParm1<<8) | (midiParm2<<16));
	mev->dwEvent = (MEVT_SHORTMSG << 24) | midiChan | midiStatus
		| (midiParm1 << 8) | (midiParm2 << 16);

	// Check if this was the last event in a group.
	if(!evDesc->last) return TRUE;

	//waitTicks = 0;
	// Check for end of score.
/*	if(scoreEnd)
	{
		playPos = MUS_SongStartAddress();
		if(!looping) playing = 0;
		// Reset the MIDI output so no notes are left playing when 
		// the song ends.
		midiOutReset(midiOut);
	}
	else
	{*/
	// Read the number of ticks to wait.
	
	// Read the time delta.
	for(readTime = 0;;)
	{
		midiParm1 = *readPos++;
		readTime = readTime*128 + (midiParm1 & 0x7f);
		if(!(midiParm1 & 0x80)) break;
	}
	return TRUE;
}

static MIDIHDR *MUS_GetFreeBuffer()
{
	int		i;

	for(i=0; i<MAX_BUFFERS; i++)
		if(midiBuffers[i].dwUser == FALSE)
		{
			MIDIHDR *mh = midiBuffers + i;
			
			// Mark the header used.
			mh->dwUser = TRUE;
			
			// Allocate some memory for buffer.
			mh->dwBufferLength = BUFFER_ALLOC;
			mh->lpData = (char *)malloc(mh->dwBufferLength);
			// -JL- Paranoia
			if (!mh->lpData)
				I_Error("MUS_GetFreeBuffer: malloc failed");
			mh->dwBytesRecorded = 0;

			mh->dwFlags = 0;
			return mh;
		}
		return NULL;
}

// Returns false if the allocation can't be done.
static int MUS_AllocMoreBuffer(MIDIHDR *mh)
{
	// Don't allocate too large buffers.
	if(mh->dwBufferLength + BUFFER_ALLOC > MAX_BUFFER_LEN)
		return FALSE;

	mh->dwBufferLength += BUFFER_ALLOC;
	mh->lpData = (char *)realloc(mh->lpData, mh->dwBufferLength);
	// -JL- Paranoia
	if (!mh->lpData)
		I_Error("MUS_AllocMoreBuffer: realloc failed");

	// Allocation was successful.
	return TRUE;
}

// The buffer is ready, prepare it and stream out.
static int MUS_StreamOut(MIDIHDR *mh)
{
//	midiOutPrepareHeader( (HMIDIOUT) midiStr, mh, sizeof(*mh));
	midiStreamOut(midiStr, mh, sizeof(*mh));
}

void CALLBACK MUS_Callback(HMIDIOUT hmo, UINT wMsg, DWORD dwInstance, 
						   DWORD dwParam1, DWORD dwParam2)
{
	MIDIHDR *mh;

	if(wMsg != MOM_DONE) return;

	if(!playing) return;

	mh = (MIDIHDR*) dwParam1;
	// This buffer has stopped. Is this the last buffer?
	// If so, check for looping.
	if(mh == loopBuffer)
	{
		// Play all buffers again.
		MUS_PlaySong(TRUE);
	}
}

//==========================================================================
//
// MUS_PrepareBuffers
//
// No song can be registered when this is called.
//
//==========================================================================

void MUS_PrepareBuffers(MUSHeader_t *song)//, int length)
{
	MIDIHDR *mh = MUS_GetFreeBuffer();
	MIDIEVENT mev;
	DWORD *ptr;

	// First add the tempo.
	ptr = (DWORD*) mh->lpData;
	*ptr++ = 0;
	*ptr++ = 0;
	*ptr++ = (MEVT_TEMPO << 24) | 1000000; // One second.
	mh->dwBytesRecorded = 3*sizeof(DWORD);
		
	// Start reading the events.
	MUS_InitSongReader(song);
	while(MUS_GetNextEvent(&mev))
	{
		// Is the buffer getting full?
		if(mh->dwBufferLength - mh->dwBytesRecorded < 3*sizeof(DWORD))
		{
			// Try to get more buffer.
			if(!MUS_AllocMoreBuffer(mh))
			{
				// Not possible, buffer size has reached the limit.
				// We need to start  working on another one.
				//MUS_StreamOut(mh);
				midiOutPrepareHeader( (HMIDIOUT) midiStr, mh, sizeof(*mh));
				mh = MUS_GetFreeBuffer();
				if(!mh) return;	// Oops.
			}
		}
		// Add the event.
		ptr = (DWORD*) (mh->lpData + mh->dwBytesRecorded);
		*ptr++ = mev.dwDeltaTime;
		*ptr++ = 0;
		*ptr++ = mev.dwEvent;
		mh->dwBytesRecorded += 3*sizeof(DWORD);
	}
	// Prepare the last buffer, too.
	midiOutPrepareHeader( (HMIDIOUT) midiStr, mh, sizeof(*mh));
}

//==========================================================================
//
// MUS_PrepareBuffers
//
//==========================================================================

void MUS_ReleaseBuffers()
{
	int	i;

	for(i=0; i<MAX_BUFFERS; i++)
		if(midiBuffers[i].dwUser)
		{
			MIDIHDR *mh = midiBuffers + i;

			midiOutUnprepareHeader( (HMIDIOUT) midiStr, mh, sizeof(*mh));
			free(mh->lpData);
			
			// Clear for re-use.
			memset(mh, 0, sizeof(*mh));
		}
}

//==========================================================================
//
// MUS_RegisterSong
//
//==========================================================================

int MUS_RegisterSong(void *data, int length)
{
	if(!midiAvail) return 0;

	// First unregister the previous song.
	MUS_UnregisterSong();	

	MUS_PrepareBuffers((MUSHeader_t *)data);

	// Now there is a registered song.
	registered = TRUE;

	//song = malloc(length);
	//memcpy(song, data, length);
	//playPos = MUS_SongStartAddress();	// Go to the beginning of the song.
	//playing = 0;	// We aren't playing this song.
	return 1;
}

//==========================================================================
//
// MUS_UnregisterSong
//
//==========================================================================

void MUS_UnregisterSong(void)
{
	if(!midiAvail || !registered) return;
/*	MUS_CheckResult(midiOutUnprepareHeader((HMIDIOUT)midiStrm,&midiHdr,sizeof(MIDIHDR)),
		"midiOutUnprepareHeader");
	// Reset the data.
	midiHdr.dwBytesRecorded = 0;*/

	// First stop the song.
	MUS_StopSong();

	registered = FALSE;

	// This is the actual unregistration.
	/*free(song);
	song = 0;*/
	MUS_ReleaseBuffers();
}

//==========================================================================
//
// MUS_IsSongPlaying
//
//==========================================================================

int MUS_IsSongPlaying()
{
	return playing;
}

//==========================================================================
//
// MUS_Reset
//
//==========================================================================

void MUS_Reset()
{
	int		i;

	midiStreamStop(midiStr);
	// Reset channel settings.
	for(i=0; i<=0xf; i++) // All channels.
		midiOutShortMsg( (HMIDIOUT) midiStr, 0xe0 | i | 64<<16); // Pitch bend.
	midiOutReset( (HMIDIOUT) midiStr);	
}

//==========================================================================
//
// MUS_StopSong
//
//==========================================================================

void MUS_StopSong()
{
//	MUS_CheckResult(midiStreamStop(midiStrm),"strmStop");

	if(!midiAvail || !playing) return;

	playing = 0;
	loopBuffer = NULL;
	//playPos = MUS_SongStartAddress();
	//MUS_Reset();

	MUS_Reset();
}

//==========================================================================
//
// MUS_PlaySong
//
//==========================================================================

void MUS_PlaySong(int loopy)
{
/*	ST_Message("MUS_PlaySong: Starting song (looping %d). %d bytes.\n",looping,midiHdr.dwBytesRecorded);
	MUS_CheckResult(midiStreamOut(midiStrm,&midiHdr,sizeof(midiHdr)),"strmOut");
	MUS_CheckResult(midiStreamRestart(midiStrm),"strmRestart");*/

	int		i;

	playing = 1;
	//playPos = MUS_SongStartAddress();
	//looping = loopy;
	
	MUS_Reset();

	// Stream out all buffers.
	for(i=0; i<MAX_BUFFERS; i++)
		if(midiBuffers[i].dwUser)
		{
			loopBuffer = &midiBuffers[i];
			midiStreamOut(midiStr, &midiBuffers[i], sizeof(midiBuffers[i]));
		}

	// If we aren't looping, don't bother.
	if(!loopy) loopBuffer = NULL;

	// Start playing.
	midiStreamRestart(midiStr);
}

//==========================================================================
//
// MUS_SongStartAddress
//
//==========================================================================

/*static unsigned char *MUS_SongStartAddress()
{
	if(!song) return 0;
	return (unsigned char*)song + song->scoreStart;
}*/

//==========================================================================
//
// MUS_PauseSong
//
//==========================================================================

void MUS_PauseSong()
{
	playing = 0;
	// Stop all the notes.
	//midiOutReset(midiOut);
	midiStreamPause(midiStr);
}

//==========================================================================
//
// MUS_ResumeSong
//
//==========================================================================

void MUS_ResumeSong()
{
	playing = 1;
	midiStreamRestart(midiStr);
}

//==========================================================================
//
// MUS_SetMasterVolume
//
// Vol is from 0 to 255.
//
//==========================================================================

void MUS_SetMasterVolume(int vol)
{
	// Clamp it to a byte.
	if(vol < 0) vol = 0;
	if(vol > 255) vol = 255;
	// Straighen the volume curve.
	vol <<= 8;	// Make it a word.
	vol = (int) ((255.9980469 * sqrt((float)vol)));
	midiOutSetVolume( (HMIDIOUT) midiStr, vol+(vol<<16));	// Expand to a dword.
}

int MUS_OpenStream()
{
	MMRESULT mmres;
	MIDIPROPTIMEDIV tdiv;

	devId = MIDI_MAPPER;
	if((mmres = midiStreamOpen(&midiStr, &devId, 1, (DWORD) MUS_Callback, 0,
		CALLBACK_FUNCTION)) != MMSYSERR_NOERROR)
	{
		ST_Message("MUS_OpenStream: midiStreamOpen error %d.\n",mmres);
		return FALSE;
	}
	// Set stream time format, 140 ticks per quarter note.
	tdiv.cbStruct = sizeof(tdiv);
	tdiv.dwTimeDiv = 140;
	if((mmres = midiStreamProperty(midiStr, (BYTE*) &tdiv, MIDIPROP_SET | MIDIPROP_TIMEDIV))
		!= MMSYSERR_NOERROR)
	{
		ST_Message( "MUS_OpenStream: time format! %d\n", mmres);
		return FALSE;
	}
	return TRUE;
}

void MUS_CloseStream()
{
	MUS_Reset();
	midiStreamClose(midiStr);
}

//==========================================================================
//
// MUS_Init
//
// Returns 0 if no problems.
//
//==========================================================================

int MUS_Init()
{
	int	i;

	if(midiAvail) return 0;	// Already initialized.

	//ST_Message("MUS_Init: %d MIDI out devices present.\n",midiOutGetNumDevs());
	// Open the midi stream.
	if(!MUS_OpenStream()) return -1;
	// Now the MIDI is available.
	ST_Message("MUS_Init: MIDI available and OK.\n");
	// Get the original MIDI volume (restored in shutdown).
	midiOutGetVolume( (HMIDIOUT) midiStr, (unsigned long *)&origVol);
	midiAvail = 1;
	disabled = 0;
	//song = 0;
	//playPos = 0;
	playing = 0;
	registered = FALSE;
	// Clear the MIDI buffers.
	memset(midiBuffers, 0, sizeof(midiBuffers));
	for(i=0; i<16; i++) chanVols[i] = 64;
	return 0;
}

//==========================================================================
//
// MUS_Shutdown
//
//==========================================================================

void MUS_Shutdown()
{
	if(!midiAvail) return;
	midiAvail = 0;

	playing = 0;
	
	MUS_UnregisterSong();

	// Restore the original volume.
	midiOutSetVolume( (HMIDIOUT) midiStr, origVol);

	//song = 0;
	//playPos = 0;
	MUS_CloseStream();
}

void MUS_Disable(int really)
{
	// Make sure no notes are left playing.
	// Only if disabled is true.
	if((disabled=really)) 
		MUS_PauseSong();
	else
		MUS_ResumeSong();
}