
//**************************************************************************
//**
//** I_SOUND.C
//**
//** Version:		1.0
//** Last Build:	-?-
//** Author:		jk
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include "h2def.h"
#include "i_sound.h"
#include "i_timer.h"
#include "i_mus.h"
#include "i_win32.h"
#include "console.h"
#include "i_mixer.h"

// MACROS ------------------------------------------------------------------

//#define PRINT_DEBUG_INFO

#define DEFAULT_ARCHIVEPATH     "o:\\sound\\archive\\"
#define PRIORITY_MAX_ADJUST 10
#define DIST_ADJUST (MAX_SND_DIST/PRIORITY_MAX_ADJUST)

#define UBYTE_TO_SHORT(x)	 ( (short) (((x) << 8) + 0x8000) )

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void MUS_SongPlayer();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

boolean S_StopSoundID(int sound_id, int priority);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern HWND		hWndMain;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			snd_Resample = 1;
int			snd_16bits = 0;

boolean		use_jtSound = true;

boolean i_CDMusic;
int i_CDCurrentTrack;
int i_CDMusicLength;
int oldTic;

int sounddebug;

// These are really cvars...
int			snd_SfxDevice = 1;
int			snd_MusicDevice = 1;
int			snd_SfxVolume = 255, snd_MusicVolume = 255, snd_CDVolume = 135;
int			originalCDVolume;

boolean		MusicPaused = false;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static byte	*resampleBuffer = NULL;
static int	rsbufSize = 0;

// CODE --------------------------------------------------------------------

/*
 *
 *                           SOUND HEADER & DATA
 *
 *
 */

int SND_PlaySound(void *data, int volume, int pan, int pitch)
{
	if(use_jtSound)
		return I_Play2DSound(data, volume, pan, pitch);
	else
		return I3_PlaySound2D(data, volume, pan, pitch);
}

void SND_UpdateSound(int handle, int volume, int pan, int pitch)
{
	if(use_jtSound)
		I_Update2DSound(handle, volume, pan, pitch);
	else
		I3_UpdateSound2D(handle, volume, pan, pitch);
}

int SND_Play3DSound(void *data, sound3d_t *desc)
{
	if(use_jtSound)
		return I_Play3DSound(data, desc);
	else
		return I3_PlaySound3D(data, desc);
}

void SND_Update3DSound(int handle, sound3d_t *desc)
{
	if(use_jtSound)
		I_Update3DSound(handle, desc);
	else
		I3_UpdateSound3D(handle, desc);
}

void SND_UpdateListener(listener3d_t *desc)
{
	if(use_jtSound)
		I_UpdateListener(desc);
	else
		I3_UpdateListener(desc);
}

void SND_StopSound(int handle)
{
	if(use_jtSound)
		I_StopSound(handle);
	else
		I3_StopSound(handle);
}

int SND_SoundIsPlaying(int handle)
{
	if(use_jtSound)
		return I_SoundIsPlaying(handle);
	else
		return I3_IsSoundPlaying(handle);
}

void I_PauseSong(void)
{
	MUS_PauseSong();
}

void I_ResumeSong(void)
{
	MUS_ResumeSong();
}

void I_SetMusicVolume(int volume)
{
	snd_MusicVolume = volume;
	if(snd_MusicVolume < 0) snd_MusicVolume = 0;
	if(snd_MusicVolume > 255) snd_MusicVolume = 255;
	MUS_SetMasterVolume(snd_MusicVolume);
}

void I_SetSfxVolume(int volume)
{
	snd_SfxVolume = volume;
	if(snd_SfxVolume < 0) snd_SfxVolume = 0;
	if(snd_SfxVolume > 255) snd_SfxVolume = 255;
	
	if(!use_jtSound) I3_SetGlobalVolume(1000*snd_SfxVolume/255);
}

/*
 *
 *                              SONG API
 *
 */


int I_QrySongPlaying(void)
{
	int rc = MUS_IsSongPlaying();
	return rc;
}

void I_StopSong(void)
{
	MUS_StopSong();
}

int I_PlaySong(void *data, int length, int looping)
{
	if(!MUS_RegisterSong(data, length)) return 0;
	MUS_PlaySong(looping);
	return 1;
}

/*
 *
 *   SOUND STARTUP STUFF
 *
 *
 */


