
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "h2def.h"
#include "r_local.h"
#include "p_local.h"    // for P_AproxDistance
#include "sounds.h"
#include "settings.h"

// MACROS ------------------------------------------------------------------

//#define PRINT_DEBUG_INFO

#define DEFAULT_ARCHIVEPATH     "o:\\sound\\archive\\"
#define PRIORITY_MAX_ADJUST 10
#define DIST_ADJUST (MAX_SND_DIST/PRIORITY_MAX_ADJUST)

enum { VX, VY, VZ };

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

//void MUS_SongPlayer();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void S_StopSoundID(int sound_id);//, int priority);
//void I_UpdateCDMusic(void);

//int I_CDMusPlay(int track, int loop);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

//extern void **lumpcache;
//extern HWND hWndMain;

extern	boolean		MusicPaused;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

#define i_CDMusic		( snd_MusicDevice == 2 )

int			s_CDTrack = 0;
boolean		reverbDebug = false;

//int snd_Channels=8;
int			snd_3D = 0;
int			snd_MaxChannels = 20;
float		snd_ReverbFactor = 1;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

sector_t *listenerSector = NULL;

// CODE --------------------------------------------------------------------


/*
===============================================================================

		MUSIC & SFX API

===============================================================================
*/

extern	sfxinfo_t	S_sfx[];
extern	musicinfo_t	S_music[];

/*#ifndef USEA3D
static	channel_t	Channel[MAX_CHANNELS];
#else*/
static	channel_t	*Channel = 0;
static	int			numChannels = 0;
//#endif

int					RegisteredSong = 0; //the current registered song.
static	int			NextCleanup;
int					Mus_Song = -1;
static	int			Mus_LumpNum;
static	void		*Mus_SndPtr;
static	byte		*SoundCurve;

static	boolean		UseSndScript;
static	char		ArchivePath[128];

extern	int			startepisode;
extern	int			startmap;

// int AmbChan;

int S_GetSfxLumpNum(sfxinfo_t *sound)
{
	return W_GetNumForName(sound->lumpname);	
}

//==========================================================================
//
// S_Start
//
//==========================================================================

void S_Start(void)
{
	S_StopAllSound();
	S_StartSong(gamemap, true);
}

//==========================================================================
//
// S_StartSong
//
//==========================================================================

void S_StartSong(int song, boolean loop)
{
	char *songLump;
	int track;

	if(i_CDMusic)
	{ // Play a CD track, instead
		if(s_CDTrack)
		{ // Default to the player-chosen track
			track = s_CDTrack;
		}
		else
		{
			track = P_GetMapCDTrack(gamemap);
		}
		if(track == I_CDControl(DD_GET_CURRENT_TRACK,0) && I_CDControl(DD_GET_TIME_LEFT,0) > 0
			&& I_CDControl(DD_STATUS,0) == DD_PLAYING)
		{
			// The chosen track is already playing.
			return;
		}
		if(I_CDControl(loop? DD_PLAY_LOOP : DD_PLAY, track))
			CON_Printf( "Error starting CD play (track %d).\n", track);
	}
	else
	{
		if(song == Mus_Song)
		{ // don't replay an old song
			return;
		}
		if(RegisteredSong)
		{
			I_StopSong();
			if(UseSndScript)
			{
				Z_Free(Mus_SndPtr);
			}
			else
			{
				W_ChangeCacheTag(Mus_LumpNum, PU_CACHE);
			}
			RegisteredSong = 0;
		}
		songLump = P_GetMapSongLump(song);
		if(!songLump)
		{
			return;
		}
		if(UseSndScript)
		{
			char name[128];
			sprintf(name, "%s%s.lmp", ArchivePath, songLump);
			M_ReadFile(name, (byte **)&Mus_SndPtr);
		}
		else
		{
			Mus_LumpNum = W_GetNumForName(songLump);
			Mus_SndPtr = W_CacheLumpNum(Mus_LumpNum, PU_MUSIC);
		}
		RegisteredSong = I_PlaySong(Mus_SndPtr, W_LumpLength(Mus_LumpNum), loop);
		Mus_Song = song;
	}
}

