
//**************************************************************************
//**
//** dd_main.c
//** Author: Jaakko Keränen
//** Integral Doomsday stuff.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <time.h>
#include <string.h>
#include "dd_def.h"
#include "gl_def.h"
#include "console.h"
#include "i_sound.h"
#include "i_win32.h"
#include "i_net.h"
#include "settings.h"

// MACROS ------------------------------------------------------------------

#define MAXWADFILES 30 

// TYPES -------------------------------------------------------------------

typedef struct
{
	int *readPtr;
	int *writePtr;
} ddvalue_t;

typedef struct
{
	char *name;
	void (*func)(char **args, int tag);
	int requiredArgs;
	int tag;
} execOpt_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void R_ExecuteSetViewSize(void);
void G_CheckDemoStatus();
void F_Drawer(void);
boolean F_Responder(event_t *ev);
void I_HideMouse (void);
void S_InitScript(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void DD_DoAdvanceDemo(void);
void DD_AdvanceDemo(void);
void DD_StartTitle(void);
void DD_PageTicker(void);

void AddWADFile(char *file);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void PageDrawer(void);
static void HandleArgs(int state);
static void CheckRecordFrom(void);
static void DrawAndBlit(void);
static void ExecOptionFILE(char **args, int tag);
static void ExecOptionMAXZONE(char **args, int tag);
static void CreateSavePath(void);
static void WarpCheck(void);

#ifdef TIMEBOMB
static void DoTimeBomb(void);
#endif

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean startupScreen;
extern boolean renderTextures;
extern char skyflatname[9];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean DevMaps;			// true = Map development mode
char *DevMapsDir = "";		// development maps directory
int shareware;				// true if only episode 1 present
boolean debugmode;			// checkparm of -debug
boolean nofullscreen;		// checkparm of -nofullscreen
boolean cdrom;				// true if cd-rom mode active
boolean cmdfrag;			// true if a CMD_FRAG packet should be sent out
boolean singletics;			// debug flag to cancel adaptiveness
int maxzone = 0x4000000;	// Maximum allocated for zone heap (64meg default)
boolean autostart;
FILE *debugfile;
event_t events[MAXEVENTS];
int eventhead;
int eventtail;

char *defaultWads = ""; // A list of wad names, whitespace in between (in .cfg).
char configFileName[256];

int queryResult = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int WarpMap;
static int demosequence;
static int pagetic;
static char *pagename;
static char *wadfiles[MAXWADFILES] =
{
	"doomsday.wad"
};

static execOpt_t ExecOptions[] =
{
	{ "-file", ExecOptionFILE, 1, 0 },
	{ "-maxzone", ExecOptionMAXZONE, 1, 0 },
	{ NULL, NULL, 0, 0 } // Terminator
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// DD_Main
//
//==========================================================================
void InitMapMusicInfo(void);

#define ATWSEPS ",; \t"
static void AddToWadList(char *list)
{
	int	i=0;
	int len = strlen(list);
	char *buffer = malloc(len+1), *token;

	// -JL- Paranoia
	if (!buffer)
		I_Error("AddToWadList: malloc failed");
	strcpy(buffer, list);
	token = strtok(buffer, ATWSEPS);
	while(token)
	{
		AddWADFile(token);
		token = strtok(NULL, ATWSEPS);
	}
	free(buffer);
}

void DD_SetConfigFile(char *filename)
{
	strcpy(configFileName, filename);
}

void DD_Main(void)
{
	int p;
	char buff[10];

	strcpy(configFileName, "doomsday.cfg");

	// We need to get the console initialized. Otherwise ST_Message() will
	// crash the system (yikes).
	CON_Init();
	ST_Message("CON_Init: Initializing the console.\n");

//	M_FindResponseFile();
	setbuf(stdout, NULL);
	autostart = false;
	shareware = false; // Always false for Hexen

	HandleArgs(0); // Everything but WADs.

	if(gx.PreInit) gx.PreInit();

	// Initialize subsystems
	I_InitNetwork(); // Network before anything else.

	// Now we can hide the mouse cursor for good.
	   I_HideMouse();

	// Load defaults before initing other systems
	ST_Message("Parsing configuration files.\n");
	// Check for a custom config file.
	if((p=M_CheckParm("-config")) && p < myargc-1)
	{
		// This will override the default config file.
		strcpy(configFileName, myargv[p+1]);
		ST_Message("Custom config file: %s\n", configFileName);
	}
	// This'll be the default config file.
	M_ParseCommands(configFileName, true);
	// Parse additional files.
	if(p = M_CheckParm("-parse") && p < myargc-1)
	{
		while(++p < myargc && myargv[p][0] != '-')
		{
			ST_Message("Parsing: %s\n", myargv[p]);
			M_ParseCommands(myargv[p], false);						
		}
	}

	if(defaultWads) AddToWadList(defaultWads); // These must take precedence.

	HandleArgs(1); // Only the WADs.

	I_InitGraphics();

	ST_Message("W_Init: Init WADfiles.\n");
	W_InitMultipleFiles(wadfiles);

	ST_Message("Z_Init: Init zone memory allocation daemon.\n");
	Z_Init();

	// Start printing messages in the startup.
	ST_Init();
	ST_Message("ST_Init: Init startup screen.\n");

	if(M_CheckParm("-nowsk")) // No Windows system keys?
	{
		// Disable Alt-Tab, Alt-Esc, Ctrl-Alt-Del.
		// A bit of a hack, I'm afraid...
		SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, TRUE, 0, 0);
		ST_Message("Windows system keys disabled.\n");
	}

	p = M_CheckParm("-dumplump");
	if(p && p < myargc-1)
	{
		char fname[100];
		FILE *file;
		int lump = W_GetNumForName(myargv[p+1]);
		byte *lumpPtr = W_CacheLumpNum(lump, PU_STATIC);

		sprintf(fname, "%s.dum", myargv[p+1]);
		file = fopen(fname, "wb");
		if(!file) I_Error("Couldn't open %s for writing. %s\n", fname, strerror(errno));
		fwrite(lumpPtr, 1, lumpinfo[lump].size, file);
		fclose(file);
		I_Error("%s dumped to %s.\n", myargv[p+1], fname);
	}

	if(M_CheckParm("-dumpwaddir"))
	{
		printf("Lumps (%d total):\n",numlumps);
		for(p=0; p<numlumps; p++)
		{
			strncpy(buff,lumpinfo[p].name,8);
			buff[8] = 0;
			printf("%04d - %-8s (hndl: %d, pos: %d, size: %d)\n",
				p, buff, lumpinfo[p].handle, lumpinfo[p].position, lumpinfo[p].size);
		}
		I_Error("---End of lumps---\n");
	}

	ST_Message("I_Init: Setting up machine state.\n");
	I_Init();

	// Show version message now, so it's visible during R_Init()
	ST_Message("Executable: "DOOMSDAY_VERSIONTEXT".\n");

	ST_Message("R_Init: Init the refresh daemon");
	R_Init();
	ST_Message("\n");

//	if (M_CheckParm("-net")) ST_NetProgress();	// Console player found

	ST_Message("D_CheckNetGame: Initializing game data.\n");
	D_CheckNetGame();

	if(gx.PostInit) gx.PostInit();

	// Load the texture materials list (for reverb).
	S_LoadTextureTypes();

	// Try to load the autoexec file. This is done here to make sure
	// everything is initialized: the user can do here anything that
	// s/he'd be able to do in the game.
	M_ParseCommands("autoexec.cfg", false);

	DD_GameLoop(); // Never returns
}

//==========================================================================
//
// HandleArgs
//
//==========================================================================

static void HandleArgs(int state)
{
	int p;
	execOpt_t *opt;

	if(state == 0)
	{
		debugmode = M_ParmExists("-debug");
		nofullscreen = M_ParmExists("-nofullscreen");
		renderTextures = !M_ParmExists("-notex");
	}

	// Process command line options
	for(opt = ExecOptions; opt->name != NULL; opt++)
	{
		if(!state && !strcmp(opt->name, "-file")) continue; // No wads yet.
		p = M_CheckParm(opt->name);
		if(p && p < myargc-opt->requiredArgs)
		{
			opt->func(&myargv[p], opt->tag);
		}
	}
}


//==========================================================================
//
// ExecOptionFILE
//
//==========================================================================

static void ExecOptionFILE(char **args, int tag)
{
	int p;

	p = M_CheckParm("-file");
	while(++p != myargc && myargv[p][0] != '-')
	{
		AddWADFile(myargv[p]);
	}
}


long superatol(char *s)
{
/*	long int n=0, r=10, x, mul=1;
	char *c=s;

	for (; *c; c++)
	{
		x = (*c & 223) - 16;

		if (x == -3)
		{
			mul = -mul;
		}
		else if (x == 72 && r == 10)
		{
			n -= (r=n);
			if (!r) r=16;
			if (r<2 || r>36) return -1;
		}
		else
		{
			if (x>10) x-=39;
			if (x >= r) return -1;
			n = (n*r) + x;
		}
	}
	return(mul*n);*/


	char *endptr;
	long val = strtol(s, &endptr, 0);

	if(*endptr == 'k' || *endptr == 'K')
		val *= 1024;
	else if(*endptr == 'm' || *endptr == 'M')
		val *= 1048576;
	return val;
}


static void ExecOptionMAXZONE(char **args, int tag)
{
	int size;
	
	size = superatol(args[1]);
	if (size < MINIMUM_HEAP_SIZE) size = MINIMUM_HEAP_SIZE;
	if (size > MAXIMUM_HEAP_SIZE) size = MAXIMUM_HEAP_SIZE;
	maxzone = size;
}

//==========================================================================
//
// DD_GameLoop
//
//==========================================================================

void DD_GameLoop(void)
{
	MSG msg;

	// Now we've surely finished startup.
	ST_Done();

	if(M_CheckParm("-debugfile"))
	{
		char filename[20];
		sprintf(filename, "debug%i.txt", consoleplayer);
		debugfile = fopen(filename,"w");
	}
	while(1)
	{
		// Start by checking the messages. This is the message pump.
		// Could be in a separate thread?
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Frame syncronous IO operations
		I_StartFrame();

		// Process one or more tics
		if(singletics)
		{
			I_StartTic();
			DD_ProcessEvents();
			
			gx.BuildTiccmd(&netticcmds[consoleplayer][TICCMD_IDX(maketic%BACKUPTICS)]);

			gx.G_Ticker();
			gx.MN_Ticker();
			CON_Ticker();

			gametic++;
			maketic++;
			systics++;
		}
		else
		{
			// Will run at least one tic
			TryRunTics();
		}

/*#ifdef USEA3D
		// Update the listener.
		I_UpdateListener(players+displayplayer);
#endif
		// Move positional sounds.
		S_UpdateSounds(players[displayplayer].mo);
*/
		I_UpdateCDMusic();
		I_EndFrame();		

		DrawAndBlit();
	}
}

//==========================================================================
//
// DD_ProcessEvents
//
// Send all the events of the given timestamp down the responder chain.
//
//==========================================================================

void DD_ProcessEvents(void)
{
	event_t *ev;

	for(; eventtail != eventhead; eventtail = (++eventtail)&(MAXEVENTS-1))
	{
		ev = &events[eventtail];

		// Does the special responder use this event?
		if(gx.PrivilegedResponder)
			if(gx.PrivilegedResponder(ev)) continue;

		// The console.
		if(CON_Responder(ev)) continue;
		// The menu.
		if(gx.MN_Responder(ev)) continue;
		// The game responder only returns true if the bindings 
		// can't be used (like when chatting).
		if(gx.G_Responder(ev)) continue;

		// The bindings responder.
		B_Responder(ev);
	}
}

//==========================================================================
//
// DD_PostEvent
//
// Called by the I/O functions when input is detected.
//
//==========================================================================

void DD_PostEvent(event_t *ev)
{
	events[eventhead] = *ev;
	eventhead = (++eventhead)&(MAXEVENTS-1);
}

//==========================================================================
//
// DrawAndBlit
//
//==========================================================================

static void DrawAndBlit(void)
{
	// Draw the game graphics.
	gx.G_Drawer();
	// The colored filter. 
	if(GL_DrawFilter()) BorderNeedRefresh = true;
	// Draw Menu
	gx.MN_Drawer();
	// Draw console.
	CON_Drawer();

	// Send out any new accumulation
	NetUpdate();

	// Flush buffered stuff to screen (blits everything).
	I_Update();
}

//==========================================================================
//
// AddWADFile
//
//==========================================================================

void AddWADFile(char *file)
{
	int i;
	char *new;

	ST_Message("Adding external file: %s\n", file);
	i = 0;
	while(wadfiles[i]) i++;
	new = malloc(strlen(file)+1);
	// -JL- Paranoia
	if (!new)
		I_Error("AddWADFile: malloc failed");
	strcpy(new, file);
	wadfiles[i] = new;
}

//==========================================================================
//
// FixedDiv
//
//==========================================================================

/*fixed_t FixedDiv(fixed_t a, fixed_t b)
{
	if((abs(a)>>14) >= abs(b))
	{
		return((a^b)<0 ? DDMININT : DDMAXINT);
	}
	return(FixedDiv2(a, b));
}*/

void DD_GameUpdate(int flags)
{
	if(flags & DDUF_BORDER) BorderNeedRefresh = true;
	if(flags & DDUF_TOP) BorderTopRefresh = true;
	if(flags & DDUF_FULLVIEW) UpdateState |= I_FULLVIEW;
	if(flags & DDUF_STATBAR) UpdateState |= I_STATBAR;
	if(flags & DDUF_MESSAGES) UpdateState |= I_MESSAGES;
	if(flags & DDUF_FULLSCREEN) UpdateState |= I_FULLSCRN;

	if(flags & DDUF_UPDATE) I_Update();
}

// Queries are a way to extend the API without adding new functions.
void DD_CheckQuery(int query, int parm)
{
	int					i;
	jtnetserver_t		*buf;
	serverdataquery_t	*sdq;
	patch_t				*patch;
	modemdataquery_t	*mdq;

	switch(query)
	{
	case DD_TEXTURE_HEIGHT_QUERY:
		queryResult = textureheight[parm];
		break;

	case DD_NET_QUERY:
		switch(parm)
		{
		case DD_PROTOCOL:
			queryResult = (int) I_GetProtocolName();
			break;

		case DD_NUM_SERVERS:
			queryResult = jtNetGetServerInfo(NULL, 0);
			if(queryResult < 0) queryResult = 0;
			break;

		case DD_MODEM:
			queryResult = jtNetGetInteger(JTNET_MODEM);
			break;

		case DD_PHONE_NUMBER:
			queryResult = (int) jtNetGetString(JTNET_PHONE_NUMBER);
			break;

		case DD_TCPIP_ADDRESS:
			queryResult = (int) jtNetGetString(JTNET_TCPIP_ADDRESS);
			break;

		case DD_TCPIP_PORT:
			queryResult = jtNetGetInteger(JTNET_TCPIP_PORT);
			break;

		case DD_COM_PORT:
			queryResult = jtNetGetInteger(JTNET_COMPORT);
			break;

		case DD_BAUD_RATE:
			queryResult = jtNetGetInteger(JTNET_BAUDRATE);
			break;

		case DD_STOP_BITS:
			queryResult = jtNetGetInteger(JTNET_STOPBITS);
			break;

		case DD_PARITY:
			queryResult = jtNetGetInteger(JTNET_PARITY);
			break;

		case DD_FLOW_CONTROL:
			queryResult = jtNetGetInteger(JTNET_FLOWCONTROL);
			break;
		}
		break;

	case DD_SERVER_DATA_QUERY:
		sdq = (serverdataquery_t*) parm;
		i = jtNetGetServerInfo(NULL, 0);
		if(i < 0) i = 0;
		sdq->found = 0;
		if(i)	// Something was found?
		{
			buf = malloc(sizeof(jtnetserver_t) * sdq->num);
			// -JL- Paranoia
			if (!buf)
				I_Error("DD_CheckQuery: malloc failed");
			sdq->found = jtNetGetServerInfo(buf, sdq->num);
			// Copy the data to the array given by the caller.
			for(i=0; i<sdq->found; i++)
			{
				strcpy(sdq->data[i].name, buf[i].name);
				strcpy(sdq->data[i].description, buf[i].description);
				sdq->data[i].players = buf[i].players;
				sdq->data[i].maxPlayers = buf[i].maxPlayers;
				sdq->data[i].canJoin = buf[i].canJoin;
				memcpy(sdq->data[i].data, buf[i].data, sizeof(sdq->data[i].data));
			}
			free(buf);
		}
		break;

	case DD_SPRITE_SIZE_QUERY:
		patch = W_CacheLumpNum(firstspritelump + parm, PU_CACHE);
		queryResult = (patch->width << 16) + patch->height;
		break;

	case DD_NUM_SPRITE_FRAMES_QUERY:
		queryResult = sprites[parm].numframes;
		break;

	case DD_MODEM_DATA_QUERY:
		mdq = (modemdataquery_t*) parm;
		mdq->list = jtNetGetStringList(JTNET_MODEM_LIST, &mdq->num);
		break;

	default:
		break;
	}
}

ddvalue_t ddValues[DD_LAST_VALUE-DD_FIRST_VALUE-1] =
{
	{ &screenWidth,		0 },
	{ &screenHeight,	0 },
	{ &netgame,			0 }, 
	{ &server,			0 },
	{ &ticdup,			0 },
	{ &skyflatnum,		0 },
	{ &numflats,		0 },
	{ &firstflat,		0 },
	{ &gametic,			0 },
	{ &maketic,			0 },
	{ &viewwindowx,		&viewwindowx },
	{ &viewwindowy,		&viewwindowy },
	{ &viewwidth,		&viewwidth },
	{ &viewheight,		&viewheight },
	{ &viewpw,			0 },
	{ &viewph,			0 },
	{ &viewx,			&viewx },
	{ &viewy,			&viewy },
	{ &viewz,			&viewz },
	{ &viewxOffset,		&viewxOffset },
	{ &viewyOffset,		&viewyOffset },
	{ &viewzOffset,		&viewzOffset },
	{ &viewangle,		&viewangle },
	{ &viewangleoffset,	&viewangleoffset },
	{ &consoleplayer,	&consoleplayer },
	{ &displayplayer,	&displayplayer },
	{ &snd_MusicDevice, 0 },
	{ &mipmapping,		0 },
	{ &linearRaw,		0 },
	{ &defResX,			&defResX },
	{ &defResY,			&defResY },
	{ &skyDetail,		0 },
	{ &snd_SfxVolume,	&snd_SfxVolume },
	{ &snd_MusicVolume,	0 },
	{ &mouseInverseY,	&mouseInverseY },
	{ &usegamma,		0 },
	{ &queryResult,		0 },
	{ &LevelFullBright,	&LevelFullBright },
	{ &CmdReturnValue,	0 },
	{ &limbo,			0 },
	{ &numspritelumps,	0 },
	{ &firstspritelump,	0 },
	{ &act_int,	&act_int }
};

int DD_GetInteger(int ddvalue)
{
	if(ddvalue <= DD_FIRST_VALUE || ddvalue >= DD_LAST_VALUE) 
	{
		// How about some specials?
		switch(ddvalue)
		{
/*		case DD_TRACE_ADDRESS:
			return (int) &trace;*/

		case DD_TRANSLATIONTABLES_ADDRESS:
			return (int) translationtables;
		}
		return 0;
	}
	if(ddValues[ddvalue].readPtr == NULL) return 0;
	return *ddValues[ddvalue].readPtr;
}

void DD_SetInteger(int ddvalue, int parm)
{
	if(ddvalue <= DD_FIRST_VALUE || ddvalue >= DD_LAST_VALUE) 
	{
		DD_CheckQuery(ddvalue, parm);
		// How about some special values?
		if(ddvalue == DD_SKYFLAT_NAME)
		{
			memset(skyflatname, 0, 9);
			strncpy(skyflatname, (char*) parm, 9);
		}
		else if(ddvalue == DD_TRANSLATED_SPRITE_TEXTURE)
		{
			// See DD_TSPR_PARM in dd_share.h.
			int lump = parm & 0xffffff, cls = (parm>>24) & 0xf, table = (parm>>28) & 0xf;
			if(table)
				GL_SetTranslatedSprite(lump, 
					translationtables-256 + cls*((MAXPLAYERS-1)*256) + (table<<8));
			else
				GL_SetSprite(lump);
		}
/*		else if(ddvalue == DD_TEXTURE_GLOW)
		{
			// See DD_TGLOW_PARM in dd_share.h.
			int tnum = parm & 0xffff, istex = (parm & 0x80000000) != 0,
				glowstate = (parm & 0x10000) != 0;
			if(istex)
			{
				if(glowstate)
					textures[tnum]->flags |= TXF_GLOW;
				else
					textures[tnum]->flags &= ~TXF_GLOW;
			}
			else
			{
				if(glowstate)
					flatflags[tnum] |= TXF_GLOW;
				else 
					flatflags[tnum] &= ~TXF_GLOW;
			}
		}*/
		else if(ddvalue == DD_SPRITE_REPLACEMENT)
		{
			spritereplacement_t *srep = (spritereplacement_t*) parm;
			R_SetSpriteReplacement(srep->spritenum, srep->modelname);
		}
		return;
	}
	if(ddValues[ddvalue].writePtr)
		*ddValues[ddvalue].writePtr = parm;
}

ddplayer_t *DD_GetPlayer(int number)
{
	return (ddplayer_t*) &players[number];
}