// inits all sound stuff
void I_StartupSound (void)
{
	snd_SfxDevice = 1;
	if(snd_MusicDevice > 2) snd_MusicDevice = 1; // Old settings revert to midi.

	// check command-line parameters- overrides config file
	//
	if (M_CheckParm("-nosound")) snd_MusicDevice = snd_SfxDevice = 0;
	if (M_CheckParm("-nosfx")) snd_SfxDevice = 0;
	if (M_CheckParm("-nomusic")) snd_MusicDevice = 0;
	if (M_CheckParm("-sounddebug")) sounddebug = true;
	if (M_CheckParm("-a3d")) use_jtSound = false;

	if (debugmode)
		ST_Message("I_StartupSound: Hope you hear a pop.\n");

	I_StartupTimer();

	if(snd_SfxDevice)
	{
		boolean fail;
		if(use_jtSound)
		{
			ST_Message( "  Initializing DirectSound.\n");
			fail = !I2_Init();
		}
		else
		{
			ST_Message( "  Initializing A3D.\n");
			fail = !I3_Init();
		}
		if(fail) ST_Message("I_StartupSound: Error initializing sound.\n");
	}
	else
		ST_Message("I_StartupSound: Sound is disabled.\n");

	// Initialize the MUS player.
	if(MUS_Init())		// True when everything OK.
		ST_Message("I_StartupSound: Error initializing music.\n");

	// Initialize the CD player, if it's the music device.
	// Attempt to setup CD music.
	if(snd_MusicDevice == 2)	// 2's for CD.
	{
		ST_Message("    Attempting to initialize CD Music: ");
		i_CDMusic = (I_CDMusInit() != -1);
		ST_Message(i_CDMusic? "initialized.\n" : "failed.\n");
	}

	I_InitMixer();

	originalCDVolume = I_Mixer3i(MIX_CDAUDIO, MIX_GET, MIX_VOLUME);
	// Set the volumes stored in the settings.
	I_CDMusSetVolume(snd_CDVolume);
	I_SetMusicVolume(snd_MusicVolume);
	I_SetSfxVolume(snd_SfxVolume);

	// Make sure the settings are in effect.
	I_UpdateMusicDevice();
}

// shuts down all sound stuff

void I_ShutdownSound (void)
{
	MUS_UnregisterSong();
	if(i_CDMusic) I_CDMusStop();
	I_CDMusSetVolume(originalCDVolume);
	I_ShutdownMixer();
	MUS_Shutdown();
	if(use_jtSound)
		I2_Shutdown();
	else
		I3_Shutdown();
	I_ShutdownTimer();
}

void I_SetChannels(int channels)
{
//	jtSoundSetChannels(channels);
}

byte *I_Resample8bitSound(byte *data, int len, int freq, int resampleFactor, 
						  boolean make16bits, int *outlen)
{
	int i, newlen;
	int cur, next;
	
	if(resampleFactor < 1 || resampleFactor == 3 || resampleFactor > 4) 
		return NULL; // No can do, pal!
	newlen = len * resampleFactor * (make16bits? 2 : 1);
	if(outlen) *outlen = newlen;
	// Do we have enough memory in the buffer?
	if(rsbufSize < newlen)
	{
		if(resampleBuffer) Z_Free(resampleBuffer);
		resampleBuffer = (byte *)Z_Malloc(newlen, PU_STATIC, NULL);
		rsbufSize = newlen;
	}
	if(make16bits)
	{
		short *ptr = (short*) resampleBuffer;
		if(resampleFactor == 1)
		{
			for(i=0; i<len; i++) *ptr++ = UBYTE_TO_SHORT(*data++);
		}
		else if(resampleFactor == 2)
		{
			for(i=0; i<len-1; i++)
			{
				cur = UBYTE_TO_SHORT(*data++);
				next = cur + UBYTE_TO_SHORT(*data);
				*ptr++ = cur;
				*ptr++ = next >> 1;
			}
			// Fill in the last two, as well.
			*ptr = *(ptr+1) = UBYTE_TO_SHORT(*data);
		}
		else if(resampleFactor == 4)
		{
			for(i=0; i<len-1; i++)
			{
				cur = UBYTE_TO_SHORT(*data++);
				next = UBYTE_TO_SHORT(*data);
				*ptr++ = cur;
				*ptr++ = (3*cur + next) >> 2;
				*ptr++ = (cur + next) >> 1;	
				*ptr++ = (cur + 3*next) >> 2;
			}
			*ptr = *(ptr+1) = *(ptr+2) = *(ptr+3) = UBYTE_TO_SHORT(*data);
		}
	}
	else
	{
		byte *ptr = resampleBuffer;
		if(resampleFactor == 1)
		{
			memcpy(resampleBuffer, data, len);
		}
		else if(resampleFactor == 2)
		{
			for(i=0; i<len-1; i++)
			{
				cur = *data++;								
				next = cur + *data;
				*ptr++ = cur;
				*ptr++ = next >> 1;
			}
			// Fill in the last two, as well.
			*ptr = *(ptr+1) = *data;
		}
		else if(resampleFactor == 4)
		{
			for(i=0; i<len-1; i++)
			{
				cur = *data++;
				next = *data;
				*ptr++ = cur;
				*ptr++ = (3*cur + next) >> 2;
				*ptr++ = (cur + next) >> 1;	
				*ptr++ = (cur + 3*next) >> 2;
			}
			*ptr = *(ptr+1) = *(ptr+2) = *(ptr+3) = *data;
		}
	}
	return resampleBuffer;			
}