//==========================================================================
//
// S_StartSongName
//
//==========================================================================

void S_StartSongName(char *songLump, boolean loop)
{
	int cdTrack;

	if(!songLump)
	{
		return;
	}
	if(i_CDMusic)
	{
		cdTrack = 0;

		if(!strcmp(songLump, "hexen"))
		{
			cdTrack = P_GetCDTitleTrack();
		}
		else if(!strcmp(songLump, "hub"))
		{
			cdTrack = P_GetCDIntermissionTrack();
		}
		else if(!strcmp(songLump, "hall"))
		{
			cdTrack = P_GetCDEnd1Track();
		}
		else if(!strcmp(songLump, "orb"))
		{
			cdTrack = P_GetCDEnd2Track();
		}
		else if(!strcmp(songLump, "chess") && !s_CDTrack)
		{
			cdTrack = P_GetCDEnd3Track();
		}
/*	Uncomment this, if Kevin writes a specific song for startup
		else if(!strcmp(songLump, "start"))
		{
			cdTrack = P_GetCDStartTrack();
		}
*/
		if(!cdTrack || (cdTrack == I_CDControl(DD_GET_CURRENT_TRACK,0) && I_CDControl(DD_GET_TIME_LEFT,0) > 0))
		{
			return;
		}
		if(!I_CDControl(loop? DD_PLAY_LOOP : DD_PLAY, cdTrack))
		{
			// Clear the user selection?
			s_CDTrack = false;
		}

	}
	else
	{
		if(RegisteredSong)
		{
			I_StopSong();
			if(UseSndScript)
			{
				Z_Free(Mus_SndPtr);
			}
			else
			{
				W_ChangeCacheTag(Mus_LumpNum, PU_CACHE);
			}
			RegisteredSong = 0;
		}
		if(UseSndScript)
		{
			char name[128];
			sprintf(name, "%s%s.lmp", ArchivePath, songLump);
			M_ReadFile(name, (byte **)&Mus_SndPtr);
		}
		else
		{
			Mus_LumpNum = W_GetNumForName(songLump);
			Mus_SndPtr = W_CacheLumpNum(Mus_LumpNum, PU_MUSIC);
		}
		RegisteredSong = I_PlaySong(Mus_SndPtr, W_LumpLength(Mus_LumpNum), loop);
		Mus_Song = -1;
	}
}

//==========================================================================
//
// S_GetSoundID
//
//==========================================================================

int S_GetSoundID(char *name)
{
	int i;

	for(i = 0; i < NUMSFX; i++)
	{
		if(!strcmp(S_sfx[i].tagName, name))
		{
			return i;
		}
	}
	return 0;
}

//==========================================================================
//
// S_StartSound
//
//==========================================================================

void S_StartSound(mobj_t *origin, int sound_id)
{
	S_StartSoundAtVolume(origin, sound_id, 127);
}

void S_StopChannel(channel_t *chan)
{
	if(chan->handle)
	{
		SND_StopSound(chan->handle);
		if(S_sfx[chan->sound_id].usefulness > 0)
			S_sfx[chan->sound_id].usefulness--;
		memset(chan, 0, sizeof(*chan));
	}
}

