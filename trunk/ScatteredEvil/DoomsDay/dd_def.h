// Doomsday-specific definitions and stuff.

#ifndef __DOOMSDAY_DEFS_H__
#define __DOOMSDAY_DEFS_H__

// Stuff common to everybody.
#include "dd_share.h"
#include "dd_api.h"
#include "dd_gl.h"

#include "z_zone.h"
#include "w_wad.h"

// Important definitions.

#define MAXPLAYERS			8

#include "dd_data.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "st_start.h"
#include <malloc.h>

#pragma warning (disable:4761 4244)

extern gl_export_t		gl;
extern game_export_t	gx;

#define DO_GUARD

// if rangecheck is undefined, most parameter validation debugging code
// will not be compiled
#ifndef NORANGECHECKING
#define RANGECHECK
#define DO_GUARD_SLOW
#endif

// Past distributions
#ifndef DOOMSDAY_VER_ID
#define DOOMSDAY_VER_ID "DGL"
#endif
#ifdef RANGECHECK
#define DOOMSDAY_VERSIONTEXT "Version 0.6 +R "__DATE__" ("DOOMSDAY_VER_ID")"
#else
#define DOOMSDAY_VERSIONTEXT "Version 0.6 "__DATE__" ("DOOMSDAY_VER_ID")"
#endif

// Ticcmd
#define		TICCMD_SIZE		gx.ticcmd_size
#define		TICCMD_IDX(i)	((i)*TICCMD_SIZE)

// Map data
#define		VTXSIZE		gx.vertex_size
#define		SEGSIZE		gx.seg_size
#define		SECTSIZE	gx.sector_size
#define		SUBSIZE		gx.subsector_size
#define		NODESIZE	gx.node_size
#define		LINESIZE	gx.line_size
#define		SIDESIZE	gx.side_size

#define		VTXIDX(i)	((i)*VTXSIZE)
#define		SEGIDX(i)	((i)*SEGSIZE)
#define		SECTIDX(i)	((i)*SECTSIZE)
#define		SUBIDX(i)	((i)*SUBSIZE)
#define		NODEIDX(i)	((i)*NODESIZE)
#define		LINEIDX(i)	((i)*LINESIZE)
#define		SIDEIDX(i)	((i)*SIDESIZE)

#define		VERTEX_PTR(i)		( (vertex_t*) (vertexes+VTXIDX(i)) )
#define		SEG_PTR(i)			( (seg_t*) (segs+SEGIDX(i)) )
#define		SECTOR_PTR(i)		( (sector_t*) (sectors+SECTIDX(i)) )
#define		SUBSECTOR_PTR(i)	( (subsector_t*) (subsectors+SUBIDX(i)) )
#define		NODE_PTR(i)			( (node_t*) (nodes+NODEIDX(i)) )
#define		LINE_PTR(i)			( (line_t*) (lines+LINEIDX(i)) )
#define		SIDE_PTR(i)			( (side_t*) (sides+SIDEIDX(i)) )

#define	BACKUPTICS				12
#define DOOMDATA_TICCMDBUFFER	384		// Maximum ticcmd size: 32 bytes.

typedef struct
{
	unsigned	checksum;					// high bit is retransmit request
	byte		retransmitfrom;				// only valid if NCMD_RETRANSMIT
	byte		starttic;
	byte		player : 4;				// from which player?
	byte		targetplayer : 4;		// the player for which this packet is meant to
	byte		numtics;
	byte		ticcmds[DOOMDATA_TICCMDBUFFER];
} doomdata_t;

typedef struct
{
	long	id;
	short	intnum;			// DOOM executes an int to execute commands

// communication between DOOM and the driver
	short	command;		// CMD_SEND or CMD_GET
	short	remotenode;		// dest for send, set by get (-1 = no packet)
	short	datalength;		// bytes in doomdata to be sent

// info common to all nodes
	short	numnodes;		// console is allways node 0
	short	ticdup;			// 1 = no duplication, 2-5 = dup for slow nets
	short	extratics;		// 1 = send a backup tic in every packet
	short	deathmatch;		// 1 = deathmatch
	short	savegame;		// -1 = new game, 0-5 = load savegame
	short	episode;		// 1-3
	short	map;			// 1-9
	short	skill;			// 1-5

// info specific to this node
	short	consoleplayer;
	short	numplayers;
	short	angleoffset;	// 1 = left, 0 = center, -1 = right
	short	drone;			// 1 = drone

// packet data to be sent
	doomdata_t	data;
} doomcom_t;

#define	DOOMCOM_ID		0x12345678l

extern	doomcom_t		*doomcom;
extern	doomdata_t		*netbuffer;		// points inside doomcom

#define	MAXNETNODES		16			// max computers in a game

#define	CMD_SEND	1
#define	CMD_GET		2
#define CMD_FRAG	3

