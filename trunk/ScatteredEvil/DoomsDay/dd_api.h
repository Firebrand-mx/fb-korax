#ifndef __DOOMSDAY_GAME_API_H__
#define __DOOMSDAY_GAME_API_H__

#include "dd_share.h"

// The routines/data imported from the Doomsday engine.
typedef struct
{
	int			version;		// Doomsday Engine version.

	void		(*SetupLevel)(int flags);
	void		(*PrecacheLevel)(void);

	// Base-level.
	void		(*AddStartupWAD)(char *file);
	void		(*SetConfigFile)(char *filename);
	void		(*DefineActions)(action_t *actions);
	void		(*Quit)(void);
	int			(*Get)(int ddValue);
	void		(*Set)(int ddValue, int parm);

	// Messages.
	void		(*Message)(char *msg, ...);
	void		(*Error)(char *error, ...);
	void		(*CoreDump)(const char *fmt, ...);

	// Console.
	void		(*conprintf)(char *format, ...);
	void		(*flconprintf)(int flags, char *format, ...);
	void		(*SetConsoleFont)(ddfont_t *cfont);
	void		(*AddCommand)(ccmd_t *command);
	void		(*AddVariable)(cvar_t *variable);	
	void		(*OpenConsole)(int open);
	cvar_t*		(*GetCVar)(char *name);
	int			(*Execute)(char *command, int silent);

	// Memory zone.
	void*		(*Z_Malloc)(int size, int tag, void *user);
	void		(*Z_Free)(void *ptr);
	void		(*Z_FreeTags)(int lowtag, int hightag);
	void		(*Z_ChangeTag)(void *ptr, int tag);
	void		(*Z_CheckHeap)(void);

	// Networking.
	ddplayer_t*	(*GetPlayer)(int number);
	char*		(*GetPlayerName)(int playerNum);
	void		(*GetTicCmd)(void *cmd, int player);
	int			(*NetSetServerData)(void *data, int size);
	int			(*NetGetServerData)(void *data, int size);
	int			(*NetSetPlayerData)(void *data, int size);
	int			(*NetGetPlayerData)(int playerNum, void *data, int size);
	void		(*SendPacket)(unsigned int playermask, void *data, int length);

	// Low-level stuff.
	int			(*GetTime)(void);	
	void		(*ClearKeyRepeaters)(void);
	void		(*EventBuilder)(char *buff, event_t *ev, boolean to_event);
	int			(*BindingsForCommand)(char *command, char *buffer);

	// Thinkers.
	void		(*RunThinkers)(void);
	void		(*InitThinkers)(void);
	void		(*AddThinker)(thinker_t *thinker);
	void		(*RemoveThinker)(thinker_t *thinker);

	// Map data. (Loading mostly done by the game dll.)
/*	void		(*LoadBlockMap)(int lump);
	void		(*LoadReject)(int lump);

	// Map utils.
	fixed_t		(*ApproxDistance)(fixed_t dx, fixed_t dy);	
	int			(*PointOnLineSide)(fixed_t x, fixed_t y, line_t *line);
	int			(*BoxOnLineSide)(fixed_t *tmbox, line_t *ld);
	void		(*MakeDivline)(line_t *li, divline_t *dl);
	int			(*PointOnDivlineSide)(fixed_t x, fixed_t y, divline_t *line);
	fixed_t		(*InterceptVector)(divline_t *v2, divline_t *v1);
	void		(*LineOpening)(line_t *linedef);
	void		(*LinkThing)(mobj_t *thing, byte flags);
	void		(*UnlinkThing)(mobj_t *thing, byte flags);
	boolean		(*BlockLinesIterator)(int x, int y, boolean(*func)(line_t*));
	boolean		(*BlockThingsIterator)(int x, int y, boolean(*func)(mobj_t*));
	boolean		(*PathTraverse)(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
					int flags, boolean (*trav) (intercept_t *));
	boolean		(*CheckSight)(mobj_t *t1, mobj_t *t2);
*/	
	// Refresh.
	void		(*SetSpriteNameList)(char **namelist);
	void		(*GetSpriteInfo)(int sprite, int frame, spriteinfo_t *sprinfo);
	void		(*SetBorderGfx)(char *lumps[9]);
	void		(*RenderPlayerView)(void *ddplayer);
	void		(*ViewWindow)(int x, int y, int w, int h);
	int			(*R_FlatNumForName)(char *name);
	int			(*R_CheckTextureNumForName)(char *name);
	int			(*R_TextureNumForName)(char *name);
	char*		(*R_TextureNameForNum)(int num);
	angle_t		(*R_PointToAngle2)(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
	void*		(*R_PointInSubsector)(fixed_t x, fixed_t y);
	int			(*FrameRate)(void);

	// WAD files.
	int			(*W_CheckNumForName)(char *name);
	int			(*W_GetNumForName)(char *name);
	void*		(*W_CacheLumpNum)(int lump, int tag);
	void*		(*W_CacheLumpName)(char *name, int tag);
	int			(*W_LumpLength)(int lump);
	void		(*W_ReadLump)(int lump, void *dest);
	void		(*W_ChangeCacheTag)(int lump, int tag);

	// Graphics.
	void*		(*GetDGL)(void);
	void		(*Update)(int flags);
	int			(*ChangeResolution)(int width, int height);
	int			(*SetFlatTranslation)(int flat, int translateTo);
	int			(*SetTextureTranslation)(int tex, int translateTo);
	void		(*GL_UseFog)(int yes);
	int			(*ScreenShot)(char *filename, int bits);
	void		(*GL_ResetData)(void);
	void		(*GL_ResetTextures)(void);
	void		(*GL_ClearTextureMem)(void);
	void		(*GL_TexFilterMode)(int target, int parm);
	void		(*GL_SetColorAndAlpha)(float r, float g, float b, float a);
	void		(*GL_SetColor)(int palidx);	// Uses playpal
	void		(*GL_SetNoTexture)();
	void		(*GL_SetFlat)(int idx);
	void		(*GL_SetPatch)(int lump);
	void		(*GL_SetRawImage)(int lump, int part);
	void		(*GL_DrawPatch)(int x, int y, int lumpnum);
	void		(*GL_DrawFuzzPatch)(int x, int y, int lumpnum);
	void		(*GL_DrawAltFuzzPatch)(int x, int y, int lumpnum);
	void		(*GL_DrawShadowedPatch)(int x, int y, int lumpnum);
	void		(*GL_DrawPatchLitAlpha)(int x, int y, float light, float alpha, int lumpnum);
	void		(*GL_DrawPatchCS)(int x, int y, int lumpnum);
	void		(*GL_DrawRawScreen)(int lump);
	void		(*GL_DrawRect)(float x, float y, float w, float h, float r, 
					float g, float b, float a);
	void		(*GL_DrawRectTiled)(int x, int y, int w, int h, int tw, int th);
	void		(*GL_DrawCutRectTiled)(int x, int y, int w, int h, int tw, 
					int th, int cx, int cy, int cw, int ch);
	void		(*GL_DrawPSprite)(int x, int y, float scale, int flip, int lump);
	void		(*GL_SetFilter)(int rgba);
	void		(*SkyParams)(int layer, int parm, float value);

	// Sound.
	int			(*PlaySound)(void *data, int volume, int pan, int pitch);
	void		(*UpdateSound)(int handle, int volume, int pan, int pitch);
	int			(*Play3DSound)(void *data, sound3d_t *desc);
	void		(*Update3DSound)(int handle, sound3d_t *desc);
	void		(*UpdateListener)(listener3d_t *desc);
	void		(*StopSound)(int handle);
	int			(*SoundIsPlaying)(int handle);
	void		(*SetSfxVolume)(int volume);	

	// MIDI music.
	int			(*PlaySong)(void *data, int length, int loop);
	int			(*SongIsPlaying)(void);
	void		(*StopSong)(void);
	void		(*PauseSong)(void);
	void		(*ResumeSong)(void);
	void		(*SetMusicDevice)(int musicdevice);
	void		(*SetMIDIVolume)(int volume);

	// CD-ROM.	
	int			(*CD)(int cmd, int parm);

	// Misc.
	int			(*Argc)(void);
	char*		(*Argv)(int i);
	char**		(*ArgvPtr)(int i);
	boolean		(*ParmExists)(char *check);
	int			(*CheckParm)(char *check);
	int			(*ReadFile)(const char *name, byte **buffer);
	int			(*ReadFileClib)(const char *name, byte **buffer);
	boolean		(*WriteFile)(const char *name, void *source, int length);
	void		(*ExtractFileBase)(char *path, char *dest);
	void		(*ClearBox)(fixed_t *box);
	void		(*AddToBox)(fixed_t *box, fixed_t x, fixed_t y);

	//
	// DATA
	//
	// General information.
	int			*validcount;
/*	fixed_t		*topslope;
	fixed_t		*bottomslope;*/

	// Thinker data (DO NOT CHANGE).
	thinker_t	*thinkercap; // The head and tail of the thinker list

	// Map data, pointers to the arrays.
	int			*numvertexes;
	int			*numsegs;
	int			*numsectors;
	int			*numsubsectors;
	int			*numnodes;
	int			*numlines;
	int			*numsides;
	void		**vertexes;
	void		**segs;
	void		**sectors;
	void		**subsectors;
	void		**nodes;
	void		**lines;
	void		**sides;
/*	short		**blockmaplump;
	short		**blockmap;
	int			*bmapwidth;
	int			*bmapheight;
	int			*bmaporgx;
	int			*bmaporgy;
	void		***blocklinks;		// mobj chains
	byte		**rejectmatrix;*/
} 
game_import_t;

// The routines/data exported from the game DLL.
typedef struct
{
	// Base-level.
	void		(*PreInit)(void);
	void		(*PostInit)(void);
	void		(*Shutdown)(void);
	void		(*UpdateState)(int step);
	char*		(*GetString)(int id);

	// Ticcmds.
	void		(*BuildTiccmd)(void *cmd);
	void		(*ModifyDupTiccmd)(void *cmd);

	// Networking.
	int			(*NetServerOpen)(int before);
	int			(*NetServerStart)(int before);
	int			(*NetServerStop)(int before);
	int			(*NetServerClose)(int before);
	int			(*NetConnect)(int before);
	int			(*NetDisconnect)(int before);
	int			(*NetPlayerEvent)(int playernum, int type, void *data);
	void		(*HandlePacket)(int fromplayer, void *data, int length);
	
	// Tickers.
	void		(*G_Ticker)(void);
	void		(*MN_Ticker)(void);

	// Responders.
	boolean		(*PrivilegedResponder)(event_t *event);
	boolean		(*MN_Responder)(event_t *event);
	boolean		(*G_Responder)(event_t *event);

	// Refresh.
	void		(*BeginFrame)(void);
	void		(*EndFrame)(void);
	void		(*G_Drawer)(void);
	void		(*MN_Drawer)(void);
	void		(*DrawPlayerSprites)(ddplayer_t *viewplr);
	void		(*ConsoleBackground)(int *width, int *height);
	void		(*R_Init)(void);

	// Miscellaneous.
	void		(*MobjThinker)(struct mobj_s *mo);

	// Main structure sizes.
	int			ticcmd_size;		// sizeof(ticcmd_t)
	int			vertex_size;
	int			seg_size;
	int			sector_size;
	int			subsector_size;
	int			node_size;
	int			line_size;
	int			side_size;
}
game_export_t;

// This is called by the main engine.
//game_export_t *GetGameAPI(game_import_t *imports);

typedef game_export_t* (*GETGAMEAPI)(game_import_t*);

#endif