//#ifdef USEA3D
// Get a free channel, but also get rid of stopped channels.
// If for_mobj is NULL, any free channel will do. Otherwise, an earlier
// channel owned by for_mobj is used.
channel_t *S_GetFreeChannel(mobj_t *for_mobj)
{
	int			i;
	channel_t	*chan = NULL;

	if(for_mobj && !for_mobj->thinker.function) for_mobj = NULL;
	// Check through all the channels.
	for(i=0; i<numChannels; i++)
	{
		// Is this a free channel?
		if(!chan && !SND_SoundIsPlaying(Channel[i].handle))
		{
			// This will be used if a clear channel is needed.
			chan = Channel + i;
			if(!for_mobj) break; // This'll do.
		}
		if(for_mobj && Channel[i].mo == for_mobj)
		{
			// Mobjs are allowed only one channel.
			chan = Channel + i;
			break;
		}
	}
	// No suitable channel found, and there already are as much channels as there can be?
	if(!chan && numChannels >= snd_MaxChannels)
	{
		// Pick one at random, then.
		chan = Channel + M_Random() % numChannels;
	}
	// Was a suitable channel found?
	if(chan)
	{
		// Make sure the sound is stopped.
		S_StopChannel(chan);
		return chan;
	}	
	// We need to allocate a new channel.
	Channel = (channel_t *)realloc(Channel, sizeof(channel_t) * (++numChannels));
	// -JL- Paranoia
	if (!Channel)
		I_Error("S_GetFreeChannel: realloc failed");
	chan = Channel + numChannels-1;
	memset(chan, 0, sizeof(channel_t));
	return chan;
}
//#endif

// Fills in the position and velocity.
void S_FillSound3D(mobj_t *mo, sound3d_t *desc)
{
	desc->flags |= DDSOUNDF_POS;
	desc->pos[VX] = mo->x;
	desc->pos[VY] = mo->z;
	desc->pos[VZ] = mo->y;

	if(mo == players[displayplayer].mo)
	{
		int tableAngle = mo->angle >> ANGLETOFINESHIFT;
		int xoff = finecosine[tableAngle] * 32, yoff = finesine[tableAngle] * 32;
		desc->pos[VX] += xoff;
		desc->pos[VZ] += yoff;
	}

	if(mo->thinker.function)
	{
		desc->pos[VY] += mo->height/2;

		desc->flags |= DDSOUNDF_MOV;
		desc->mov[VX] = mo->momx * 35;
		desc->mov[VY] = mo->momz * 35;
		desc->mov[VZ] = mo->momy * 35;
	}
}

// Return value is in range 0-255. 0 means the sound is coming from the left.
// 128 is the center.
static int CalcSep(mobj_t *listener, mobj_t *sound, unsigned int dirangle)
{
	unsigned int angle = R_PointToAngle2(listener->x, listener->y, sound->x, sound->y);
	int sep;
	
	sep = (int)(angle>>24) - (int)(dirangle>>24);
	if(sep > 128) 
		sep -= 256;
	else if(sep < -128)
		sep += 256;
	sep = 128 - sep*2;
	if(sep > 256) sep = 512-sep;
	else if(sep < 0) sep = -sep;
	return sep;
}

//==========================================================================
//
// S_StartSoundAtVolume
//
//==========================================================================

