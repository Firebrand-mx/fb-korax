
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

// PUBLIC DATA DEFINITIONS -------------------------------------------------

#define i_CDMusic		( gi.Get(DD_MUSIC_DEVICE)==2 )

int			s_CDTrack = 0;
boolean		MusicPaused, reverbDebug = false;

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
//static	boolean		MusicPaused;
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
	return gi.W_GetNumForName(sound->lumpname);	
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
		if(track == gi.CD(DD_GET_CURRENT_TRACK,0) && gi.CD(DD_GET_TIME_LEFT,0) > 0
			&& gi.CD(DD_STATUS,0) == DD_PLAYING)
		{
			// The chosen track is already playing.
			return;
		}
		if(gi.CD(loop? DD_PLAY_LOOP : DD_PLAY, track))
			gi.conprintf( "Error starting CD play (track %d).\n", track);
	}
	else
	{
		if(song == Mus_Song)
		{ // don't replay an old song
			return;
		}
		if(RegisteredSong)
		{
			gi.StopSong();
			if(UseSndScript)
			{
				gi.Z_Free(Mus_SndPtr);
			}
			else
			{
				gi.W_ChangeCacheTag(Mus_LumpNum, PU_CACHE);
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
			gi.ReadFile(name, (byte **)&Mus_SndPtr);
		}
		else
		{
			Mus_LumpNum = gi.W_GetNumForName(songLump);
			Mus_SndPtr = gi.W_CacheLumpNum(Mus_LumpNum, PU_MUSIC);
		}
		RegisteredSong = gi.PlaySong(Mus_SndPtr, gi.W_LumpLength(Mus_LumpNum), loop);
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
		if(!cdTrack || (cdTrack == gi.CD(DD_GET_CURRENT_TRACK,0) && gi.CD(DD_GET_TIME_LEFT,0) > 0))
		{
			return;
		}
		if(!gi.CD(loop? DD_PLAY_LOOP : DD_PLAY, cdTrack))
		{
			// Clear the user selection?
			s_CDTrack = false;
		}

	}
	else
	{
		if(RegisteredSong)
		{
			gi.StopSong();
			if(UseSndScript)
			{
				gi.Z_Free(Mus_SndPtr);
			}
			else
			{
				gi.W_ChangeCacheTag(Mus_LumpNum, PU_CACHE);
			}
			RegisteredSong = 0;
		}
		if(UseSndScript)
		{
			char name[128];
			sprintf(name, "%s%s.lmp", ArchivePath, songLump);
			gi.ReadFile(name, (byte **)&Mus_SndPtr);
		}
		else
		{
			Mus_LumpNum = gi.W_GetNumForName(songLump);
			Mus_SndPtr = gi.W_CacheLumpNum(Mus_LumpNum, PU_MUSIC);
		}
		RegisteredSong = gi.PlaySong(Mus_SndPtr, gi.W_LumpLength(Mus_LumpNum), loop);
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
		gi.StopSound(chan->handle);
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
		if(!chan && !gi.SoundIsPlaying(Channel[i].handle))
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
		/*gi.StopSound(chan->handle);
		if(chan->handle)
		{
			if(S_sfx[chan->sound_id].usefulness > 0)
				S_sfx[chan->sound_id].usefulness--;
		}
		memset(chan, 0, sizeof(*chan));*/
		S_StopChannel(chan);
		return chan;
	}	
	// We need to allocate a new channel.
	Channel = (channel_t *)realloc(Channel, sizeof(channel_t) * (++numChannels));
	// -JL- Paranoia
	if (!Channel)
		gi.Error("S_GetFreeChannel: realloc failed");
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

	if(mo == players[displayplayer].plr->mo)
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
	mobj_t *plrmo = players[displayplayer].plr->mo;

	static int sndcount = 0;
//#endif

//#ifdef USEA3D
	channel_t *channel;
//#endif

	// We aren't playing any sounds now.
	//return;

	if(sound_id == 0 || gi.Get(DD_SFX_VOLUME) == 0)
		return;

	if(volume == 0)	return;

	/*gi.Message( "Play %s, origin %p, vol %i, use %i.\n", S_sfx[sound_id].tagName, origin, volume,
		S_sfx[sound_id].usefulness);*/

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
		gi.StopSound(channel->handle);
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
				gi.ReadFile(name, (byte **)&S_sfx[sound_id].snd_ptr);
			}
			else
			{
				S_sfx[sound_id].snd_ptr = gi.W_CacheLumpNum(S_sfx[sound_id].lumpnum,PU_SOUND);
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
			channel->handle = gi.Play3DSound(S_sfx[sound_id].snd_ptr, &desc);
		else
			gi.Update3DSound(channel->handle, &desc);
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
			sep = CalcSep(plrmo, origin, gi.Get(DD_VIEWANGLE));
		}
		if(!channel->handle)
		{
			channel->handle = gi.PlaySound(S_sfx[sound_id].snd_ptr, (vol*1000)/127,
				(sep*500)/128, (channel->pitch*1000)/128);
		}
		else
		{
			gi.UpdateSound(channel->handle, (vol*1000)/127, (sep*500)/128, 
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

//#ifndef USEA3D
void S_StopSoundID(int sound_id)//, int priority)
{
	int		i;

	for(i=0; i<numChannels; i++)
		if(Channel[i].sound_id == sound_id && Channel[i].mo)
		{
			/*gi.StopSound(Channel[i].handle);
			if(S_sfx[Channel[i].sound_id].usefulness > 0)
				S_sfx[Channel[i].sound_id].usefulness--;
			Channel[i].handle = 0;
			Channel[i].mo = NULL;
			Channel[i].sound_id = 0;*/
			S_StopChannel(Channel + i);
			break;
		}
/*	int i;
	int lp; //least priority
	int found;

	if(S_sfx[sound_id].numchannels == -1)
	{
		return(true);
	}
	lp = -1; //denote the argument sound_id
	found = 0;
	for(i=0; i<snd_Channels; i++)
	{
		if(Channel[i].sound_id == sound_id && Channel[i].mo)
		{
			found++; //found one.  Now, should we replace it??
			if(priority >= Channel[i].priority)
			{ // if we're gonna kill one, then this'll be it
				lp = i;
				priority = Channel[i].priority;
			}
		}
	}
	if(found < S_sfx[sound_id].numchannels)
	{
		return(true);
	}
	else if(lp == -1)
	{
		return(false); // don't replace any sounds
	}
	if(Channel[lp].handle)
	{
		if(gi.SoundIsPlaying(Channel[lp].handle))
		{
			gi.StopSound(Channel[lp].handle);
		}
		if(S_sfx[Channel[lp].sound_id].usefulness > 0)
		{
			S_sfx[Channel[lp].sound_id].usefulness--;
		}
		Channel[lp].mo = NULL;
	}
	return(true);*/
}
//#endif

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
		//I_CDMusStop();
		gi.CD(DD_STOP,0);
	}
	else
	{
		//I_PauseSong();
		gi.PauseSong();
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
		gi.CD(DD_RESUME, 0);
	}
	else
	{
		gi.ResumeSong();
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
	
	if(!listener || gi.Get(DD_SFX_VOLUME) == 0) return;

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
		lis.pitch = listener->player? LOOKDIR2DEG(listener->player->plr->lookdir) : 0;
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
				gi.Message( "Sec %i: s:%.2f dc:%.2f v:%.2f dm:%.2f\n", 
					listenerSector-sectors, lis.reverb.space, 
					lis.reverb.decay, lis.reverb.volume, lis.reverb.damping);
			}
		}
		if(snd_ReverbFactor == 0 && listenerSector)
		{
			listenerSector = NULL;
			lis.flags |= DDLISTENERF_DISABLE_REVERB;
		}
		gi.UpdateListener(&lis);
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
					gi.W_ChangeCacheTag(S_sfx[i].lumpnum, PU_CACHE);
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
		if(!gi.SoundIsPlaying(Channel[i].handle))
		{
			/*if(S_sfx[Channel[i].sound_id].usefulness > 0)
			{
				S_sfx[Channel[i].sound_id].usefulness--;
			}
			gi.StopSound(Channel[i].handle); // Free it.
			Channel[i].handle = 0;
			Channel[i].mo = NULL;
			Channel[i].sound_id = 0;*/
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
				gi.Update3DSound(Channel[i].handle, &desc);
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
/*					angle = R_PointToAngle2(listener->x, listener->y,
						Channel[i].mo->x, Channel[i].mo->y);
					angle = (angle - gi.Get(DD_VIEWANGLE)) >> 24;

					sep = angle*2-128;
					if(sep < 64)
						sep = -sep;
					if(sep > 192)
						sep = 512-sep;*/
					sep = CalcSep(listener, Channel[i].mo, gi.Get(DD_VIEWANGLE));
				}
				gi.UpdateSound(Channel[i].handle, (vol*1000)/127, (500*sep)/128, 
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
	SoundCurve = (byte *)gi.W_CacheLumpName("SNDCURVE", PU_STATIC);
//      SoundCurve = Z_Malloc(MAX_SND_DIST, PU_STATIC, NULL);

/*#ifndef USEA3D
	if(snd_Channels > 16)
	{
		snd_Channels = 16;
	}
	//I_SetChannels(snd_Channels);
#else*/
	numChannels = 0;
	Channel = NULL;
//#endif

	//gi.SetMIDIVolume(snd_MusicVolume*16);
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
			c->distance = P_AproxDistance(c->mo->x-gi.Get(DD_VIEWX), 
			c->mo->y-gi.Get(DD_VIEWY))>>FRACBITS;
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
			if(gi.SoundIsPlaying(Channel[i].handle))
				return true;
	return false;
}