#define	SBARHEIGHT	39			// status bar height at bottom of screen
#define MAXEVENTS	64
#define MINIMUM_HEAP_SIZE	0x800000		//  8 meg
#define MAXIMUM_HEAP_SIZE	0x2000000		// 32 meg

enum { BLEFT, BTOP, BRIGHT, BBOTTOM };

// Networking models.
enum
{
	// Everybody communicates directly with each other.
	NETM_PEER_TO_PEER,		

	// All data goes through the server, otherwise the same as peer-to-peer.
	NETM_PURE_MULTICAST,	

	// Server knows all; answers retransmits. Clients send single packets to
	// the server, which sends them onwards to all other clients.
	NETM_MULTICAST			
};

extern int UpdateState;
extern int haloMode;


extern int myargc;
extern char **myargv;

//==========================================================================
//
//	Guard macros
//
//==========================================================================

#ifdef DO_GUARD
#define guard(name)		static const char *__FUNC_NAME__ = #name; try {
#define unguard			} catch (...) { DD_CoreDump(__FUNC_NAME__); throw; }
#define unguardf(msg)	} catch (...) { DD_CoreDump(__FUNC_NAME__); DD_CoreDump msg; throw; }
#else
#define guard(name)		static const char *__FUNC_NAME__ = #name; {
#define unguard			}
#define unguardf(msg)	}
#endif

#ifdef DO_GUARD_SLOW
#define guardSlow(name)		guard(name)
#define unguardSlow			unguard
#define unguardfSlow(msg)	unguardf(msg)
#else
#define guardSlow(name)		{
#define unguardSlow			}
#define unguardfSlow(msg)	}
#endif

#include "dd_net.h"

//========================================================================
//
// dd_winit.c
//
void DD_Shutdown();


//========================================================================
//
// dd_main.c
//
extern int maxzone;	
extern int shareware;		// true if only episode 1 present
extern boolean cdrom;			// true if cd-rom mode active ("-cdrom")
extern boolean debugmode;		// checkparm of -debug
extern boolean nofullscreen;	// checkparm of -nofullscreen
extern boolean singletics;		// debug flag to cancel adaptiveness
extern FILE *debugfile;
extern event_t events[MAXEVENTS];
extern int eventhead;
extern int eventtail;

void DD_Main();
void DD_PostEvent(event_t *ev);
void DD_ProcessEvents(void);
void DD_GameLoop(void);
void DD_GameUpdate(int flags);
void AddWADFile(char *file);
void DD_SetConfigFile(char *filename);
int DD_GetInteger(int ddvalue);
void DD_SetInteger(int ddvalue, int parm);
ddplayer_t *DD_GetPlayer(int number);
void DD_CoreDump(const char *fmt, ...);
const char *DD_GetCoreDump(void);


//========================================================================
//
// dd_misc.c
//
extern int		rndindex;

int				Argc(void);
char *			Argv(int i);
char **			ArgvPtr(int i);
int				M_CheckParm(char *check);
boolean			M_ParmExists(char *check);
void			M_SaveDefaults();
int				M_ParseCommands(char *fileName, int setdefault);
fixed_t			M_AproxDistance (fixed_t dx, fixed_t dy);
int				M_ReadFile(char const *name, byte **buffer);
int				M_ReadFileCLib(char const *name, byte **buffer);
boolean			M_WriteFile (char const *name, void *source, int length);
void			M_ExtractFileBase(char *path, char *dest);
void			SetRandomTable(unsigned char *table);
unsigned char	P_Random (void);
int				M_Random (void);
void			M_ClearRandom (void);
void			M_ClearBox (fixed_t *box);
void			M_AddToBox (fixed_t *box, fixed_t x, fixed_t y);
int				M_ScreenShot(char *filename, int bits);
void			M_WriteTextEsc(FILE *file, char *text);


//========================================================================
//
// dd_think.c
//
extern thinker_t thinkercap; // both the head and tail of the thinker list

void RunThinkers(void);
void P_InitThinkers(void);
void P_AddThinker(thinker_t *thinker);
void P_RemoveThinker(thinker_t *thinker);


//========================================================================
//
// dd_setup.c
//
//float AccurateDistance(fixed_t dx, fixed_t dy);
void DD_SetupLevel(int flags);
void DD_ValidateLevel(void);