void S_StartSoundAtVolume(mobj_t *origin, int sound_id, int volume)
{
	int dist;
	int absx;
	int absy;
//#ifndef USEA3D
	int /*i, */vol;
//	int priority;
	int sep;
//	int angle;
//	int chan;
	mobj_t *plrmo = players[displayplayer].mo;

	static int sndcount = 0;
//#endif

//#ifdef USEA3D
	channel_t *channel;
//#endif

	// We aren't playing any sounds now.
	//return;

	if(sound_id == 0 || snd_SfxVolume == 0)
		return;

	if(volume == 0)	return;

	// calculate the distance before other stuff so that we can throw out
	// sounds that are beyond the hearing range.
	if(volume >= 255) // A very loud sound?
	{
		dist = 0;	// Might as well be; the sound can be heard from a great distance.
	}
	else if(plrmo && origin)
	{
		absx = abs(origin->x - plrmo->x);
		absy = abs(origin->y - plrmo->y);
		dist = absx+absy-(absx > absy ? absy>>1 : absx>>1);
		dist >>= FRACBITS;
		if(dist >= MAX_SND_DIST)
		{
			return; // sound is beyond the hearing range...
		}
		if(dist < 0)
		{	
			dist = 0;
		}
	}
	else
	{
		// The player has no mo! Kind of strange...
		origin = NULL;	
		dist = 0;		// The sound is very close.
	}

	// We won't play the same sound too many times.
	if(S_sfx[sound_id].usefulness >= 3 && origin)
	{
		// We'll replace the most distant of the existing sounds.
		int	i, maxdist = 0;
		for(i=0; i<numChannels; i++)
		{
			if(Channel[i].sound_id == sound_id && Channel[i].priority >= maxdist)
			{
				maxdist = Channel[i].priority;
				channel = Channel + i;
			}
		}
		if(dist >= maxdist) return; // Don't replace a close sound with a far one.
		SND_StopSound(channel->handle);
	}
	else
	{
		if(S_sfx[sound_id].lumpnum == 0)
		{
			S_sfx[sound_id].lumpnum = S_GetSfxLumpNum(&S_sfx[sound_id]);
		}
		if(S_sfx[sound_id].snd_ptr == NULL)
		{
			if(UseSndScript)
			{
				char name[128];
				sprintf(name, "%s%s.lmp", ArchivePath, S_sfx[sound_id].lumpname);
				M_ReadFile(name, (byte **)&S_sfx[sound_id].snd_ptr);
			}
			else
			{
				S_sfx[sound_id].snd_ptr = W_CacheLumpNum(S_sfx[sound_id].lumpnum,PU_SOUND);
			}
		}
		if(S_sfx[sound_id].usefulness < 0)
		{
			S_sfx[sound_id].usefulness = 1;
		}
		else
		{
			S_sfx[sound_id].usefulness++;
		}		
		// The player can have any number of sounds, but other mobjs only one.
		channel = S_GetFreeChannel(origin);
	}
	channel->mo = origin;
	channel->volume = volume;
	if(channel->volume > 127) channel->volume = 127;
	if(S_sfx[sound_id].changePitch)
		channel->pitch = (byte) (127+(M_Random()&7)-(M_Random()&7));
	else
		channel->pitch = 127;
	channel->sound_id = sound_id;
	channel->priority = dist;

	if(snd_3D && origin) // Play the sound in 3D?
	{
		sound3d_t desc;
		desc.flags = DDSOUNDF_VOLUME | DDSOUNDF_PITCH;
		// Very loud sounds have virtually no rolloff.
		if(volume >= 255) desc.flags |= DDSOUNDF_VERY_LOUD;
		desc.volume = (channel->volume*1000)/127;
		desc.pitch = (channel->pitch*1000)/128;
		if(origin == plrmo) desc.flags |= DDSOUNDF_LOCAL;
		S_FillSound3D(origin, &desc);

		if(!channel->handle)
			channel->handle = SND_Play3DSound(S_sfx[sound_id].snd_ptr, &desc);
		else
			SND_Update3DSound(channel->handle, &desc);
	}
	else // Play the sound in 2D.
	{
		if(!origin || !plrmo || origin == plrmo)
		{
			sep = 128;
			vol = channel->volume;
		}
		else
		{
			vol = (SoundCurve[dist]*(15*8)*channel->volume)>>14;
			sep = CalcSep(plrmo, origin, viewangle);
		}
		if(!channel->handle)
		{
			channel->handle = SND_PlaySound(S_sfx[sound_id].snd_ptr, (vol*1000)/127,
				(sep*500)/128, (channel->pitch*1000)/128);
		}
		else
		{
			SND_UpdateSound(channel->handle, (vol*1000)/127, (sep*500)/128, 
				(channel->pitch*1000)/128);
		}
	}

#ifdef PRINT_DEBUG_INFO
	printf( "PLAY:   i:%d handle:%d volume:%3d vol:%3d lump:%8s mo:%p\n", i, Channel[i].handle, 
		Channel[i].volume, vol, S_sfx[Channel[i].sound_id].lumpname, Channel[i].mo);
#endif
}

//==========================================================================
//
// S_StopSoundID
//
//==========================================================================

void S_StopSoundID(int sound_id)
{
	int		i;

	for(i=0; i<numChannels; i++)
		if(Channel[i].sound_id == sound_id && Channel[i].mo)
		{
			S_StopChannel(Channel + i);
			break;
		}
}

//==========================================================================
//
// S_StopSound
//
//==========================================================================

