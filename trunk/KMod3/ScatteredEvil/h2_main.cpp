
//**************************************************************************
//**
//** h2_main.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <time.h>
#include <string.h>
#include "h2def.h"
#include "p_local.h"
#include "soundst.h"
#include "Settings.h"
#include "h2_actn.h"
#include "h2_net.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef struct
{
	char *name;
	void (*func)(char **args, int tag);
	int requiredArgs;
	int tag;
} execOpt_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void S_InitScript(void);
void H2_ConsoleRegistration();
void H2_DefaultBindings();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void H2_ProcessEvents(void);
void H2_DoAdvanceDemo(void);
void H2_AdvanceDemo(void);
void H2_StartTitle(void);
void H2_PageTicker(void);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void HandleArgs();
static boolean CheckRecordFrom(void);
static void DrawAndBlit(void);
static void ExecOptionSCRIPTS(char **args, int tag);
static void ExecOptionDEVMAPS(char **args, int tag);
static void ExecOptionSKILL(char **args, int tag);
static void ExecOptionPLAYDEMO(char **args, int tag);
static void CreateSavePath(void);
static void WarpCheck(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern char *SavePath;
extern boolean startupScreen;
extern int demosequence;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean DevMaps;			// true = Map development mode
char *DevMapsDir = "";		// development maps directory
boolean nomonsters;			// checkparm of -nomonsters
boolean respawnparm;		// checkparm of -respawn
boolean randomclass;		// checkparm of -randclass
boolean ravpic;				// checkparm of -ravpic
//boolean singletics;			// debug flag to cancel adaptiveness
boolean artiskip;			// whether shift-enter skips an artifact
boolean netcheat;			// allow cheating in netgames (-netcheat)
boolean dontrender;			// don't render the player view (debug)
skill_t startskill;
int startepisode;
int startmap;

// Network games parameters.
byte netMap=1, netClass=0, netColor=0, netSkill=2, netSlot=0; // netSlot zero is 'no slot'
byte netDeathmatch=1, netNomonsters=1, netRandomclass=0, netRespawn=0;
byte netMobDamageModifier = 1;
byte netMobHealthModifier = 1;

boolean autostart;
boolean advancedemo;
int screenblocks=10;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int WarpMap;

static execOpt_t ExecOptions[] =
{
	{ "-scripts", ExecOptionSCRIPTS, 1, 0 },
	{ "-devmaps", ExecOptionDEVMAPS, 1, 0 },
	{ "-skill", ExecOptionSKILL, 1, 0 },
	{ "-playdemo", ExecOptionPLAYDEMO, 1, 0 },
	{ "-timedemo", ExecOptionPLAYDEMO, 1, 0 },
	{ NULL, NULL, 0, 0 } // Terminator
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// H2_Main
//
//==========================================================================
void InitMapMusicInfo(void);

char *borderLumps[] =
{
	"F_022",	// background
	"bordt",	// top
	"bordr",	// right
	"bordb",	// bottom
	"bordl",	// left
	"bordtl",	// top left
	"bordtr",	// top right
	"bordbr",	// bottom right
	"bordbl"	// bottom left
};

void H2_PreInit(void)
{
	R_SetSpriteNameList(sprnames);
	R_SetBorderGfx(borderLumps);
	// Add the JHexen cvars and ccmds to the console databases.
	H2_ConsoleRegistration();

	// The startup WADs.
	AddWADFile("hexen.wad");
	AddWADFile("KMod3.wad");
/*	AddWADFile("Red_ch.wad");
	AddWADFile("mtncent.wad");
	AddWADFile("Reddemon.wad");
	AddWADFile("wendi.wad");
	AddWADFile("journal.wad");
	AddWADFile("Shdw.wad");*/

	startepisode = 1;
	startskill = sk_medium;
	startmap = 1;

	HandleArgs();
}

void H2_PostInit(void)
{
	int			pClass, p;

	ST_Message("--- Scattered Evil Init ---\n%s\n", VERSIONTEXT);

	H2_DefaultBindings();
//	R_LoadTextureTypes();

	// Check the -class argument.
	pClass = PCLASS_FIGHTER;
	if(p = M_CheckParm("-class"))
	{
		pClass = atoi(Argv(p+1));
		if(pClass > PCLASS_CORVUS || pClass < PCLASS_FIGHTER)
		{
			I_Error("Invalid player class: %d\n", pClass);
		}
		ST_Message("\nPlayer Class: %d\n", pClass);
	}
	PlayerClass[consoleplayer] = (pclass_t)pClass;

	// Init the view.
	R_SetViewSize(screenblocks);

	// Set sprite/model replacements.
	for (p=0; p<NUMSPRITES; p++)
	{
		R_SetSpriteReplacement(p, sprnames[p]);
	}


	CreateSavePath();
	
	ST_Message("S_Init...\n");
	S_Init();

	ST_Message("P_Init: Init Playloop state.\n");
	P_Init();
	
	ST_Message("MN_Init: Init menu system.\n");
	MN_Init();

	ST_Message("CT_Init: Init chat mode data.\n");
	CT_Init();

	InitMapMusicInfo();		// Init music fields in mapinfo

	ST_Message("S_InitScript\n");
	S_InitScript();

	ST_Message("SN_InitSequenceScript: Registering sound sequences.\n");
	SN_InitSequenceScript();

	// Check for command line warping. Follows P_Init() because the
	// MAPINFO.TXT script must be already processed.
	WarpCheck();

	if(autostart)
	{
		ST_Message("Warp to Map %d (\"%s\":%d), Skill %d\n",
			WarpMap, P_GetMapName(startmap), startmap, startskill+1);
	}

	ST_Message("SB_Init: Loading patches.\n");
	SB_Init();

	if(CheckRecordFrom()) return;

	p = M_CheckParm("-record");
	if(p && p < Argc()-1)
	{
		singledemo = true;	// Quit after recording.
		G_RecordDemo(startskill, 1, startepisode, startmap, Argv(p+1));
		return;
	}

	p = M_CheckParm("-playdemo");
	if(p && p < Argc()-1)
	{
		singledemo = true; // Quit after one demo
		G_DeferedPlayDemo(Argv(p+1));
		return;
	}

	p = M_CheckParm("-timedemo");
	if(p && p < Argc()-1)
	{
		singledemo = true; // Quit after timing
		G_TimeDemo(Argv(p+1));
		return;
	}

	p = M_CheckParm("-loadgame");
	if(p && p < Argc()-1)
	{
		G_LoadGame(atoi(Argv(p+1)));
	}

	if(gameaction != ga_loadgame)
	{
		DD_GameUpdate(DDUF_FULLSCREEN | DDUF_BORDER);
		if(autostart || netgame)
		{
			G_StartNewInit();
			G_InitNew(startskill, startepisode, startmap);
		}
		else
		{
			H2_StartTitle();
		}
	}
}

//==========================================================================
//
// HandleArgs
//
//==========================================================================

static void HandleArgs()
{
	int p;
	execOpt_t *opt;

	nomonsters = M_ParmExists("-nomonsters");
	respawnparm = M_ParmExists("-respawn");
	randomclass = M_ParmExists("-randclass");
	ravpic = M_ParmExists("-ravpic");
	artiskip = M_ParmExists("-artiskip");
	netDeathmatch = M_ParmExists("-deathmatch");
	netcheat = M_ParmExists("-netcheat");
	dontrender = M_ParmExists("-noview");
	
	// Process command line options
	for(opt = ExecOptions; opt->name != NULL; opt++)
	{
		p = M_CheckParm(opt->name);
		if(p && p < Argc()-opt->requiredArgs)
		{
			opt->func(ArgvPtr(p), opt->tag);
		}
	}
}

//==========================================================================
//
// WarpCheck
//
//==========================================================================

static void WarpCheck(void)
{
	int p;
	int map;

	p = M_CheckParm("-warp");
	if(p && p < Argc()-1)
	{
		WarpMap = atoi(Argv(p+1));
		map = P_TranslateMap(WarpMap);
		if(map == -1)
		{ // Couldn't find real map number
			startmap = 1;
			ST_Message("-WARP: Invalid map number.\n");
		}
		else
		{ // Found a valid startmap
			startmap = map;
			autostart = true;
		}
	}
	else
	{
		WarpMap = 1;
		startmap = P_TranslateMap(1);
		if(startmap == -1)
		{
			startmap = 1;
		}
	}
}

//==========================================================================
//
// ExecOptionSKILL
//
//==========================================================================

static void ExecOptionSKILL(char **args, int tag)
{
	startskill = (skill_t)(args[1][0]-'1');
	autostart = true;
}


//==========================================================================
//
// ExecOptionPLAYDEMO
//
//==========================================================================

static void ExecOptionPLAYDEMO(char **args, int tag)
{
	char file[256];

	sprintf(file, "%s.lmp", args[1]);
	AddWADFile(file);
	ST_Message("Playing demo %s.lmp.\n", args[1]);
}

//==========================================================================
//
// ExecOptionSCRIPTS
//
//==========================================================================

static void ExecOptionSCRIPTS(char **args, int tag)
{
	sc_FileScripts = true;
	sc_ScriptsDir = args[1];
}

//==========================================================================
//
// ExecOptionDEVMAPS
//
//==========================================================================

static void ExecOptionDEVMAPS(char **args, int tag)
{
	DevMaps = true;
	ST_Message("Map development mode enabled:\n");
	ST_Message("[config    ] = %s\n", args[1]);
	SC_OpenFileCLib(args[1]);
	SC_MustGetStringName("mapsdir");
	SC_MustGetString();
	ST_Message("[mapsdir   ] = %s\n", sc_String);
	DevMapsDir = (char *)malloc(strlen(sc_String)+1);
	strcpy(DevMapsDir, sc_String);
	SC_MustGetStringName("scriptsdir");
	SC_MustGetString();
	ST_Message("[scriptsdir] = %s\n", sc_String);
	sc_FileScripts = true;
	sc_ScriptsDir = (char *)malloc(strlen(sc_String)+1);
	strcpy(sc_ScriptsDir, sc_String);
	while(SC_GetString())
	{
		if(SC_Compare("file"))
		{
			SC_MustGetString();
			AddWADFile(sc_String);
		}
		else
		{
			SC_ScriptError(NULL);
		}
	}
	SC_Close();
}


//==========================================================================
//
// H2_AdvanceDemo
//
// Called after each demo or intro demosequence finishes.
//
//==========================================================================

void H2_AdvanceDemo(void)
{
	advancedemo = true;
}

//==========================================================================
//
// H2_StartTitle
//
//==========================================================================

void H2_StartTitle(void)
{
	gameaction = ga_nothing;
	demosequence = -1;
	H2_AdvanceDemo();
}

//==========================================================================
//
// CheckRecordFrom
//
// -recordfrom <savegame num> <demoname>
//
//==========================================================================

static boolean CheckRecordFrom(void)
{
	int p;

	p = M_CheckParm("-recordfrom");
	if(!p || p > Argc()-2)
	{ // Bad args
		return false;
	}
	G_LoadGame(atoi(Argv(p+1)));
	G_DoLoadGame(); // Load the gameskill etc info from savegame
	G_RecordDemo(gameskill, 1, gameepisode, gamemap, Argv(p+2));
	return true;
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


//==========================================================================
//
// CreateSavePath
//
//==========================================================================

static void CreateSavePath(void)
{
	char creationPath[121];
	int len;

	len = strlen(SavePath);
	if (len >= 120) I_Error("Save path too long\n");
	strcpy(creationPath, SavePath);

	creationPath[len-1] = 0;
	_mkdir(creationPath);
}

void H2_Ticker(void)
{
	if(advancedemo) H2_DoAdvanceDemo();
	MN_Ticker();
}

void H2_UpdateState(int step)
{
	if(step == DD_PRE)
	{
		// Do a sound reset.
		S_Reset();
	}
	else if(step == DD_POST)
	{
		P_Init();
		SB_Init(); // Updates the status bar patches.
		MN_Init();
		S_InitScript();
		SN_InitSequenceScript();
	}
}


void H2_Shutdown(void)
{
//	R_FreeTextypeTypes();
}
