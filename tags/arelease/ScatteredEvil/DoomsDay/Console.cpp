
//**************************************************************************
//**
//** CONSOLE.C
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "dd_def.h"
#include "gl_def.h"
#include "gl_font.h"
#include "console.h"
#include "u_args.h"
#include "jtNetEx.h"
#include "i_win32.h"
#include "i_timer.h"
#include "i_sound.h"
#include "settings.h"

#include <math.h>

#ifdef TextOut
// Windows has a way of making things difficult.
#undef TextOut
#endif

// MACROS ------------------------------------------------------------------

#define MAX_RECURSION		10

// Length of the print buffer. Used in CON_Printf. If console messages are
// longer than this, an error will occur.
#define PRBUFF_LEN			8000

#define DEFCC(x)	int x(int argc, char **argv)

// TYPES -------------------------------------------------------------------

typedef struct
{
	char *name;
	char *command;
} calias_t;

typedef struct
{
	unsigned int	marker : 31;	// The tic on which the cmd should be executed.
	unsigned int	used : 1;		// Is this in use?
	char			subcmd[256];	// A single command w/args.
} execbuff_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void G_CheckDemoStatus();
void S_InitScript();
void SN_InitSequenceScript(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

DEFCC(CCmdListCmds);
DEFCC(CCmdListVars);
DEFCC(CCmdConsole);
DEFCC(CCmdVersion);
DEFCC(CCmdQuit);
DEFCC(CCmdLoadFile);
DEFCC(CCmdUnloadFile);
DEFCC(CCmdResetLumps);
DEFCC(CCmdListFiles);
DEFCC(CCmdBackgroundTurn);
DEFCC(CCmdDump);
DEFCC(CCmdResetTextures);
DEFCC(CCmdMipMap);
DEFCC(CCmdSmoothRaw);
DEFCC(CCmdSkyDetail);
DEFCC(CCmdSetMusicDevice);
DEFCC(CCmdSetMIDIVolume);
DEFCC(CCmdSetCDVolume);
DEFCC(CCmdFog);
DEFCC(CCmdBind);
DEFCC(CCmdListBindings);
DEFCC(CCmdFont);
DEFCC(CCmdAlias);
DEFCC(CCmdListAliases);
DEFCC(CCmdSetGamma);
DEFCC(CCmdNet);
DEFCC(CCmdChat);
DEFCC(CCmdParse);
DEFCC(CCmdDeleteBind);
DEFCC(CCmdLowRes);
DEFCC(CCmdWait);
DEFCC(CCmdEcho);
DEFCC(CCmdFlareConfig);
DEFCC(CCmdListActs);
DEFCC(CCmdClearBindings);

#ifdef _DEBUG
DEFCC(CCmdTranslateFont);
#endif

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static int executeSubCmd(char *subcmd);
static void SplitIntoSubCommands(char *command, int markerOffset);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern char *serverName, *serverInfo, *playerName;
extern boolean paletted, r_s3tc;	// Use GL_EXT_paletted_texture
extern boolean renderTextures;
extern float fieldOfView;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

ddfont_t Cfont;				// The console font.

float	CcolYellow[3] = { 1, .85f, .3f };

boolean	ConsoleSilent = false;
int		CmdReturnValue = 0;	

float	ConsoleOpenY;			// Where the console bottom is when open.

int		consoleTurn;			// The rotation variable.
int		consoleLight=50, consoleAlpha=75;
int		conCompMode=1;			// Completion mode.
int		conSilentCVars=1;		
boolean	consoleDump = true;

cvar_t *cvars = NULL;
int numCVars = 0;

ccmd_t *ccmds = NULL;		// The list of console commands.
int numCCmds = 0;			// Number of console commands.

calias_t *caliases = NULL;
int numCAliases = 0;


// Console variables.
cvar_t engineCVars[] = 
{
	"bgalpha",			0,			CVT_INT,	&consoleAlpha,	0, 100,	"Console background translucency.",
	"bglight",			0,			CVT_INT,	&consoleLight,	0, 100,	"Console background light level.",
	"borderupd",		0,			CVT_INT,	&borderUpd,		0, 1,	"1=Continuous screen border redrawing.",
	"completionmode",	0,			CVT_INT,	&conCompMode,	0, 1,	"How to complete words when pressing Tab:\n0=Show completions, 1=Cycle through them.",
	"consoledump",		0,			CVT_BYTE,	&consoleDump,	0, 1,	"1=Dump all console messages to Doomsday.out.",
	"silentcvars",		0,			CVT_BYTE,	&conSilentCVars,0, 1,	"1=Don't show the value of a cvar when setting it.",
	"dlblend",			0,			CVT_INT,	&dlBlend,		0, 3,	"Dynamic lights color blending mode:\n0=normal, 1=additive, 2=no blending.",
	"dlclip",			0,			CVT_INT,	&clipLights,	0, 1,	"1=Clip dynamic lights (try using with dlblend 2).",
	"dlfactor",			0,			CVT_FLOAT,	&dlFactor,		0, 1,	"Intensity factor for dynamic lights.",
	"dynlights",		0,			CVT_INT,	&useDynLights,	0, 1,	"1=Render dynamic lights.",
	"flareintensity",	0,			CVT_INT,	&flareBoldness,	0, 100,	"Lens flare intensity.",
	"flares",			0,			CVT_INT,	&haloMode,		0, 5,	"Number of lens flares to draw per light.",
	"flaresize",		0,			CVT_INT,	&flareSize,		0, 10,	"Lens flare size.",
	"i_useJoystick", CVF_PROTECTED,	CVT_INT,	&usejoystick,	0, 1,	"1=Use joystick.",
	"i_joySensi",		0,			CVT_INT,	&joySensitivity,0, 9,	"Joystick sensitivity.",
	"i_mouseInvY",		0,			CVT_INT,	&mouseInverseY, 0, 1,	"1=Inversed mouse Y axis.",
	"i_mWheelSensi",CVF_NO_MAX,		CVT_INT,	&mouseWheelSensi, 0, 0, "Mouse wheel sensitivity.",
	"i_keywait1",	CVF_NO_MAX,		CVT_INT,	&repWait1,		6, 0,	"The number of 35 Hz ticks to wait before first key repeat.",
	"i_keywait2",	CVF_NO_MAX,		CVT_INT,	&repWait2,		1, 0,	"The number of 35 Hz ticks to wait between key repeats.",
	"i_mouseDisX",		0,			CVT_INT,	&mouseDisableX,	0, 1,	"1=Disable mouse X axis.",
	"i_mouseDisY",		0,			CVT_INT,	&mouseDisableY,	0, 1,	"1=Disable mouse Y axis.",
	"maxdl",		CVF_NO_MAX,		CVT_INT,	&maxDynLights,	0, 0,	"The maximum number of dynamic lights. 0=no limit.",
	"n_model",		CVF_PROTECTED,	CVT_INT,	&netModel,		0, 2,	"The networking model:\n0=Peer-to-peer, (1=Pure multicast), 2=Smart multicast.",
	"n_servername",		0,			CVT_CHARPTR, &serverName,	0, 0,	"The name of this computer if it's a server.",
	"n_serverinfo",		0,			CVT_CHARPTR, &serverInfo,	0, 0,	"The description given of this computer if it's a server.",
	"n_plrname",		0,			CVT_CHARPTR, &playerName,	0, 0,	"Your name in multiplayer games.",
	"r_ambient",		0,			CVT_INT,	&r_ambient,		0, 255, "Ambient light level.",
	"r_fov",		CVF_NO_ARCHIVE,	CVT_FLOAT,	&fieldOfView,	1, 179, "Field of view.",
	"r_gamma",		CVF_PROTECTED,	CVT_INT,	&usegamma,		0, 4,	"The gamma correction level (0-4).",
	"r_smoothraw",	CVF_PROTECTED,	CVT_INT,	&linearRaw,		0, 1,	"1=Fullscreen images (320x200) use linear interpolation.",
	"r_mipmapping",	CVF_PROTECTED,	CVT_INT,	&mipmapping,	0, 5,	"The mipmapping mode for textures.",
	"r_skydetail",	CVF_PROTECTED,	CVT_INT,	&skyDetail,		3, 7,	"Number of sky sphere quadrant subdivisions.",
	"r_skyrows",	CVF_PROTECTED,	CVT_INT,	&skyRows,		1, 8,	"Number of sky sphere rows.",
	"r_skydist",	CVF_NO_MAX,		CVT_FLOAT,	&skyDist,		1, 0,	"Sky sphere radius.",
	"r_paletted",	CVF_PROTECTED,	CVT_BYTE,	&paletted,		0, 1,	"1=Use the GL_EXT_shared_texture_palette extension.",
	"r_spritefilter",	0,			CVT_INT,	&filterSprites,	0, 1,	"1=Render smooth sprites.",
	//"r_s3tc",		CVF_PROTECTED,	CVT_BYTE,	&r_s3tc,		0, 1,	"1=Use the GL_S3_s3tc extension.",
	"r_textures",	CVF_NO_ARCHIVE,	CVT_BYTE,	&renderTextures,0, 1,	"1=Render with textures.",
	"r_texquality",		0,			CVT_INT,	&texQuality,	0, 8,	"The quality of textures (0-8).",
	"r_flarefar",	CVF_NO_MAX,		CVT_FLOAT,	&flareFadeMax,	0, 0,	"Lens flare fadeout start distance.",
	"r_flarenear",	CVF_NO_MAX,		CVT_FLOAT,	&flareFadeMin,	0, 0,	"Lens flare minimum distance.",
	"r_flareminsize", CVF_NO_MAX,	CVT_FLOAT,	&minFlareSize,	0, 0,	"Minimum size for bright flares (have secondary flares).",
	"r_maxSpriteAngle", 0,			CVT_FLOAT,	&maxSpriteAngle, 0, 90,	"Maximum angle for slanted sprites (spralign 2).",
	"r_dlMaxRad",		0,			CVT_INT,	&dlMaxRad,		64,	512, "Maximum radius of dynamic lights (default: 128).",
	"r_dlRadFactor",	0,			CVT_FLOAT,	&dlRadFactor,	0.1f, 10, "A multiplier for dynlight radii (default: 1).",
	"defaultWads",		0,			CVT_CHARPTR, &defaultWads,	0, 0,	"The list of WADs to be loaded at startup.",
	"defResX",		CVF_NO_MAX,		CVT_INT,	&defResX,		320, 0,	"Default resolution (X).",
	"defResY",		CVF_NO_MAX,		CVT_INT,	&defResY,		240, 0, "Default resolution (Y).",
	"simplesky",		0,			CVT_INT,	&simpleSky,		0, 2,	"Sky rendering mode: 0=normal, 1=quads.",
	"spralign",			0,			CVT_INT,	&alwaysAlign,	0, 3,	"1=Always align sprites with the view plane.\n2=Align to camera, unless slant > r_maxSpriteAngle.",
	"sprblend",			0,			CVT_INT,	&missileBlend,	0, 1,	"1=Use additive blending for explosions.",
	"sprlight",			0,			CVT_INT,	&litSprites,	0, 1,	"1=Sprites lit using dynamic lights.",
	"s_musicDevice", CVF_PROTECTED, CVT_INT,	&snd_MusicDevice, 0, 2,	"Music device: 0=none, 1=MIDI, 2=CD.",
	"s_volcd",		CVF_PROTECTED,	CVT_INT,	&snd_CDVolume,	0, 255,	"CD player volume (0-255).",
	"s_volmidi",	CVF_PROTECTED,	CVT_INT,	&snd_MusicVolume, 0, 255, "MIDI music volume (0-255).",
	"s_volsfx",			0,			CVT_INT,	&snd_SfxVolume,	0, 255,	"Sound effects volume (0-255).",
	"s_resample",		0,			CVT_INT,	&snd_Resample,  1, 4,	"Sample frequency: 11 kHz * resample factor.",
	"s_16bit",			0,			CVT_INT,	&snd_16bits,	0, 1,	"1=Convert all samples to 16 bits before playing.",
	"useModels",		0,			CVT_BYTE,	&useModels,		0, 1,	"Render using 3D models when possible.",
	"modelLight",		0,			CVT_INT,	&modelLight,	0, 10,	"Maximum number of light sources on models.",
	"modelInter",		0,			CVT_INT,	&frameInter,	0, 1,	"1=Interpolate frames.",
	"l_down",           0,          CVT_INT,	&l_down,    	-110, -10,	"Set the lower looking angle (-110..-10)",
	"l_up",             0,          CVT_INT,	&l_up,      	10, 110,	"Set the upper looking angle (10-110)",
	"far_clip",         0,          CVT_FLOAT,	&farClip,      	1000, 99000,"Set the View distance (1000-99000)",
	"time_mod",         0,          CVT_INT,	&time_mod,     	1, 1000,"Set the time modificator (1-1000)",
	"act_int",          0,          CVT_INT,	&act_int,	    0,  255,"Set Light modificator (0-255)",
	NULL
};

// Console commands. Names in LOWER CASE (yeah, that's consistant).
// Keep the list in alphabetic order for convenience.
ccmd_t engineCCmds[] =
{
	"actions",		CCmdListActs,		"List all action commands.",
	"alias",		CCmdAlias,			"Create aliases for a (set of) console commands.",
	"bind",			CCmdBind,			"Bind a console command to an event.",
	"bindr",		CCmdBind,			"Bind a console command to an event (keys with repeat).",
	"safebind",		CCmdBind,			"Bind a command to an event, unless the event is already bound.",
	"safebindr",	CCmdBind,			"Bind a command to an event, unless the event is already bound.",
	"bgturn",		CCmdBackgroundTurn, "Set console background rotation speed.",
	"cdvolume",		CCmdSetCDVolume,	"Set CD player volume (0-255).",
	"chat",			CCmdChat,			"Broadcast a chat message.",
	"chatNum",		CCmdChat,			"Send a chat message to the specified player.",
	"chatTo",		CCmdChat,			"Send a chat message to the specified player.",
	"clear",		CCmdConsole,		"Clear the console buffer.",
	"clearbinds",	CCmdClearBindings,	"Deletes all existing bindings.",
	"delbind",		CCmdDeleteBind,		"Deletes all bindings to the given console command.",
	"dump",			CCmdDump,			"Dump a data lump currently loaded in memory.",
	"echo",			CCmdEcho,			"Echo the parameters on separate lines.",
	"fog",			CCmdFog,			"Modify fog settings.",
	"font",			CCmdFont,			"Modify console font settings.",
//	"glLowRes",		CCmdLowRes,			"Select the poorest rendering quality.",
	"help",			CCmdConsole,		"Show information about the console.",
	"listaliases",	CCmdListAliases,	"List all aliases and their expanded forms.",
	"listbindings",	CCmdListBindings,	"List all event bindings.",
	"listcmds",		CCmdListCmds,		"List all console commands.",
	"listfiles",	CCmdListFiles,		"List all the loaded data files and show information about them.",
	"listvars",		CCmdListVars,		"List all console variables and their values.",
	"net",			CCmdNet,			"Network setup and control.",
	"load",			CCmdLoadFile,		"Load a data file (a WAD or a lump).",
	"quit!",		CCmdQuit,			"Exit the game immediately.",
	"exec",			CCmdParse,			"Loads and executes a file containing console commands.",
	"reset",		CCmdResetLumps,		"Reset the data files into what they were at startup.",
	"texreset",		CCmdResetTextures,	"Force a texture reload.",
	"mipmap",		CCmdMipMap,			"Set the mipmapping mode.",
	"midivolume",	CCmdSetMIDIVolume,	"Set MIDI volume.",
	"musicdevice",	CCmdSetMusicDevice,	"Set the music device.",
	"setgamma",		CCmdSetGamma,		"Set the gamma correction level.",
	"smoothscr",	CCmdSmoothRaw,		"Set the rendering mode of fullscreen images.",
	"skydetail",	CCmdSkyDetail,		"Set the number of sky sphere quadrant subdivisions.",
	"skyrows",		CCmdSkyDetail,		"Set the number of sky sphere rows.",
	"unload",		CCmdUnloadFile,		"Unload a data file from memory.",
	"version",		CCmdVersion,		"Show detailed version information.",
	"after",		CCmdWait,			"Execute the specified command after a delay.",
	"flareconfig",	CCmdFlareConfig,	"Configure lens flares.",

#ifdef _DEBUG
	"TranslateFont", CCmdTranslateFont,	"Ha ha.",
#endif
	NULL
};

char **knownWords = 0;			// The list of known words (for completion).
int numKnownWords = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean ConsoleActive;	// Is the console active?
static float ConsoleY;			// Where the console bottom is currently?
static float ConsoleDestY;		// Where the console bottom should be?
static int ConsoleTime;			// How many ticks has the console been open?

static float funnyAng;
static boolean openingOrClosing = true;

static float fontFx, fontSy;	// Font x factor and y size.
static cbline_t *cbuffer;		// This is the buffer.
int bufferLines;				// How many lines are there in the buffer?
static int maxBufferLines;		// Maximum number of lines in the buffer.
static int maxLineLen;			// Maximum length of a line.
static int bPos;				// Where the write cursor is? (which line)
static int bFirst;				// The first visible line.
static int bLineOff;			// How many lines from bPos? (+vislines)
static char cmdLine[81];		// The command line.
static cbline_t *oldCmds;		// The old commands buffer.
static int numOldCmds;
static int ocPos;				// Old commands buffer position.
static int complPos;			// Where is the completion cursor?
static int lastCompletion;		// The index of the last completion (in knownWords).
static boolean shiftDown = false;

// Command waiting.
/*static int waitCount;			// The wait counter (tics, 0 if none).
static boolean waitStackEnabled;// Should new commands go to the stack, if needed?
static char **waitStack;
static int waitStackSize;		// How many commands on the wait stack?
static int wsAddWhere = -1;		// Where to add in the stack? (for aliases)*/

static execbuff_t *exBuff;
static int exBuffSize;
//static unsigned int idDealer = 0;
static execbuff_t *curExec;

static char shiftTable[96] =	// Contains characters 32 to 127.
{
/* 32 */	' ', 0, 0, 0, 0, 0, 0, '"',
/* 40 */	0, 0, 0, 0, '<', '_', '>', '?', ')', '!',
/* 50 */	'@', '#', '$', '%', '^', '&', '*', '(', 0, ':',
/* 60 */	0, '+', 0, 0, 0, 'a', 'b', 'c', 'd', 'e',
/* 70 */	'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
/* 80 */	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
/* 90 */	'z', '{', '|', '}', 0, 0, 0, 'A', 'B', 'C',
/* 100 */	'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
/* 110 */	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
/* 120 */	'X', 'Y', 'Z', 0, 0, 0, 0, 0
};

// CODE --------------------------------------------------------------------

//--------------------------------------------------------------------------
//
// Console Variable Handling
//
//--------------------------------------------------------------------------

void CvarSetString(cvar_t *cvar, char *text)
{
	if(cvar->type == CVT_CHARPTR)
	{
		// Free the old string, if one exists.
		if(cvar->flags & CVF_CAN_FREE && *(char**) cvar->ptr) 
			free( *(char**) cvar->ptr);
		// Allocate a new string.
		cvar->flags |= CVF_CAN_FREE;
		*(char**) cvar->ptr = (char *)malloc(strlen(text)+1);
		// -JL- Paranoia
		if (!(*(char**) cvar->ptr))
			I_Error("CvarSetString: malloc failed");
		strcpy( *(char**) cvar->ptr, text);
	}
	else 
		I_Error("CvarSetString: cvar is not of type char*.\n");
}

cvar_t *CvarGet(char *name)
{
	int		i;

	for(i=0; i<numCVars; i++)
		if(!stricmp(name, cvars[i].name))
			return cvars+i;
	// No match...
	return NULL;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

static int wordListSorter(const void *e1, const void *e2)
{
	return stricmp(*(char**)e1, *(char**)e2);
}

void CON_Init()
{
	int		i;

	ConsoleActive = false;
	ConsoleY = 0;
	ConsoleOpenY = 90;
	ConsoleDestY = 0;
	ConsoleTime = 0;

	funnyAng = 0;

	// Font size in VGA coordinates. (Everything is in VGA coords.)
	fontFx = 1;
	fontSy = 9;

	// The buffer.
	cbuffer = 0;
	bufferLines = 0;	
	maxBufferLines = 256;
	maxLineLen = 70;		// Should fit the screen.

	// The old commands buffer.
	oldCmds = 0;
	numOldCmds = 0;
	ocPos = 0;				// No commands yet.

	bPos = 0;
	bFirst = 0;
	bLineOff = 0;

	complPos = 0;
	lastCompletion = -1;

	exBuff = NULL;
	exBuffSize = 0;

	// Register the engine commands and variables.
	for(i=0; engineCCmds[i].name; i++) CON_AddCommand(engineCCmds + i);
	for(i=0; engineCVars[i].name; i++) CON_AddVariable(engineCVars + i);
}

void CON_UpdateKnownWords()
{
	int		i, c;

	// Fill the known words table.
	numKnownWords = numCCmds + numCVars + numCAliases;
	knownWords = (char **)realloc(knownWords, sizeof(char*) * numKnownWords);
	// -JL- Paranoia
	if (!knownWords)
		I_Error("CON_UpdateKnownWords: realloc failed");

	for(i=0, c=0; i<numCCmds; i++, c++)
		knownWords[c] = ccmds[i].name;
	for(i=0; i<numCVars; i++, c++) 
		knownWords[c] = cvars[i].name;
	for(i=0; i<numCAliases; i++, c++)
		knownWords[c] = caliases[i].name;
	qsort(knownWords, numKnownWords, sizeof(char*), wordListSorter);
}

void CON_AddCommand(ccmd_t *cmd)
{
	numCCmds++;
	ccmds = (ccmd_t *)realloc(ccmds, sizeof(ccmd_t) * numCCmds);
	// -JL- Paranoia
	if (!ccmds)
		I_Error("CON_AddCommand: realloc failed");
	memcpy(ccmds+numCCmds-1, cmd, sizeof(ccmd_t));

	// Sort them.
	qsort(ccmds, numCCmds, sizeof(ccmd_t), wordListSorter);

	// Update the list of known words.
	// This must be done right away because ccmds' address can change.
	CON_UpdateKnownWords();
}

void CON_AddVariable(cvar_t *var)
{
	numCVars++;
	cvars = (cvar_t *)realloc(cvars, sizeof(cvar_t) * numCVars);
	// -JL- Paranoia
	if (!cvars)
		I_Error("CON_AddVariable: realloc failed");
	memcpy(cvars+numCVars-1, var, sizeof(cvar_t));

	// Sort them.
	qsort(cvars, numCVars, sizeof(cvar_t), wordListSorter);

	// Update the list of known words.
	// This must be done right away because ccmds' address can change.
	CON_UpdateKnownWords();
}

// Returns NULL if the specified alias can't be found.
calias_t *CON_GetAlias(char *name)
{
	int			i;

	// Try to find the alias.
	for(i=0; i<numCAliases; i++)
		if(!stricmp(caliases[i].name, name))
			return caliases+i;
	return NULL;
}

void CON_Alias(char *aName, char *command)
{
	calias_t	*cal = CON_GetAlias(aName);
	boolean		remove = false;

	// Will we remove this alias?
	if(command == NULL) 
		remove = true;
	else if(command[0] == 0)
		remove = true;
	
	if(cal && remove)
	{
		// This alias will be removed.
		int idx = cal - caliases;
		free(cal->name);
		free(cal->command);
		if(idx < numCAliases-1)
			memmove(caliases+idx, caliases+idx+1, sizeof(calias_t)*(numCAliases-idx-1));
		caliases = (calias_t *)realloc(caliases, sizeof(calias_t) * --numCAliases);
		// -JL- Paranoia
		if (!caliases)
			I_Error("CON_Alias: realloc failed");
		// We're done.
		return;		
	}

	// Does the alias already exist?
	if(cal)
	{
		cal->command = (char *)realloc(cal->command, strlen(command)+1);
		// -JL- Paranoia
		if (!cal->command)
			I_Error("CON_Alias: command realloc failed");
		strcpy(cal->command, command);
		return;
	}

	// We need to create a new alias.
	caliases = (calias_t *)realloc(caliases, sizeof(calias_t) * (++numCAliases));
	// -JL- Paranoia
	if (!caliases)
		I_Error("CON_Alias: realloc failed");
	cal = caliases + numCAliases-1;
	// Allocate memory for them.
	cal->name = (char *)malloc(strlen(aName)+1);
	cal->command = (char *)malloc(strlen(command)+1);
	// -JL- Paranoia
	if (!cal->name || !cal->command)
		I_Error("CON_Alias: malloc failed");
	strcpy(cal->name, aName);
	strcpy(cal->command, command);
//	cal->refcount = 0;

	// Sort them.
	qsort(caliases, numCAliases, sizeof(calias_t), wordListSorter);

	CON_UpdateKnownWords();
}

void CON_ClearBuffer()
{
	int		i;

	// Free the buffer.
	for(i=0; i<bufferLines; i++) free(cbuffer[i].text);
	free(cbuffer);
	cbuffer = 0;
	bufferLines = 0;
	bPos = 0;
	bFirst = 0;
	bLineOff = 0;
}

static void ClearExecBuffer()
{
	free(exBuff);
	exBuff = NULL;
	exBuffSize = 0;
}

static void QueueCmd(char *singleCmd, int at_tic)
{
	execbuff_t	*ptr = NULL;
	int			i;

	// Look for an empty spot.
	for(i=0; i<exBuffSize; i++)
		if(!exBuff[i].used)
		{
			ptr = exBuff + i;
			break;
		}

	if(ptr == NULL)
	{
		// No empty places, allocate a new one.
		exBuff = (execbuff_t *)realloc(exBuff, sizeof(execbuff_t) * ++exBuffSize);
		// -JL- Paranoia
		if (!exBuff)
			I_Error("QueueCmd: realloc failed");
		ptr = exBuff + exBuffSize-1;
	}
	ptr->used = true;
	strcpy(ptr->subcmd, singleCmd);
	ptr->marker = at_tic;
}

void CON_Shutdown()
{	
	int		i;

	// Free the buffer.
	CON_ClearBuffer();

	// Free the old commands.
	for(i=0; i<numOldCmds; i++) free(oldCmds[i].text);
	free(oldCmds);
	oldCmds = 0;
	numOldCmds = 0;

	free(knownWords);
	knownWords = 0;
	numKnownWords = 0;

	// Free the data of the data cvars.
	for(i=0; i<numCVars; i++)
		if(cvars[i].flags & CVF_CAN_FREE && cvars[i].type == CVT_CHARPTR)
			free( *(char**) cvars[i].ptr);
	free(cvars);
	cvars = NULL;
	numCVars = 0;

	free(ccmds);
	ccmds = NULL;
	numCCmds = 0;

	// Free the alias data.
	for(i=0; i<numCAliases; i++)
	{
		free(caliases[i].command);
		free(caliases[i].name);
	}
	free(caliases);
	caliases = NULL;
	numCAliases = 0;

	ClearExecBuffer();
}

// Returns false if an executed command fails.
static boolean CheckExecBuffer()
{
	boolean alldone;
	boolean	ret = true;
	int		i, count = 0;
	char	storage[256];
	
	do // We'll keep checking until all is done.
	{
		alldone = true;
		// Execute the commands marked for this or a previous tic.
		for(i=0; i<exBuffSize; i++)
		{	
			execbuff_t *ptr = exBuff + i;
			if(!ptr->used || ptr->marker > (unsigned) systics) continue;
			// We'll now execute this command.
			curExec = ptr;
			ptr->used = false;
			strcpy(storage, ptr->subcmd);
			if(!executeSubCmd(storage)) ret = false;
			alldone = false;
		}

		if(count++ > 100)
		{
			ST_Message("Console execution buffer overflow! Everything canceled.\n");
			ClearExecBuffer();
			break;
		}
	}
	while(!alldone);
	return ret;
}

void CON_Ticker(void)
{
	CheckExecBuffer();

	if(ConsoleY == 0) openingOrClosing = true;

	// Move the console to the destination Y.
	if(ConsoleDestY > ConsoleY)
	{
		float diff = (ConsoleDestY - ConsoleY)/4;
		if(diff < 1) diff = 1;
		ConsoleY += diff;		
		if(ConsoleY > ConsoleDestY) ConsoleY = ConsoleDestY;
	}
	else if(ConsoleDestY < ConsoleY)
	{
		float diff = (ConsoleY - ConsoleDestY)/4;
		if(diff < 1) diff = 1;
		ConsoleY -= diff;		
		if(ConsoleY < ConsoleDestY) ConsoleY = ConsoleDestY;
	}

	if(ConsoleY == ConsoleOpenY) openingOrClosing = false;

	funnyAng += consoleTurn/10000.0f;//.002f;

	if(!ConsoleActive) return;	// We have nothing further to do here.

	ConsoleTime++;	// Increase the ticker.
}

cbline_t *CON_GetBufferLine(int num)
{
	int i, newLines;

	if(num < 0) return 0;	// This is unacceptable!
	// See if we already have that line.
	if(num < bufferLines) return cbuffer + num;
	// Then we'll have to allocate more lines. Usually just one, though.
	newLines = num+1 - bufferLines;
	bufferLines += newLines;
	cbuffer = (cbline_t *)realloc(cbuffer, sizeof(cbline_t)*bufferLines);
	// -JL- Paranoia
	if (!cbuffer)
		I_Error("CON_GetBufferLine: realloc failed");
	for(i=0; i<newLines; i++)
	{
		cbline_t *line = cbuffer + i+bufferLines-newLines;
		memset(line, 0, sizeof(cbline_t));
	}
	return cbuffer + num;
}

static void addLineText(cbline_t *line, char *txt)
{
	int newLen = line->len + strlen(txt);

	if(newLen > maxLineLen)
		//I_Error("addLineText: Too long console line.\n");
		return;	// Can't do anything.

	// Allocate more memory.
	line->text = (char *)realloc(line->text, newLen+1);
	// -JL- Paranoia
	if (!line->text)
		I_Error("addLineText: realloc failed");
	// Copy the new text to the appropriate location.
	strcpy(line->text+line->len, txt);
	// Update the length of the line.
	line->len = newLen;
}

static void setLineFlags(int num, int fl)
{
	cbline_t *line = CON_GetBufferLine(num);

	if(!line) return;
	line->flags = fl;
}

static void addOldCmd(const char *txt)
{
	cbline_t *line;

	if(!strcmp(txt, "")) return; // Don't add empty commands.

	// Allocate more memory.
	oldCmds = (cbline_t *)realloc(oldCmds, sizeof(cbline_t) * ++numOldCmds);
	// -JL- Paranoia
	if (!oldCmds)
		I_Error("addOldCmd: realloc failed");
	line = oldCmds + numOldCmds-1;
	// Clear the line.
	memset(line, 0, sizeof(cbline_t));
	line->len = strlen(txt);
	line->text = (char *)malloc(line->len+1); // Also room for the zero in the end.
	// -JL- Paranoia
	if (!line->text)
		I_Error("addOldCmd: malloc failed\n");
	strcpy(line->text, txt);
}

static void printcvar(cvar_t *var, char *prefix)
{
	char equals = '=';
	
	if(var->flags & CVF_PROTECTED) equals = ':';

	CON_Printf(prefix);
	switch(var->type)
	{
	case CVT_NULL:
		CON_Printf( "%s", var->name);
		break;
	case CVT_BYTE:
		CON_Printf( "%s %c %d", var->name, equals, *(byte*)var->ptr);
		break;
	case CVT_INT:
		CON_Printf( "%s %c %d", var->name, equals, *(int*)var->ptr);
		break;
	case CVT_FLOAT:
		CON_Printf( "%s %c %.4f", var->name, equals, *(float*)var->ptr);
		break;
	case CVT_CHARPTR:
		CON_Printf( "%s %c %s", var->name, equals, *(char**)var->ptr);
		break;
	default:
		CON_Printf( "%s (bad type!)", var->name);
		break;
	}
	CON_Printf( "\n");
}

// expcommand gets reallocated in the expansion process.
static void expandWithArguments(char **expcommand, cmdargs_t *args)
{
	char *text = *expcommand;
	int size = strlen(text)+1;
	int	i, off;

	for(i=0; text[i]; i++)
	{
		if(text[i] == '%' && (text[i+1] >= '1' && text[i+1] <= '9'))
		{
			char *substr;
			int aidx = text[i+1]-'1'+1;
			
			// Expand! (or delete)
			if(aidx > args->argc-1)
				substr = "";
			else
				substr = args->argv[aidx];

			// First get rid of the %n.
			memmove(text+i, text+i+2, size-i-2);
			// Reallocate.
			off = strlen(substr);
			text = *expcommand = (char *)realloc(*expcommand, size+=off-2);
			// -JL- Paranoia
			I_Error("expandWithArguments: realloc failed");
			if(off)
			{
				// Make room for the insert.
				memmove(text+i+off, text+i, size-i-off);												
				memcpy(text+i, substr, off);
			}
			i += off-1;
		}
	}
}

// The command is executed forthwith!!
static int executeSubCmd(char *subcmd)
{
	int			i;
	char		prefix;
	cmdargs_t	args;

	PrepareCmdArgs(&args, subcmd);
	if(!args.argc) return true;

	if(args.argc == 1)	// An action?
	{
		prefix = args.argv[0][0];
		if(prefix == '+' || prefix == '-')
		{
			return DD_ActionCommand(args.argv[0], true);
		}
		// What about a prefix-less action?
		if(strlen(args.argv[0]) <= 8 && DD_ActionCommand(args.argv[0], false))
			return true; // There was one!
	}

	// Try to find a matching command.
	for(i=0; i<numCCmds; i++)
		if(!stricmp(ccmds[i].name, args.argv[0]))
		{
			int cret = ccmds[i].func(args.argc, args.argv);
			if(cret == false)
				CON_Printf( "Error: '%s' failed.\n", ccmds[i].name);
			// We're quite done.
			return true;
		}
	// Then try the cvars?
	for(i=0; i<numCVars; i++)
		if(!stricmp(cvars[i].name, args.argv[0]))
		{
			cvar_t *var = cvars+i;
			boolean out_of_range = false, setting = false;
			if(args.argc == 2 || (args.argc == 3 && !stricmp(args.argv[1], "force")))
			{
				char *argptr = args.argv[ args.argc-1 ];
				boolean forced = args.argc==3;
				setting = true;
				if(var->flags & CVF_PROTECTED && !forced)
				{
					CON_Printf( "%s is protected. You shouldn't change its value.\n", var->name);
					CON_Printf( "Use the command: '%s force %s' to modify it anyway.\n",
						var->name, argptr);
				}
				else if(var->type == CVT_BYTE)
				{
					byte val = (byte) strtol(argptr, NULL, 0);
					if(!forced && ((!(var->flags & CVF_NO_MIN) && val < var->min) ||
						(!(var->flags & CVF_NO_MAX) && val > var->max)))
						out_of_range = true;
					else
						*(byte*)var->ptr = val;
				}
				else if(var->type == CVT_INT)
				{
					int val = strtol(argptr, NULL, 0);
					if(!forced && ((!(var->flags & CVF_NO_MIN) && val < var->min) ||
						(!(var->flags & CVF_NO_MAX) && val > var->max)))
						out_of_range = true;
					else
						*(int*)var->ptr = val;
				}
				else if(var->type == CVT_FLOAT)
				{
					float val = strtod(argptr, NULL);
					if(!forced && ((!(var->flags & CVF_NO_MIN) && val < var->min) ||
						(!(var->flags & CVF_NO_MAX) && val > var->max)))
						out_of_range = true;
					else
						*(float*)var->ptr = val;
				}
				else if(var->type == CVT_CHARPTR)
				{
					CvarSetString(var, argptr);
				}
			}
			if(out_of_range)
			{
				if(!(var->flags & (CVF_NO_MIN|CVF_NO_MAX)))
				{
					CON_Printf( "Error: %.4f <= %s <= %.4f\n", 
						var->min, var->name, var->max);
				}
				else if(var->flags & CVF_NO_MAX)
				{
					CON_Printf( "Error: %s >= %.4f\n", var->name, var->min);
				}
				else 
				{
					CON_Printf( "Error: %s <= %.4f\n", var->name, var->max);
				}
			}
			else if(!setting || setting && !conSilentCVars)	// Show the value.
				printcvar(var, "");
			return true;
		}

	// How about an alias then?
	for(i=0; i<numCAliases; i++)
		if(!stricmp(args.argv[0], caliases[i].name))
		{
			calias_t *cal = caliases + i;
			char *expcommand;
			// This alias matches. We're going to execute it, but first
			// check the refcount.
/*			if(cal->refcount > 0)
			{
				CON_Printf( "Error: recursion in aliases.\n");
				return false;
			}
			cal->refcount++;*/
			// Expand the command with arguments.
			expcommand = (char *)malloc(strlen(cal->command)+1);
			// -JL- Paranoia
			if (!expcommand)
				I_Error("executeSubCmd: malloc failed");
			strcpy(expcommand, cal->command);
			expandWithArguments(&expcommand, &args);
			// Do it, man!
			SplitIntoSubCommands(expcommand, 0);
			free(expcommand);
			// Decrement the refcount.
			//cal->refcount--;
			//return ret;
			return true;
		}

	// What *is* that?
	CON_Printf( "%s: no such command or cvar.\n", args.argv[0]);
	return false;
}

// Splits the command into subcommands and queues them into the 
// execution buffer.
static void SplitIntoSubCommands(char *command, int markerOffset)
{
	int			gpos = 0, scpos = 0;
	char		subcmd[256];
	int			nextsub = false;
	int			ret = true, inquotes = false, escape = false;

	// Is there a command to execute?
	if(!command || command[0] == 0) return;

	// Jump over initial semicolons.
	while(command[gpos] == ';' && command[gpos] != 0) gpos++;
	// The command may actually contain many commands, separated
	// with semicolons. This isn't a very clear algorithm...
	for(strcpy(subcmd, ""); command[gpos];)
	{
		escape = false;
		if(inquotes && command[gpos] == '\\') // Escape sequence?
		{
			subcmd[scpos++] = command[gpos++];
			escape = true;
		}
		if(command[gpos] == '"' && !escape) 
			inquotes = !inquotes;

		// Collect characters.
		subcmd[scpos++] = command[gpos++];
		if(subcmd[0] == ' ') scpos = 0;	// No spaces in the beginning.

		if((command[gpos] == ';' && !inquotes) || command[gpos] == 0)
		{
			while(command[gpos] == ';' && command[gpos] != 0) gpos++;
			// The subcommand ends.
			subcmd[scpos] = 0;
		}
		else continue;
	
		// Queue it.
		QueueCmd(subcmd, systics + markerOffset); 
		
		scpos = 0;
	}
}

// Returns false if a command fails.
int CON_Execute(char *command, int silent)
{
	//int			gpos = 0, scpos = 0;
	//char		subcmd[256];
	//int			nextsub = false;
	//int			ret = true, inquotes = false, escape = false;
	int ret;

//	waitStackEnabled = canwait;

/*	// Is there a command to execute?
	if(command[0] == 0) goto outta_here;

	// Jump over initial semicolons.
	while(command[gpos] == ';' && command[gpos] != 0) gpos++;
	// The command may actually contain many commands, separated
	// with semicolons. This isn't a very clear algorithm...
	for(strcpy(subcmd, ""); command[gpos];)
	{
		escape = false;
		if(inquotes && command[gpos] == '\\') // Escape sequence?
		{
			subcmd[scpos++] = command[gpos++];
			escape = true;
		}
		if(command[gpos] == '"' && !escape) 
			inquotes = !inquotes;

		// Collect characters.
		subcmd[scpos++] = command[gpos++];
		
		if(subcmd[0] == ' ') scpos = 0;	// No spaces in the beginning.

		if((command[gpos] == ';' && !inquotes) || command[gpos] == 0)
		{
			while(command[gpos] == ';' && command[gpos] != 0) gpos++;
			// The subcommand ends.
			subcmd[scpos] = 0;
		}
		else continue;
	
		//CON_Printf( "exec: %s\n", subcmd);
		
		// Prepare the arguments for execution 
		// (notice: same as with program cmdline args).
		ret = executeSubCmd(subcmd);
		scpos = 0;
	}

outta_here:*/

	if(silent) ConsoleSilent = true;
	
	SplitIntoSubCommands(command, 0);
	ret = CheckExecBuffer();

	if(silent) ConsoleSilent = false;
	return ret;
}

static void processCmd()
{
	I_ClearKeyRepeaters();

	// Add the command line to the oldCmds buffer.
	addOldCmd(cmdLine);
	ocPos = numOldCmds;

	CON_Execute(cmdLine, false);
}

static void updateCmdLine()
{
	if(ocPos == numOldCmds)
		strcpy(cmdLine, "");
	else
		strcpy(cmdLine, oldCmds[ocPos].text);
	complPos = strlen(cmdLine);
	lastCompletion = -1;
}

// Look at the last word and try to complete it. If there are
// several possibilities, print them.
static void completeWord()
{
	int		pass, i, c, cp = strlen(cmdLine)-1;
	int		numcomp = 0;
	char	word[100], *wordBegin;
	char	*completion=0;	// Pointer to the completed word.

	if(conCompMode == 1) cp = complPos-1;

	if(cp<0) return;
	if(cmdLine[cp] == ' ') return;	// Nothing to complete.
	// Rewind the word pointer until space or a semicolon is found.
	while(cp > 0 && cmdLine[cp-1] != ' ' && cmdLine[cp-1] != ';' && cmdLine[cp-1] != '"') 
		cp--;
	// Now cp is at the beginning of the word that needs completing.
	strcpy(word, wordBegin = cmdLine + cp);

	if(conCompMode == 1)
		word[complPos-cp] = 0;	// Only check a partial word.
	
	// The completions we know are the cvars and ccmds.
	for(pass=1; pass<=2; pass++)
	{
		if(pass == 2)	// Print the possible completions.
			CON_Printf( "Completions:\n");

		// Look through the known words.
		for(c=0, i = (conCompMode==0? 0 : (lastCompletion+1)); c<numKnownWords; c++, i++)
		{
			if(i > numKnownWords-1) i = 0;
			if(!strnicmp(knownWords[i], word, strlen(word)))
			{
				// This matches!
				if(pass == 1) // Pass one: count completions, update completion.
				{
					numcomp++;
					completion = knownWords[i];
					if(conCompMode == 1)
					{
						lastCompletion = i;
						break;
					}
				}
				else // Pass two: print it.
					CON_Printf( "  %s\n", knownWords[i]);
			}
		}
		if(numcomp <= 1 || conCompMode == 1) break;				
	}
	// Was a single completion found?
	if(numcomp == 1)
	{
		strcpy(wordBegin, completion);
		//strupr(wordBegin);
	}
}

// Returns true if the event is eaten.
boolean CON_Responder(event_t *event)
{
	int pos;
	char ch;

	// We should track the state of the shift key.
	if(event->data1 == DDKEY_RSHIFT)
	{
		if(event->type == ev_keydown)
			shiftDown = true;
		else if(event->type == ev_keyup)
			shiftDown = false;
	}

	if(!ConsoleActive)
	{
		// In this case we are only interested in the activation key.
		if(event->type == ev_keydown && event->data1 == '`'/* && !MenuActive*/)
		{
			/*ConsoleActive = true;
			ConsoleDestY = ConsoleOpenY;
			ConsoleTime = 0;
			openingOrClosing = true;*/
			CON_Open(true);
			return true;
		}
		return false;
	}

	// All keyups are eaten by the console.
	if(event->type == ev_keyup) return true; 

	// We only want keydown events.
	if(event->type != ev_keydown && event->type != ev_keyrepeat) 
		return false;

	// In this case the console is active and operational.
	// Check the shutdown key.
	if(event->data1 == '`')
	{
		ConsoleActive = false;
		ConsoleDestY = 0;
		openingOrClosing = true;
		return true;
	}

	// Hitting Escape in the console...
	if(event->data1 == DDKEY_ESCAPE)
	{
		ConsoleActive = false;
		ConsoleDestY = 0;
		return false;	// Let the menu know about this.
	}

	switch(event->data1)
	{
	case DDKEY_UPARROW:
		if(--ocPos < 0) ocPos = 0;
		// Update the command line.
		updateCmdLine();
		return true;
	
	case DDKEY_DOWNARROW:
		if(++ocPos > numOldCmds) ocPos = numOldCmds;
		updateCmdLine();
		return true;
	
	case DDKEY_PGUP:
		bLineOff++;
		if(bLineOff > bPos-1) bLineOff = bPos-1;
		return true;
	
	case DDKEY_PGDN:
		bLineOff--;
		if(bLineOff < 0) bLineOff = 0;
		return true;
	
	case DDKEY_INS:
		ConsoleOpenY -= fontSy;
		if(ConsoleOpenY < fontSy) ConsoleOpenY = fontSy;
		ConsoleDestY = ConsoleOpenY;
		return true;
	
	case DDKEY_DEL:
		ConsoleOpenY += fontSy;
		if(ConsoleOpenY > 200) ConsoleOpenY = 200;
		ConsoleDestY = ConsoleOpenY;
		return true;
	
	case DDKEY_END:
		bLineOff = 0;
		return true;
	
	case DDKEY_HOME:
		bLineOff = bPos-1;
		return true;

	case DDKEY_ENTER:
		// Print the command line with yellow text.
		CON_FPrintf(CBLF_YELLOW, ">%s\n", cmdLine);
		// Process the command line.
		processCmd();		
		// Clear it.
		cmdLine[0] = 0;
		complPos = 0;
		lastCompletion = -1;
		return true;

	case DDKEY_BACKSPACE:
		pos = strlen(cmdLine);
		if(pos) cmdLine[pos-1] = 0;
		complPos = strlen(cmdLine);
		lastCompletion = -1;
		return true;

	case DDKEY_TAB:
		completeWord();
		return true;

	default:	// Check for a character.
		ch = event->data1;
		if(ch < 32 || ch > 127) return true;
//		ch = toupper(ch);
		if(shiftDown && ch <= 127) ch = shiftTable[ch-32];
//		if(ch >= 32 && ch <= 'Z')	// A printable character?
//		{
		pos = strlen(cmdLine);
		if(pos > 79) pos = 79;
		cmdLine[pos] = ch;
		cmdLine[pos+1] = 0;
		complPos = strlen(cmdLine);
		lastCompletion = -1;
		return true;
//		}
	}
	// The console is very hungry for keys...
	return true;
}

/*static int FindPrevBufferLineStart(int numlines)
{
	int count = 0;
	int	cp = bPos;
	
	while(--cp >= 0)
		if(buffer[cp] == '\n') 
			if(++count == numlines+1) return cp+1;
	// We won't go past the beginning.
	return 0;
}*/

static void consoleSetColor(int fl, float alpha)
{
	float	r=0, g=0, b=0;
	int		count=0;

	// Calculate the average of the given colors.
	if(fl & CBLF_BLACK)
	{
		count++;
	}
	if(fl & CBLF_BLUE)
	{
		b += 1;
		count++;
	}
	if(fl & CBLF_GREEN)
	{
		g += 1;
		count++;
	}
	if(fl & CBLF_CYAN)
	{
		g += 1;
		b += 1;
		count++;
	}
	if(fl & CBLF_RED)
	{
		r += 1;
		count++;
	}
	if(fl & CBLF_MAGENTA)
	{
		r += 1;
		b += 1;
		count++;
	}
	if(fl & CBLF_YELLOW)
	{
		r += CcolYellow[0];
		g += CcolYellow[1];
		b += CcolYellow[2];
		count++;
	}
	if(fl & CBLF_WHITE)
	{
		r += 1;
		g += 1;
		b += 1;
		count++;
	}
	// Calculate the average.
	if(count)
	{
		r /= count;
		g /= count;
		b /= count;
	}
	if(fl & CBLF_LIGHT)
	{
		r += (1-r)/2;
		g += (1-g)/2;
		b += (1-b)/2;
	}
	gl.Color4f(r, g, b, alpha);
}

void CON_SetFont(ddfont_t *cfont)
{
	Cfont = *cfont;
}

// Slightly messy...
void CON_Drawer(void)
{
//	int fontALump = W_GetNumForName("FONTA_S")+1;
	int i;	// Line count and buffer cursor.
	float y;
	float closeFade = 1;
	float gtosMulX = screenWidth/320.0f, gtosMulY = screenHeight/200.0f;
	char buff[256];
	float fontScaledY = Cfont.height * Cfont.sizeY;
	int bgX=64, bgY=64;

	if(ConsoleY == 0) return;	// We have nothing to do here.

	fontSy = fontScaledY/gtosMulY;

	// Do we have a font?
	if(Cfont.TextOut == NULL)
	{
		Cfont.flags = DDFONT_WHITE;
		Cfont.height = FR_TextHeight("Con");
		Cfont.sizeX = 1;
		Cfont.sizeY = 1;
		Cfont.TextOut = FR_TextOut;
		Cfont.Width = FR_TextWidth;
		Cfont.Filter = NULL;
	}

	// Go into screen projection mode.
	gl.MatrixMode(DGL_PROJECTION);
	gl.PushMatrix();
	gl.LoadIdentity();
	gl.Ortho(0, 0, screenWidth, screenHeight, -1, 1);

	BorderNeedRefresh = true;

	if(openingOrClosing)
	{
		closeFade = ConsoleY / (float) ConsoleOpenY;
	}

	// The console is composed of two parts: the main area background and the 
	// border.
	gl.Color4f(consoleLight/100.0f, consoleLight/100.0f, 
		consoleLight/100.0f, closeFade * consoleAlpha/100);

	// The background.
	if(gx.ConsoleBackground) gx.ConsoleBackground(&bgX, &bgY);

	// Let's make it a bit more interesting.
	gl.MatrixMode(DGL_TEXTURE);
	gl.PushMatrix();
	gl.LoadIdentity();
	gl.Translatef(2*sin(funnyAng/4), 2*cos(funnyAng/4), 0);
	gl.Rotatef(funnyAng*3, 0, 0, 1);
	/*GL_DrawRectTiled(0, (int)ConsoleY*gtosMulY-screenHeight, screenWidth, 
		screenHeight, bgX, bgY);*/
	GL_DrawRectTiled(0, (int)ConsoleY*gtosMulY+4, screenWidth, -screenHeight-4,
		bgX, bgY);
	gl.PopMatrix();

	// The border.
	GL_DrawRect(0, (int)ConsoleY*gtosMulY+3, screenWidth, 2,
		0,0,0, closeFade);

	gl.MatrixMode(DGL_MODELVIEW);
	gl.PushMatrix();
	gl.LoadIdentity();
	gl.Scalef(Cfont.sizeX, Cfont.sizeY, 1);

	// The version.
	y = ConsoleY*gtosMulY - fontScaledY*2;
	if(y > -fontScaledY)
	{
//		char temp[10];
		int ssw = (int) (screenWidth/Cfont.sizeX);	 // scaled screen width
		// Print the version.
		strcpy(buff, gx.GetString(DD_VERSION_SHORT));
		//strupr(temp);
		gl.Color4f(CcolYellow[0], CcolYellow[1], CcolYellow[2], closeFade);
		if(Cfont.Filter) Cfont.Filter(buff);
		Cfont.TextOut(buff, ssw - Cfont.Width(buff), y/Cfont.sizeY);
//		MN_DrTextAYellow_CS(temp, rsw-MN_TextAWidth(temp), y);
	}

	gl.Color4f(1, 1, 1, closeFade);

	// The text in the console buffer will be drawn from the bottom up (!).
	for(i=bPos-bLineOff-1, y=ConsoleY*gtosMulY-fontScaledY*2; 
		i>=0 && i<bufferLines && y>-fontScaledY; i--)
	{
		cbline_t *line = cbuffer + i;
//		int c, k;
/*		for(c=0, k=0; c<line->len; c++)
		{
			char ch = toupper(line->text[c]);
			if(ch == '_') ch = '[';	// Mysterious... (from save slots).
			else if(ch == '\\') ch = '/';
			// Check that the character is printable.
			else if(ch < 32 || ch > 'Z') ch = 32;//continue; // Can't handle it.
			buff[k++] = ch;			
		}*/
		memset(buff, 0, sizeof(buff));
		strncpy(buff, line->text, 255);
		//if(Cfont.flags & DDFONT_UPPERCASE) strupr(buff);

		if(Cfont.Filter) Cfont.Filter(buff);
		
		//buff[k] = 0;	// End the buffer.

		// Set the color.
		if(Cfont.flags & DDFONT_WHITE) // Can it be colored?
			consoleSetColor(line->flags, closeFade);
		Cfont.TextOut(buff, 2, y/Cfont.sizeY);
		
		// Move up.
		y -= fontScaledY;
	}
	
	// The command line.
	if(Cfont.flags & DDFONT_WHITE)
		gl.Color4f(CcolYellow[0], CcolYellow[1], CcolYellow[2], closeFade);
	else
		gl.Color4f(1, 1, 1, closeFade);
	
	strcpy(buff, ">");
	strcat(buff, cmdLine);
	if(ConsoleTime & 0x10) strcat(buff, "_");

	if(Cfont.Filter) Cfont.Filter(buff);
	Cfont.TextOut(buff, 2, (ConsoleY*gtosMulY-fontScaledY)/Cfont.sizeY);

//	Cfont.TextOut(">", 2, (ConsoleY*gtosMulY-fontScaledY)/Cfont.sizeY);
	//Cfont.TextOut(cmdLine, i = 2+Cfont.Width(">"), (ConsoleY*gtosMulY-fontScaledY)/Cfont.sizeY);
/*	MN_DrTextAYellow_CS(")", 2, ConsoleY-fontSy);
	MN_DrTextAYellow_CS(cmdLine, 10, ConsoleY-fontSy);*/
	// The blinking cursor.
/*	if(ConsoleTime & 0x10)
	{
//		MN_DrTextAYellow_CS("[", 10+MN_TextAWidth(cmdLine), ConsoleY-fontSy);
		Cfont.TextOut("_", i+Cfont.Width(cmdLine), (ConsoleY*gtosMulY-fontScaledY)/Cfont.sizeY);
	*/

	gl.MatrixMode(DGL_MODELVIEW);
	gl.PopMatrix();

	gl.MatrixMode(DGL_PROJECTION);
	gl.PopMatrix();
}

void conPrintf(int flags, char *format, va_list args)
{
	unsigned int i;
	int			lbc; // line buffer cursor
	char		prbuff[PRBUFF_LEN], *lbuf = (char *)malloc(maxLineLen+1);
	cbline_t	*line;

	// -JL- Paranoia
	if (!lbuf)
		I_Error("conPrintf: malloc failed\n");

	// Format the message to prbuff.
	vsprintf(prbuff, format, args);

	if(consoleDump) printf(prbuff);

	if(strlen(prbuff) > PRBUFF_LEN) 
		I_Error("CON_Printf: Too long console message has overwritten memory.\n");

	// We have the text we want to add in the buffer in prbuff.
	line = CON_GetBufferLine(bPos);	// Get a pointer to the current line.
	line->flags = flags;
	memset(lbuf, 0, maxLineLen+1);	
	for(i=0, lbc=0; i<strlen(prbuff); i++)
	{
		if(prbuff[i] == '\n' || lbc+line->len >= maxLineLen)	// A new line?
		{
			// Set the line text.
			addLineText(line, lbuf);
			// Clear the line write buffer.
			memset(lbuf, 0, maxLineLen+1);
			lbc = 0;
			// Get the next line.
			line = CON_GetBufferLine(++bPos);
			line->flags = flags;
			// Newlines won't get in the buffer at all.
			if(prbuff[i] == '\n') continue;
		}
		lbuf[lbc++] = prbuff[i];
	}
	// Something still in the write buffer?
	if(lbc) addLineText(line, lbuf);

	// Clean up.
	free(lbuf);

	// Now that something new has been printed, it will be shown.
	bLineOff = 0;

	// Check if there are too many lines.
	if(bufferLines > maxBufferLines)
	{
		int rev = bufferLines - maxBufferLines;
		// The first 'rev' lines get removed.
		for(i=0; (int)i<rev; i++) free(cbuffer[i].text);
		memmove(cbuffer, cbuffer+rev, sizeof(cbline_t)*(bufferLines-rev));
		//for(i=0; (int)i<rev; i++) memset(cbuffer+bufferLines-rev+i, 0, sizeof(cbline_t));
		cbuffer = (cbline_t *)realloc(cbuffer, sizeof(cbline_t)*(bufferLines-=rev));
		// -JL- Paranoia
		if (!cbuffer)
			I_Error("conPrintf: realloc failed");
		// Move the current position.
		bPos -= rev;
	}
}

// Print into the buffer.
void CON_Printf(char *format, ...)
{
	va_list		args;

	if(ConsoleSilent) return;

	va_start(args, format);	
	conPrintf(CBLF_WHITE, format, args);
	va_end(args);
}

void CON_FPrintf(int flags, char *format, ...) // Flagged printf
{
	va_list		args;
	
	if(ConsoleSilent) return;

	va_start(args, format);
	conPrintf(flags, format, args);
	va_end(args);
}

// As you can see, several commands can be handled inside one command function.
int CCmdConsole(int argc, char **argv)
{
	if(!stricmp(argv[0], "help"))
	{
		if(argc == 2)
		{
			int	i;
			if(!stricmp(argv[1], "(what)"))
			{
				CON_Printf( "You've got to be kidding!\n");
				return true;
			}
			// We need to look through the cvars and ccmds to see if there's a match.
			for(i=0; i<numCCmds; i++)
				if(!stricmp(argv[1], ccmds[i].name))
				{
					CON_Printf("%s\n", ccmds[i].help);
					return true;
				}
			for(i=0; i<numCVars; i++)
				if(!stricmp(argv[1], cvars[i].name))
				{
					CON_Printf("%s\n", cvars[i].help);
					return true;
				}
		}
		else
		{
			CON_Printf("\n");
			CON_FPrintf(CBLF_YELLOW, " --- This is the Doomsday console ---\n");
			CON_Printf( "Use the tilde key to open and close it.\n");
			CON_Printf( "Page up/down scrolls up/down a line.\n");
			CON_Printf( "Ins moves the console up a bit, Del moves it down.\n");
			CON_Printf( "Home jumps to the beginning of the buffer, End to the end.\n");
			CON_Printf( "Type \"listcmds\" to see a list of available commands.\n");
			CON_Printf( "Type \"help (what)\" to see information about (what).\n\n");
		}
	}
	else if(!stricmp(argv[0], "clear"))
	{
		CON_ClearBuffer();
	}
	return true;	
}

int CCmdListCmds(int argc, char **argv)
{
	int		i;

	CON_Printf( "Console commands:\n");
	for(i=0; i<numCCmds; i++)
	{
		if(argc > 1) // Is there a filter?
			if(strnicmp(ccmds[i].name, argv[1], strlen(argv[1])))
				continue;
		CON_Printf( "  %s\n", ccmds[i].name);
	}
	return true;
}

int CCmdListVars(int argc, char **argv)
{
	int		i;

	CON_Printf( "Console variables:\n");
	for(i=0; i<numCVars; i++)
	{
		if(argc > 1) // Is there a filter?
			if(strnicmp(cvars[i].name, argv[1], strlen(argv[1])))
				continue;
		printcvar(cvars+i, "  ");
	}
	return true;
}

DEFCC(CCmdListAliases)
{
	int		i;

	CON_Printf( "Aliases:\n");
	for(i=0; i<numCAliases; i++)
	{
		if(argc > 1) // Is there a filter?
			if(strnicmp(caliases[i].name, argv[1], strlen(argv[1])))
				continue;
		CON_Printf( "  %s == %s\n", caliases[i].name, caliases[i].command);
	}
	return true;
}

int CCmdVersion(int argc, char **argv)
{
	CON_Printf( "Korax Engine %s ("__TIME__")\n", DOOMSDAY_VERSIONTEXT);
	CON_Printf( "%s\n", gl.GetString(DGL_VERSION));
	CON_Printf( "%s\n", jtNetGetString(JTNET_VERSION));
//	CON_Printf( "Compiler: Microsoft Visual C++ 6.0 (SP3)\n");
	CON_Printf( "Game DLL: %s\n", gx.GetString(DD_VERSION_LONG));
	return true;
}

int CCmdQuit(int argc, char **argv)
{
	// No questions asked.
	I_Quit();

	/*extern boolean askforquit, typeofask;

	ConsoleActive = false;
	ConsoleDestY = 0;
	askforquit = true;
	typeofask = 1; //quit game
	if(!netgame && !demoplayback)
		paused = true;*/
	return true;
}

void CON_Open(int yes)
{
	// Clear all action keys, keyup events won't go 
	// to bindings processing when the console is open.
	DD_ClearActions();
	openingOrClosing = true;
	if(yes)
	{
		ConsoleActive = true;
		ConsoleDestY = ConsoleOpenY;
		ConsoleTime = 0;
	}
	else
	{
		ConsoleActive = false;
		ConsoleDestY = 0;
	}
}

/*int CCmdPlayerInfo(int argc, char **argv)
{
	int		i;
	
	CON_Printf( "Player info:\n");
	for(i=0; i<MAXPLAYERS; i++)
		if(playeringame[i])
		{
			CON_Printf( "%d: trflags %x\n", i+1, (players[i].mo->flags & MF_TRANSLATION)
				>> MF_TRANSSHIFT);			
		}
	return true;
}*/

//extern int Mus_Song;

void UpdateEngineState()
{
	// Update refresh.
	ST_Message( "Updating state");

	if(gx.UpdateState) gx.UpdateState(DD_PRE);
	R_Update();
	DD_ValidateLevel();
	if(gx.UpdateState) gx.UpdateState(DD_POST);

	/*gx->P_Init();
	P_Validate();
	SB_Init(); // Updates the status bar patches.
	MN_Init();
	S_Reset();
	S_InitScript();
	SN_InitSequenceScript();*/
	ST_Message( "\n");
}

int CCmdLoadFile(int argc, char **argv)
{
	//extern int RegisteredSong;
	int		i, succeeded = false;	
	
	if(argc == 1)
	{
		CON_Printf( "Usage: load (file) ...\n");
		return true;
	}
	for(i=1; i<argc; i++)
	{
		ST_Message( "Loading %s... ", argv[i]);
		if(W_AddFile(argv[i]))	
		{
			ST_Message( "OK\n");
			succeeded = true; // At least one has been loaded.
		}
		else
			ST_Message( "Failed!\n");
	}
	// We only need to update if something was actually loaded.
	if(succeeded)
	{
		// Update the lumpcache.
		//W_UpdateCache();
		// The new wad may contain lumps that alter the current ones
		// in use.
		UpdateEngineState();
	}
	return true;
}

int CCmdUnloadFile(int argc, char **argv)
{
	//extern int RegisteredSong;
	int		i, succeeded = false;	

	if(argc == 1)
	{
		CON_Printf( "Usage: unload (file) ...\n");
		return true;
	}
	// Stop the engine.
//	S_StopAllSound();
//	SN_StopAllSequences();
//	I_StopSong();
//	I_UnRegisterSong();
	//RegisteredSong = 0;	
	//I_ShutdownTimer();
	
	//I_SaveTime();

	for(i=1; i<argc; i++)
	{
		ST_Message("Unloading %s... ", argv[i]);
		if(W_RemoveFile(argv[i]))
		{
			ST_Message("OK\n");
			succeeded = true;
		}
		else
			ST_Message("Failed!\n");
	}
	if(succeeded) UpdateEngineState();
	//I_StartupTimer();
	//I_ResumeSong();
	//I_RestoreTime();
	return true;
}

int CCmdListFiles(int argc, char **argv)
{
	extern int numrecords;
	extern filerecord_t *records;
	int		i;
	
	for(i=0; i<numrecords; i++)
		CON_Printf( "%s (%d lump%s%s)\n",
			records[i].filename,
			records[i].numlumps,
			records[i].numlumps!=1? "s" : "",
			!(records[i].flags & FRF_RUNTIME)? ", startup" : "");

	CON_Printf("Total: %d lumps in %d files.\n", numlumps, numrecords);
	return true;
}

int CCmdResetLumps(int argc, char **argv)
{
	//extern int RegisteredSong;

	// This game ends here.
//	P_ClearMessage(&players[consoleplayer]);
//	paused = false;
	GL_SetFilter(0);
//	H2_StartTitle(); // go to intro/demo mode.

	// This may take a while, so stop the music and the timer.
//	S_StopAllSound();
//	SN_StopAllSequences();
//	I_StopSong();
//	I_UnRegisterSong();
	//RegisteredSong = 0;	
//	I_ShutdownTimer();

	//I_SaveTime();

	W_Reset();
	ST_Message( "Only startup files remain.\n");	

	UpdateEngineState();

	//I_RestoreTime();
//	I_StartupTimer();
	//I_ResumeSong();
	return true;
}

int CCmdBackgroundTurn(int argc, char **argv)
{
	if(argc != 2)
	{
		CON_Printf( "Usage: bgturn (speed)\n");
		CON_Printf( "Negative speeds are allowed. Default: 20.\n");
		CON_Printf( "Current bgturn = %d.\n", consoleTurn);
		return true;
	}
	consoleTurn = atoi(argv[1]);
	if(!consoleTurn) funnyAng = 0;
	return true;
}

/*int CCmdTest(int argc, char **argv)
{
	CON_Printf( "floorpic: %d\n", players[consoleplayer].mo->floorpic);
	CON_Printf( "plr on floortype: %d\n", P_GetThingFloorType(players[consoleplayer].mo));
	return true;
}*/

int CCmdDump(int argc, char **argv)
{
	char fname[100];
	FILE *file;
	int lump;
	byte *lumpPtr;

	if(argc != 2)
	{
		CON_Printf( "Usage: dump (name)\n");
		CON_Printf( "Writes out the specified lump to (name).dum.\n");
		return true;		
	}
	if(W_CheckNumForName(argv[1]) == -1)
	{
		CON_Printf( "No such lump.\n");
		return false;
	}
	lump = W_GetNumForName(argv[1]);
	lumpPtr = (byte *)W_CacheLumpNum(lump, PU_STATIC);

	sprintf(fname, "%s.dum", argv[1]);
	file = fopen(fname, "wb");
	if(!file) 
	{
		CON_Printf("Couldn't open %s for writing. %s\n", fname, strerror(errno));
		Z_ChangeTag(lumpPtr, PU_CACHE);
		return false;
	}
	fwrite(lumpPtr, 1, lumpinfo[lump].size, file);
	fclose(file);
	Z_ChangeTag(lumpPtr, PU_CACHE);

	CON_Printf( "%s dumped to %s.\n", argv[1], fname);
	return true;
}


int CCmdResetTextures(int argc, char **argv)
{
	GL_ClearTextureMemory();
	CON_Printf( "All DGL textures deleted.\n");
	return true;
}

int CCmdMipMap(int argc, char **argv)
{
	if(argc != 2)
	{
		CON_Printf( "Usage: %s (0-5)\n", argv[0]);
		CON_Printf( "0 = GL_NEAREST\n");
		CON_Printf( "1 = GL_LINEAR\n");
		CON_Printf( "2 = GL_NEAREST_MIPMAP_NEAREST\n");
		CON_Printf( "3 = GL_LINEAR_MIPMAP_NEAREST\n");
		CON_Printf( "4 = GL_NEAREST_MIPMAP_LINEAR\n");
		CON_Printf( "5 = GL_LINEAR_MIPMAP_LINEAR\n");
		return true;
	}
	GL_UpdateTexParams(strtol(argv[1], NULL, 0));
	return true;
}

int CCmdSmoothRaw(int argc, char **argv)
{
	if(argc != 2)
	{
		CON_Printf( "Usage: %s (0-1)\n", argv[0]);
		CON_Printf( "Set the rendering mode of fullscreen images.\n");
		CON_Printf( "0 = GL_NEAREST\n");
		CON_Printf( "1 = GL_LINEAR\n");
		return true;
	}
	GL_UpdateRawScreenParams(strtol(argv[1], NULL, 0));	
	return true;
}

int CCmdSkyDetail(int argc, char **argv)
{
	if(!stricmp(argv[0], "skydetail"))
	{
		if(argc != 2)
		{
			CON_Printf( "Usage: skydetail (num)\n");
			CON_Printf( "(num) is the number of sky sphere quadrant subdivisions.\n");
			return true;
		}
		R_SkyDetail(strtol(argv[1], NULL, 0), skyRows);				
	}
	else if(!stricmp(argv[0], "skyrows"))
	{
		if(argc != 2)
		{
			CON_Printf( "Usage: skyrows (num)\n");
			CON_Printf( "(num) is the number of sky sphere rows.\n");
			return true;
		}
		R_SkyDetail(skyDetail, strtol(argv[1], NULL, 0));
	}
	return true;
}

int CCmdSetMusicDevice(int argc, char **argv)
{
	int		musdev;

	if(argc != 2)
	{
		CON_Printf( "Usage: %s (0-2)/none/midi/cd\n", argv[0]);
		return true;
	}
	musdev = strtol(argv[1], NULL, 0);
	if(!stricmp(argv[1], "none")) musdev = 0;
	if(!stricmp(argv[1], "midi")) musdev = 1;
	if(!stricmp(argv[1], "cd")) musdev = 2;
	I_SetMusicDevice(musdev);
	return true;
}

int CCmdSetMIDIVolume(int argc, char **argv)
{
	if(argc != 2)
	{
		CON_Printf( "Usage: %s (0-255)\n", argv[0]);
		return true;
	}
	I_SetMusicVolume(strtol(argv[1], NULL, 0));
	return true;
}

int CCmdSetCDVolume(int argc, char **argv)
{
	if(argc != 2)
	{
		CON_Printf( "Usage: %s (0-255)\n", argv[0]);
		return true;
	}
	I_CDMusSetVolume(strtol(argv[1], NULL, 0));
	return true;
}

DEFCC(CCmdFont)
{
	if(argc == 1 || argc > 3)
	{
		CON_Printf( "Usage: %s (cmd) (args)\n", argv[0]);
		CON_Printf( "Commands: default, size, xsize, ysize.\n");
		CON_Printf( "Size 1.0 is normal.\n");
		return true;
	}
	if(!stricmp(argv[1], "default"))
	{
		Cfont.flags = DDFONT_WHITE;
		Cfont.height = FR_TextHeight("Con");
		Cfont.sizeX = 1;
		Cfont.sizeY = 1;
		Cfont.TextOut = FR_TextOut;
		Cfont.Width = FR_TextWidth;
		Cfont.Filter = NULL;
	}
	else if(argc == 3)
	{
		if(!stricmp(argv[1], "xsize") || !stricmp(argv[1], "size"))
			Cfont.sizeX = strtod(argv[2], NULL);
		if(!stricmp(argv[1], "ysize") || !stricmp(argv[1], "size"))
			Cfont.sizeY = strtod(argv[2], NULL);
		// Make sure the sizes are valid.
		if(Cfont.sizeX <= 0) Cfont.sizeX = 1;
		if(Cfont.sizeY <= 0) Cfont.sizeY = 1;
	}
	return true;
}

DEFCC(CCmdAlias)
{
	if(argc != 3 && argc != 2)
	{
		CON_Printf( "Usage: %s (alias) (cmd)\n", argv[0]);
		CON_Printf( "Example: alias bigfont \"font size 3\".\n");
		CON_Printf( "Use %%1-%%9 to pass the alias arguments to the command.\n");
		return true;
	}
	CON_Alias(argv[1], argc==3? argv[2] : NULL);
	if(argc != 3)
		//CON_Printf( "%s == %s\n", argv[1], argv[2]);
	//else
		CON_Printf( "Alias '%s' deleted.\n", argv[1]);
	return true;
}

DEFCC(CCmdSetGamma)
{
	int	newlevel;

	if(argc != 2)
	{
		CON_Printf( "Usage: %s (0-4)\n", argv[0]);
		return true;
	}
	newlevel = strtol(argv[1], NULL, 0);
	// Clamp it to the min and max.
	if(newlevel < 0) newlevel = 0;
	if(newlevel > 4) newlevel = 4;
	// Only reload textures if it's necessary.
	if(newlevel != usegamma)
	{
		usegamma = newlevel;
		GL_UpdateGamma();
		CON_Printf( "Gamma correction set to level %d.\n", usegamma);
	}
	else
		CON_Printf( "Gamma correction already set to %d.\n", usegamma);
	return true;
}

DEFCC(CCmdParse)
{
	int		i;

	if(argc == 1)
	{
		CON_Printf( "Usage: %s (file) ...\n", argv[0]);
		return true;
	}
	for(i=1; i<argc; i++)
	{
		CON_Printf( "Parsing %s.\n", argv[i]);
		M_ParseCommands(argv[i], false);
	}
	return true;
}

DEFCC(CCmdDeleteBind)
{
	int		i;

	if(argc == 1)
	{
		CON_Printf( "Usage: %s (cmd) ...\n", argv[0]);
		return true;
	}
	for(i=1; i<argc; i++) B_ClearBinding(argv[i]);
	return true;
}	

DEFCC(CCmdLowRes)
{
	GL_LowRes();
	return true;
}

DEFCC(CCmdWait)
{
/*	int	 mode = !stricmp(argv[0], "wait")? 0 : 1;

	if(!stricmp(argv[0], "waitlist"))
	{
		int	i;
		if(!waitCount)
		{
			CON_Printf( "No suspended commands.\n");
		}
		else
		{
			CON_Printf( "%i tics remaining before:\n", waitCount);
			for(i=0; i<waitStackSize; i++)
				CON_Printf( "  %s\n", waitStack[i]);
		}
		return true;
	}

	if(!mode && argc != 2)
	{
		CON_Printf( "Usage: %s (tics)\n", argv[0]); // wait
		CON_Printf( "Sets the number of 35 Hz tics to wait.\n");
		CON_Printf( "'wait list' shows the list of suspended commands.\n");
		return true;
	}

	if(!mode) // wait
	{ 
		if(!stricmp(argv[1], "list"))
		{
		}
		// This allows waiting even in bindings.
		waitStackEnabled = true;
		waitCount += atoi(argv[1]);
		// No negative waiting.
		if(waitCount < 0) waitCount = 0;
	}
	else // stopwait
	{
		if(argc == 2)
			if(!stricmp(argv[1], "reset"))
			{
				// Should we also reset the wait stack?
				clearWaitStack();
			}
		waitCount = 0;
	}*/
	
/*	if(!stricmp(argv[0], "wait"))
	{
		int	i;
		for(i=0; i<exBuffSize; i++)
		{
			execbuff_t *ptr = exBuff + i;
			if(!ptr->used || ptr->marker != systics || ptr->id <= curExec->id)
				continue;
			ptr->marker++;
		}
	}
	else if(!stricmp(argv[0], "after"))
	{*/
		int off;
		if(argc != 3)
		{
			CON_Printf( "Usage: %s (tics) (cmd)\n", argv[0]);
			CON_Printf( "For example, '%s 35 \"echo End\"'.\n", argv[0]);
			return true;
		}
		off = atoi(argv[1]);
		if(off < 0) off = 0;
		SplitIntoSubCommands(argv[2], off);
	//}
	return true;
}

DEFCC(CCmdEcho)
{
	int		i;

	for(i=1; i<argc; i++) CON_Printf( "%s\n", argv[i]);
	return true;
}