void S_StopSound(mobj_t *origin)
{
	int i;

	for(i=0; i<numChannels; i++)
	{
		if(Channel[i].mo == origin)
		{
			/*gi.StopSound(Channel[i].handle);
			if(S_sfx[Channel[i].sound_id].usefulness > 0)
			{
				S_sfx[Channel[i].sound_id].usefulness--;
			}
			Channel[i].handle = 0;
			Channel[i].mo = NULL;*/
			S_StopChannel(Channel + i);
		}
	}
}

//==========================================================================
//
// S_StopAllSound
//
//==========================================================================

void S_StopAllSound(void)
{
	int i;

	// Stop all sounds.
	for(i=0; i<numChannels; i++) //gi.StopSound(Channel[i].handle);
		S_StopChannel(Channel + i);
	memset(Channel, 0, numChannels * sizeof(channel_t));
	listenerSector = NULL;
}

//==========================================================================
//
// S_SoundLink
//
//==========================================================================

void S_SoundLink(mobj_t *oldactor, mobj_t *newactor)
{
	int i;

	for(i=0; i<numChannels; i++)
		if(Channel[i].mo == oldactor)
			Channel[i].mo = newactor;
}

//==========================================================================
//
// S_PauseSound
//
//==========================================================================

void S_PauseSound(void)
{
	S_StopAllSound();
	if(i_CDMusic)
	{
		I_CDControl(DD_STOP,0);
	}
	else
	{
		I_PauseSong();
	}
}

//==========================================================================
//
// S_ResumeSound
//
//==========================================================================

void S_ResumeSound(void)
{
	if(i_CDMusic)
	{
		I_CDControl(DD_RESUME, 0);
	}
	else
	{
		I_ResumeSong();
	}
}

//==========================================================================
//
// S_UpdateSounds
//
//==========================================================================

//#define NO_SOUND_UPDATE	// Somehow it helps not to update the sounds.

