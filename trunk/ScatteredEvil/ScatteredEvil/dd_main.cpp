
//**************************************************************************
//**
//** dd_main.c
//** Author: Jaakko Keränen
//** Integral Doomsday stuff.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdarg.h>
#include "h2def.h"
#include "gl_def.h"
#include "console.h"
#include "i_sound.h"
#include "i_win32.h"

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
void I_DisableWindowsKeys(void);

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

boolean debugmode;			// checkparm of -debug
boolean nofullscreen;		// checkparm of -nofullscreen
boolean singletics;			// debug flag to cancel adaptiveness
int maxzone = 0x4000000;	// Maximum allocated for zone heap (64meg default)
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
static char *wadfiles[MAXWADFILES];

static execOpt_t ExecOptions[] =
{
	{ "-file", ExecOptionFILE, 1, 0 },
	{ "-maxzone", ExecOptionMAXZONE, 1, 0 },
	{ NULL, NULL, 0, 0 } // Terminator
};

static char *error_string;

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
	char *buffer = (char *)malloc(len+1), *token;

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

void DD_Main(void)
{
	guard(DD_Main);
	int p;
	char buff[10];

	strcpy(configFileName, "ScatteredEvil.cfg");

	// We need to get the console initialized. Otherwise ST_Message() will
	// crash the system (yikes).
	CON_Init();
	ST_Message("CON_Init: Initializing the console.\n");

//	M_FindResponseFile();
	setbuf(stdout, NULL);

	HandleArgs(0); // Everything but WADs.

	H2_PreInit();

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
		I_DisableWindowsKeys();
		ST_Message("Windows system keys disabled.\n");
	}

	p = M_CheckParm("-dumplump");
	if(p && p < myargc-1)
	{
		char fname[100];
		FILE *file;
		int lump = W_GetNumForName(myargv[p+1]);
		byte *lumpPtr = (byte *)W_CacheLumpNum(lump, PU_STATIC);

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
	ST_Message("Executable: "VERSIONTEXT".\n");

	ST_Message("R_Init: Init the refresh daemon");
	R_Init();
	ST_Message("\n");

//	if (M_CheckParm("-net")) ST_NetProgress();	// Console player found

	ST_Message("D_CheckNetGame: Initializing game data.\n");
	D_CheckNetGame();

	H2_PostInit();

	// Load the texture materials list (for reverb).
	S_LoadTextureTypes();

	// Try to load the autoexec file. This is done here to make sure
	// everything is initialized: the user can do here anything that
	// s/he'd be able to do in the game.
	M_ParseCommands("autoexec.cfg", false);

	DD_GameLoop(); // Never returns
	unguard;
}

//==========================================================================
//
// HandleArgs
//
//==========================================================================

static void HandleArgs(int state)
{
	guard(HandleArgs);
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
	unguard;
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
	guard(DD_GameLoop);

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
		// Frame syncronous IO operations
		I_StartFrame();

		// Process one or more tics
		if(singletics)
		{
			I_StartTic();
			DD_ProcessEvents();
			
			G_BuildTiccmd(&netcmds[consoleplayer][maketic%BACKUPTICS]);

			G_Ticker();
			H2_Ticker();
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
	unguard;
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
	guard(DD_ProcessEvents);
	event_t *ev;

	for(; eventtail != eventhead; eventtail = (++eventtail)&(MAXEVENTS-1))
	{
		ev = &events[eventtail];

		// Does the special responder use this event?
		if(H2_PrivilegedResponder(ev)) continue;

		// The console.
		if(CON_Responder(ev)) continue;
		// The menu.
		if(MN_Responder(ev)) continue;
		// The game responder only returns true if the bindings 
		// can't be used (like when chatting).
		if(G_Responder(ev)) continue;

		// The bindings responder.
		B_Responder(ev);
	}
	unguard;
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
	guard(DD_PostEvent);
	events[eventhead] = *ev;
	eventhead = (++eventhead)&(MAXEVENTS-1);
	unguard;
}

//==========================================================================
//
// DrawAndBlit
//
//==========================================================================

static void DrawAndBlit(void)
{
	guard(DrawAndBlit);
	// Draw the game graphics.
	G_Drawer();
	// The colored filter. 
	if(GL_DrawFilter()) BorderNeedRefresh = true;
	// Draw Menu
	MN_Drawer();
	// Draw console.
	CON_Drawer();

	// Send out any new accumulation
	NetUpdate();

	// Flush buffered stuff to screen (blits everything).
	I_Update();
	unguard;
}

//==========================================================================
//
// AddWADFile
//
//==========================================================================

void AddWADFile(char *file)
{
	guard(AddWADFile);
	int i;
	char *newwad;

	ST_Message("Adding external file: %s\n", file);
	i = 0;
	while(wadfiles[i]) i++;
	newwad = (char *)malloc(strlen(file)+1);
	// -JL- Paranoia
	if (!newwad)
		I_Error("AddWADFile: malloc failed");
	strcpy(newwad, file);
	wadfiles[i] = newwad;
	unguard;
}

void DD_GameUpdate(int flags)
{
	guardSlow(DD_GameUpdate);
	if(flags & DDUF_BORDER) BorderNeedRefresh = true;
	if(flags & DDUF_TOP) BorderTopRefresh = true;
	if(flags & DDUF_FULLVIEW) UpdateState |= I_FULLVIEW;
	if(flags & DDUF_STATBAR) UpdateState |= I_STATBAR;
	if(flags & DDUF_MESSAGES) UpdateState |= I_MESSAGES;
	if(flags & DDUF_FULLSCREEN) UpdateState |= I_FULLSCRN;

	if(flags & DDUF_UPDATE) I_Update();
	unguardSlow;
}

//==========================================================================
//
//	DD_CoreDump
//
//==========================================================================

void DD_CoreDump(const char *fmt, ...)
{
	static bool first = true;

	if (!error_string)
	{
		error_string = new char[32];
		strcpy(error_string, "Stack trace: ");
		first = true;
	}

	va_list argptr;
	char string[1024];
	
	va_start(argptr, fmt);
	vsprintf(string, fmt, argptr);
	va_end(argptr);

	printf("- %s\n", string);

	char *new_string = new char[strlen(error_string) + strlen(string) + 6];
	strcpy(new_string, error_string);
	if (first)
		first = false;
	else
		strcat(new_string, " <- ");
	strcat(new_string, string);
	delete error_string;
	error_string = new_string;
}

const char *DD_GetCoreDump(void)
{
	return error_string ? error_string : "";
}