// REAL CD MUSIC CODE ------------------------------------------------------

//==========================================================================
//
// I_UpdateCDMusic
//
// Updates playing time for current track, and restarts the track, if
// needed
//
//==========================================================================

void I_UpdateCDMusic(void)
{
//	extern boolean MenuActive;

	if(MusicPaused || i_CDMusicLength < 0 /* || (paused && !MenuActive)*/)
	{ // Non-looping song/song paused
		return;
	}
	i_CDMusicLength -= gametic-oldTic;
	oldTic = gametic;
	if(i_CDMusicLength <= 0)
	{
		//S_StartSong(gamemap, true);

		// Restart the current track.
		I_CDMusPlay(i_CDCurrentTrack, true);
	}
}

static int OkInit = 0;
static int cd_FirstTrack;
static int cd_LastTrack;

//==========================================================================
//
// I_CDMusInit
//
// Initializes the CD audio system.  Must be called before using any
// other I_CDMus functions.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusInit(void)
{
	if(!OkInit)	// If not yet initialized...
	{
		MCIERROR res;
		char mciCmd[200], ret[200];
		strcpy(mciCmd, "open cdaudio alias mycd");
		if((res=mciSendString(mciCmd, NULL, 0, NULL)))
		{
			// A failure!
			mciGetErrorString(res, mciCmd, 199);
			CON_Printf("CD Audio Error:\n%s\n", mciCmd);
			return -1;
		}		
		// Set the time format.
		strcpy(mciCmd, "set mycd time format tmsf");
		if((res=mciSendString(mciCmd, NULL, 0, NULL)))
		{
			// A failure!
			mciGetErrorString(res, mciCmd, 199);
			CON_Printf("CD Audio Error:\n%s\n", mciCmd);
			return -1;
		}
		// Now we're initialized.
		OkInit = 1;	// Now we're initialized.
		//CON_Printf( "CD Audio initialized succesfully.\n");
		/*CON_Printf( "\nNumber of tracks: ");
		if((res=mciSendString("status mycd number of tracks", ret, 200, NULL)))
		{
			// A failure!
			mciGetErrorString(res, mciCmd, 79);
			CON_Printf("\nCD Audio Error:\n%s\n", mciCmd);
			return -1;
		}
		CON_Printf( "%s\n", ret);*/
		cd_FirstTrack = 1;
		if(mciSendString("status mycd number of tracks", ret, 200, NULL))
			cd_LastTrack = 0;
		else
			cd_LastTrack = atoi(ret);
	}
	return 0;
}

//==========================================================================
//
// I_CDMusPlay
//
// Play an audio CD track.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusPlay(int track, int loop)
{
	char str[200];
	//MCIERROR res;

	i_CDCurrentTrack = track;
	//I_CDMusStop();
	//CON_Printf( "Trying to play track %d.\n", track);
	sprintf(str, "play mycd from %d to %d", track, 
		MCI_MAKE_TMSF(track, 0, I_CDMusTrackLength(track), 0));
		//MCI_MAKE_TMSF(track+2,0,0,0));
	if(mciSendString(str, NULL, 0, NULL)) 
	{
		i_CDMusicLength = -1;
		return -1;
	}
	//mciGetErrorString(res, str, 200);
	//CON_Printf( "MCI: %s\n", str);
	if(loop)
	{
		i_CDMusicLength = 35*I_CDMusTrackLength(track);
	}
	else
	{
		i_CDMusicLength = -1;
	}
	oldTic = gametic;
	MusicPaused = false;
	return 0;
}