void S_UpdateSounds(mobj_t *listener)
{
	int i;
	int dist, vol;
	int sep;
	int absx;
	int absy;
	
	if(!listener || snd_SfxVolume == 0) return;

	// Update the listener first.
	if(snd_3D)
	{
		listener3d_t lis;
		lis.flags = DDLISTENERF_POS | DDLISTENERF_MOV | DDLISTENERF_YAW | DDLISTENERF_PITCH;
		lis.pos[VX] = listener->x;
		lis.pos[VY] = listener->z + listener->height - (5<<FRACBITS);
		lis.pos[VZ] = listener->y;
		lis.mov[VX] = listener->momx * 35;
		lis.mov[VY] = listener->momz * 35;
		lis.mov[VZ] = listener->momy * 35;
		lis.yaw = -(listener->angle / (float) ANGLE_MAX * 360 - 90);
		lis.pitch = listener->player? LOOKDIR2DEG(listener->player->lookdir) : 0;
		// If the sector changes, so does the reverb.
		if(listener->subsector->sector != listenerSector && snd_ReverbFactor > 0)
		{
			listenerSector = listener->subsector->sector;
			lis.flags |= DDLISTENERF_SET_REVERB;
			lis.reverb.space = listenerSector->reverb[SRD_SPACE];
			lis.reverb.decay = listenerSector->reverb[SRD_DECAY];
			lis.reverb.volume = listenerSector->reverb[SRD_VOLUME] * snd_ReverbFactor;
			lis.reverb.damping = listenerSector->reverb[SRD_DAMPING];
			if(reverbDebug)
			{
				ST_Message( "Sec %i: s:%.2f dc:%.2f v:%.2f dm:%.2f\n", 
					listenerSector-sectors, lis.reverb.space, 
					lis.reverb.decay, lis.reverb.volume, lis.reverb.damping);
			}
		}
		if(snd_ReverbFactor == 0 && listenerSector)
		{
			listenerSector = NULL;
			lis.flags |= DDLISTENERF_DISABLE_REVERB;
		}
		SND_UpdateListener(&lis);
	}
	
	// Update any Sequences
	SN_UpdateActiveSequences();

	if(NextCleanup < gametic)
	{
		if(UseSndScript)
		{
			for(i = 0; i < NUMSFX; i++)
			{
				if(S_sfx[i].usefulness == 0 && S_sfx[i].snd_ptr)
				{
					S_sfx[i].usefulness = -1;
				}
			}
		}
		else
		{
			for(i = 0; i < NUMSFX; i++)
			{
				if(S_sfx[i].usefulness == 0 && S_sfx[i].snd_ptr)
				{
					W_ChangeCacheTag(S_sfx[i].lumpnum, PU_CACHE);
					S_sfx[i].usefulness = -1;
					S_sfx[i].snd_ptr = NULL;
				}
			}
		}
		NextCleanup = gametic+35*30; // every 30 seconds
	}
	for(i=0; i<numChannels; i++)
	{
		if(!Channel[i].handle || !Channel[i].sound_id) continue; // Empty channel.

		// Take care of stopped sounds.
		if(!SND_SoundIsPlaying(Channel[i].handle))
		{
			S_StopChannel(Channel + i);
			continue;
		}

		// Does this sound need updating?
		if(Channel[i].mo == NULL)
		{
#ifdef PRINT_DEBUG_INFO
			printf( "UPD/CN: i:%d handle:%d volume:%3d vol:--- lump:%8s mo:%p\n", i, Channel[i].handle, 
				Channel[i].volume, S_sfx[Channel[i].sound_id].lumpname, Channel[i].mo);
#endif
			// No, apparently not.
			continue;
		}
		else
		{
			// The sound has a source.
			if(snd_3D)
			{
				sound3d_t desc;
				desc.flags = 0;
				// Fill in position and velocity.
				S_FillSound3D(Channel[i].mo, &desc);
				SND_Update3DSound(Channel[i].handle, &desc);
			}
			else // 2D mode.
			{
				absx = abs(Channel[i].mo->x - listener->x);
				absy = abs(Channel[i].mo->y - listener->y);
				dist = absx+absy-(absx > absy ? absy>>1 : absx>>1);
				dist >>= FRACBITS;
				if(dist >= MAX_SND_DIST)
				{
					S_StopSound(Channel[i].mo);
					continue;
				}
				if(dist < 0)
				{
					dist = 0;
				}
				vol = (SoundCurve[dist]*(15*8)*Channel[i].volume)>>14;
				if(Channel[i].mo == listener)
				{
					sep = 128;
				}
				else
				{
					sep = CalcSep(listener, Channel[i].mo, viewangle);
				}
				SND_UpdateSound(Channel[i].handle, (vol*1000)/127, (500*sep)/128, 
					(1000*Channel[i].pitch)/128);

				/*priority = S_sfx[Channel[i].sound_id].priority;
				priority *= PRIORITY_MAX_ADJUST-(dist/DIST_ADJUST);
				Channel[i].priority = priority;*/
			}

/*#else // USEA3D
			
			I_UpdateSoundParams(Channel+i);

#endif*/

#ifdef PRINT_DEBUG_INFO
			printf( "UPDATE: i:%d handle:%d volume:%3d vol:%3d lump:%8s mo:%p\n", i, Channel[i].handle, 
				Channel[i].volume, vol, S_sfx[Channel[i].sound_id].lumpname, Channel[i].mo);
#endif
		}
	}
}

//==========================================================================
//
// S_Init
//
//==========================================================================

void S_Init(void)
{
	SoundCurve = (byte *)W_CacheLumpName("SNDCURVE", PU_STATIC);

	numChannels = 0;
	Channel = NULL;
}

//==========================================================================
//
// S_GetChannelInfo
//
//==========================================================================

void S_GetChannelInfo(SoundInfo_t *s)
{
	int i;
	ChanInfo_t *c;

	s->channelCount = numChannels<16? numChannels : 16;
	s->musicVolume = 0;//snd_MusicVolume;
	s->soundVolume = 0;//snd_MaxVolume;
//#ifdef USEA3D
	for(i = 0; i < s->channelCount; i++)
/*#else
	for(i = 0; i < numChannels; i++)
#endif*/
	{
		c = &s->chan[i];
		c->id = Channel[i].sound_id;
		c->priority = Channel[i].priority;
		c->name = S_sfx[c->id].lumpname;
		c->mo = Channel[i].mo;
		if(c->mo)
			c->distance = P_AproxDistance(c->mo->x-viewx, 
			c->mo->y-viewy)>>FRACBITS;
		else
			c->distance = -1;
	}
}