/*void P_LoadBlockMap(int lump);
void P_LoadReject(int lump);


//========================================================================
//
// dd_maputl.c
//
#define	MAXINTERCEPTS	128
extern fixed_t opentop, openbottom, openrange, lowfloor;
extern divline_t trace;

fixed_t P_AproxDistance (fixed_t dx, fixed_t dy);
int P_PointOnLineSide (fixed_t x, fixed_t y, line_t *line);
int P_BoxOnLineSide (fixed_t *tmbox, line_t *ld);
void P_MakeDivline (line_t *li, divline_t *dl);
int P_PointOnDivlineSide (fixed_t x, fixed_t y, divline_t *line);
fixed_t P_InterceptVector (divline_t *v2, divline_t *v1);
void P_LineOpening (line_t *linedef);
void P_LinkThing(mobj_t *thing, byte flags);
void P_UnlinkThing(mobj_t *thing, byte flags);
boolean P_BlockLinesIterator (int x, int y, boolean(*func)(line_t*) );
boolean P_BlockThingsIterator (int x, int y, boolean(*func)(mobj_t*) );
boolean P_PathTraverse (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
	int flags, boolean (*trav) (intercept_t *));


//========================================================================
//
// dd_sight.c
//
extern fixed_t topslope;
extern fixed_t bottomslope;		

boolean P_CheckReject(sector_t *sec1, sector_t *sec2);
boolean P_CheckFrustum(int plrNum, mobj_t *mo);
boolean P_CheckSight(mobj_t *t1, mobj_t *t2);

*/
//========================================================================
//
// dd_bind.c
//
void B_Bind(event_t *event, char *command);
void B_EventConverter(char *buff, event_t *ev, boolean to_event);
int B_BindingsForCommand(char *command, char *buffer);
void B_ClearBinding(char *command);
boolean B_Responder(event_t *ev);
void B_WriteToFile(FILE *file);
void B_Shutdown();


//========================================================================
//
// dd_actn.c
//
void DD_DefineActions(action_t *acts);
void DD_ClearActions(void);
// The command begins with a '+' or a '-'.
// Returns true if the action was changed successfully.
int DD_ActionCommand(char *cmd, boolean has_prefix);


//========================================================================
//
// dd_3dsnd.c
//
void readline(char *buffer, int len, FILE *file);
char *firstchar(char *buffer);
void S_LoadTextureTypes();
void S_FreeTextureTypes();
int S_TextureTypeForName(char *name);
void S_CalcSectorReverbs();


//========================================================================
//
// d_net.c
//
extern int		netgame; // only true if >1 player
extern int		netModel;		// The networking model.
extern int		limboplayer;
extern int		consoleplayer;
extern int		displayplayer;
extern int		gametic, maketic;
extern int		ticdup, server, limbo;
extern boolean	allow_net_traffic;	// Should net traffic be allowed?
extern boolean	map_rendered;		// Has the map been already rendered?
extern ddplayer_t	players[MAXPLAYERS];
extern byte		*netticcmds[MAXPLAYERS];

void D_GetTicCmd(void *cmd, int player);
void D_AllocNetArrays();
void D_StartNetGame();
void D_StopNetGame(boolean closing);
void D_SyncNetStart();
void D_SendCustomPacket(unsigned int playermask, void *data, int length);
void NetUpdate();
void TryRunTics();


//========================================================================
//
// r_main.c
//
extern boolean	setsizeneeded;
extern int		framecount;
extern int		viewwidth, viewheight, viewwindowx, viewwindowy;
extern int		viewangleoffset;

extern  int		skyflatnum;
extern int		Sky1Texture, Sky2Texture;

void R_Init (void);
void R_Update (void);
void R_RenderPlayerView (ddplayer_t *player);
void R_SetViewSize(int x, int y, int w, int h);
angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
subsector_t *R_PointInSubsector (fixed_t x, fixed_t y);


//========================================================================
//
// r_data.c
//
int	R_FlatNumForName (char *name);
int	R_CheckTextureNumForName (char *name);
int	R_TextureNumForName (char *name);
char* R_TextureNameForNum(int num);
int R_SetFlatTranslation(int flat, int translateTo);
int R_SetTextureTranslation(int tex, int translateTo);


//========================================================================
//
// r_things.c
//
extern char	**spritenamelist;

void R_SetSpriteNameList(char **namelist);
void R_GetSpriteInfo(int sprite, int frame, spriteinfo_t *sprinfo);


//========================================================================
//
// r_model.c
//
modelframe_t *R_CheckModelFor(int sprite, int frame);
void R_SetSpriteReplacement(int sprite, char *modelname);


//========================================================================
//
// r_draw.c
//
extern boolean BorderNeedRefresh;
extern boolean BorderTopRefresh;

void R_InitViewBorder();
void R_SetBorderGfx(char *gfx[9]);
void R_DrawViewBorder (void);
void R_DrawTopBorder (void);


//========================================================================
//
// r_model.c
//
extern boolean	useModels;

void R_InitModels(void);



//========================================================================
//
// v_video.c
//
extern byte gammatable[5][256];
extern int usegamma;


//========================================================================
//
// tables.c
//
extern fixed_t finesine[5*FINEANGLES/4];
extern fixed_t *finecosine;


//========================================================================
//
// i_sound.c
//
extern boolean			use_jtSound;

void I_SetMusicDevice(int musdev);
int I_CDControl(int cmd, int parm);

#endif