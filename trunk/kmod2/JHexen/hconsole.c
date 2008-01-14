
//**************************************************************************
//**
//** HCONSOLE.C
//**
//** Hexen specific console stuff.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "settings.h"

// MACROS ------------------------------------------------------------------

#define DEFCC(x)	int x(int argc, char **argv)

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void H2_StartTitle(void);
void G_CheckDemoStatus();
void S_InitScript();
void SN_InitSequenceScript(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

int CCmdCD(int argc, char **argv);
int CCmdMidi(int argc, char **argv);
int CCmdPause(int argc, char **argv);
int CCmdCheat(int argc, char **argv);
int CCmdPlayDemo(int argc, char **argv);
int CCmdRecordDemo(int argc, char **argv);
int CCmdStopDemo(int argc, char **argv);
int CCmdTimeDemo(int argc, char **argv);
int CCmdScriptInfo(int argc, char **argv);
int CCmdSuicide(int argc, char **argv);
int CCmdSetDemoMode(int argc, char **argv);
int CCmdCrosshair(int argc, char **argv);
int CCmdViewSize(int argc, char **argv);
int CCmdInventory(int argc, char **argv);
int CCmdScreenShot(int argc, char **argv);

DEFCC(CCmdHexenFont);
DEFCC(CCmdMenuAction);
DEFCC(CCmdCycleSpy);
DEFCC(CCmdTest);

// The cheats.
int CCmdCheatGod(int argc, char **argv);
int CCmdCheatClip(int argc, char **argv);
int CCmdCheatGive(int argc, char **argv);
int CCmdCheatWarp(int argc, char **argv);
int CCmdCheatPig(int argc, char **argv);
int CCmdCheatMassacre(int argc, char **argv);
int CCmdCheatShadowcaster(int argc, char **argv);
int CCmdCheatWhere(int argc, char **argv);
int CCmdCheatRunScript(int argc, char **argv);
int CCmdCheatReveal(int argc, char **argv);
int CCmdCheatArtifactAll(int argc, char **argv);
int CCmdCheatWeapons(int argc, char **argv);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern char *SavePath;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int consoleFlat = 60;
float consoleZoom = 1;


cvar_t gameCVars[] =
{
	"i_mouseSensiX",CVF_NO_MAX,		CVT_INT,	&mouseSensitivityX, 0, 25, "Mouse X axis sensitivity.",
	"i_mouseSensiY",CVF_NO_MAX,		CVT_INT,	&mouseSensitivityY, 0, 25, "Mouse Y axis sensitivity.",
	"i_jlookInvY",		0,			CVT_INT,	&jlookInverseY, 0, 1,	"1=Inverse joystick look Y axis.",
	"i_mlookInvY",		0,			CVT_INT,	&mlookInverseY, 0, 1,	"1=Inverse mouse look Y axis.",
	"fps",			CVF_NO_ARCHIVE,	CVT_INT,	&showFPS,		0, 1,	"1=Show the frames per second counter.",
	"echomsg",			0,			CVT_INT,	&echoMsg,		0, 1,	"1=Echo all messages to the console.",
	"icecorpse",		0,			CVT_INT,	&translucentIceCorpse, 0, 1, "1=Translucent frozen monsters.",
	"immediateuse",		0,			CVT_INT,	&chooseAndUse,	0, 1,	"1=Use items immediately from the inventory.",
	"lookspeed",		0,			CVT_INT,	&lookdirSpeed,	1, 5,	"The speed of looking up/down.",
	"quakefly",			0,			CVT_INT,	&quakeFly,		0, 1,	"1=Use Quake's flight model (fly where you're looking).",
	"bgflat",		CVF_NO_MAX,		CVT_INT,	&consoleFlat,	0, 0,	"The number of the flat to use for the console background.",
	"bgzoom",			0,			CVT_FLOAT,	&consoleZoom,	0.1f, 100.0f, "Zoom factor for the console background.",
	"povlook",			0,			CVT_BYTE,	&povLookAround,	0, 1,	"1=Look around using the POV hat.",
	
	"i_mlook",			0,			CVT_INT,	&usemlook,		0, 1,	"1=Mouse look active.",
	"i_jlook",			0,			CVT_INT,	&usejlook,		0, 1,	"1=Joystick look active.",
	"alwaysrun",		0,			CVT_INT,	&alwaysRun,		0, 1,	"1=Always run.",
	"lookspring",		0,			CVT_INT,	&lookSpring,	0, 1,	"1=Lookspring active.",
	"noautoaim",		0,			CVT_INT,	&noAutoAim,		0, 1,	"1=Autoaiming disabled.",
	"h_viewsize",	CVF_PROTECTED,	CVT_INT,	&screenblocks,	3, 11,	"View window size (3-11).",
	"h_sbsize",		CVF_PROTECTED,	CVT_INT,	&sbarscale,		1, 20,	"Status bar size (1-20).",
	"dclickuse",		0,			CVT_INT,	&dclickuse,		0, 1,	"1=Double click forward/strafe equals pressing the use key.",
	
	"xhair", CVF_NO_MAX|CVF_PROTECTED, CVT_INT,	&xhair,			0, 0,	"The current crosshair.",
	"xhairR",			0,			CVT_BYTE,	&xhairColor[0], 0, 255,	"Red crosshair color component.",
	"xhairG",			0,			CVT_BYTE,	&xhairColor[1], 0, 255, "Green crosshair color component.",
	"xhairB",			0,			CVT_BYTE,	&xhairColor[2], 0, 255, "Blue crosshair color component.",
	"xhairSize",	CVF_NO_MAX,		CVT_INT,	&xhairSize,		0, 0,	"Crosshair size: 1=Normal.",

	//"s_channels",	CVF_PROTECTED,	CVT_INT,	&snd_Channels,	1, 16,	"The number of sounds that can be played simultaneously.",
	"s_3d",				0,			CVT_INT,	&snd_3D,		0, 1,	"1=Play sounds in 3D.",
	"s_reverbVol",		0,			CVT_FLOAT,	&snd_ReverbFactor, 0, 1, "General reverb strength (0-1).",
	"sounddebug",	CVF_NO_ARCHIVE,	CVT_INT,	&DebugSound,	0, 1,	"1=Display sound debug information.",
	"reverbdebug",	CVF_NO_ARCHIVE,	CVT_BYTE,	&reverbDebug,	0, 1,	"1=Reverberation debug information in the console.",

	"messages",			0,			CVT_INT,	&messageson,	0, 1,	"1=Show messages.",
	"showmana",			0,			CVT_INT,	&showFullscreenMana, 0, 2, "Show mana when the status bar is hidden.",
	"savedir",		CVF_PROTECTED,	CVT_CHARPTR, &SavePath,		0, 0,	"The directory for saved games.",
	"chatmacro0",		0,			CVT_CHARPTR, &chat_macros[0], 0, 0, "Chat macro 1.",
	"chatmacro1",		0,			CVT_CHARPTR, &chat_macros[1], 0, 0, "Chat macro 2.",
	"chatmacro2",		0,			CVT_CHARPTR, &chat_macros[2], 0, 0, "Chat macro 3.",
	"chatmacro3",		0,			CVT_CHARPTR, &chat_macros[3], 0, 0, "Chat macro 4.",
	"chatmacro4",		0,			CVT_CHARPTR, &chat_macros[4], 0, 0, "Chat macro 5.",
	"chatmacro5",		0,			CVT_CHARPTR, &chat_macros[5], 0, 0, "Chat macro 6.",
	"chatmacro6",		0,			CVT_CHARPTR, &chat_macros[6], 0, 0, "Chat macro 7.",
	"chatmacro7",		0,			CVT_CHARPTR, &chat_macros[7], 0, 0, "Chat macro 8.",
	"chatmacro8",		0,			CVT_CHARPTR, &chat_macros[8], 0, 0, "Chat macro 9.",
	"chatmacro9",		0,			CVT_CHARPTR, &chat_macros[9], 0, 0, "Chat macro 10.",

	// Game settings for servers.
	"nomonsters",		0,			CVT_BYTE,	&netNomonsters,	0, 1,	"1=No monsters.",
	"respawn",			0,			CVT_BYTE,	&netRespawn,	0, 1,	"1= -respawn was used.",
	"randclass",		0,			CVT_BYTE,	&netRandomclass,0, 1,	"1=Respawn in a random class (deathmatch).",
	"n_skill",			0,			CVT_BYTE,	&netSkill,		0, 4,	"Skill level in multiplayer games.",
	"n_map",			0,			CVT_BYTE,	&netMap,		1, 99,	"Map to use in multiplayer games.",
	"n_slot",			0,			CVT_BYTE,	&netSlot,		0, 6,	"The savegame slot to start from. 0=none.",
	"deathmatch",		0,			CVT_BYTE,	&netDeathmatch,	0, 1,	"1=Start multiplayers games as deathmatch.",

	// Player data.
	"n_class",			0,			CVT_BYTE,	&netClass,		0, 2,	"Player class in multiplayer games.",
	"n_color",			0,			CVT_BYTE,	&netColor,		0, 7,	"Player color: 0=blue, 1=red, 2=yellow, 3=green, 4=jade, 5=white,\n6=hazel, 7=purple.",
	"n_mobDamage",		0,			CVT_BYTE,	&netMobDamageModifier, 1, 100,	"Enemy (mob) damage modifier, multiplayer (1..100).",
	"n_mobHealth",		0,			CVT_BYTE,	&netMobHealthModifier, 1, 20,	"Enemy (mob) health modifier, multiplayer (1..20).",
	"overrideHubMsg",	0,			CVT_BYTE,	&overrideHubMsg,0, 2,	"Override the transition hub message.",
	"demoDisabled",		0,			CVT_BYTE,	&demoDisabled,	0, 2,	"Disable demos.",

	"maulatortime",	CVF_NO_MAX,		CVT_INT,	&MaulatorSeconds, 1, 0,	"Dark Servant lifetime, in seconds (default: 25).",
	"fastMonsters",		0,			CVT_BYTE,	&fastMonsters,	0, 1,	"1=Fast monsters in non-demo single player.",

	NULL
};

ccmd_t gameCCmds[] =
{
	"cd",			CCmdCD,					"CD player control.",
//	"cheat",		CCmdCheat,				"Issue a cheat code using the original Hexen cheats.",
//	"class",		CCmdCheatShadowcaster,	"Change player class.",
	"bunny",		CCmdCheatClip,			"Movement clipping on/off.",
	"crosshair",	CCmdCrosshair,			"Crosshair settings.",	
#ifdef DEMOCAM
	"demomode",		CCmdSetDemoMode,		"Set demo external camera mode.",
#endif
	"octomont",		CCmdCheatGive,			"Cheat command to give you various kinds of things.",
	"fag",			CCmdCheatGod,			"I don't think He needs any help...",
	"mad",		    CCmdCheatMassacre,		"Kill all the monsters on the level.",
	"shopping",		CCmdCheatArtifactAll,	"Get max Artifacts",
	"armload",		CCmdCheatWeapons,		"Get all weapons",
	"hexenfont",	CCmdHexenFont,			"Use the Hexen font.",
	"invleft",		CCmdInventory,			"Move inventory cursor to the left.",
	"invright",		CCmdInventory,			"Move inventory cursor to the right.",
	"midi",			CCmdMidi,				"MIDI music control.",
	"pause",		CCmdPause,				"Pause the game (same as pressing the pause key).",
	"playdemo",		CCmdPlayDemo,			"Play a demo.",
//	"pig",			CCmdCheatPig,			"Turn yourself into a pig. Go ahead.",
	"recorddemo",	CCmdRecordDemo,			"Record a demo.",
//	"reveal",		CCmdCheatReveal,		"Map cheat.",
//	"runscript",	CCmdCheatRunScript,		"Run a script.",
//	"scriptinfo",	CCmdScriptInfo,			"Show information about all scripts or one particular script.",
	"viewsize",		CCmdViewSize,			"Set the view size.",
	"sbsize",		CCmdViewSize,			"Set the status bar size.",
	"screenshot",	CCmdScreenShot,			"Take a screenshot.",
//	"sndchannels",	CCmdSndChannels,		"Set or query the number of sound channels.",
	"stopdemo",		CCmdStopDemo,			"Stop the currently played/recorded demo.",
	//"timedemo",		CCmdTimeDemo, 
	"warp",			CCmdCheatWarp,			"Warp to a map.",
//	"where",		CCmdCheatWhere,			"Prints your map number and exact location.",

	"spy",			CCmdCycleSpy,			"Change the viewplayer when not in deathmatch.",

	// Menu actions.
	"infoScreen",	CCmdMenuAction,			"Display the original Hexen help screens.",
	"saveGame",		CCmdMenuAction,			"Save the game.",
	"loadGame",		CCmdMenuAction,			"Load a saved game.",
	"soundMenu",	CCmdMenuAction,			"Open the sound menu.",
	"suicide",		CCmdMenuAction,			"Kill yourself. What did you think?",
	"quickSave",	CCmdMenuAction,			"Quicksave the game.",
	"endGame",		CCmdMenuAction,			"End the current game.",
	"toggleMsgs",	CCmdMenuAction,			"Toggle messages on/off (cvar messages).",
	"quickLoad",	CCmdMenuAction,			"Load the last quicksaved game.",
	"quit",			CCmdMenuAction,			"Quit Korax.", //JHexen
	"toggleGamma",	CCmdMenuAction,			"Change the gamma correction level.",
	
//	"test",			CCmdTest,				"Test.",
	NULL
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

// Add the console variables and commands.
void H2_ConsoleRegistration()
{
	int		i;
	
	for(i=0; gameCVars[i].name; i++) gi.AddVariable(gameCVars+i);
	for(i=0; gameCCmds[i].name; i++) gi.AddCommand(gameCCmds+i);
}

char playDemoName[9];

int CCmdPlayDemo(int argc, char **argv)
{	
	if(argc != 2)
	{
		gi.conprintf( "Usage: playdemo (name)\n");
		return true;
	}
	if(gi.W_CheckNumForName(argv[1]) == -1)
	{
		gi.conprintf( "There is a no lump named '%s'.\n", argv[1]);
		return false;
	}
	// Use the name in lumpinfo because it has to last.
	strcpy(playDemoName, argv[1]);
	G_DeferedPlayDemo(playDemoName);
	return true;
}

int CCmdRecordDemo(int argc, char **argv)
{
	int	map = gamemap;

	if(demorecording)
	{
		gi.conprintf( "A demo is already being recorded!\n");
		return false;
	}
	if(argc == 1 || argc > 3)
	{
		gi.conprintf( "Usage: recorddemo (name) (map)\n");
		gi.conprintf( "The demo will be saved in a file named (name).lmp.\n");
		gi.conprintf( "If you don't specify a map the current one is used.\n");
		return true;
	}
	if(argc == 3) map = atoi(argv[2]);
	G_RecordDemo(gameskill, 1, gameepisode, map, argv[1]);
	return true;
}

int CCmdStopDemo(int argc, char **argv)
{
	if(!demoplayback && !demorecording)
	{
		gi.conprintf( "No demo being played or recorded.\n");
		return false;
	}
	G_CheckDemoStatus();
	H2_StartTitle();
	return true;
}

int CCmdTimeDemo(int argc, char **argv)
{
	if(argc != 2)
	{
		gi.conprintf( "Usage: timedemo (name)\n");
		return true;
	}
	if(gi.W_CheckNumForName(argv[1]) == -1)
	{
		gi.conprintf( "There is a no lump named '%s'.\n", argv[1]);
		return false;
	}
	G_TimeDemo(argv[1]);
	return true;
}

int CCmdViewSize(int argc, char **argv)
{
	int	min=3, max=11, *val = &screenblocks;

	if(argc != 2)
	{
		gi.conprintf( "Usage: %s (size)\n", argv[0]);
		gi.conprintf( "Size can be: +, -, (num).\n");
		return true;
	}
	if(!stricmp(argv[0], "sbsize"))
	{
		min = 1;
		max = 20;
		val = &sbarscale;
	}
	if(!stricmp(argv[1], "+"))
		(*val)++;
	else if(!stricmp(argv[1], "-"))
		(*val)--;
	else
		*val = strtol(argv[1], NULL, 0);

	if(*val < min) *val = min;
	if(*val > max) *val = max;

	// Update the view size if necessary.
	R_SetViewSize(screenblocks, 0);
	return true;
}

int CCmdScreenShot(int argc, char **argv)
{
	G_ScreenShot();
	return true;
}

DEFCC(CCmdHexenFont)
{
	ddfont_t	cfont;

	cfont.flags = DDFONT_WHITE;
	cfont.height = 9;
	cfont.sizeX = 1.2f;
	cfont.sizeY = 2;
	cfont.TextOut = MN_DrTextA_CS;
	cfont.Width = MN_TextAWidth;
	cfont.Filter = MN_TextFilter;
	gi.SetConsoleFont(&cfont);
	return true;
}

DEFCC(CCmdTest)
{
	/*int		i;

	for(i=0; i<MAXPLAYERS; i++)
		if(players[i].plr->ingame)
		{
			gi.conprintf( "%i: cls:%i col:%i look:%f\n", i, PlayerClass[i],
				PlayerColor[i], players[i].plr->lookdir);
		}*/
	if(argc != 2) return false;
	S_StartSoundAtVolume(NULL, SFX_CHAT, atoi(argv[1]));
	return true;
}