//==========================================================================
//
// S_GetSoundPlayingInfo
//
//==========================================================================

boolean S_GetSoundPlayingInfo(mobj_t *mobj, int sound_id)
{
	int i;

	for(i=0; i<numChannels; i++)
		if(Channel[i].sound_id == sound_id && Channel[i].mo == mobj)
			if(SND_SoundIsPlaying(Channel[i].handle))
				return true;
	return false;
}

//==========================================================================
//
// S_ShutDown
//
//==========================================================================

void S_ShutDown(void)
{
/*	extern boolean timerInstalled;
	if(timerInstalled)
	{
		I_StopSong();
		I_UnRegisterSong();
		I_ShutdownSound();
	}
	if(i_CDMusic)
	{
		I_CDMusStop();
	}*/
//#ifdef USEA3D
	free(Channel);
	Channel = 0;
	numChannels = 0;
//#endif
}

void S_Reset(void)
{
	int		i;

	S_StopAllSound();
	Z_FreeTags(PU_SOUND, PU_SOUND);
	for(i=0; i<NUMSFX; i++)
	{
		S_sfx[i].lumpnum = 0;
		S_sfx[i].snd_ptr = NULL;				
	}
	// Music, too.
	if(RegisteredSong)
	{
		I_StopSong();
		if(UseSndScript)
			Z_Free(Mus_SndPtr);
		else
			W_ChangeCacheTag(Mus_LumpNum, PU_CACHE);
		RegisteredSong = 0;
	}
	Mus_Song = -1;
}

//==========================================================================
//
// S_InitScript
//
//==========================================================================

void S_InitScript(void)
{
	int p;
	int i;

	strcpy(ArchivePath, DEFAULT_ARCHIVEPATH);
	if(!(p = M_CheckParm("-devsnd")))
	{
		UseSndScript = false;
		SC_OpenLump("sndinfo");
	}
	else
	{
		UseSndScript = true;
		SC_OpenFile(Argv(p+1));
	}
	while(SC_GetString())
	{
		if(*sc_String == '$')
		{
			if(!stricmp(sc_String, "$ARCHIVEPATH"))
			{
				SC_MustGetString();
				strcpy(ArchivePath, sc_String);
			}
			else if(!stricmp(sc_String, "$MAP"))
			{
				SC_MustGetNumber();
				SC_MustGetString();
				if(sc_Number)
				{
					P_PutMapSongLump(sc_Number, sc_String);
				}
			}
			continue;
		}
		else
		{
			for(i = 0; i < NUMSFX; i++)
			{
				if(!strcmp(S_sfx[i].tagName, sc_String))
				{
					SC_MustGetString();
					if(*sc_String != '?')
					{
						strcpy(S_sfx[i].lumpname, sc_String);
					}
					else
					{
						strcpy(S_sfx[i].lumpname, "default");
					}
					break;
				}
			}
			if(i == NUMSFX)
			{
				SC_MustGetString();
			}
		}
	}
	SC_Close();

	for(i = 0; i < NUMSFX; i++)
	{
		if(!strcmp(S_sfx[i].lumpname, ""))
		{
			strcpy(S_sfx[i].lumpname, "default");
		}
	}
}


//==========================================================================
//
// CONSOLE COMMANDS
//
//==========================================================================

