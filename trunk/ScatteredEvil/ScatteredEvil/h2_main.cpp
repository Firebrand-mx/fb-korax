
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

void R_ExecuteSetViewSize(void);
void G_BuildTiccmd(ticcmd_t *cmd);
void F_Drawer(void);
void I_HideMouse (void);
void S_InitScript(void);
void G_Drawer(void);
void H2_ConsoleBg(int *width, int *height);
void H2_EndFrame(void);
int H2_PrivilegedResponder(event_t *event);
void R_DrawPlayerSprites(ddplayer_t *viewplr);
void H2_ConsoleRegistration();
void H2_DefaultBindings();
/*void SB_HandleCheatNotification(int fromplayer, void *data, int length);*/

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

#ifdef TIMEBOMB
static void DoTimeBomb(void);
#endif

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern char *SavePath;
extern boolean startupScreen;
extern int demosequence;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

game_export_t	gx;
game_import_t	gi;
gl_export_t		gl;


boolean DevMaps;			// true = Map development mode
char *DevMapsDir = "";		// development maps directory
boolean shareware;			// true if only episode 1 present
boolean nomonsters;			// checkparm of -nomonsters
boolean respawnparm;		// checkparm of -respawn
boolean randomclass;		// checkparm of -randclass
boolean debugmode;			// checkparm of -debug
boolean ravpic;				// checkparm of -ravpic
boolean nofullscreen;		// checkparm of -nofullscreen
boolean cdrom;				// true if cd-rom mode active
boolean cmdfrag;			// true if a CMD_FRAG packet should be sent out
boolean singletics;			// debug flag to cancel adaptiveness
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
FILE *debugfile;
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

#ifdef TIC_DEBUG
FILE *rndDebugfile;
#endif

void H2_PreInit(void)
{
	int		i;

#ifdef TIC_DEBUG	
	rndDebugfile = fopen("rndtrace.txt", "wt");
#endif

	if(gi.version < DOOMSDAY_VERSION) 
		gi.Error("Scattered Evil requires at least Doomsday "DOOMSDAY_VERSION_TEXT"!\n");

	// Setup the players.
	for(i=0; i<MAXPLAYERS; i++)
	{
		players[i].plr = gi.GetPlayer(i);
		players[i].plr->extradata = (void*) &players[i];
	}
	gi.SetSpriteNameList(sprnames);
	gi.SetConfigFile("korax.cfg");
	gi.SetBorderGfx(borderLumps);
	gi.DefineActions(actions);
	// Add the JHexen cvars and ccmds to the console databases.
	H2_ConsoleRegistration();

	// The startup WADs.
	gi.AddStartupWAD("hexen.wad");
	gi.AddStartupWAD("scatteredevil.wad");

	startepisode = 1;
	startskill = sk_medium;
	startmap = 1;
	shareware = false; // Always false for Hexen

	HandleArgs();
}