//==========================================================================
//
// S_SetMusicVolume
//
//==========================================================================

/*void S_SetMusicVolume(int vol)
{
	if(i_CDMusic)
	{
		//I_CDMusSetVolume(snd_MusicVolume*16); // 0-255
		gi.CD(DD_SET_VOLUME, vol);//snd_MusicVolume*16);
	}
	else
	{
		//I_SetMusicVolume(snd_MusicVolume);
		gi.SetMIDIVolume(vol);//snd_MusicVolume);
	}
	if(vol == 0)
	{
		if(!i_CDMusic)
		{
			//I_PauseSong();
			gi.PauseSong();
		}
		MusicPaused = true;
	}
	else if(MusicPaused)
	{
		if(!i_CDMusic)
		{
			//I_ResumeSong();
			gi.ResumeSong();
		}
		MusicPaused = false;
	}
}*/

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
	gi.Z_FreeTags(PU_SOUND, PU_SOUND);
	for(i=0; i<NUMSFX; i++)
	{
		S_sfx[i].lumpnum = 0;
		S_sfx[i].snd_ptr = NULL;				
	}
	// Music, too.
	if(RegisteredSong)
	{
		gi.StopSong();
		if(UseSndScript)
			gi.Z_Free(Mus_SndPtr);
		else
			gi.W_ChangeCacheTag(Mus_LumpNum, PU_CACHE);
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
	if(!(p = gi.CheckParm("-devsnd")))
	{
		UseSndScript = false;
		SC_OpenLump("sndinfo");
	}
	else
	{
		UseSndScript = true;
		SC_OpenFile(gi.Argv(p+1));
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
			if(!gi.CD(DD_INIT,0))
				gi.conprintf( "CD init successful.\n");
			else
				gi.conprintf( "CD init failed.\n");
		}
		else if(!strcmpi(argv[1], "info") && argc == 2)
		{
			int secs = gi.CD(DD_GET_TIME_LEFT, 0);
			gi.conprintf( "CD available: %s\n", gi.CD(DD_AVAILABLE,0)? "yes" : "no");
			gi.conprintf( "First track: %d\n", gi.CD(DD_GET_FIRST_TRACK,0));
			gi.conprintf( "Last track: %d\n", gi.CD(DD_GET_LAST_TRACK,0));
			gi.conprintf( "Current track: %d\n", gi.CD(DD_GET_CURRENT_TRACK,0));
			gi.conprintf( "Time left: %d:%02d\n", secs/60, secs%60);
			gi.conprintf( "Play mode: ");
			if(MusicPaused)
				gi.conprintf( "paused\n");
			else if(s_CDTrack)
				gi.conprintf( "looping track %d\n", s_CDTrack);
			else
				gi.conprintf( "map track\n");
			return true;
		}
		else if(!strcmpi(argv[1], "play") && argc == 3)
		{
			s_CDTrack = atoi(argv[2]);
			if(!gi.CD(DD_PLAY_LOOP, s_CDTrack)) 
			{
				gi.conprintf( "Playing track %d.\n", s_CDTrack);
			}
			else
			{
				gi.conprintf( "Error playing track %d.\n", s_CDTrack);
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
			if(!gi.CD(DD_PLAY_LOOP, track)) // Uses s_CDTrack.
			{
				gi.conprintf( "Playing track %d.\n", track);
			}
			else
			{
				gi.conprintf( "Error playing track %d.\n", track);
				return false;
			}
		}
		else if(!strcmpi(argv[1], "stop") && argc == 2)
		{
			gi.CD(DD_STOP,0);
			gi.conprintf( "CD stopped.\n");
		}
		else if(!strcmpi(argv[1], "resume") && argc == 2)
		{
			gi.CD(DD_RESUME,0);//I_CDMusResume();
			gi.conprintf( "CD resumed.\n");
		}
		else
			gi.conprintf( "Bad command. Try 'cd'.\n");
	}
	else
	{
		gi.conprintf( "CD player control. Usage: CD (cmd)\n");
		gi.conprintf( "Commands are: init, info, play (track#), map, map (#), stop, resume.\n");
	}
	return true;
}