//==========================================================================
//
// I_CDMusStop
//
// Stops the playing of an audio CD.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusStop(void)
{
	mciSendString("pause mycd", NULL, 0, NULL);
	MusicPaused = true;
	return 0;
}

//==========================================================================
//
// I_CDMusResume
//
// Resumes the playing of an audio CD.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusResume(void)
{
	MCIERROR res = mciSendString("play mycd", NULL, 0, NULL);
	if(res)
	{
		char str[200];
		mciGetErrorString(res, str, 200);
		CON_Printf( "MCI: %s\n", str);
	}
	MusicPaused = false;
	oldTic = gametic; // Make up for the lost time.
	return 0;
}

//==========================================================================
//
// I_CDMusSetVolume
//
// Sets the CD audio volume (0 - 255).
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusSetVolume(int volume)
{
	if(volume < 0) volume = 0;
	if(volume > 255) volume = 255;
	snd_CDVolume = volume;
	return I_Mixer4i(MIX_CDAUDIO, MIX_SET, MIX_VOLUME, volume);
}

int I_CDMusGetVolume()
{
	return snd_CDVolume;
}

//==========================================================================
//
// I_CDMusFirstTrack
//
// Returns: the number of the first track.
//
//==========================================================================

int I_CDMusFirstTrack(void)
{
	return cd_FirstTrack;
}

//==========================================================================
//
// I_CDMusLastTrack
//
// Returns: the number of the last track.
//
//==========================================================================

int I_CDMusLastTrack(void)
{
	return cd_LastTrack;
}

//==========================================================================
//
// I_CDMusTrackLength
//
// Returns: Length of the given track in seconds, or -1 (error, in
// cd_Error).
//
//==========================================================================

int I_CDMusTrackLength(int track)
{
	char str[80], ret[200];
	int min=0, sec=0;

	sprintf(str, "status mycd length track %d", track);
	if(mciSendString(str, ret, 200, NULL)) return 0;	// No success.
	sscanf(ret, "%d:%d", &min, &sec);	
	return min*60 + sec;
}

int I_CDMusGetStatus()
{
	char str[80], ret[200];
	
	strcpy(str, "status mycd mode");
	if(mciSendString(str, ret, 200, NULL)) return DD_NOT_READY; // No success.
	
	if(!stricmp(ret, "playing")) return DD_PLAYING;
	if(!stricmp(ret, "paused")) return DD_PAUSED;
	if(!stricmp(ret, "stopped")) return DD_STOPPED;

	return DD_NOT_READY;
}

int I_CDControl(int cmd, int parm)
{
	switch(cmd)
	{
	case DD_INIT:
		return I_CDMusInit();
	case DD_AVAILABLE:
		return OkInit;
	case DD_PLAY:
		return I_CDMusPlay(parm, false);
	case DD_PLAY_LOOP:
		return I_CDMusPlay(parm, true);
	case DD_STOP:
		return I_CDMusStop();
	case DD_RESUME:
		return I_CDMusResume();
	case DD_GET_VOLUME:
		return I_CDMusGetVolume();
	case DD_SET_VOLUME:
		return I_CDMusSetVolume(parm);
	case DD_GET_FIRST_TRACK:
		return I_CDMusFirstTrack();
	case DD_GET_LAST_TRACK:
		return I_CDMusLastTrack();
	case DD_GET_TRACK_LENGTH:
		return I_CDMusTrackLength(parm);
	case DD_GET_CURRENT_TRACK:
		return i_CDCurrentTrack;
	case DD_GET_TIME_LEFT:
		return i_CDMusicLength/35;
	case DD_STATUS:
		return I_CDMusGetStatus();
	default:
		I_Error("I_CDControl: Bad command (%d).\n", cmd);
	}
	return 0;
}

void I_UpdateMusicDevice()
{
	if(snd_MusicDevice == 0)
	{
		// Stop everything.
		MUS_Disable(true);
		I_CDMusStop();
		i_CDMusic = false;
	}
	else if(snd_MusicDevice == 1) // MIDI
	{
		MUS_Disable(false);
		I_CDMusStop();
		i_CDMusic = false;
	}
	else if(snd_MusicDevice == 2) // CD
	{
		// Disable MUS.
		MUS_Disable(true);
		// Make sure it's initialized.
		i_CDMusic = !I_CDMusInit();
	}
}

void I_SetMusicDevice(int musdev)
{
	snd_MusicDevice = musdev;
	I_UpdateMusicDevice();
}