void H2_PostInit(void)
{
	int			pClass, p;
//	ddfont_t	cfont;

	gi.Message("--- Scattered Evil Init ---\n%s\n", VERSIONTEXT);

	H2_DefaultBindings();
//	R_LoadTextureTypes();

	// Check the -class argument.
	pClass = PCLASS_FIGHTER;
	if(p = gi.CheckParm("-class"))
	{
		pClass = atoi(gi.Argv(p+1));
		if(pClass > PCLASS_CORVUS || pClass < PCLASS_FIGHTER)
		{
			gi.Error("Invalid player class: %d\n", pClass);
		}
		gi.Message("\nPlayer Class: %d\n", pClass);
	}
	PlayerClass[consoleplayer] = (pclass_t)pClass;

	// Init the view.
	R_SetViewSize(screenblocks, 0);

	// Set sprite/model replacements.
	for(p=0; p<NUMSPRITES; p++)
	{
		spritereplacement_t rep;
		rep.spritenum = p;
		rep.modelname = sprnames[p];
		gi.Set(DD_SPRITE_REPLACEMENT, (int) &rep);
	}


	CreateSavePath();
	
	// Setup the console font.
	// Hexen font is not the default.
	/*
	cfont.flags = DDFONT_WHITE;
	cfont.height = 9;
	cfont.sizeX = 1.2f;
	cfont.sizeY = 2;
	cfont.TextOut = MN_DrTextA_CS;
	cfont.Width = MN_TextAWidth;
	cfont.Filter = MN_TextFilter;
	//gi.SetConsoleFont(&cfont); 
	*/

	gi.Message("S_Init...\n");
	S_Init();

	gi.Message("P_Init: Init Playloop state.\n");
	P_Init();
	
	gi.Message("MN_Init: Init menu system.\n");
	MN_Init();

	gi.Message("CT_Init: Init chat mode data.\n");
	CT_Init();

	InitMapMusicInfo();		// Init music fields in mapinfo

	gi.Message("S_InitScript\n");
	S_InitScript();

	gi.Message("SN_InitSequenceScript: Registering sound sequences.\n");
	SN_InitSequenceScript();

//	S_StartSongName("orb", true);

	// Check for command line warping. Follows P_Init() because the
	// MAPINFO.TXT script must be already processed.
	WarpCheck();

	if(autostart)
	{
		gi.Message("Warp to Map %d (\"%s\":%d), Skill %d\n",
			WarpMap, P_GetMapName(startmap), startmap, startskill+1);
	}

	gi.Message("SB_Init: Loading patches.\n");
	SB_Init();

	if(CheckRecordFrom()) return;

	p = gi.CheckParm("-record");
	if(p && p < gi.Argc()-1)
	{
		singledemo = true;	// Quit after recording.
		G_RecordDemo(startskill, 1, startepisode, startmap, gi.Argv(p+1));
		return;
	}

	p = gi.CheckParm("-playdemo");
	if(p && p < gi.Argc()-1)
	{
		singledemo = true; // Quit after one demo
		G_DeferedPlayDemo(gi.Argv(p+1));
		return;
	}

	p = gi.CheckParm("-timedemo");
	if(p && p < gi.Argc()-1)
	{
		singledemo = true; // Quit after timing
		G_TimeDemo(gi.Argv(p+1));
		return;
	}

	p = gi.CheckParm("-loadgame");
	if(p && p < gi.Argc()-1)
	{
		G_LoadGame(atoi(gi.Argv(p+1)));
	}

	if(gameaction != ga_loadgame)
	{
		gi.Update(DDUF_FULLSCREEN | DDUF_BORDER);
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

	nomonsters = gi.ParmExists("-nomonsters");
	respawnparm = gi.ParmExists("-respawn");
	randomclass = gi.ParmExists("-randclass");
	ravpic = gi.ParmExists("-ravpic");
	artiskip = gi.ParmExists("-artiskip");
	debugmode = gi.ParmExists("-debug");
	netDeathmatch = gi.ParmExists("-deathmatch");
	cdrom = gi.ParmExists("-cdrom");
	cmdfrag = gi.ParmExists("-cmdfrag");
	nofullscreen = gi.ParmExists("-nofullscreen");
	netcheat = gi.ParmExists("-netcheat");
	dontrender = gi.ParmExists("-noview");
	
/*	if(gi.ParmExists("-betademo")) 
	{
		gi.Set(DD_SHAREWARE, true);
		shareware = true;
		gi.Message( "*** Hexen 4-level Beta Demo ***\n");
	}*/

	// Process command line options
	for(opt = ExecOptions; opt->name != NULL; opt++)
	{
		p = gi.CheckParm(opt->name);
		if(p && p < gi.Argc()-opt->requiredArgs)
		{
			opt->func(gi.ArgvPtr(p), opt->tag);
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

	p = gi.CheckParm("-warp");
	if(p && p < gi.Argc()-1)
	{
		WarpMap = atoi(gi.Argv(p+1));
		map = P_TranslateMap(WarpMap);
		if(map == -1)
		{ // Couldn't find real map number
			startmap = 1;
			gi.Message("-WARP: Invalid map number.\n");
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
	gi.AddStartupWAD(file);
	gi.Message("Playing demo %s.lmp.\n", args[1]);
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
	gi.Message("Map development mode enabled:\n");
	gi.Message("[config    ] = %s\n", args[1]);
	SC_OpenFileCLib(args[1]);
	SC_MustGetStringName("mapsdir");
	SC_MustGetString();
	gi.Message("[mapsdir   ] = %s\n", sc_String);
	DevMapsDir = (char *)malloc(strlen(sc_String)+1);
	strcpy(DevMapsDir, sc_String);
	SC_MustGetStringName("scriptsdir");
	SC_MustGetString();
	gi.Message("[scriptsdir] = %s\n", sc_String);
	sc_FileScripts = true;
	sc_ScriptsDir = (char *)malloc(strlen(sc_String)+1);
	strcpy(sc_ScriptsDir, sc_String);
	while(SC_GetString())
	{
		if(SC_Compare("file"))
		{
			SC_MustGetString();
			gi.AddStartupWAD(sc_String);
		}
		else
		{
			SC_ScriptError(NULL);
		}
	}
	SC_Close();
}


long superatol(char *s)
{
	long int n=0, r=10, x, mul=1;
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
	return(mul*n);
}


/*static void ExecOptionMAXZONE(char **args, int tag)
{
	int size;
	
	size = superatol(args[1]);
	if (size < MINIMUM_HEAP_SIZE) size = MINIMUM_HEAP_SIZE;
	if (size > MAXIMUM_HEAP_SIZE) size = MAXIMUM_HEAP_SIZE;
	maxzone = size;
}*/


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

	p = gi.CheckParm("-recordfrom");
	if(!p || p > gi.Argc()-2)
	{ // Bad args
		return false;
	}
	G_LoadGame(atoi(gi.Argv(p+1)));
	G_DoLoadGame(); // Load the gameskill etc info from savegame
	G_RecordDemo(gameskill, 1, gameepisode, gamemap, gi.Argv(p+2));
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

	if(cdrom)
	{
		SavePath = "c:\\hexndata\\";
	}
	len = strlen(SavePath);
	if (len >= 120) gi.Error("Save path too long\n");
	strcpy(creationPath, SavePath);

	creationPath[len-1] = 0;
	_mkdir(creationPath);
}

void H2_Ticker(void)
{
	if(advancedemo) H2_DoAdvanceDemo();
	MN_Ticker();
}

void G_ModifyDupTiccmd(ticcmd_t *cmd)
{
	if(cmd->buttons & BT_SPECIAL) cmd->buttons = 0;
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

char *H2_GetString(int id)
{
	switch(id)
	{
	case DD_VERSION_SHORT:
		return VERSION_TEXT;

	case DD_VERSION_LONG:
		return VERSIONTEXT"\nKorax is based on Hexen v1.1.";//JHexen
	
	default:
		break;
	}
	return "";
}


void H2_Shutdown(void)
{
//	R_FreeTextypeTypes();
}

extern "C" __declspec(dllexport) game_export_t *GetGameAPI(game_import_t *imports)
{
	// Take a copy of the imports.
	gi = *imports;
	gl = *(gl_export_t*) imports->GetDGL();

	memset(&gx, 0, sizeof(gx));

	// Fill in the data for the exports.
	gx.PreInit = H2_PreInit;
	gx.PostInit = H2_PostInit;
	gx.Shutdown = H2_Shutdown;
	gx.BuildTiccmd = (void(*)(void*))G_BuildTiccmd;
	gx.ModifyDupTiccmd = (void(*)(void*))G_ModifyDupTiccmd;
	gx.G_Ticker = G_Ticker;
	gx.G_Drawer = G_Drawer;
	gx.MN_Ticker = H2_Ticker;
	gx.MN_Drawer = MN_Drawer;
	gx.PrivilegedResponder = H2_PrivilegedResponder;
	gx.MN_Responder = MN_Responder;
	gx.G_Responder = G_Responder;
	gx.MobjThinker = P_MobjThinker;
	gx.EndFrame = H2_EndFrame;
	gx.ConsoleBackground = H2_ConsoleBg;
	gx.UpdateState = H2_UpdateState;
	gx.DrawPlayerSprites = R_DrawPlayerSprites;
	gx.GetString = H2_GetString;

	gx.NetServerOpen = H2_NetServerOpen;
	gx.NetServerStart = H2_NetServerStarted;
	gx.NetServerStop = H2_NetServerClose;
	gx.NetServerClose = H2_NetServerClose;
	gx.NetConnect = H2_NetConnect;
	gx.NetDisconnect = H2_NetDisconnect;
	gx.NetPlayerEvent = H2_NetPlayerEvent;
/*	gx.HandlePacket = SB_HandleCheatNotification;*/

	// The structure sizes.
	gx.ticcmd_size = sizeof(ticcmd_t);
	gx.vertex_size = sizeof(vertex_t);
	gx.seg_size = sizeof(seg_t);
	gx.sector_size = sizeof(sector_t);
	gx.subsector_size = sizeof(subsector_t);
	gx.node_size = sizeof(node_t);
	gx.line_size = sizeof(line_t);
	gx.side_size = sizeof(side_t);

	return &gx;
}