int CCmdMidi(int argc, char **argv)
{
	if(argc == 1)
	{
		gi.conprintf( "Usage: midi (cmd)\n");
		gi.conprintf( "Commands are: reset, play (name), map, map (num).\n");
		return true;
	}
	if(argc == 2)
	{
		if(!strcmpi(argv[1], "reset"))
		{
			if(RegisteredSong)
			{
				gi.StopSong();
				if(UseSndScript)
					gi.Z_Free(Mus_SndPtr);
				else
					gi.W_ChangeCacheTag(Mus_LumpNum, PU_CACHE);
				RegisteredSong = 0;
			}
			Mus_Song = -1;
			gi.conprintf( "MIDI has been reset.\n");
		}
		else if(!strcmpi(argv[1], "map")) 
		{
			gi.conprintf( "Playing the song of the current map (%d).\n", gamemap);
			S_StartSong(gamemap, true);
		}
		else
			return false;
	}
	else if(argc == 3)
	{
		if(i_CDMusic)
		{	
			gi.conprintf( "MIDI is not the current music device.\n");
			return true;
		}
		if(!strcmpi(argv[1], "play"))
		{
			gi.conprintf( "Playing song '%s'.\n", argv[2]);
			S_StartSongName(argv[2], true);
		}
		else if(!strcmpi(argv[1], "map"))
		{
			gi.conprintf( "Playing song for map %d.\n", atoi(argv[2]));
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