int CCmdCD(int argc, char **argv)
{
	if(argc > 1)
	{
		if(!strcmpi(argv[1], "init"))
		{
			if(!I_CDControl(DD_INIT,0))
				CON_Printf( "CD init successful.\n");
			else
				CON_Printf( "CD init failed.\n");
		}
		else if(!strcmpi(argv[1], "info") && argc == 2)
		{
			int secs = I_CDControl(DD_GET_TIME_LEFT, 0);
			CON_Printf( "CD available: %s\n", I_CDControl(DD_AVAILABLE,0)? "yes" : "no");
			CON_Printf( "First track: %d\n", I_CDControl(DD_GET_FIRST_TRACK,0));
			CON_Printf( "Last track: %d\n", I_CDControl(DD_GET_LAST_TRACK,0));
			CON_Printf( "Current track: %d\n", I_CDControl(DD_GET_CURRENT_TRACK,0));
			CON_Printf( "Time left: %d:%02d\n", secs/60, secs%60);
			CON_Printf( "Play mode: ");
			if(MusicPaused)
				CON_Printf( "paused\n");
			else if(s_CDTrack)
				CON_Printf( "looping track %d\n", s_CDTrack);
			else
				CON_Printf( "map track\n");
			return true;
		}
		else if(!strcmpi(argv[1], "play") && argc == 3)
		{
			s_CDTrack = atoi(argv[2]);
			if(!I_CDControl(DD_PLAY_LOOP, s_CDTrack)) 
			{
				CON_Printf( "Playing track %d.\n", s_CDTrack);
			}
			else
			{
				CON_Printf( "Error playing track %d.\n", s_CDTrack);
				return false;
			}
		}
		else if(!strcmpi(argv[1], "map"))
		{
			int track;
			int mapnum = gamemap;
			if(argc == 3) mapnum = atoi(argv[2]);
			s_CDTrack = false;	// Clear the user selection.
			track = P_GetMapCDTrack(mapnum);			
			if(!I_CDControl(DD_PLAY_LOOP, track)) // Uses s_CDTrack.
			{
				CON_Printf( "Playing track %d.\n", track);
			}
			else
			{
				CON_Printf( "Error playing track %d.\n", track);
				return false;
			}
		}
		else if(!strcmpi(argv[1], "stop") && argc == 2)
		{
			I_CDControl(DD_STOP,0);
			CON_Printf( "CD stopped.\n");
		}
		else if(!strcmpi(argv[1], "resume") && argc == 2)
		{
			I_CDControl(DD_RESUME,0);//I_CDMusResume();
			CON_Printf( "CD resumed.\n");
		}
		else
			CON_Printf( "Bad command. Try 'cd'.\n");
	}
	else
	{
		CON_Printf( "CD player control. Usage: CD (cmd)\n");
		CON_Printf( "Commands are: init, info, play (track#), map, map (#), stop, resume.\n");
	}
	return true;
}

int CCmdMidi(int argc, char **argv)
{
	if(argc == 1)
	{
		CON_Printf( "Usage: midi (cmd)\n");
		CON_Printf( "Commands are: reset, play (name), map, map (num).\n");
		return true;
	}
	if(argc == 2)
	{
		if(!strcmpi(argv[1], "reset"))
		{
			if(RegisteredSong)
			{
				I_StopSong();
				if(UseSndScript)
					Z_Free(Mus_SndPtr);
				else
					W_ChangeCacheTag(Mus_LumpNum, PU_CACHE);
				RegisteredSong = 0;
			}
			Mus_Song = -1;
			CON_Printf( "MIDI has been reset.\n");
		}
		else if(!strcmpi(argv[1], "map")) 
		{
			CON_Printf( "Playing the song of the current map (%d).\n", gamemap);
			S_StartSong(gamemap, true);
		}
		else
			return false;
	}
	else if(argc == 3)
	{
		if(i_CDMusic)
		{	
			CON_Printf( "MIDI is not the current music device.\n");
			return true;
		}
		if(!strcmpi(argv[1], "play"))
		{
			CON_Printf( "Playing song '%s'.\n", argv[2]);
			S_StartSongName(argv[2], true);
		}
		else if(!strcmpi(argv[1], "map"))
		{
			CON_Printf( "Playing song for map %d.\n", atoi(argv[2]));
			S_StartSong(atoi(argv[2]), true);
		}
		else
			return false;
	}
	else 
		return false;
	// Oh, we're done.
	return true;
}
