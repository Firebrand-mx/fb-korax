
//**************************************************************************
//**
//** mn_menu.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <ctype.h>
#include <math.h>
#include "h2def.h"
#include "p_local.h"
#include "r_local.h"
#include "soundst.h"
#include "h2_actn.h"
#include "mn_def.h"
#include "Settings.h"
#include "journal.h"

// MACROS ------------------------------------------------------------------

// Control flags.
#define CLF_ACTION		0x1		// The control is an action (+/- in front).
#define CLF_REPEAT		0x2		// Bind down + repeat.

#define LEFT_DIR 0
#define RIGHT_DIR 1
#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SLOTTEXTLEN	16
#define ASCII_CURSOR '['

//#define USE_JOURNAL_MOUSE

// TYPES -------------------------------------------------------------------

typedef struct
{
	int	width, height;
} MenuRes_t;

typedef struct
{
	char		*command;		// The command to execute.
	int			flags;
	int			defKey;			// 
	int			defMouse;		// Zero means there is no default.
	int			defJoy;			//
} Control_t;

class KMenuWindow:public KWindow
{
	DECLARE_CLASS(KMenuWindow, KWindow, 0);

	KMenuWindow()
	{
		Width = 320;
		Height = 200;
	}
};

typedef enum
{
	ITT_EMPTY,
	ITT_EFUNC,
	ITT_LRFUNC,
	ITT_SETMENU,
	ITT_INERT
} ItemType_t;

typedef enum
{
	MENU_MAIN,
	MENU_CLASS,
	MENU_SKILL,
	MENU_CHAR,
	MENU_OPTIONS,
	MENU_OPTIONS2,
	MENU_GAMEPLAY,
	MENU_GRAPHICS,
	MENU_EFFECTS,
	MENU_RESOLUTION,
	MENU_CONTROLS,
	MENU_MOUSEOPTS,
	MENU_JOYCONFIG,
	MENU_LOAD,
	MENU_SAVE,
	MENU_NONE
} MenuType_t;

typedef struct
{
	ItemType_t type;
	char *text;
	void (*func)(int option);
	int option;
	MenuType_t menu;
} MenuItem_t;

class KMenuItem_t:public KMenuWindow
{
	DECLARE_CLASS(KMenuItem_t, KMenuWindow, 0);

	ItemType_t type;
	char *text;
	void (*func)(int option);
	int option;
	MenuType_t menu;

	KMenuItem_t()
	{
		Height = ITEM_HEIGHT;
	}

	virtual void DrawWindow(KGC *gc);
};

class Menu_t:public KMenuWindow
{
	DECLARE_CLASS(Menu_t, KMenuWindow, 0);
	NO_DEFAULT_CONSTRUCTOR(Menu_t);

	int x;
	int y;
	int itemCount;
	MenuItem_t *items;
	int oldItPos;
	MenuType_t prevMenu;
	// Enhancements. -jk
	int	itemHeight;
	KMenuItem_t *Items[128];

	//	Constructor
	Menu_t(int Ax, int Ay, int AitemCount, MenuItem_t *Aitems,
		MenuType_t AprevMenu, int AitemHeight = ITEM_HEIGHT) : x(Ax), y(Ay), 
		itemCount(AitemCount), items(Aitems),
		prevMenu(AprevMenu), itemHeight(AitemHeight)
	{
		X = 160;
		Y = 100;
		Font = KCanvas::BigFont;
		bIsVisible = false;
	}

	virtual void Init(KWindow *AParent);
	virtual void CreateChoices(void);
	virtual void PostDrawWindow(KGC *gc);
	virtual bool KeyPressed(int key);
};

extern boolean shiftdown;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

boolean G_CheckDemoStatus(void);
boolean F_Responder(event_t *ev);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SetMenu(MenuType_t menu);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void InitFonts(void);
static void SCQuitGame(int option);
static void SCClass(int option);
static void SCSkill(int option);
static void SCChar(int option);
static void SCMouseXSensi(int option);
static void SCMouseYSensi(int option);
static void SCMouseLook(int option);
static void SCMouseLookInverse(int option);
static void SCInverseY(int option);
static void SCJoyLook(int option);
static void SCPOVLook(int option);
static void SCInverseJoyLook(int option);
static void SCLookSpring(int option);
static void SCAutoAim(int option);
static void SCSkyDetail(int option);
static void SCMipmapping(int option);
static void SCLinearRaw(int option);
static void SCForceTexReload(int option);
static void SCResSelector(int option);
static void SCResMakeCurrent(int option);
static void SCResMakeDefault(int option);
static void SCSfxVolume(int option);
static void SCMusicVolume(int option);
static void SCCDVolume(int option);
static void SCScreenSize(int option);
static void SCMusicDevice(int option);
static boolean SCNetCheck(int option);
static void SCNetCheck2(int option);
static void SCLoadGame(int option);
static void SCSaveGame(int option);
static void SCMessages(int option);
static void SCAlwaysRun(int option);
static void SCControlConfig(int option);
static void SCJoySensi(int option);
static void SCEndGame(int option);
static void SCInfo(int option);
static void SCCrosshair(int option);
static void SCCrosshairSize(int option);

static void SCBorderUpdate(int option);
static void SCTexQuality(int option);
static void SCFPSCounter(int option);
static void SCIceCorpse(int option);
static void SCDynLights(int option);
static void SCDLBlend(int option);
static void SCDLIntensity(int option);
static void SCFlares(int option);
static void SCFlareIntensity(int option);
static void SCFlareSize(int option);
static void SCSpriteAlign(int option);
static void SCSpriteBlending(int option);
static void SCSpriteLight(int option);

static void SC3DSounds(int option);
static void SCReverbVolume(int option);
static void SCSfxFrequency(int option);
static void SCSfx16bit(int option);

static void DrawFileSlots(Menu_t *menu);
static void DrawFilesMenu(void);
static void MN_DrawInfo(void);
static void MN_DrawJournal(void);
static void MN_DrawSpells(void);
static void MN_DrawUpdating(void);
static void InitUpdating(void);
static void AcceptUpdating(void);
static void DrawSlider(Menu_t *menu, int item, int width, int slot);
void MN_LoadSlotText(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean gamekeydown[256]; // The NUMKEYS macro is local to g_game

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean	MenuActive;
int		InfoType;
boolean messageson = true;
boolean mn_SuicideConsole;
Menu_t	*CurrentMenu;
int		CurrentItPos;
int		MenuTime;

boolean	SpellsActive;
boolean	UpdatingActive;
unsigned int navpoints;
int currentUpdatingRow;

boolean	JournalActive;
int mousesx,mousesy;
int currentJournal;
int currentJournalPage;

IMPLEMENT_CLASS(KMenuWindow);
IMPLEMENT_CLASS(KMenuItem_t);
IMPLEMENT_CLASS(Menu_t);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int MauloBaseLump;
static int MenuPClass;
static int MenuPSkill;
typedef struct
{
	int val[5];
} chr_val;
chr_val MenuPValues = { 10, 80, 8, 2, 10 };	// -JL- Added default values
static boolean soundchanged;

static MenuRes_t resolutions[] =
{
	320, 240,
	640, 480,
	800, 600,
	1024, 768,
	1152, 864,
	1280, 1024,
	1600, 1200,
	0, 0	// The terminator.
};
static int selRes = 0;	// Will be determined when needed.

//
// !!! Add new controls to the end, the existing indices must remain unchanged !!!
//
static Control_t controls[] =
{
	// Actions (must be first so the H2A_* constants can be used).
	"left",			CLF_ACTION,		DDKEY_LEFTARROW, 0, 0,
	"right",		CLF_ACTION,		DDKEY_RIGHTARROW, 0, 0,
	"forward",		CLF_ACTION,		DDKEY_UPARROW, 0, 0,
	"backward",		CLF_ACTION,		DDKEY_DOWNARROW, 0, 0,
	"strafel",		CLF_ACTION,		',', 0, 0,
	"strafer",		CLF_ACTION,		'.', 0, 0,
	"jump",			CLF_ACTION,		'/', 2, 5,
	"fire",			CLF_ACTION,		DDKEY_RCTRL, 1, 1,
	"use",			CLF_ACTION,		' ', 0, 4,
	"strafe",		CLF_ACTION,		DDKEY_RALT, 3, 2,
	
	"speed",		CLF_ACTION,		DDKEY_RSHIFT, 0, 3,
	"flyup",		CLF_ACTION,		DDKEY_PGUP, 0, 8,
	"flydown",		CLF_ACTION,		DDKEY_INS, 0, 9,
	"falldown",		CLF_ACTION,		DDKEY_HOME, 0, 0,
	"lookup",		CLF_ACTION,		DDKEY_PGDN, 0, 6,
	"lookdown",		CLF_ACTION,		DDKEY_DEL, 0, 7,
	"lookcntr",		CLF_ACTION,		DDKEY_END, 0, 0,
	"usearti",		CLF_ACTION,		DDKEY_ENTER, 0, 0,
	"mlook",		CLF_ACTION,		'm', 0, 0,
	"jlook",		CLF_ACTION,		'j', 0, 0,
	
	"nextwpn",		CLF_ACTION,		0, 0, 0,
	"prevwpn",		CLF_ACTION,		0, 0, 0,
	"weapon1",		CLF_ACTION,		'1', 0, 0,
	"weapon2",		CLF_ACTION,		'2', 0, 0,
	"weapon3",		CLF_ACTION,		'3', 0, 0,
	"weapon4",		CLF_ACTION,		'4', 0, 0,
	"panic",		CLF_ACTION,		DDKEY_BACKSPACE, 0, 0,
	"torch",		CLF_ACTION,		0, 0, 0,
	"health",		CLF_ACTION,		'\\', 0, 0,
	"mystic",		CLF_ACTION,		0, 0, 0,
	
	"krater",		CLF_ACTION,		0, 0, 0,
	"spdboots",		CLF_ACTION,		0, 0, 0,
	"blast",		CLF_ACTION,		'9', 0, 0,
	"teleport",		CLF_ACTION,		'8', 0, 0,
	"teleothr",		CLF_ACTION,		'7', 0, 0,
	"poison",		CLF_ACTION,		'0', 0, 0,
	"cantdie",		CLF_ACTION,		'5', 0, 0,
	"servant",		CLF_ACTION,		0, 0, 0,
	"egg",			CLF_ACTION,		'6', 0, 0,
	"demostop",		CLF_ACTION,		'o', 0, 0,
	"duck",		    CLF_ACTION,		'c', 0, 0,
	"spell1",		CLF_ACTION,		'q', 0, 0,
	"spell2",		CLF_ACTION,		'w', 0, 0,
	"spell3",		CLF_ACTION,		'e', 0, 0,
	"spell4",		CLF_ACTION,		'r', 0, 0,
	"spell5",		CLF_ACTION,		't', 0, 0,
	"spell6",		CLF_ACTION,		'y', 0, 0,
	"spell7",		CLF_ACTION,		'u', 0, 0,
	"spell8",		CLF_ACTION,		'i', 0, 0,
/*t	"sell",		    CLF_ACTION,		's', 0, 0,
	"reply",	    CLF_ACTION,		'r', 0, 0,*/

	// Menu hotkeys (default: F1 - F12).
	"infoscreen",	0,				DDKEY_F1, 0, 0,
	"loadgame",		0,				DDKEY_F2, 0, 0,
	"savegame",		0,				DDKEY_F3, 0, 0,
	"soundmenu",	0,				DDKEY_F4, 0, 0,
	"suicide",		0,				DDKEY_F5, 0, 0,
	"quicksave",	0,				DDKEY_F6, 0, 0,
	"endgame",		0,				DDKEY_F7, 0, 0,
	"togglemsgs",	0,				DDKEY_F8, 0, 0,
	"quickload",	0,				DDKEY_F9, 0, 0,
	"quit",			0,				DDKEY_F10, 0, 0,
	"togglegamma",	0,				DDKEY_F11, 0, 0,
	"spy",			0,				DDKEY_F12, 0, 0,

	// Inventory.
	"invleft",		CLF_REPEAT,		'[', 0, 0,
	"invright",		CLF_REPEAT,		']', 0, 0,

	// Screen controls.
	"viewsize +",	CLF_REPEAT,		'=', 0, 0,
	"viewsize -",	CLF_REPEAT,		'-', 0, 0,
	"sbsize +",		CLF_REPEAT,		0, 0, 0,
	"sbsize -",		CLF_REPEAT,		0, 0, 0,

	// Misc.
	"pause",		0,				'p', 0, 0,
	"",				0,				0, 0, 0
};

static Control_t *grabbing = NULL;
static float bgAlpha=0, outFade=0;
static boolean fadingOut = false;
static int menuDarkTicks = 15;
static int slamInTicks = 9;

boolean askforquit;
boolean typeofask;
static boolean FileMenuKeySteal;
static boolean slottextloaded;
static char SlotText[6][SLOTTEXTLEN+2];
static char oldSlotText[SLOTTEXTLEN+2];
static int SlotStatus[6];
static int slotptr;
static int currentSlot;
static int quicksave;
static int quickload;

static MenuItem_t MainItems[] =
{
	{ ITT_SETMENU, "NEW GAME", SCNetCheck2, 1, MENU_CLASS },
	{ ITT_SETMENU, "LOAD GAME", SCNetCheck2, 2, MENU_LOAD },
	{ ITT_SETMENU, "SAVE GAME", NULL, 0, MENU_SAVE },
	{ ITT_SETMENU, "OPTIONS", NULL, 0, MENU_OPTIONS },
	{ ITT_EFUNC, "INFO", SCInfo, 0, MENU_NONE },
	{ ITT_EFUNC, "QUIT GAME", SCQuitGame, 0, MENU_NONE }
};

class KMenuScreenMain:public Menu_t
{
	DECLARE_CLASS(KMenuScreenMain, Menu_t, 0);

	KMenuScreenMain() : Menu_t(110, 56, 6, MainItems, MENU_NONE)
	{ }

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenMain);

static MenuItem_t ClassItems[] =
{
	{ ITT_EFUNC, "FIGHTER", SCClass, 0, MENU_NONE },
	{ ITT_EFUNC, "CLERIC", SCClass, 1, MENU_NONE },
	{ ITT_EFUNC, "MAGE", SCClass, 2, MENU_NONE },
	//{ ITT_EFUNC, "CORVUS", SCClass, 3, MENU_NONE } //Da Corvus man
};

class KMenuScreenClass:public Menu_t
{
	DECLARE_CLASS(KMenuScreenClass, Menu_t, 0);

	KMenuScreenClass() : Menu_t(66, 66, 3, ClassItems, MENU_MAIN)
	{ }

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenClass);

static MenuItem_t LoadItems[] =
{
	{ ITT_EFUNC, NULL, SCLoadGame, 0, MENU_NONE },
	{ ITT_EFUNC, NULL, SCLoadGame, 1, MENU_NONE },
	{ ITT_EFUNC, NULL, SCLoadGame, 2, MENU_NONE },
	{ ITT_EFUNC, NULL, SCLoadGame, 3, MENU_NONE },
	{ ITT_EFUNC, NULL, SCLoadGame, 4, MENU_NONE },
	{ ITT_EFUNC, NULL, SCLoadGame, 5, MENU_NONE }
};

class KMenuScreenLoadGame:public Menu_t
{
	DECLARE_CLASS(KMenuScreenLoadGame, Menu_t, 0);

	KMenuScreenLoadGame() : Menu_t(70, 30, 6, LoadItems, MENU_MAIN)
	{ }

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenLoadGame);

static MenuItem_t SaveItems[] =
{
	{ ITT_EFUNC, NULL, SCSaveGame, 0, MENU_NONE },
	{ ITT_EFUNC, NULL, SCSaveGame, 1, MENU_NONE },
	{ ITT_EFUNC, NULL, SCSaveGame, 2, MENU_NONE },
	{ ITT_EFUNC, NULL, SCSaveGame, 3, MENU_NONE },
	{ ITT_EFUNC, NULL, SCSaveGame, 4, MENU_NONE },
	{ ITT_EFUNC, NULL, SCSaveGame, 5, MENU_NONE }
};

class KMenuScreenSaveGame:public Menu_t
{
	DECLARE_CLASS(KMenuScreenSaveGame, Menu_t, 0);

	KMenuScreenSaveGame() : Menu_t(70, 30, 6, SaveItems, MENU_MAIN)
	{ }

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenSaveGame);

static MenuItem_t SkillItems[] =
{
	{ ITT_EFUNC, NULL, SCSkill, sk_baby, MENU_NONE },
	{ ITT_EFUNC, NULL, SCSkill, sk_easy, MENU_NONE },
	{ ITT_EFUNC, NULL, SCSkill, sk_medium, MENU_NONE },
	{ ITT_EFUNC, NULL, SCSkill, sk_hard, MENU_NONE },
	{ ITT_EFUNC, NULL, SCSkill, sk_nightmare, MENU_NONE }
};

class KMenuScreenSkill:public Menu_t
{
	DECLARE_CLASS(KMenuScreenSkill, Menu_t, 0);

	KMenuScreenSkill() : Menu_t(120, 44, 5, SkillItems, MENU_CLASS)
	{
		oldItPos = 2;
	}

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenSkill);

static MenuItem_t CharItems[] =
{
	{ ITT_EFUNC, "REROLL", SCChar, 0, MENU_NONE },
	{ ITT_EFUNC, "DEFAULT", SCChar, 1, MENU_NONE },
	{ ITT_EFUNC, "OK", SCChar, 2, MENU_NONE }/*,
	{ ITT_EFUNC, "EFFICIENCY: %d", SCChar, 2, MENU_NONE },
	{ ITT_EFUNC, "HEALTH: %d", SCChar, 3, MENU_NONE },
	{ ITT_EFUNC, "%s: %d", SCChar, 4, MENU_NONE }*/
};

class KMenuScreenChar:public Menu_t
{
	DECLARE_CLASS(KMenuScreenChar, Menu_t, 0);

	KMenuScreenChar() : Menu_t(120, 130, 3, CharItems, MENU_SKILL)
	{
		oldItPos = 2;
	}

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenChar);

static MenuItem_t OptionsItems[] =
{
	{ ITT_EFUNC, "END GAME", SCEndGame, 0, MENU_NONE },
	{ ITT_SETMENU, "GAMEPLAY...", NULL, 0, MENU_GAMEPLAY },
	{ ITT_SETMENU, "GRAPHICS...", NULL, 0, MENU_GRAPHICS },
	{ ITT_SETMENU, "SOUND...", NULL, 0, MENU_OPTIONS2 },
	{ ITT_SETMENU, "CONTROLS...", NULL, 0, MENU_CONTROLS },
	{ ITT_SETMENU, "MOUSE OPTIONS...", NULL, 0, MENU_MOUSEOPTS },
	{ ITT_SETMENU, "JOYSTICK OPTIONS...", NULL, 0, MENU_JOYCONFIG }
};

class KMenuScreenOptions:public Menu_t
{
	DECLARE_CLASS(KMenuScreenOptions, Menu_t, 0);

	KMenuScreenOptions() : Menu_t(110, 40, 7, OptionsItems, MENU_MAIN)
	{ }

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenOptions);

static MenuItem_t Options2Items[] =
{
	{ ITT_LRFUNC, "SFX VOLUME", SCSfxVolume, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_LRFUNC, "MIDI VOLUME", SCMusicVolume, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_LRFUNC, "CD VOLUME", SCCDVolume, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_LRFUNC, "MUSIC DEVICE :", SCMusicDevice, 0, MENU_NONE },
	{ ITT_EFUNC, "3D SOUNDS :", SC3DSounds, 0, MENU_NONE },
	{ ITT_LRFUNC, "REVERB VOLUME :", SCReverbVolume, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_LRFUNC, "SFX FREQUENCY :", SCSfxFrequency, 0, MENU_NONE },
	{ ITT_EFUNC, "16 BIT INTERPOLATION :", SCSfx16bit, 0, MENU_NONE }
};

class KMenuScreenOptions2:public Menu_t
{
	DECLARE_CLASS(KMenuScreenOptions2, Menu_t, 0);

	KMenuScreenOptions2() : Menu_t(70, 20, 16, Options2Items, MENU_OPTIONS, 10)
	{
		Font = KCanvas::SmallFont;
	}

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenOptions2);

static MenuItem_t GameplayItems[] =
{
	{ ITT_EFUNC, "MESSAGES :", SCMessages, 0, MENU_NONE },
	{ ITT_EFUNC, "ALWAYS RUN :", SCAlwaysRun, 0, MENU_NONE },
	{ ITT_EFUNC, "LOOKSPRING :", SCLookSpring, 0, MENU_NONE },
	{ ITT_EFUNC, "NO AUTOAIM :", SCAutoAim, 0, MENU_NONE },
	{ ITT_LRFUNC, "CROSSHAIR :", SCCrosshair, 0, MENU_NONE },
	{ ITT_LRFUNC, "CROSSHAIR SIZE :", SCCrosshairSize, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_LRFUNC, "SCREEN SIZE", SCScreenSize, 0, MENU_NONE },
};

class KMenuScreenGameplay:public Menu_t
{
	DECLARE_CLASS(KMenuScreenGameplay, Menu_t, 0);

	KMenuScreenGameplay() : Menu_t(64, 25, 9, GameplayItems, MENU_OPTIONS, 10)
	{
		Font = KCanvas::SmallFont;
	}

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenGameplay);

static MenuItem_t GraphicsItems[] = 
{
	{ ITT_LRFUNC, "SKY DETAIL", SCSkyDetail, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_LRFUNC, "MIPMAPPING :", SCMipmapping, 0, MENU_NONE },
	{ ITT_EFUNC, "SMOOTH GFX :", SCLinearRaw, 0, MENU_NONE },
	{ ITT_EFUNC, "UPDATE BORDERS :", SCBorderUpdate, 0, MENU_NONE },
	{ ITT_LRFUNC, "TEX QUALITY :", SCTexQuality, 0, MENU_NONE },
	{ ITT_EFUNC, "FORCE TEX RELOAD", SCForceTexReload, 0, MENU_NONE },
	{ ITT_SETMENU, "EFFECTS...", NULL, 0, MENU_EFFECTS },
	{ ITT_SETMENU, "RESOLUTION...", NULL, 0, MENU_RESOLUTION }
};

class KMenuScreenGraphics:public Menu_t
{
	DECLARE_CLASS(KMenuScreenGraphics, Menu_t, 0);

	KMenuScreenGraphics() : Menu_t(58, 10, 9, GraphicsItems, MENU_OPTIONS)
	{}

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenGraphics);

static MenuItem_t EffectsItems[] =
{
	{ ITT_EFUNC, "FPS COUNTER :", SCFPSCounter, 0, MENU_NONE },
	{ ITT_EFUNC, "FROZEN THINGS TRANSLUCENT :", SCIceCorpse, 0, MENU_NONE },
	{ ITT_EFUNC, "DYNAMIC LIGHTS :", SCDynLights, 0, MENU_NONE },
	{ ITT_LRFUNC, "DYNLIGHT BLENDING :", SCDLBlend, 0, MENU_NONE },
	{ ITT_EFUNC, "LIGHTS ON SPRITES :", SCSpriteLight, 0, MENU_NONE },
	{ ITT_LRFUNC, "DYNLIGHT INTENSITY :", SCDLIntensity, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_LRFUNC, "LENS FLARES :", SCFlares, 0, MENU_NONE },
	{ ITT_LRFUNC, "FLARE INTENSITY :", SCFlareIntensity, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_LRFUNC, "FLARE SIZE :", SCFlareSize, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_LRFUNC, "ALIGN SPRITES TO :", SCSpriteAlign, 0, MENU_NONE },
	{ ITT_EFUNC, "SPRITE BLENDING :", SCSpriteBlending, 0, MENU_NONE }
};

class KMenuScreenEffects:public Menu_t
{
	DECLARE_CLASS(KMenuScreenEffects, Menu_t, 0);

	KMenuScreenEffects() : Menu_t(60, 15, 17, EffectsItems, MENU_GRAPHICS, 10)
	{
		Font = KCanvas::SmallFont;
	}

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenEffects);

static MenuItem_t ResolutionItems[] =
{
	{ ITT_LRFUNC, "RESOLUTION :", SCResSelector, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EFUNC, "MAKE CURRENT", SCResMakeCurrent, 0, MENU_NONE },
	{ ITT_EFUNC, "MAKE DEFAULT", SCResMakeDefault, 0, MENU_NONE }
};

class KMenuScreenResolution:public Menu_t
{
	DECLARE_CLASS(KMenuScreenResolution, Menu_t, 0);

	KMenuScreenResolution() : Menu_t(88, 60, 4, ResolutionItems, MENU_GRAPHICS)
	{}

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenResolution);

static MenuItem_t ControlsItems[] =
{
	{ ITT_EMPTY, "PLAYER ACTIONS", NULL, 0, MENU_NONE },
	{ ITT_EFUNC, "LEFT :", SCControlConfig, H2A_TURNLEFT, MENU_NONE },
	{ ITT_EFUNC, "RIGHT :", SCControlConfig, H2A_TURNRIGHT, MENU_NONE },
	{ ITT_EFUNC, "FORWARD :", SCControlConfig, H2A_FORWARD, MENU_NONE },
	{ ITT_EFUNC, "BACKWARD :", SCControlConfig, H2A_BACKWARD, MENU_NONE },
	{ ITT_EFUNC, "STRAFE LEFT :", SCControlConfig, H2A_STRAFELEFT, MENU_NONE },
	{ ITT_EFUNC, "STRAFE RIGHT :", SCControlConfig, H2A_STRAFERIGHT, MENU_NONE },
	{ ITT_EFUNC, "JUMP :", SCControlConfig, H2A_JUMP, MENU_NONE },
	{ ITT_EFUNC, "DUCK :", SCControlConfig, H2A_DUCK, MENU_NONE },
	{ ITT_EFUNC, "FIRE :", SCControlConfig, H2A_FIRE, MENU_NONE },
	{ ITT_EFUNC, "USE :", SCControlConfig, H2A_USE, MENU_NONE },	
	{ ITT_EFUNC, "STRAFE :", SCControlConfig, H2A_STRAFE, MENU_NONE },
	{ ITT_EFUNC, "SPEED :", SCControlConfig, H2A_SPEED, MENU_NONE },
	{ ITT_EFUNC, "FLY UP :", SCControlConfig, H2A_FLYUP, MENU_NONE },
	{ ITT_EFUNC, "FLY DOWN :", SCControlConfig, H2A_FLYDOWN, MENU_NONE },
	{ ITT_EFUNC, "FALL DOWN :", SCControlConfig, H2A_FLYCENTER, MENU_NONE },
	{ ITT_EFUNC, "LOOK UP :", SCControlConfig, H2A_LOOKUP, MENU_NONE },
	{ ITT_EFUNC, "LOOK DOWN :", SCControlConfig, H2A_LOOKDOWN, MENU_NONE },
	{ ITT_EFUNC, "LOOK CENTER :", SCControlConfig, H2A_LOOKCENTER, MENU_NONE },
	{ ITT_EFUNC, "MOUSE LOOK :", SCControlConfig, H2A_MLOOK, MENU_NONE },
	{ ITT_EFUNC, "JOYSTICK LOOK :", SCControlConfig, H2A_JLOOK, MENU_NONE },
	{ ITT_EFUNC, "NEXT WEAPON :", SCControlConfig, H2A_NEXTWEAPON, MENU_NONE },
	{ ITT_EFUNC, "PREV WEAPON :", SCControlConfig, H2A_PREVIOUSWEAPON, MENU_NONE },
	{ ITT_EFUNC, "WEAPON 1 :", SCControlConfig, H2A_WEAPON1, MENU_NONE },
	{ ITT_EFUNC, "WEAPON 2 :", SCControlConfig, H2A_WEAPON2, MENU_NONE },
	{ ITT_EFUNC, "WEAPON 3 :", SCControlConfig, H2A_WEAPON3, MENU_NONE },
	{ ITT_EFUNC, "WEAPON 4 :", SCControlConfig, H2A_WEAPON4, MENU_NONE },
	{ ITT_EFUNC, "PANIC :", SCControlConfig, H2A_PANIC, MENU_NONE },
	{ ITT_EFUNC, "SPELL1 :", SCControlConfig, H2A_SPELL1, MENU_NONE },
	{ ITT_EFUNC, "SPELL2 :", SCControlConfig, H2A_SPELL2, MENU_NONE },
	{ ITT_EFUNC, "SPELL3 :", SCControlConfig, H2A_SPELL3, MENU_NONE },
	{ ITT_EFUNC, "SPELL4 :", SCControlConfig, H2A_SPELL4, MENU_NONE },
	{ ITT_EFUNC, "SPELL5 :", SCControlConfig, H2A_SPELL5, MENU_NONE },
	{ ITT_EFUNC, "SPELL6 :", SCControlConfig, H2A_SPELL6, MENU_NONE },
	{ ITT_EFUNC, "SPELL7 :", SCControlConfig, H2A_SPELL7, MENU_NONE },
/*	{ ITT_EFUNC, "SPELL8 :", SCControlConfig, H2A_SPELL8, MENU_NONE },*/
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, "ARTIFACTS", NULL, 0, MENU_NONE },
	{ ITT_EFUNC, "TORCH :", SCControlConfig, H2A_TORCH, MENU_NONE },
	{ ITT_EFUNC, "QUARTZ FLASK :", SCControlConfig, H2A_HEALTH, MENU_NONE },
	{ ITT_EFUNC, "MYSTIC URN :", SCControlConfig, H2A_MYSTICURN, MENU_NONE },
	{ ITT_EFUNC, "KRATER OF MIGHT :", SCControlConfig, H2A_KRATER, MENU_NONE },
	{ ITT_EFUNC, "BOOTS OF SPEED :", SCControlConfig, H2A_SPEEDBOOTS, MENU_NONE },
	{ ITT_EFUNC, "REPULSION :", SCControlConfig, H2A_BLASTRADIUS, MENU_NONE },
	{ ITT_EFUNC, "CHAOS DEVICE :", SCControlConfig, H2A_TELEPORT, MENU_NONE },
	{ ITT_EFUNC, "BANISHMENT :", SCControlConfig, H2A_TELEPORTOTHER, MENU_NONE },
	{ ITT_EFUNC, "BOOTS OF SPEED :", SCControlConfig, H2A_SPEEDBOOTS, MENU_NONE },
	{ ITT_EFUNC, "FLECHETTE :", SCControlConfig, H2A_POISONBAG, MENU_NONE },		
	{ ITT_EFUNC, "DEFENDER :", SCControlConfig, H2A_INVULNERABILITY, MENU_NONE },
	{ ITT_EFUNC, "DARK SERVANT :", SCControlConfig, H2A_DARKSERVANT, MENU_NONE },
	{ ITT_EFUNC, "PORKELATOR :", SCControlConfig, H2A_EGG, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, "INVENTORY", NULL, 0, MENU_NONE },
	{ ITT_EFUNC, "INVENTORY LEFT :", SCControlConfig, NUM_ACTIONS+12, MENU_NONE },
	{ ITT_EFUNC, "INVENTORY RIGHT :", SCControlConfig, NUM_ACTIONS+13, MENU_NONE },
	{ ITT_EFUNC, "USE ARTIFACT :", SCControlConfig, H2A_USEARTIFACT, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, "MENU HOTKEYS", NULL, 0, MENU_NONE },
	{ ITT_EFUNC, "INFO :", SCControlConfig, NUM_ACTIONS, MENU_NONE },
	{ ITT_EFUNC, "SOUND MENU :", SCControlConfig, NUM_ACTIONS+3, MENU_NONE },
	{ ITT_EFUNC, "LOAD GAME :", SCControlConfig, NUM_ACTIONS+1, MENU_NONE },
	{ ITT_EFUNC, "SAVE GAME :", SCControlConfig, NUM_ACTIONS+2, MENU_NONE },
	{ ITT_EFUNC, "QUICK LOAD :", SCControlConfig, NUM_ACTIONS+8, MENU_NONE },
	{ ITT_EFUNC, "QUICK SAVE :", SCControlConfig, NUM_ACTIONS+5, MENU_NONE },
	{ ITT_EFUNC, "SUICIDE :", SCControlConfig, NUM_ACTIONS+4, MENU_NONE },
	{ ITT_EFUNC, "END GAME :", SCControlConfig, NUM_ACTIONS+6, MENU_NONE },
	{ ITT_EFUNC, "QUIT :", SCControlConfig, NUM_ACTIONS+9, MENU_NONE },
	{ ITT_EFUNC, "MESSAGES ON/OFF:", SCControlConfig, NUM_ACTIONS+7, MENU_NONE },
	{ ITT_EFUNC, "GAMMA CORRECTION :", SCControlConfig, NUM_ACTIONS+10, MENU_NONE },
	{ ITT_EFUNC, "SPY MODE :", SCControlConfig, NUM_ACTIONS+11, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, "SCREEN", NULL, 0, MENU_NONE },
	{ ITT_EFUNC, "SMALLER VIEW :", SCControlConfig, NUM_ACTIONS+15, MENU_NONE },
	{ ITT_EFUNC, "LARGER VIEW :", SCControlConfig, NUM_ACTIONS+14, MENU_NONE },
	{ ITT_EFUNC, "SMALLER ST. BAR :", SCControlConfig, NUM_ACTIONS+17, MENU_NONE },
	{ ITT_EFUNC, "LARGER ST. BAR :", SCControlConfig, NUM_ACTIONS+16, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EMPTY, "MISCELLANEOUS", NULL, 0, MENU_NONE },
	{ ITT_EFUNC, "STOP DEMO :", SCControlConfig, H2A_STOPDEMO, MENU_NONE },
	{ ITT_EFUNC, "PAUSE :", SCControlConfig, NUM_ACTIONS+18, MENU_NONE }/*,
	{ ITT_EFUNC, "DUCK :", SCControlConfig, H2A_DUCK, MENU_NONE },
	{ ITT_EFUNC, "SELL :", SCControlConfig, H2A_SELL, MENU_NONE },
	{ ITT_EFUNC, "REPLY :", SCControlConfig, H2A_REPLY, MENU_NONE }*/
};

class KMenuScreenControls:public Menu_t
{
	DECLARE_CLASS(KMenuScreenControls, Menu_t, 0);

	// For scrolling menus.
	int numVisItems;

	KMenuScreenControls() : Menu_t(32, 26, 79, ControlsItems, MENU_OPTIONS, 9)
	{
		X = 0;
		Y = 0;
		Width = 640;
		Height = 480;
		Font = KCanvas::SmallFont;
		numVisItems = 40;
		oldItPos = 1;
	}

	void CreateChoices();
	void DrawWindow(KGC *gc);
	void PostDrawWindow(KGC *gc);
	bool KeyPressed(int key);
};
IMPLEMENT_CLASS(KMenuScreenControls);

static MenuItem_t MouseOptsItems[] =
{
	{ ITT_EFUNC, "INVERSE Y :", SCInverseY, 0, MENU_NONE },
	{ ITT_EFUNC, "MOUSE LOOK :", SCMouseLook, 0, MENU_NONE },
	{ ITT_EFUNC, "INVERSE MLOOK :", SCMouseLookInverse, 0, MENU_NONE },
	{ ITT_LRFUNC, "X SENSITIVITY", SCMouseXSensi, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_LRFUNC, "Y SENSITIVITY", SCMouseYSensi, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
};

class KMenuScreenMouseOptions:public Menu_t
{
	DECLARE_CLASS(KMenuScreenMouseOptions, Menu_t, 0);

	KMenuScreenMouseOptions() : Menu_t(72, 30, 7, MouseOptsItems, MENU_OPTIONS)
	{}

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenMouseOptions);

static MenuItem_t JoyConfigItems[] =
{
	{ ITT_LRFUNC, "SENSITIVITY", SCJoySensi, 0, MENU_NONE },
	{ ITT_EMPTY, NULL, NULL, 0, MENU_NONE },
	{ ITT_EFUNC, "JOY LOOK :", SCJoyLook, 0, MENU_NONE },
	{ ITT_EFUNC, "INVERSE LOOK :", SCInverseJoyLook, 0, MENU_NONE },
	{ ITT_EFUNC, "POV LOOK :", SCPOVLook, 0, MENU_NONE }
};

class KMenuScreenJoyConfig:public Menu_t
{
	DECLARE_CLASS(KMenuScreenJoyConfig, Menu_t, 0);

	KMenuScreenJoyConfig() : Menu_t(80, 50, 5, JoyConfigItems, MENU_OPTIONS)
	{}

	void DrawWindow(KGC *);
};
IMPLEMENT_CLASS(KMenuScreenJoyConfig);

static Menu_t *Menus[MENU_NONE];

static char *GammaText[] = 
{
	TXT_GAMMA_LEVEL_OFF,
	TXT_GAMMA_LEVEL_1,
	TXT_GAMMA_LEVEL_2,
	TXT_GAMMA_LEVEL_3,
	TXT_GAMMA_LEVEL_4
};
	
static int upd_values[4][2];
static int maxupd_values[4][3]=
{
	{0,0,0},
	{-1,60,5},
	{5,40,-1},
	{30,20,10}
};

static int av_pos[5]=
{
	{2},
	{4},
	{5},
	{6},
	{10}
};

static char* sp_wording[NUMCLASSES]=
{
	{"BATTLE RAGE:"},
	{"FAITH:"},
	{"MAGIC:"},
	{""},
	{""}
};

// CODE --------------------------------------------------------------------

//---------------------------------------------------------------------------
//
// PROC MN_Init
//
//---------------------------------------------------------------------------

static int findRes(int w, int h)
{
	int i;

	for(i=0; resolutions[i].width; i++)
		if(resolutions[i].width == w && resolutions[i].height == h)
			return i;
	return -1;
}

void MN_Init(void)
{
	KCanvas::StaticInit();
	MenuActive = false;
	MauloBaseLump = gi.W_GetNumForName("FBULA0"); // ("M_SKL00");

	GRootWindow = Spawn<KRootWindow>();
	GRootWindow->Init();
	Menus[MENU_MAIN] = NewWindow(KMenuScreenMain, GRootWindow);
	Menus[MENU_CLASS] = NewWindow(KMenuScreenClass, GRootWindow);
	Menus[MENU_SKILL] = NewWindow(KMenuScreenSkill, GRootWindow);
	Menus[MENU_CHAR] = NewWindow(KMenuScreenChar, GRootWindow);
	Menus[MENU_OPTIONS] = NewWindow(KMenuScreenOptions, GRootWindow);
	Menus[MENU_OPTIONS2] = NewWindow(KMenuScreenOptions2, GRootWindow);
	Menus[MENU_GAMEPLAY] = NewWindow(KMenuScreenGameplay, GRootWindow);
	Menus[MENU_GRAPHICS] = NewWindow(KMenuScreenGraphics, GRootWindow);
	Menus[MENU_EFFECTS] = NewWindow(KMenuScreenEffects, GRootWindow);
	Menus[MENU_RESOLUTION] = NewWindow(KMenuScreenResolution, GRootWindow);
	Menus[MENU_CONTROLS] = NewWindow(KMenuScreenControls, GRootWindow);
	Menus[MENU_MOUSEOPTS] = NewWindow(KMenuScreenMouseOptions, GRootWindow);
	Menus[MENU_JOYCONFIG] = NewWindow(KMenuScreenJoyConfig, GRootWindow);
	Menus[MENU_LOAD] = NewWindow(KMenuScreenLoadGame, GRootWindow);
	Menus[MENU_SAVE] = NewWindow(KMenuScreenSaveGame, GRootWindow);

	CurrentMenu = Menus[MENU_MAIN];

	// Find the correct resolution.
	selRes = findRes(gi.Get(DD_SCREEN_WIDTH), gi.Get(DD_SCREEN_HEIGHT));

	// If the game is running in a window you can only change the default
	// resolution.
	selRes = findRes(gi.Get(DD_DEFAULT_RES_X), gi.Get(DD_DEFAULT_RES_Y));
}

//---------------------------------------------------------------------------
//
//	Menu_t::Init
//
//---------------------------------------------------------------------------

void Menu_t::Init(KWindow *AParent)
{
	Super::Init(AParent);
	CreateChoices();
}

//---------------------------------------------------------------------------
//
//	Menu_t::CreateChoices
//
//---------------------------------------------------------------------------

void Menu_t::CreateChoices(void)
{
	for (int i = 0; i < itemCount; i++)
	{
		KMenuItem_t *It = NewWindow(KMenuItem_t, this);
		It->SetPos(x, y + i * itemHeight);
		It->type = items[i].type;
		It->text = items[i].text;
		It->func = items[i].func;
		It->option = items[i].option;
		It->menu = items[i].menu;
		Items[i] = It;
	}
}

//---------------------------------------------------------------------------
//
// PROC MN_DrTextA
//
// Draw text using font A.
//
//---------------------------------------------------------------------------

void MN_DrTextA_CS(char *text, int x, int y)
{
	GCanvas->SetFont(GCanvas->SmallFont);
	GCanvas->DrawText(x, y, text);
}

void MN_DrTextA(char *text, int x, int y)
{
	gi.GL_SetColorAndAlpha(1, 1, 1, 1);
	MN_DrTextA_CS(text, x, y);
}

//==========================================================================
//
// MN_DrTextAYellow
//
//==========================================================================

void MN_DrTextAYellow_CS(char *text, int x, int y)
{
	GCanvas->SetFont(GCanvas->YellowFont);
	GCanvas->DrawText(x, y, text);
}

void MN_DrTextAYellow(char *text, int x, int y)
{
	gi.GL_SetColorAndAlpha(1, 1, 1, 1);
	MN_DrTextAYellow_CS(text, x, y);
}

//---------------------------------------------------------------------------
//
// FUNC MN_TextAWidth
//
// Returns the pixel width of a string using font A.
//
//---------------------------------------------------------------------------

int MN_TextAWidth(char *text)
{
	return KCanvas::SmallFont->TextWidth(text);
}

//---------------------------------------------------------------------------
//
// PROC MN_DrTextB
//
// Draw text using font B.
//
//---------------------------------------------------------------------------

void MN_DrTextB_CS(char *text, int x, int y)
{
	GCanvas->SetFont(GCanvas->BigFont);
	GCanvas->DrawText(x, y, text);
}

void MN_DrTextB(char *text, int x, int y)
{
	gi.GL_SetColorAndAlpha(1, 1, 1, 1);
	MN_DrTextB_CS(text, x, y);
}

//---------------------------------------------------------------------------
//
// FUNC MN_TextBWidth
//
// Returns the pixel width of a string using font B.
//
//---------------------------------------------------------------------------

int MN_TextBWidth(char *text)
{
	return KCanvas::BigFont->TextWidth(text);
}

//---------------------------------------------------------------------------
//
// PROC MN_Ticker
//
//---------------------------------------------------------------------------

void MN_Ticker(void)
{
	if(demoplayback && actions[H2A_STOPDEMO].on)
	{
		G_CheckDemoStatus();
	}
	if(MenuActive == false)
	{
		if(bgAlpha > 0) 
		{
			bgAlpha -= .5/(float)menuDarkTicks;
			if(bgAlpha < 0) bgAlpha = 0;
		}
		if(fadingOut)
		{
			outFade += 1/(float)slamInTicks;
			if(outFade > 1) fadingOut = false;
		}
		return;
	}
	MenuTime++;
}

//==========================================================================
//
// DrawMessage
//
//==========================================================================

static void DrawMessage(void)
{
	player_t *player;
	int i;

	i = 1;
	player = &players[consoleplayer];
	/*if(player->messageTics <= 0 || !player->message)
	{ // No message
		return;
	}*/
	if(player->messageTics2 >= 1 && player->message2)
	{
		if(player->yellowMessage2)
		{
			MN_DrTextAYellow(player->message2, 
				160-MN_TextAWidth(player->message2)/2, i);
		}	
		else
		{
			MN_DrTextA(player->message2, 160-MN_TextAWidth(player->message2)/2, i);
		}
		i += 10;
	}
	if(player->messageTics >= 1 && player->message)
	{
		if(player->yellowMessage)
		{
			MN_DrTextAYellow(player->message, 
				160-MN_TextAWidth(player->message)/2, i);
		}	
		else
		{
			MN_DrTextA(player->message, 160-MN_TextAWidth(player->message)/2, i);
		}
	}
}

char *QuitEndMsg[] =
{
	"ARE YOU SURE YOU WANT TO QUIT?",
	"ARE YOU SURE YOU WANT TO END THE GAME?",
	"DO YOU WANT TO QUICKSAVE THE GAME NAMED",
	"DO YOU WANT TO QUICKLOAD THE GAME NAMED",
	"ARE YOU SURE YOU WANT TO SUICIDE?"
};


float MN_GL_SetupState(float time)
{
	float alpha;

	GCanvas->SetOrigin(160, 120);
	gl.MatrixMode(DGL_MODELVIEW);
	gl.PushMatrix();
	if(time > 1 && time <= 2)
	{
		time = 2-time;
		gl.Translatef(320, 240, 0);
		gl.Scalef(.9f+time*.1f, .9f+time*.1f, 1);
		gl.Translatef(-320, -240, 0);
		gl.Color4f(1, 1, 1, alpha = time);
	}
	else
	{
		gl.Translatef(320, 240, 0);
		gl.Scalef(2-time, 2-time, 1);
		gl.Translatef(-320, -240, 0);
		gl.Color4f(1, 1, 1, alpha = time*time);
	}
	return alpha;
}

void MN_GL_RestoreState()
{
	gl.MatrixMode(DGL_MODELVIEW);
	gl.PopMatrix();
	GCanvas->SetOrigin(0, 0);
}

//---------------------------------------------------------------------------
//
// PROC MN_Drawer
//
//---------------------------------------------------------------------------

void MN_Drawer(void)
{
	GCanvas->SetOrigin(160, 0);
	DrawMessage();
	GCanvas->SetOrigin(0, 0);

	// FPS.
	if (showFPS)
	{
		char fpsbuff[80];
		sprintf(fpsbuff, "%d FPS", gi.FrameRate());
		MN_DrTextA(fpsbuff, 640-MN_TextAWidth(fpsbuff), 0);
		gi.Update(DDUF_TOP);
	}
	
	if (MenuActive == false)
	{
		if (bgAlpha > 0)
		{
			gi.Update(DDUF_FULLSCREEN | DDUF_BORDER);
			gi.GL_SetNoTexture();
			GCanvas->DrawRect(0, 0, 640, 480, 0, 0, 0, bgAlpha);
		}
		if (askforquit)  //Draw questioning
		{
			GCanvas->SetOrigin(160, 120);
			MN_DrTextA(QuitEndMsg[typeofask-1], 160-
				MN_TextAWidth(QuitEndMsg[typeofask-1])/2, 80);
			if (typeofask == 3)
			{
				MN_DrTextA(SlotText[quicksave-1], 160-
					MN_TextAWidth(SlotText[quicksave-1])/2, 90);
				MN_DrTextA("?", 160+
					MN_TextAWidth(SlotText[quicksave-1])/2, 90);
			}
			if (typeofask == 4)
			{
				MN_DrTextA(SlotText[quickload-1], 160-
					MN_TextAWidth(SlotText[quickload-1])/2, 90);
				MN_DrTextA("?", 160+
					MN_TextAWidth(SlotText[quicksave-1])/2, 90);
			}
			gi.Update(DDUF_FULLSCREEN);
			GCanvas->SetOrigin(0, 0);
		}
	}
	if (MenuActive || fadingOut || JournalActive || SpellsActive || UpdatingActive)
	{
		int effTime = (MenuTime>menuDarkTicks)? menuDarkTicks : MenuTime;
		float temp = .5 * effTime/(float)menuDarkTicks;

		gi.Update(DDUF_FULLSCREEN | DDUF_BORDER);

		if (!fadingOut)
		{
			if(temp > bgAlpha) bgAlpha = temp;
			effTime = (MenuTime>slamInTicks)? slamInTicks : MenuTime;
			temp = effTime / (float)slamInTicks;
	
			// Draw a dark background. It makes it easier to read the menus.
			gi.GL_SetNoTexture();
			GCanvas->DrawRect(0, 0, 640, 480, 0, 0, 0, bgAlpha);
		}

		MN_GL_SetupState(temp);
		if (JournalActive) //Drawing for Journal
		{
			MN_DrawJournal();
		}
		else if (SpellsActive) //Drawing for Spells
		{
			MN_DrawSpells();
		}
		else if (UpdatingActive) //Drawing for Updating
		{
			MN_DrawUpdating();
		}
		else if (InfoType)
		{
			MN_DrawInfo();
		}
		else
		{
			CurrentMenu->Show();
			GRootWindow->PaintWindows(GCanvas);
			CurrentMenu->Hide();
		}
		MN_GL_RestoreState();
	}
}

//---------------------------------------------------------------------------
//
//	KMenuItem_t::DrawWindow
//
//---------------------------------------------------------------------------

void KMenuItem_t::DrawWindow(KGC *gc)
{
	Super::DrawWindow(gc);

	if (text)
	{
		// Decide which color to use.
		if (type == ITT_EMPTY)
			gc->SetFont(KCanvas::YellowFont);
		else
			gc->SetFont(Parent->Font);

		gc->DrawText(0, 0, text);
	}
}

//---------------------------------------------------------------------------
//
//	Menu_t::PostDrawWindow
//
//---------------------------------------------------------------------------

void Menu_t::PostDrawWindow(KGC *gc)
{
	gc->DrawIcon(x + SELECTOR_XOFFSET, y + SELECTOR_YOFFSET + 
		(CurrentItPos * itemHeight) - (10 - itemHeight / 2), 
		FindTexture(MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2"));
}

//---------------------------------------------------------------------------
//
//	KMenuScreenMain::DrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreenMain::DrawWindow(KGC *gc)
{
	int frame;

	frame = (MenuTime/5)%7;
	
	gc->DrawIcon(88, 0, FindTexture("M_HTIC"));

	gc->GetCanvas()->DrawPatch(37, 80, MauloBaseLump+(frame+2)%7);
	gc->GetCanvas()->DrawPatch(278, 80, MauloBaseLump+frame);
}

//==========================================================================
//
//	KMenuScreenClass::DrawWindow
//
//==========================================================================

void KMenuScreenClass::DrawWindow(KGC *gc)
{
//	spriteinfo_t	sprInfo;
//	int				w, h, alpha;

	static char *boxLumpName[4] =
	{
		"m_fbox",
		"m_cbox",
		"m_mbox",
		"m_cbox"
	};
	static char *walkLumpName[3] =
	{
		"m_fwalk1",
		"m_cwalk1",
		"m_mwalk1"
	};

	gc->DrawText(34, 24, "CHOOSE CLASS:");
	int pclass = Items[CurrentItPos]->option;
	gc->DrawIcon(174, 8, FindTexture(boxLumpName[pclass]));
	/*if (pclass == 3) //Remi: Corvus, never happens in this source state
	{
		gl.GetIntegerv(DGL_A, &alpha);
		gi.GetSpriteInfo(SPR_PLAY, 0, &sprInfo);	
		gi.Set(DD_TRANSLATED_SPRITE_TEXTURE, DD_TSPR_PARM(sprInfo.lump, pclass, 0));
		gi.Set(DD_SPRITE_SIZE_QUERY, sprInfo.lump);
		w = gi.Get(DD_QUERY_RESULT) >> 16;		// High word.
		h = gi.Get(DD_QUERY_RESULT) & 0xffff;	// Low word.
		gi.GL_DrawRect(162-(sprInfo.offset>>16), 90 - (sprInfo.topOffset>>16), w, h, 1, 1, 1, alpha/255.0f);
		gl.Color4ub(255, 255, 255, alpha);
	}
	else*/
	gc->GetCanvas()->DrawPatch(174+24, 8+12, 
		gi.W_GetNumForName(walkLumpName[pclass])+((MenuTime>>3)&3));
}

//---------------------------------------------------------------------------
//
//	KMenuScreenSkill::DrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreenSkill::DrawWindow(KGC *gc)
{
	gc->DrawText(74, 16, "CHOOSE SKILL LEVEL:");
}

//---------------------------------------------------------------------------
//
//	KMenuScreenChar::DrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreenChar::DrawWindow(KGC *gc)
{
	char tmp[2][5][13];
	int i;
	gc->DrawText(50, 16, "GENERATE YOUR CHARACTER:");
	sprintf(tmp[0][0],"HEALTH:");
	sprintf(tmp[0][1],"%s",sp_wording[MenuPClass]);
	sprintf(tmp[0][2],"STRENGTH:");
	sprintf(tmp[0][3],"EFFICIENCY:");
	sprintf(tmp[0][4],"SPEED:");
	sprintf(tmp[1][0],"%3d",MenuPValues.val[0]);
	sprintf(tmp[1][1],((MenuPClass!=3)?"%3d":""),MenuPValues.val[1]);
	sprintf(tmp[1][2],"%02d",MenuPValues.val[2]);
	sprintf(tmp[1][3],"%02d",MenuPValues.val[3]);
	sprintf(tmp[1][4],"%02d",MenuPValues.val[4]);
	for (i=0;i<5;i++)
	{
		gc->DrawText(74, 44+i*16, tmp[0][i]);
		gc->DrawText(194, 44+i*16, tmp[1][i]);
	}
}


//---------------------------------------------------------------------------
//
// PROC DrawFilesMenu
//
//---------------------------------------------------------------------------

static void DrawFilesMenu(void)
{
// clear out the quicksave/quickload stuff
	quicksave = 0;
	quickload = 0;
	P_ClearMessage(&players[consoleplayer]);
}

//---------------------------------------------------------------------------
//
//	KMenuScreenLoadGame::DrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreenLoadGame::DrawWindow(KGC *gc)
{
	gc->DrawText(160 - Font->TextWidth("LOAD GAME")/2, 10, "LOAD GAME");
	if (!slottextloaded)
	{
		MN_LoadSlotText();
	}
	DrawFileSlots(this);
}

//---------------------------------------------------------------------------
//
//	KMenuScreenSaveGame::DrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreenSaveGame::DrawWindow(KGC *gc)
{
	gc->DrawText(160 - Font->TextWidth("SAVE GAME")/2, 10, "SAVE GAME");
	if(!slottextloaded)
	{
		MN_LoadSlotText();
	}
	DrawFileSlots(this);
}

//===========================================================================
//
// MN_LoadSlotText
//
// For each slot, looks for save games and reads the description field.
//
//===========================================================================

void MN_LoadSlotText(void)
{
	int slot;
	FILE *fp;
	char name[100];
	char versionText[HXS_VERSION_TEXT_LENGTH];
	char description[HXS_DESCRIPTION_LENGTH];
	boolean found;

	for(slot = 0; slot < 6; slot++)
	{
		found = false;
		sprintf(name, "%shex%d.hxs", SavePath, slot);
		fp = fopen(name, "rb");
		if(fp)
		{
			fread(description, HXS_DESCRIPTION_LENGTH, 1, fp);
			fread(versionText, HXS_VERSION_TEXT_LENGTH, 1, fp);
			fclose(fp);
			if(!strcmp(versionText, HXS_VERSION_TEXT))
			{
				found = true;
			}
		}
		if(found)
		{
			memcpy(SlotText[slot], description, SLOTTEXTLEN);
			SlotStatus[slot] = 1;
		}
		else
		{
			memset(SlotText[slot], 0, SLOTTEXTLEN);
			SlotStatus[slot] = 0;
		}
	}
	slottextloaded = true;
}

//---------------------------------------------------------------------------
//
// PROC DrawFileSlots
//
//---------------------------------------------------------------------------

static void DrawFileSlots(Menu_t *menu)
{
	int i;
	int x;
	int y;

	x = menu->x;
	y = menu->y;
	for(i = 0; i < 6; i++)
	{
		GCanvas->DrawPatch(x, y, gi.W_GetNumForName("M_FSLOT"));
		if(SlotStatus[i])
		{
			MN_DrTextA_CS(SlotText[i], x+5, y+5);
		}
		y += menu->itemHeight;
	}
}

//---------------------------------------------------------------------------
//
//	KMenuScreenOptions::DrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreenOptions::DrawWindow(KGC *gc)
{
	gc->DrawText(154-Font->TextWidth("OPTIONS")/2, 10, "OPTIONS");
}

//---------------------------------------------------------------------------
//
//	KMenuScreenOptions2::DrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreenOptions2::DrawWindow(KGC *gc)
{
	static char *musDevStr[3] = { "NONE", "MIDI", "CD" };
	static char *freqStr[4] = { "11 KHZ", "22 KHZ", "INVALID!", "44 KHZ" };
	int temp = (int) (*(float*) gi.GetCVar("s_reverbVol")->ptr * 10 + .5f);

	DrawSlider(this, 1, 18, gi.Get(DD_SFX_VOLUME)/15);
	DrawSlider(this, 4, 18, gi.Get(DD_MIDI_VOLUME)/15);
	DrawSlider(this, 7, 18, gi.CD(DD_GET_VOLUME,0)/15);
	gc->DrawText(x + Font->TextWidth("MUSIC DEVICE : "), y + itemHeight*9, 
		musDevStr[gi.Get(DD_MUSIC_DEVICE)]);
	gc->DrawText(x + Font->TextWidth("3D SOUNDS : "), 
		y + itemHeight*10, *(int*) gi.GetCVar("s_3d")->ptr? "ON" : "OFF");
	DrawSlider(this, 12, 11, temp);
	gc->DrawText(x + Font->TextWidth("SFX FREQUENCY : "), 
		y + itemHeight*14, freqStr[*(int*) gi.GetCVar("s_resample")->ptr - 1]);
	gc->DrawText(x + Font->TextWidth("16 BIT INTERPOLATION : "), y + itemHeight*15,
		*(int*) gi.GetCVar("s_16bit")->ptr? "ON" : "OFF");
}

static void SCMusicDevice(int option)
{
	int snd_MusicDevice = gi.Get(DD_MUSIC_DEVICE);
	
	if(option == RIGHT_DIR)
	{
		if(snd_MusicDevice < 2) snd_MusicDevice++;
	}
	else if(snd_MusicDevice > 0) snd_MusicDevice--;

	// Setup the music.
	gi.SetMusicDevice(snd_MusicDevice);
	
	// Restart the song of the current map.
	S_StartSong(gamemap, true);
}

void KMenuScreenGameplay::DrawWindow(KGC *gc)
{
	static char *xhairnames[7] = { "NONE", "CROSS", "ANGLES", "SQUARE",
		"OPEN SQUARE", "DIAMOND", "V" };
	
	gc->DrawText(x+Font->TextWidth("MESSAGES : "), y, 
		messageson ? "YES" : "NO");
	gc->DrawText(x+Font->TextWidth("ALWAYS RUN : "), y + itemHeight, 
		alwaysRun? "YES" : "NO");
	gc->DrawText(x+Font->TextWidth("LOOKSPRING : "), y + itemHeight * 2,
		lookSpring ? "YES" : "NO");
	gc->DrawText(x + Font->TextWidth("NO AUTOAIM : "), y + itemHeight * 3,
		noAutoAim ? "YES" : "NO");
	gc->DrawText(x + Font->TextWidth("CROSSHAIR : "), y + itemHeight * 4,
		xhairnames[xhair]);
	DrawSlider(this, 6, 9, xhairSize);
	DrawSlider(this, 9, 9, screenblocks-3);
}

void KMenuScreenGraphics::DrawWindow(KGC *gc)
{
	static const char *mipStr[6] = 
	{
		"N", "L", "N, MIP N", "L, MIP N", "N, MIP L", "L, MIP L"
	};
	static const char *texQStr[9] =
	{
		"0 - MINIMUM",
		"1 - VERY LOW",
		"2 - LOW",
		"3 - POOR",
		"4 - AVERAGE",
		"5 - GOOD",
		"6 - HIGH",
		"7 - VERY HIGH",
		"8 - MAXIMUM"
	};
	cvar_t *cv = gi.GetCVar("r_texquality");

	DrawSlider(this, 1, 5, gi.Get(DD_SKY_DETAIL)-3);
	gc->DrawText(x + Font->TextWidth("MIPMAPPING : ") + 8, y + itemHeight * 2,
		mipStr[gi.Get(DD_MIPMAPPING)]);
	gc->DrawText(x + Font->TextWidth("SMOOTH GFX : ") + 8, y + itemHeight * 3,
		gi.Get(DD_SMOOTH_IMAGES)? "YES" : "NO");
	
	gc->DrawText(x + Font->TextWidth("UPDATE BORDERS : ") + 8, y + itemHeight * 4,
		*(int*) gi.GetCVar("borderupd")->ptr? "YES" : "NO");
	
	gc->DrawText(x + Font->TextWidth("TEX QUALITY : ") + 8, y + itemHeight * 5,
		texQStr[*(int*) cv->ptr]);

	// This isn't very good programming, but here we can reset the 
	// current resolution selection.
	selRes = findRes(gi.Get(DD_SCREEN_WIDTH), gi.Get(DD_SCREEN_HEIGHT));
}

void KMenuScreenEffects::DrawWindow(KGC *gc)
{
	static const char	*dlblendStr[4] = { "MULTIPLY", "ADD", "NONE", "DON'T RENDER" };
	static const char	*flareStr[6] = { "OFF", "1", "2", "3", "4", "5" };
	static const char	*alignStr[4] = { "CAMERA", "VIEW PLANE", "CAMERA (R)", "VIEW PLANE (R)" };
	int		x = this->x + 140, y = this->y, h = this->itemHeight;
	int		temp;

	gc->DrawText(x, y, showFPS? "YES" : "NO");
	gc->DrawText(this->x + Font->TextWidth("FROZEN THINGS TRANSLUCENT : "), y+h,
		translucentIceCorpse ? "YES" : "NO");
	gc->DrawText(x, y+h*2, *(int*) gi.GetCVar("dynlights")->ptr? "ON" : "OFF");
	gc->DrawText(x, y+h*3, dlblendStr[*(int*) gi.GetCVar("dlblend")->ptr]);
	gc->DrawText(x, y+h*4, *(int*) gi.GetCVar("sprlight")->ptr? "YES" : "NO");
	temp = (int) (*(float*) gi.GetCVar("dlfactor")->ptr * 10 + .5f);
	DrawSlider(this, 6, 11, temp);
	gc->DrawText(x, y+h*8, flareStr[*(int*) gi.GetCVar("flares")->ptr]);
	DrawSlider(this, 10, 11, *(int*) gi.GetCVar("flareintensity")->ptr / 10);
	DrawSlider(this, 13, 11, *(int*) gi.GetCVar("flaresize")->ptr);
	gc->DrawText(x, y+h*15, alignStr[*(int*) gi.GetCVar("spralign")->ptr]);
	gc->DrawText(x, y+h*16, *(int*) gi.GetCVar("sprblend")->ptr? "ON" : "OFF");
}

void KMenuScreenResolution::DrawWindow(KGC *gc)
{
	char buffer[40];
	
	if(selRes == -1)
		strcpy(buffer, "NOT AVAILABLE");
	else
	{
		sprintf(buffer, "%d X %d%s",
			resolutions[selRes].width,
			resolutions[selRes].height,
			selRes == findRes(gi.Get(DD_DEFAULT_RES_X), gi.Get(DD_DEFAULT_RES_Y))? 
				" (DEFAULT)" : "");			
	}
	gc->SetFont(KCanvas::SmallFont);
	gc->DrawText(x+8, y+itemHeight+4, buffer);
	gc->SetFont(Font);
}

static void SCResSelector(int option)
{
	if(option == RIGHT_DIR)
	{
		if(resolutions[selRes+1].width)
			selRes++;
	}
	else if(selRes > 0) selRes--;
}

static void SCResMakeCurrent(int option)
{
	if(gi.ChangeResolution(resolutions[selRes].width, resolutions[selRes].height))
		P_SetMessage(&players[consoleplayer], "RESOLUTION CHANGED", true);
	else
		P_SetMessage(&players[consoleplayer], "RESOLUTION CHANGE FAILED", true);
}

static void SCResMakeDefault(int option)
{
	gi.Set(DD_DEFAULT_RES_X, resolutions[selRes].width);
	gi.Set(DD_DEFAULT_RES_Y, resolutions[selRes].height);
}

static void SCLookSpring(int option)
{
	lookSpring ^= 1;
	if(lookSpring)
		P_SetMessage(&players[consoleplayer], "USING LOOKSPRING", true);
	else
		P_SetMessage(&players[consoleplayer], "NO LOOKSPRING", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void SCAutoAim(int option)
{
	P_SetMessage(&players[consoleplayer], (noAutoAim^=1)? "NO AUTOAIM" : "AUTOAIM ON", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void SCCrosshair(int option)
{
	xhair += option==RIGHT_DIR? 1 : -1;
	if(xhair < 0) xhair = 0;
	if(xhair > NUM_XHAIRS) xhair = NUM_XHAIRS;
}

static void SCCrosshairSize(int option)
{
	xhairSize += option==RIGHT_DIR? 1 : -1;
	if(xhairSize < 0) xhairSize = 0;
	if(xhairSize > 9) xhairSize = 9;
}

static void SCSkyDetail(int option)
{
	int skyDetail = gi.Get(DD_SKY_DETAIL);

	if(option == RIGHT_DIR)
	{
		if(skyDetail < 7)
		{
			skyDetail++;
		}
	}
	else if(skyDetail > 3)
	{
		skyDetail--;
	}
	gi.SkyParams(DD_SKY, DD_COLUMNS, skyDetail);
}

static void SCMipmapping(int option)
{
	int mipmapping = gi.Get(DD_MIPMAPPING);

	if(option == RIGHT_DIR)
	{
		if(mipmapping < 5) mipmapping++;
	}
	else if(mipmapping > 0) mipmapping--;

	gi.GL_TexFilterMode(DD_TEXTURES, mipmapping);
}

static void SCLinearRaw(int option)
{
	int linearRaw = gi.Get(DD_SMOOTH_IMAGES);

	linearRaw ^= 1;
	if(linearRaw)
	{
		P_SetMessage(&players[consoleplayer], "GRAPHICS SCREENS USE LINEAR INTERPOLATION", true);
	}
	else
	{
		P_SetMessage(&players[consoleplayer], "GRAPHICS SCREENS AREN'T INTERPOLATED", true);
	}
	S_StartSound(NULL, SFX_CHAT);
	gi.GL_TexFilterMode(DD_RAWSCREENS, linearRaw);
}

static void SCBorderUpdate(int option)
{
	cvar_t	*cv = gi.GetCVar("borderupd");

	P_SetMessage(&players[consoleplayer], (*(int*) cv->ptr ^= 1)? 
		"BORDERS UPDATED EVERY FRAME" : "BORDERS UPDATED WHEN NEEDED", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void ChangeIntCVar(char *name, int delta)
{
	cvar_t	*cv = gi.GetCVar(name);
	int		val = *(int*) cv->ptr;

	val += delta;
	if(val > cv->max) val = cv->max;
	if(val < cv->min) val = cv->min;
	*(int*) cv->ptr = val;
}

static void SCTexQuality(int option)
{
	ChangeIntCVar("r_texquality", option==RIGHT_DIR? 1 : -1);
}

static void SCForceTexReload(int option)
{
	gi.GL_ClearTextureMem();
	P_SetMessage(&players[consoleplayer], "ALL TEXTURES DELETED", true);
}

static void SCFPSCounter(int option)
{
	showFPS ^= 1;
	S_StartSound(NULL, SFX_CHAT);
}

static void SCIceCorpse(int option)
{
	P_SetMessage(&players[consoleplayer], (translucentIceCorpse ^= 1)? 
		"FROZEN MONSTERS ARE NOW TRANSLUCENT" : "FROZEN MONSTERS NOT TRANSLUCENT", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void SCDynLights(int option)
{
	cvar_t	*cvDL = gi.GetCVar("dynlights");

	P_SetMessage(&players[consoleplayer], (*(int*) cvDL->ptr ^= 1)? 
		"DYNAMIC LIGHTS ENABLED" : "DYNAMIC LIGHTS DISABLED", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void SCDLBlend(int option)
{
	ChangeIntCVar("dlblend", option==RIGHT_DIR? 1 : -1);
}

static void SCDLIntensity(int option)
{
	cvar_t	*cv = gi.GetCVar("dlfactor");
	float	val = *(float*) cv->ptr;

	val += option==RIGHT_DIR? .1f : -.1f;
	if(val > cv->max) val = cv->max;
	if(val < cv->min) val = cv->min;
	*(float*) cv->ptr = val;
}

static void SCFlares(int option)
{
	ChangeIntCVar("flares", option==RIGHT_DIR? 1 : -1);
}

static void SCFlareIntensity(int option)
{
	ChangeIntCVar("flareintensity", option==RIGHT_DIR? 10 : -10);
}

static void SCFlareSize(int option)
{
	ChangeIntCVar("flaresize", option==RIGHT_DIR? 1 : -1);
}

static void SCSpriteAlign(int option)
{
/*	cvar_t *cv = gi.GetCVar("spralign");

	P_SetMessage(&players[consoleplayer], (*(int*) cv->ptr ^= 1)? 
		"SPRITES ALIGNED TO VIEW PLANE" : "SPRITES ALIGNED TO CAMERA", true);
	S_StartSound(NULL, SFX_CHAT);*/

	ChangeIntCVar("spralign", option==RIGHT_DIR? 1 : -1);
}

static void SCSpriteBlending(int option)
{
	cvar_t *cv = gi.GetCVar("sprblend");

	P_SetMessage(&players[consoleplayer], (*(int*) cv->ptr ^= 1)? 
		"ADDITIVE BLENDING FOR EXPLOSIONS" : "NO SPRITE BLENDING", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void SCSpriteLight(int option)
{
	cvar_t *cv = gi.GetCVar("sprlight");

	P_SetMessage(&players[consoleplayer], (*(int*) cv->ptr ^= 1)? 
		"SPRITES LIT BY DYNAMIC LIGHTS" : "SPRITES NOT LIGHT BY LIGHTS", true);
	S_StartSound(NULL, SFX_CHAT);
}

//---------------------------------------------------------------------------
//
//	KMenuScreenMouseOptions::DrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreenMouseOptions::DrawWindow(KGC *gc)
{
	gc->DrawText(x + Font->TextWidth(Items[0]->text) + 12, y,
		gi.Get(DD_MOUSE_INVERSE_Y) ? "YES" : "NO");
	gc->DrawText(x + Font->TextWidth(Items[1]->text) + 12, y + itemHeight,
		usemlook ? "YES" : "NO");
	gc->DrawText(x + Font->TextWidth(Items[2]->text) + 12, y + itemHeight * 2,
		mlookInverseY ? "YES" : "NO");
	DrawSlider(this, 4, 18, mouseSensitivityX);
	DrawSlider(this, 6, 18, mouseSensitivityY);
}

static void SCControlConfig(int option)
{
	if(grabbing != NULL) gi.Error("SCControlConfig: grabbing is not NULL!!!\n");
	
	grabbing = controls + option;
}

void spacecat(char *str, const char *catstr)
{
	if(str[0]) strcat(str, " ");
	
	// Also do some filtering.
	switch(catstr[0])
	{
	case '\\':
		strcat(str, "bkslash");
		break;
	
	case '[':
		strcat(str, "sqbtopen");
		break;

	case ']':
		strcat(str, "sqbtclose");
		break;

	default:
		strcat(str, catstr);
	}
}

//---------------------------------------------------------------------------
//
//	KMenuScreenControls::CreateChoices
//
//---------------------------------------------------------------------------

void KMenuScreenControls::CreateChoices(void)
{
	for (int i = 0; i < itemCount; i++)
	{
		KMenuItem_t *It = NewWindow(KMenuItem_t, this);
		It->SetPos(x + (i / numVisItems) * 320, y + (i % numVisItems) * itemHeight);
		It->type = items[i].type;
		It->text = items[i].text;
		It->func = items[i].func;
		It->option = items[i].option;
		It->menu = items[i].menu;
		Items[i] = It;
	}
}

//---------------------------------------------------------------------------
//
//	KMenuScreenControls::DrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreenControls::DrawWindow(KGC *gc)
{
	int			i;
	char		controlCmd[80];
	char		buff[80], prbuff[80], *token;
	Control_t	*ctrl;

	gc->SetFont(KCanvas::BigFont);
	gc->DrawText(280, 4, "CONTROLS");

	// Draw the page arrows.
	gc->SetFont(KCanvas::YellowFont);
	for (i = 0; i < itemCount; i++)
	{
		KMenuItem_t	*item = Items[i];
		if (item->type == ITT_EMPTY) continue;
		
		ctrl = controls + item->option;
		strcpy(buff, "");
		if (ctrl->flags & CLF_ACTION)
			sprintf(controlCmd, "+%s", ctrl->command);
		else
			strcpy(controlCmd, ctrl->command);
		// Let's gather all the bindings for this command.
		if(!gi.BindingsForCommand(controlCmd, buff))
			strcpy(buff, "NONE");
	
		// Now we must interpret what the bindings string says.
		// It may contain characters we can't print.
		strcpy(prbuff, "");
		token = strtok(buff, " ");
		while (token)
		{
			if (token[0] == '+')
				spacecat(prbuff, token+1);
			if (token[0] == '*' && !(ctrl->flags & CLF_REPEAT) || token[0] == '-')
				spacecat(prbuff, token);
			token = strtok(NULL, " ");
		}

		if(grabbing == ctrl)
		{
			// We're grabbing for this control.
			spacecat(prbuff, "...");
		}

		gc->DrawText(x + 134 + (i / numVisItems) * 320, 
			y + (i % numVisItems) * itemHeight, prbuff);
	}
	gc->SetFont(Font);
}

//---------------------------------------------------------------------------
//
//	KMenuScreenControls::PostDrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreenControls::PostDrawWindow(KGC *gc)
{
	gc->DrawIcon(x + SELECTOR_XOFFSET + (CurrentItPos / numVisItems) * 320, 
		y + SELECTOR_YOFFSET + ((CurrentItPos % numVisItems) * itemHeight) - 
		(10 - itemHeight / 2), 
		FindTexture(MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2"));
}

//---------------------------------------------------------------------------
//
//	KMenuScreenControls::KeyPressed
//
//---------------------------------------------------------------------------

bool KMenuScreenControls::KeyPressed(int key)
{
	KMenuItem_t *item;

	int firstVI = (CurrentItPos / numVisItems) * numVisItems;
	int lastVI = firstVI + numVisItems - 1;
	if (lastVI > itemCount - 1)
		lastVI = itemCount - 1;
	item = Items[CurrentItPos];
	switch (key)
	{
	case DDKEY_DOWNARROW:
		do
		{
			if (CurrentItPos + 1 > lastVI)
			{
				CurrentItPos = firstVI;
			}
			else
			{
				CurrentItPos++;
			}
		} while (Items[CurrentItPos]->type == ITT_EMPTY);
		S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
		return true;

	case DDKEY_UPARROW:
		do
		{
			if (CurrentItPos <= firstVI)
			{
				CurrentItPos = lastVI;
			}
			else
			{
				CurrentItPos--;
			}
		} while (Items[CurrentItPos]->type == ITT_EMPTY);
		S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
		return true;

	case DDKEY_LEFTARROW:
		// Let's try to change to the previous page.
		if (CurrentItPos - numVisItems >= 0)
		{
			CurrentItPos -= numVisItems;
			// Make a sound, too.
			S_StartSound(NULL, SFX_PICKUP_KEY);
		}
		return true;

	case DDKEY_RIGHTARROW:
		// Move on to the next page, if possible.
		if (CurrentItPos + numVisItems < itemCount)
		{
			CurrentItPos += numVisItems;
			if (CurrentItPos > itemCount - 1)
				CurrentItPos = itemCount - 1;
			S_StartSound(NULL, SFX_PICKUP_KEY);						
		}
		return true;
	}
	return Super::KeyPressed(key);
}

static void SCJoySensi(int option)
{
	if(option == RIGHT_DIR)
	{
		if(joySensitivity < 9) joySensitivity++;
	}
	else if(joySensitivity > 1) joySensitivity--;
}

void KMenuScreenJoyConfig::DrawWindow(KGC *gc)
{
	DrawSlider(this, 1, 9, joySensitivity-1);	
	gc->DrawText(x + Font->TextWidth(Items[2]->text) + 12, y + itemHeight * 2,
		usejlook ? "YES" : "NO");
	gc->DrawText(x + Font->TextWidth(Items[3]->text) + 12, y + itemHeight * 3,
		jlookInverseY ? "YES" : "NO");
	gc->DrawText(x + Font->TextWidth(Items[4]->text) + 12, y + itemHeight * 4,
		povLookAround? "YES" : "NO");
}

//---------------------------------------------------------------------------
//
// PROC SCQuitGame
//
//---------------------------------------------------------------------------

static void SCQuitGame(int option)
{
	gi.OpenConsole(false);
	MenuActive = false;
	askforquit = true;
	typeofask = 1; //quit game
	if(!netgame && !demoplayback)
	{
		paused = true;
	}
}

//---------------------------------------------------------------------------
//
// PROC SCEndGame
//
//---------------------------------------------------------------------------

static void SCEndGame(int option)
{
	if(demoplayback)
	{
		return;
	}
	if(SCNetCheck(3))
	{
		MenuActive = false;
		askforquit = true;
		typeofask = 2; //endgame
		if(!netgame && !demoplayback)
		{
			paused = true;
		}
	}
}

//---------------------------------------------------------------------------
//
// PROC SCMessages
//
//---------------------------------------------------------------------------

static void SCMessages(int option)
{
	messageson ^= 1;
	if(messageson)
	{
		P_SetMessage(&players[consoleplayer], "MESSAGES ON", true);
	}
	else
	{
		P_SetMessage(&players[consoleplayer], "MESSAGES OFF", true);
	}
	S_StartSound(NULL, SFX_CHAT);
}

static void SCAlwaysRun(int option)
{
	alwaysRun ^= 1;
	if(alwaysRun)
	{
		P_SetMessage(&players[consoleplayer], "ALWAYS RUNNING", true);
	}
	else
	{
		P_SetMessage(&players[consoleplayer], "NORMAL RUNNING", true);
	}
	S_StartSound(NULL, SFX_CHAT);
}

static void SCMouseLook(int option)
{
//	extern int mousepresent;

	usemlook ^= 1;
	if(usemlook)
	{
		// Make sure the mouse is initialized.
/*		usemouse = 1;
		if(!mousepresent) I_StartupMouse();
		if(!mousepresent)
			P_SetMessage(&players[consoleplayer], "MOUSE INIT FAILED", true);
		else*/
		P_SetMessage(&players[consoleplayer], "MOUSE LOOK ON", true);
	}
	else
	{
		P_SetMessage(&players[consoleplayer], "MOUSE LOOK OFF", true);
	}
	S_StartSound(NULL, SFX_CHAT);
}

static void SCJoyLook(int option)
{
	P_SetMessage(&players[consoleplayer], (usejlook^=1)? "JOYSTICK LOOK ON" : "JOYSTICK LOOK OFF", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void SCPOVLook(int option)
{
	P_SetMessage(&players[consoleplayer], (povLookAround^=1)? "POV LOOK ON" : "POV LOOK OFF", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void SCInverseJoyLook(int option)
{
	P_SetMessage(&players[consoleplayer], (jlookInverseY^=1)? "INVERSE JOYLOOK" : "NORMAL JOYLOOK", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void SCMouseLookInverse(int option)
{
	mlookInverseY ^= 1;
	if(mlookInverseY)
		P_SetMessage(&players[consoleplayer], "INVERSE MOUSE LOOK", true);
	else
		P_SetMessage(&players[consoleplayer], "NORMAL MOUSE LOOK", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void SCInverseY(int option)
{
	int val = gi.Get(DD_MOUSE_INVERSE_Y);
		
	P_SetMessage(&players[consoleplayer], 
		(val^=1)? "INVERSE MOUSE Y AXIS" : "NORMAL MOUSE Y AXIS", true);
	gi.Set(DD_MOUSE_INVERSE_Y, val);
	S_StartSound(NULL, SFX_CHAT);
}

//===========================================================================
//
// SCNetCheck
//
//===========================================================================

static boolean SCNetCheck(int option)
{
	if(!netgame)
	{
		return true;
	}
	switch(option)
	{
		case 1: // new game
			P_SetMessage(&players[consoleplayer],
				"YOU CAN'T START A NEW GAME IN NETPLAY!", true);
			break;
		case 2: // load game
			P_SetMessage(&players[consoleplayer],
				"YOU CAN'T LOAD A GAME IN NETPLAY!", true);
			break;
		case 3: // end game
			P_SetMessage(&players[consoleplayer],
				"YOU CAN'T END A GAME IN NETPLAY!", true);
			break;
	}
	MenuActive = false;
	S_StartSound(NULL, SFX_CHAT);
	return false;
}

//===========================================================================
//
// SCNetCheck2
//
//===========================================================================

static void SCNetCheck2(int option)
{
	SCNetCheck(option);
	return;
}

//---------------------------------------------------------------------------
//
// PROC SCLoadGame
//
//---------------------------------------------------------------------------

static void SCLoadGame(int option)
{
	if(!SlotStatus[option])
	{ // Don't try to load from an empty slot
		return;
	}
	// Update save game menu position.
	CurrentMenu->oldItPos = option;

	G_LoadGame(option);
	MN_DeactivateMenu();
	//BorderNeedRefresh = true;
	gi.Update(DDUF_BORDER);
	if(quickload == -1)
	{
		quickload = option+1;
		P_ClearMessage(&players[consoleplayer]);
	}
}

//---------------------------------------------------------------------------
//
// PROC SCSaveGame
//
//---------------------------------------------------------------------------

static void SCSaveGame(int option)
{
	char *ptr;

	if(!FileMenuKeySteal)
	{
		FileMenuKeySteal = true;
		strcpy(oldSlotText, SlotText[option]);
		ptr = SlotText[option];
		while(*ptr)
		{
			ptr++;
		}
		*ptr = '[';
		*(ptr+1) = 0;
		SlotStatus[option]++;
		currentSlot = option;
		slotptr = ptr-SlotText[option];
		return;
	}
	else
	{
		G_SaveGame(option, SlotText[option]);
		FileMenuKeySteal = false;
		// Update save game menu position.
		CurrentMenu->oldItPos = option;
		MN_DeactivateMenu();
	}
	//BorderNeedRefresh = true;
	gi.Update(DDUF_BORDER);
	if(quicksave == -1)
	{
		quicksave = option+1;
		P_ClearMessage(&players[consoleplayer]);
	}
}

//==========================================================================
//
// SCClass
//
//==========================================================================

static void SCClass(int option)
{
	if(netgame)
	{
		P_SetMessage(&players[consoleplayer],
			"YOU CAN'T START A NEW GAME FROM WITHIN A NETGAME!", true);
		return;
	}
	MenuPClass = option;
	switch(MenuPClass)
	{
		case PCLASS_FIGHTER:
			Menus[MENU_SKILL]->x = 120;
			Menus[MENU_SKILL]->Items[0]->text = "SQUIRE";
			Menus[MENU_SKILL]->Items[1]->text = "KNIGHT";
			Menus[MENU_SKILL]->Items[2]->text = "WARRIOR";
			Menus[MENU_SKILL]->Items[3]->text = "BERSERKER";
			Menus[MENU_SKILL]->Items[4]->text = "TITAN";
			break;
		case PCLASS_CLERIC:
			Menus[MENU_SKILL]->x = 116;
			Menus[MENU_SKILL]->Items[0]->text = "ALTAR BOY";
			Menus[MENU_SKILL]->Items[1]->text = "ACOLYTE";
			Menus[MENU_SKILL]->Items[2]->text = "PRIEST";
			Menus[MENU_SKILL]->Items[3]->text = "CARDINAL";
			Menus[MENU_SKILL]->Items[4]->text = "POPE";
			break;
		case PCLASS_MAGE:
			Menus[MENU_SKILL]->x = 112;
			Menus[MENU_SKILL]->Items[0]->text = "APPRENTICE";
			Menus[MENU_SKILL]->Items[1]->text = "ENCHANTER";
			Menus[MENU_SKILL]->Items[2]->text = "SORCERER";
			Menus[MENU_SKILL]->Items[3]->text = "WARLOCK";
			Menus[MENU_SKILL]->Items[4]->text = "ARCHIMAGE";
			break;
		case PCLASS_CORVUS:
			Menus[MENU_SKILL]->x = 112;
			Menus[MENU_SKILL]->Items[0]->text = "VERY EASY";
			Menus[MENU_SKILL]->Items[1]->text = "EASY";
			Menus[MENU_SKILL]->Items[2]->text = "MEDIUM";
			Menus[MENU_SKILL]->Items[3]->text = "HARD";
			Menus[MENU_SKILL]->Items[4]->text = "VERY HARD";
	}
	SetMenu(MENU_SKILL);
}

//---------------------------------------------------------------------------
//
// PROC SCSkill
//
//---------------------------------------------------------------------------
fixed_t init_speed[NUMCLASSES] =
{
	{10},{8},{6},{7}
};

fixed_t init_health[NUMCLASSES] =
{
	{80},{50},{20},{30}
};

fixed_t player_strength[NUMCLASSES]=
{
	{0x08},
	{0x05},
	{0x02},
	{0x06},
};

fixed_t player_agility[NUMCLASSES]=
{
	{0x02},
	{0x05},
	{0x08},
	{0x06},
};


chr_val Defaultroll(int Sclass)
{
	chr_val values;
	values.val[0]=init_health[Sclass];
	if (Sclass == PCLASS_FIGHTER)
		values.val[1]=5;
	else values.val[1]=80-values.val[0];
	values.val[2]=player_strength[Sclass];
	values.val[3]=player_agility[Sclass];
	values.val[4]=init_speed[Sclass];
	return values;
}


chr_val Reroll(int Sclass)
{
	chr_val values;
	switch(Sclass)
	{
	case PCLASS_FIGHTER:
		values.val[2]=6+(P_Random()&3);  //strength
		values.val[4]=6+(P_Random()&6);  //speed
		values.val[3]=1+(P_Random()&2);  //efficiency
		values.val[0]=100-values.val[2]-values.val[3]-values.val[4];  //health
		values.val[1]=3+(P_Random()&2);  //spiritual power
		break;
	case PCLASS_CLERIC:
		values.val[2]=2+(P_Random()&4);  //strength
		values.val[4]=2+(P_Random()&6);  //speed
		values.val[3]=2+(P_Random()&4);  //efficiency
		values.val[0]=66-values.val[2]-values.val[3]-values.val[4];  //health
		values.val[1]=80-values.val[0];               //spiritual power
		break;
	case PCLASS_MAGE:
		values.val[2]=1+(P_Random()&3);  //strength
		values.val[4]=1+(P_Random()&5);  //speed
		values.val[3]=6+(P_Random()&6);  //efficiency
		values.val[0]=40-values.val[2]-values.val[3]-values.val[4];  //health
		values.val[1]=80-values.val[0];               //spiritual power
		break;
	case PCLASS_CORVUS:
		values.val[2]=2+(P_Random()&4);  //strength
		values.val[4]=2+(P_Random()&6);  //speed
		values.val[3]=2+(P_Random()&4);  //efficiency
		values.val[0]=66-values.val[2]-values.val[3]-values.val[4];  //health
		values.val[1]=80-values.val[0];               //spiritual power
		break;
	default:
		break;
	}
	return values;
}

static void SCSkill(int option)
{
	MenuPValues=Reroll(MenuPClass);
	MenuPSkill=option;
	SetMenu(MENU_CHAR);
}

//---------------------------------------------------------------------------
//
// PROC SCChar
//
//---------------------------------------------------------------------------

static void SCChar(int option)
{
	extern int SB_state;
	if (option==0) 
	{
		MenuPValues=Reroll(MenuPClass);
		return;
	}
	if (option==1) 
	{
		MenuPValues=Defaultroll(MenuPClass);
		return;
	}

	SetMenu(MENU_CHAR);

	PlayerClass[consoleplayer] = (pclass_t)MenuPClass;
	G_DeferredNewGame((skill_t)MenuPSkill);
	SB_SetClassData();
	SB_state = -1;
	MN_DeactivateMenu();
}


//---------------------------------------------------------------------------
//
// PROC SCMouseXSensi
//
//---------------------------------------------------------------------------

static void SCMouseXSensi(int option)
{
	if(option == RIGHT_DIR)
	{
		if(mouseSensitivityX < 17)
		{
			mouseSensitivityX++;
		}
	}
	else if(mouseSensitivityX)
	{
		mouseSensitivityX--;
	}
}

//---------------------------------------------------------------------------
//
// PROC SCMouseYSensi
//
//---------------------------------------------------------------------------

static void SCMouseYSensi(int option)
{
	if(option == RIGHT_DIR)
	{
		if(mouseSensitivityY < 17)
		{
			mouseSensitivityY++;
		}
	}
	else if(mouseSensitivityY)
	{
		mouseSensitivityY--;
	}
}

//---------------------------------------------------------------------------
//
// PROC SCSfxVolume
//
//---------------------------------------------------------------------------

static void SCSfxVolume(int option)
{
	int vol = gi.Get(DD_SFX_VOLUME);

	vol += option==RIGHT_DIR? 15 : -15;
	//soundchanged = true; // we'll set it when we leave the menu
	gi.SetSfxVolume(vol);
}

//---------------------------------------------------------------------------
//
// PROC SCMusicVolume
//
//---------------------------------------------------------------------------

static void SCMusicVolume(int option)
{
	int vol = gi.Get(DD_MIDI_VOLUME);

	vol += option==RIGHT_DIR? 15 : -15;
	gi.SetMIDIVolume(vol);
}

//---------------------------------------------------------------------------
//
// PROC SCCDVolume
//
//---------------------------------------------------------------------------

static void SCCDVolume(int option)
{
	int vol = gi.CD(DD_GET_VOLUME, 0);

	vol += option==RIGHT_DIR? 15 : -15;
	gi.CD(DD_SET_VOLUME, vol);
}

static void SC3DSounds(int option)
{
	cvar_t	*cv3ds = gi.GetCVar("s_3d");

	P_SetMessage(&players[consoleplayer], (*(int*) cv3ds->ptr ^= 1)? 
		"3D SOUND MODE" : "2D SOUND MODE", true);
	S_StartSound(NULL, SFX_CHAT);
}

static void SCReverbVolume(int option)
{
	extern sector_t *listenerSector;
	cvar_t	*cv = gi.GetCVar("s_reverbVol");
	float	val = *(float*) cv->ptr;

	val += option==RIGHT_DIR? .1f : -.1f;
	if(val > cv->max) val = cv->max;
	if(val < cv->min) val = cv->min;
	*(float*) cv->ptr = val;

	// This'll force the sound updater to set the reverb.
	listenerSector = NULL;
}

static void SCSfxFrequency(int option)
{
	cvar_t	*cv = gi.GetCVar("s_resample");
	int		oldval = *(int*) cv->ptr, val = oldval;
	
	val += option==RIGHT_DIR? 1 : -1;
	if(val > 4) val = 4;
	if(val < 1) val = 1;
	if(val == 3)
	{
		if(oldval == 4) 
			val = 2; 
		else 
			val = 4;
	}
	*(int*) cv->ptr = val;
}

static void SCSfx16bit(int option)
{
	cvar_t	*cv = gi.GetCVar("s_16bit");

	P_SetMessage(&players[consoleplayer], (*(int*) cv->ptr ^= 1)? 
		"16 BIT INTERPOLATION" : "8 BIT INTERPOLATION", true);
	S_StartSound(NULL, SFX_CHAT);
}

//---------------------------------------------------------------------------
//
// PROC SCScreenSize
//
//---------------------------------------------------------------------------

static void SCScreenSize(int option)
{
	if(option == RIGHT_DIR)
	{
		if(screenblocks < 11)
		{
			screenblocks++;
		}
	}
	else if(screenblocks > 3)
	{
		screenblocks--;
	}
	R_SetViewSize(screenblocks);
}

//---------------------------------------------------------------------------
//
// PROC SCInfo
//
//---------------------------------------------------------------------------

static void SCInfo(int option)
{
	InfoType = 1;
	S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
	if(!netgame && !demoplayback)
	{
		paused = true;
	}
}

// Set default bindings for unbound Controls.
void H2_DefaultBindings()
{
	int			i;
	Control_t	*ctr;
	char		evname[80], cmd[256], buff[256];
	event_t		event;
	
	// Check all Controls.
	for(i=0; controls[i].command[0]; i++)
	{
		ctr = controls + i;
		// If this command is bound to something, skip it.
		sprintf(cmd, "%s%s", ctr->flags & CLF_ACTION? "+" : "",
			ctr->command);
		if(gi.BindingsForCommand(cmd, buff)) continue;

		// This Control has no bindings, set it to the default.
		sprintf(buff, "\"%s\"", ctr->command);
		if(ctr->defKey)
		{
			event.type = ev_keydown;
			event.data1 = ctr->defKey;
			gi.EventBuilder(evname, &event, false);
			sprintf(cmd, "%s %s %s", ctr->flags & CLF_REPEAT? "safebindr" : "safebind",
				evname+1, buff);
			gi.Execute(cmd, true);
		}
		if(ctr->defMouse)
		{
			event.type = ev_mousebdown;
			event.data1 = 1 << (ctr->defMouse-1);
			gi.EventBuilder(evname, &event, false);
			sprintf(cmd, "%s %s %s", ctr->flags & CLF_REPEAT? "safebindr" : "safebind",
				evname+1, buff);
			gi.Execute(cmd, true);
		}
		if(ctr->defJoy)
		{
			event.type = ev_joybdown;
			event.data1 = 1 << (ctr->defJoy-1);
			gi.EventBuilder(evname, &event, false);
			sprintf(cmd, "%s %s %s", ctr->flags & CLF_REPEAT? "safebindr" : "safebind",
				evname+1, buff);
			gi.Execute(cmd, true);
		}
	}
}

//---------------------------------------------------------------------------
//
// FUNC H2_PrivilegedResponder
//
//---------------------------------------------------------------------------

int findtoken(char *string, char *token, char *delim)
{
	char *ptr = strtok(string, delim);
	while(ptr)
	{
		if(!stricmp(ptr, token)) return true;
		ptr = strtok(NULL, delim);
	}
	return false;
}

int H2_PrivilegedResponder(event_t *event)
{
	// We're interested in key or button down events.
	if(grabbing && (event->type == ev_keydown || event->type == ev_mousebdown 
		|| event->type == ev_joybdown || event->type == ev_povdown))
	{
		// We'll grab this event.
		char cmd[256], buff[256], evname[80];
		boolean del = false;

		// Check for a cancel.
		if(event->type == ev_keydown)
		{
			if(event->data1 == '`') // Tilde clears everything.
			{
				if(grabbing->flags & CLF_ACTION)
					sprintf(cmd, "delbind +%s -%s", grabbing->command,
						grabbing->command);
				else
					sprintf(cmd, "delbind \"%s\"", grabbing->command);
				gi.Execute(cmd, true);
				grabbing = NULL;
				return true;
			}
			else if(event->data1 == DDKEY_ESCAPE)
			{
				grabbing = NULL;
				return true;
			}
		}

		// We shall issue a silent console command, but first we need
		// a textual representation of the event.
		gi.EventBuilder(evname, event, false); // "Deconstruct" into a name.

		// If this binding already exists, remove it.
		sprintf(cmd, "%s%s", grabbing->flags & CLF_ACTION? "+" : "",
			grabbing->command);
		if(gi.BindingsForCommand(cmd, buff))
			if(findtoken(buff, evname, " "))		// Get rid of it?
			{
				del = true;
				strcpy(buff, "");
			}
		if(!del) sprintf(buff, "\"%s\"", grabbing->command);
		sprintf(cmd, "%s %s %s", grabbing->flags & CLF_REPEAT? "bindr" : "bind",
			evname+1, buff);
		gi.Execute(cmd, true);
		// We've finished the grab.
		grabbing = NULL;
		S_StartSound(NULL, SFX_CHAT);
		return true;
	}

	// Process the screen shot key right away.
	if(ravpic && event->data1 == DDKEY_F1)
	{
		if(event->type == ev_keydown) G_ScreenShot();
		// All F1 events are eaten.
		return true;
	}
	return F_Responder(event);
}

//---------------------------------------------------------------------------
//
// FUNC MN_Responder
//
//---------------------------------------------------------------------------

boolean MN_Responder(event_t *event)
{
	int key;
	extern boolean automapactive;
	extern void H2_StartTitle(void);
	char *textBuffer;

	if(event->data1 == DDKEY_RSHIFT)
	{
		shiftdown = (event->type == ev_keydown);
	}

	if(event->type != ev_keydown && event->type != ev_keyrepeat)
	{
#ifdef USE_JOURNAL_MOUSE
		if (JournalActive && ((event->type== ev_mouse)||(event->type== ev_mousebdown))) ;
		else
#endif
		return(false);
	}
	key = event->data1;

	if (JournalActive)
	{
#ifdef USE_JOURNAL_MOUSE
		if (event->type== ev_mouse)
		{
//			int mousex = (event->data1 * (mouseSensitivityX*2+5)) / 6;
//			int mousey = (event->data2 * (mouseSensitivityY*2+5)) / 6;
			int mousex = event->data1;
			int mousey = event->data2;
			mousesx+=mousex;mousesy-=mousey;
			if (mousesx<0) mousesx=0; else
			if (mousesx>300) mousesx=300;
			if (mousesy<0) mousesy=0; else
			if (mousesy>180) mousesy=180;
			return(true); // eat events
		}

		if (event->type== ev_mousebdown)
		{
			for(i=0; i<3; i++)
				if(event->data1 & (1<<i))
					if (mousesx<80) currentJournal=mousesy/40;
					else if (mousesx>300 && mousesy>180)
						currentJournalPage= currentJournalPage==1 ? 1 : currentJournalPage+1;
//					mousebutton[i] = true;
//
			currentJournalPage=0;
			return(false);
		}
#endif

		if(key == DDKEY_ESCAPE || key == DDKEY_TAB)
		{
			JournalActive = false;
			if(!netgame && !demoplayback && !demorecording)
			{
				paused = false;
			}
			return(true);
		}
		if(key == DDKEY_DOWNARROW)
		{
			currentJournal= currentJournal==4 ? 0 : currentJournal+1;
			currentJournalPage=0;
			return(true);
		}
		if(key == DDKEY_LEFTARROW)
		{
			currentJournalPage= currentJournalPage==0 ? 0 : currentJournalPage-1;
			return(true);
		}
		if(key == DDKEY_RIGHTARROW)
		{
			currentJournalPage= currentJournalPage==journalPages[currentJournal].num_pages-1 ? journalPages[currentJournal].num_pages-1 : currentJournalPage+1;
			return(true);
		}
		if(key == DDKEY_UPARROW)
		{
			currentJournal= currentJournal==0 ? 4 : currentJournal-1;
			currentJournalPage=0;
			return(true);
		}
	}

	if (SpellsActive)
	{
		if(key == DDKEY_ESCAPE || key == 's')
		{
			SpellsActive = false;
			if(!netgame && !demoplayback && !demorecording)
			{
				paused = false;
			}
			return(true);
		}
		if(key == DDKEY_DOWNARROW)
		{
			return(true);
		}
		if(key == DDKEY_LEFTARROW)
		{
			return(true);
		}
		if(key == DDKEY_RIGHTARROW)
		{
			return(true);
		}
		if(key == DDKEY_UPARROW)
		{
			return(true);
		}
	}

	if (UpdatingActive)
	{
		if(key == DDKEY_ESCAPE || key == 'u')
		{
			UpdatingActive = false;
			if(!netgame && !demoplayback && !demorecording)
			{
				paused = false;
			}
			return(true);
		}
		if(key == DDKEY_DOWNARROW)
		{
			currentUpdatingRow= currentUpdatingRow==4 ? 1 : currentUpdatingRow+1;
			return(true);
		}
		if(!players[consoleplayer].berserkTics)
		{ //Remi: Player can't mess with stats while berserking		
			if(key == DDKEY_LEFTARROW)
			{
				if (upd_values[currentUpdatingRow][0]<upd_values[currentUpdatingRow][1] && currentUpdatingRow<4 && currentUpdatingRow>0) 
				{
					upd_values[currentUpdatingRow][1]=upd_values[currentUpdatingRow][1]-1;
					navpoints++;
				}
				return(true);
			}
			if(key == DDKEY_RIGHTARROW)
			{
				if (navpoints>0 && currentUpdatingRow<4 && currentUpdatingRow>0  && (upd_values[currentUpdatingRow][1]<maxupd_values[currentUpdatingRow][players[consoleplayer].pclass] || maxupd_values[currentUpdatingRow][players[consoleplayer].pclass]==-1)) 
				{
					upd_values[currentUpdatingRow][1]=upd_values[currentUpdatingRow][1]+1;
					navpoints--;
				}
				return(true);
			}
		}
		if(key == DDKEY_UPARROW)
		{
			currentUpdatingRow= currentUpdatingRow==1 ? 4 : currentUpdatingRow-1;
			return(true);
		}
		if(key == DDKEY_ENTER)
		{
			if (currentUpdatingRow!=4) return(false);
			AcceptUpdating();
			UpdatingActive = false;
			if(!netgame && !demoplayback && !demorecording)
			{
				paused = false;
			}
			return(true);
		}
	}

	if(InfoType)
	{
		InfoType = (InfoType+1)%4;
		if(key == DDKEY_ESCAPE)
		{
			InfoType = 0;
		}
		if(!InfoType)
		{
			if(!netgame && !demoplayback)
			{
				paused = false;
			}
			MN_DeactivateMenu();
			fadingOut = false;			
			bgAlpha = 0;
			SB_state = -1; //refresh the statbar
			//BorderNeedRefresh = true;
			gi.Update(DDUF_BORDER);
		}
		S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
		return(true); //make the info screen eat the keypress
	}

	if(askforquit)
	{
		switch(key)
		{
			case 'y':
				if(askforquit)
				{
					switch(typeofask) //Switch questioning
					{
						case 1:
							G_CheckDemoStatus(); 
							gi.Quit();
							break;
						case 2:
							P_ClearMessage(&players[consoleplayer]);
							typeofask = 0;
							askforquit = false;
							paused = false;
							gi.GL_SetFilter(0);
							H2_StartTitle(); // go to intro/demo mode.
							break;
						case 3:
							P_SetMessage(&players[consoleplayer], 
								"QUICKSAVING....", false);
							FileMenuKeySteal = true;
							SCSaveGame(quicksave-1);
							askforquit = false;
							typeofask = 0;
							//BorderNeedRefresh = true;
							gi.Update(DDUF_BORDER);
							return true;
						case 4:
							P_SetMessage(&players[consoleplayer], 
								"QUICKLOADING....", false);
							SCLoadGame(quickload-1);
							askforquit = false;
							typeofask = 0;
							//BorderNeedRefresh = true;
							gi.Update(DDUF_BORDER);
							return true;
						case 5:
							askforquit = false;
							typeofask = 0;	
							//BorderNeedRefresh = true;
							gi.Update(DDUF_BORDER);
							mn_SuicideConsole = true;
							return true;
							break;
						default:
							return true; // eat the 'y' keypress
					}
				}
				return false;
			case 'n':
			case DDKEY_ESCAPE:
				if(askforquit)
				{
					players[consoleplayer].messageTics = 0;
					players[consoleplayer].messageTics2 = 0;
					askforquit = false;
					typeofask = 0;
					paused = false;
					/*UpdateState |= I_FULLSCRN;
					BorderNeedRefresh = true;*/
					gi.Update(DDUF_FULLSCREEN | DDUF_BORDER);
					return true;
				}
				return false;
		}
		return false; // don't let the keys filter thru
	}
	if(MenuActive == false && !chatmodeon && !(JournalActive || SpellsActive || UpdatingActive))
	{
		switch(key)
		{
			case DDKEY_TAB:
				JournalActive=true;
				if(!netgame && !demoplayback && !demorecording)
				{
					paused = true;
				}
				return true;  //kmod
/*			case 's':
				SpellsActive=true;
				if(!netgame && !demoplayback && !demorecording)
				{
					paused = true;
				}
				return true;*/  //kmod
			case 'u':
				InitUpdating();
				UpdatingActive=true;
				if(!netgame && !demoplayback && !demorecording)
				{
					paused = true;
				}
				return true;
		}

	}

	if(MenuActive == false)
	{
		if(key == DDKEY_ESCAPE || gamestate == GS_DEMOSCREEN || demoplayback)
		{
			MN_ActivateMenu();
			return(false); // allow bindings (like demostop)
		} //getting into menu
		return(false);
	}
	if(!FileMenuKeySteal)
	{
		return CurrentMenu->KeyPressed(key);
	}
	else
	{ // Editing file names
		KMenuItem_t *item;
		textBuffer = &SlotText[currentSlot][slotptr];
		if(key == DDKEY_BACKSPACE)
		{
			if(slotptr)
			{
				*textBuffer-- = 0;
				*textBuffer = ASCII_CURSOR;
				slotptr--;
			}
			return(true);
		}
		if(key == DDKEY_ESCAPE)
		{
			memset(SlotText[currentSlot], 0, SLOTTEXTLEN+2);
			strcpy(SlotText[currentSlot], oldSlotText);
			SlotStatus[currentSlot]--;
			MN_DeactivateMenu();
			return(true);
		}
		if(key == DDKEY_ENTER)
		{
			SlotText[currentSlot][slotptr] = 0; // clear the cursor
			item = CurrentMenu->Items[CurrentItPos];
			CurrentMenu->oldItPos = CurrentItPos;
			if(item->type == ITT_EFUNC)
			{
				item->func(item->option);
				if(item->menu != MENU_NONE)
				{
					SetMenu(item->menu);
				}
			}
			return(true);
		}
		if(slotptr < SLOTTEXTLEN && key != DDKEY_BACKSPACE)
		{
			if((key >= 'a' && key <= 'z'))
			{
				*textBuffer++ = key-32;
				*textBuffer = ASCII_CURSOR;
				slotptr++;
				return(true);
			}
			if(((key >= '0' && key <= '9') || key == ' '
				|| key == ',' || key == '.' || key == '-')
				&& !shiftdown)
			{
				*textBuffer++ = key;
				*textBuffer = ASCII_CURSOR;
				slotptr++;
				return(true);
			}
			if(shiftdown && key == '1')
			{
				*textBuffer++ = '!';
				*textBuffer = ASCII_CURSOR;
				slotptr++;
				return(true);
			}
		}
		return(true);
	}
	return(false);
}

//---------------------------------------------------------------------------
//
//	Menu_t::KeyPressed
//
//---------------------------------------------------------------------------

bool Menu_t::KeyPressed(int key)
{
	int i;
	KMenuItem_t *item;

	item = Items[CurrentItPos];
	switch (key)
	{
	case DDKEY_DOWNARROW:
		do
		{
			if (CurrentItPos + 1 > itemCount - 1)
			{
				CurrentItPos = 0;
			}
			else
			{
				CurrentItPos++;
			}
		} while(Items[CurrentItPos]->type == ITT_EMPTY);
		S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
		return true;

	case DDKEY_UPARROW:
		do
		{
			if (CurrentItPos <= 0)
			{
				CurrentItPos = itemCount - 1;
			}
			else
			{
				CurrentItPos--;
			}
		} while (Items[CurrentItPos]->type == ITT_EMPTY);
		S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
		return true;

	case DDKEY_LEFTARROW:
		if (item->type == ITT_LRFUNC && item->func != NULL)
		{
			item->func(LEFT_DIR);
			S_StartSound(NULL, SFX_PICKUP_KEY);
		}
		return true;

	case DDKEY_RIGHTARROW:
		if (item->type == ITT_LRFUNC && item->func != NULL)
		{
			item->func(RIGHT_DIR);
			S_StartSound(NULL, SFX_PICKUP_KEY);
		}
		return true;

	case DDKEY_ENTER:
		if (item->type == ITT_SETMENU)
		{
			if (item->func != NULL)	
			{
				item->func(item->option);
			}
			SetMenu(item->menu);
		}
		else if (item->func != NULL)
		{
			oldItPos = CurrentItPos;
			if (item->type == ITT_LRFUNC)
			{
				item->func(RIGHT_DIR);
			}
			else if (item->type == ITT_EFUNC)
			{
				item->func(item->option);
			}
		}
		S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
		return true;

	case DDKEY_ESCAPE:  //getting out of the menu
		MN_DeactivateMenu();
		return true;

	case DDKEY_BACKSPACE:
		S_StartSound(NULL, SFX_PICKUP_KEY);
		if (prevMenu == MENU_NONE)
		{
			MN_DeactivateMenu();
		}
		else
		{
			SetMenu(prevMenu);
		}
		return true;

	default:
		for (i = 0; i < itemCount; i++)
		{
			if (Items[i]->text && Items[i]->type != ITT_EMPTY)
			{
				if (toupper(key) == toupper(Items[i]->text[0]))
				{
					CurrentItPos = i;
					return true;
				}
			}
		}
		break;
	}
	return Super::KeyPressed(key);
}

//---------------------------------------------------------------------------
//
//	CCmdMenuAction
//
//---------------------------------------------------------------------------

int CCmdMenuAction(int argc, char **argv)
{
	// Can we get out of here early?
	if(/*MenuActive == true || */chatmodeon) return true;

	if(!stricmp(argv[0], "infoscreen"))
	{
		SCInfo(0); // start up info screens
		MenuActive = true;
		fadingOut = false;
	}
	else if(!stricmp(argv[0], "savegame"))
	{
		if(gamestate == GS_LEVEL && !demoplayback)
		{
			MenuActive = true;
			fadingOut = false;
			FileMenuKeySteal = false;
			MenuTime = 0;
			CurrentMenu = Menus[MENU_SAVE];
			CurrentItPos = CurrentMenu->oldItPos;
			if(!netgame && !demoplayback)
			{
				paused = true;
			}
			S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
			slottextloaded = false; //reload the slot text, when needed
		}
	}
	else if(!stricmp(argv[0], "loadgame"))
	{
		if(SCNetCheck(2))
		{
			MenuActive = true;
			fadingOut = false;
			FileMenuKeySteal = false;
			MenuTime = 0;
			CurrentMenu = Menus[MENU_LOAD];
			CurrentItPos = CurrentMenu->oldItPos;
			if(!netgame && !demoplayback)
			{
				paused = true;
			}
			S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
			slottextloaded = false; //reload the slot text, when needed
		}
	}
	else if(!stricmp(argv[0], "soundmenu"))
	{
		MenuActive = true;
		fadingOut = false;
		FileMenuKeySteal = false;
		MenuTime = 0;
		CurrentMenu = Menus[MENU_OPTIONS2];
		CurrentItPos = CurrentMenu->oldItPos;
		if(!netgame && !demoplayback)
		{
			paused = true;
		}
		S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
		slottextloaded = false; //reload the slot text, when needed
	}
	else if(!stricmp(argv[0], "suicide"))
	{
		gi.OpenConsole(false);
		MenuActive = false;
		askforquit = true;
		typeofask = 5; // suicide
		return true;
	}
	else if(!stricmp(argv[0], "quicksave"))
	{
		if(gamestate == GS_LEVEL && !demoplayback)
		{
			if(!quicksave || quicksave == -1)
			{
				MenuActive = true;
				fadingOut = false;
				FileMenuKeySteal = false;
				MenuTime = 0;
				CurrentMenu = Menus[MENU_SAVE];
				CurrentItPos = CurrentMenu->oldItPos;
				if(!netgame && !demoplayback)
				{
					paused = true;
				}
				S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
				slottextloaded = false; //reload the slot text
				quicksave = -1;
				P_SetMessage(&players[consoleplayer],
					"CHOOSE A QUICKSAVE SLOT", true);
			}
			else
			{
				askforquit = true;
				typeofask = 3;
				if(!netgame && !demoplayback)
				{
					paused = true;
				}
				S_StartSound(NULL, SFX_CHAT);
			}
		}
	}
	else if(!stricmp(argv[0], "endgame"))
	{
		if(SCNetCheck(3))
		{
			if(gamestate == GS_LEVEL && !demoplayback)
			{
				S_StartSound(NULL, SFX_CHAT);
				SCEndGame(0);
			}
		}
	}
	else if(!stricmp(argv[0], "toggleMsgs"))
	{
		SCMessages(0);
	}
	else if(!stricmp(argv[0], "quickload"))
	{
		if(SCNetCheck(2))
		{
			if(!quickload || quickload == -1)
			{
				MenuActive = true;
				fadingOut = false;
				FileMenuKeySteal = false;
				MenuTime = 0;
				CurrentMenu = Menus[MENU_LOAD];
				CurrentItPos = CurrentMenu->oldItPos;
				if(!netgame && !demoplayback)
				{
					paused = true;
				}
				S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
				slottextloaded = false; // reload the slot text
				quickload = -1;
				P_SetMessage(&players[consoleplayer],
					"CHOOSE A QUICKLOAD SLOT", true);
			}
			else
			{
				askforquit = true;
				if(!netgame && !demoplayback)
				{
					paused = true;
				}
				typeofask = 4;
				S_StartSound(NULL, SFX_CHAT);
			}
		}
	}
	else if(!stricmp(argv[0], "quit"))
	{
		SCQuitGame(0);
		S_StartSound(NULL, SFX_CHAT);
	}
	else if(!stricmp(argv[0], "toggleGamma"))
	{
		int gamma = gi.Get(DD_GAMMA) + 1;
		char cmd[20];
		if(gamma > 4)
		{
			gamma = 0;
		}
		sprintf(cmd, "setgamma %d", gamma);
		gi.Execute(cmd, true);
		P_SetMessage(&players[consoleplayer], GammaText[gamma],
			false);
	}
/*	else if(!stricmp(argv[0], 
			case DDKEY_F12: // F12 - reload current map (devmaps mode)
				if(netgame || DevMaps == false)
				{
					return false;
				}
				if(actions[H2A_SPEED].on)
				{ // Monsters ON
					nomonsters = false;
				}
				if(actions[H2A_STRAFE].on)
				{ // Monsters OFF
					nomonsters = true;
				}
				G_DeferedInitNew(gameskill, gameepisode, gamemap);
				P_SetMessage(&players[consoleplayer], TXT_CHEATWARP,
					false);
				return true;
*/	
	return true;
}

//---------------------------------------------------------------------------
//
// PROC MN_ActivateMenu
//
//---------------------------------------------------------------------------

void MN_ActivateMenu(void)
{
	if(MenuActive)
	{
		return;
	}
	if(paused)
	{
		S_ResumeSound();
	}
	MenuActive = true;
	FileMenuKeySteal = false;
	MenuTime = 0;
	fadingOut = false;
	CurrentMenu = Menus[MENU_MAIN];
	CurrentItPos = CurrentMenu->oldItPos;
	if(!netgame && !demoplayback)
	{
		paused = true;
	}
	S_StartSound(NULL, SFX_PLATFORM_STOP);
	slottextloaded = false; //reload the slot text, when needed
}

//---------------------------------------------------------------------------
//
// PROC MN_DeactivateMenu
//
//---------------------------------------------------------------------------

void MN_DeactivateMenu(void)
{
	if(!CurrentMenu) return;

	CurrentMenu->oldItPos = CurrentItPos;
	MenuActive = false;
	if(!netgame)
	{
		paused = false;
	}
	S_StartSound(NULL, SFX_PLATFORM_STOP);
	P_ClearMessage(&players[consoleplayer]);

	fadingOut = true;
	outFade = 0;
}

//---------------------------------------------------------------------------
//
// PROC SetMenu
//
//---------------------------------------------------------------------------

void SetMenu(MenuType_t menu)
{
	CurrentMenu->oldItPos = CurrentItPos;
	CurrentMenu = Menus[menu];
	CurrentItPos = CurrentMenu->oldItPos;
}

//---------------------------------------------------------------------------
//
// PROC DrawSlider
//
//---------------------------------------------------------------------------

static void DrawSlider(Menu_t *menu, int item, int width, int slot)
{
	int		x;
	int		y;
	int		i;

	x = menu->x+24;
	y = menu->y+2+(item*menu->itemHeight);

	for (i = 0; i < width; i++)
	{
		GCanvas->DrawPatch(x + i * 8, y, 
			gi.W_GetNumForName(i & 1 ?  "M_SLDMD2" : "M_SLDMD1"));
	}

	GCanvas->DrawPatch(x-32, y, gi.W_GetNumForName("M_SLDLT"));
	GCanvas->DrawPatch(x + width*8, y, gi.W_GetNumForName("M_SLDRT"));
	GCanvas->DrawPatch(x+4+slot*8, y+7, gi.W_GetNumForName("M_SLDKB"));
}

//---------------------------------------------------------------------------
//
// PROC MN_DrawInfo
//
//---------------------------------------------------------------------------

void MN_DrawInfo(void)
{
	static char *InfoPages[] = { "HELP1", "HELP2", "CREDIT" };

	gi.GL_SetFilter(0);
	GCanvas->DrawRawScreen(gi.W_GetNumForName(InfoPages[InfoType - 1]));
}

//---------------------------------------------------------------------------
//
// PROC MN_DrawSpells
//
//---------------------------------------------------------------------------

void MN_DrawSpells(void)
{
	gi.GL_SetFilter(0);
	//Draw back
	GCanvas->DrawRawScreen(gi.W_GetNumForName("JOURNAL"));
}

//---------------------------------------------------------------------------
//
// PROC MN_DrawUpdating
//
//---------------------------------------------------------------------------

void InitUpdating(void)
{
	player_t player;
	player = players[consoleplayer];
	navpoints = player.av_points;
//	navpoints=5;
	upd_values[0][0] = player.maxhealth_old;
	upd_values[0][1] = player.maxhealth;
	upd_values[1][0] = player.strength;
	upd_values[1][1] = player.strength;
	upd_values[2][0] = player.agility;
	upd_values[2][1] = player.agility;
	upd_values[3][0] = player.speed;
	upd_values[3][1] = player.speed;
	currentUpdatingRow=1;
}

void AcceptUpdating(void)
{
	player_t player;
	player = players[consoleplayer];
	if (/*(upd_values[0][0]<=upd_values[0][1])&&*/
		(upd_values[1][0]<=upd_values[1][1])&&
		(upd_values[2][0]<=upd_values[2][1])&&
		(upd_values[3][0]<=upd_values[3][1])&&
		(player.av_points>=navpoints))
	{
		players[consoleplayer].av_points = navpoints;
		players[consoleplayer].maxhealth_old = players[consoleplayer].maxhealth;//upd_values[0][1];
		players[consoleplayer].sp_power_old = players[consoleplayer].maxsp_power;//upd_values[0][1];
//		players[consoleplayer].maxhealth = health_table[player.pclass]*upd_values[0][1];*/
		players[consoleplayer].strength = upd_values[1][1];
		players[consoleplayer].agility = upd_values[2][1];
		players[consoleplayer].speed = upd_values[3][1];
	}
}

void MN_DrawUpdating(void)
{
	char tmp[3][12][13];
	char leveldesc[4][40];
	int i;
	player_t &player = players[consoleplayer];

	sprintf(leveldesc[0],"YOU'RE LEVEL  %d",player.exp_level);
	sprintf(leveldesc[1],"EXPERIENCE POINTS: %d",player.experience);
	sprintf(leveldesc[2],"YOU NEED %d POINTS FOR LEVEL %d",player.next_level-player.experience,player.exp_level+1);
	sprintf(leveldesc[3],"SILVER: %d",player.money);


	sprintf(tmp[0][0],"POINTS:");
	sprintf(tmp[0][1],"");
	sprintf(tmp[0][2],"HEALTH:");
	sprintf(tmp[0][3],"%s",sp_wording[player.pclass]);
	sprintf(tmp[0][4],"STRENGTH:");
	sprintf(tmp[0][5],"EFFICIENCY:");
	sprintf(tmp[0][6],"SPEED:");
	sprintf(tmp[0][7],"");
	sprintf(tmp[0][8],"REMAINING:");

	sprintf(tmp[1][0],"OLD");
	sprintf(tmp[1][1],"");
	sprintf(tmp[1][2],"%3d",upd_values[0][0]);
	sprintf(tmp[1][3],((player.pclass!=3)?"%3d":""),player.sp_power_old);
	sprintf(tmp[1][4],"%02d",upd_values[1][0]);
	sprintf(tmp[1][5],"%02d",upd_values[2][0]);
	sprintf(tmp[1][6],"%02d",upd_values[3][0]);

	sprintf(tmp[2][0]," NEW ");
	sprintf(tmp[2][1],"");
	sprintf(tmp[2][2],"  %3d  ",upd_values[0][1]);
	sprintf(tmp[2][3],((player.pclass!=3)?"  %3d  ":""),player.maxsp_power);
	sprintf(tmp[2][4],"- %02d +",upd_values[1][1]);
	sprintf(tmp[2][5],"- %02d +",upd_values[2][1]);
	sprintf(tmp[2][6],"- %02d +",upd_values[3][1]);
	sprintf(tmp[2][7],"");
	sprintf(tmp[2][8],"%02d",navpoints);
	sprintf(tmp[2][9],"");
	sprintf(tmp[2][10],"OK");

	/*for (i=0;i<3;i++)
		MN_DrTextA(leveldesc[i],10,i*13+10);*/
	for (i=0;i<4;i++)
		MN_DrTextA(leveldesc[i],10,i*13+1);

	for (i=0;i<9;i++)
		MN_DrTextA(tmp[0][i],100,i*13+56);
	for (i=0;i<7;i++)
		MN_DrTextA(tmp[1][i],180,i*13+56);
	for (i=0;i<11;i++)
		MN_DrTextA(tmp[2][i],220,i*13+56);

	MN_DrTextAYellow(tmp[2][av_pos[currentUpdatingRow]],220,av_pos[currentUpdatingRow]*13+56);
}

//---------------------------------------------------------------------------
//
// PROC MN_DrawJournal
//
//---------------------------------------------------------------------------

static char* journal[5]=
{
	{"STATS"},
	{"BESTIARY"},
	{"THINGS"},
	{"MAP"},
	{"NOTES"}
};

static int coords[5][2]=
{
	{21,18},
	{64,20},
	{103,28},
	{140,22},
	{176,22}
};


static float height_float [4] =
{
	{6},{5.5f},{5},{2}
};


extern int AutoArmorSave[NUMCLASSES];

void MN_DrawJournal(void)
{
	int i,temp=0/*,ma,co*/;
    int time_mod=*(int*) gi.GetCVar("time_mod")->ptr;
	int months=1;
	int days=1;
	int hours=0;
	int minutes=0;
	int worldTimer;
	int tempMon;
	char tmp[12][40];
	player_t player;
	gi.GL_SetFilter(0);
	//Draw back
	GCanvas->DrawRawScreen(gi.W_GetNumForName("JOURNAL"));
	player=players[consoleplayer];
	//Draw Menu, and actual one
	for (i=0;i<5;i++)
		MN_DrTextA(journal[i],coords[i][1],coords[i][0]);
	MN_DrTextAYellow(journal[currentJournal],coords[currentJournal][1],coords[currentJournal][0]);
	/*co=consoleplayer;
	ma=players[consoleplayer].maxhealth;*/
	//itoa(players[consoleplayer].maxhealth,tmp,10);
	worldTimer = player.worldTimer;

	worldTimer /= 35;
	worldTimer = worldTimer*60/time_mod;
	months = worldTimer/43200;
	worldTimer -= months*43200;
	days = worldTimer/1440;
	worldTimer -= days*1440;
	hours = worldTimer/60;
	worldTimer -= hours*60;
	minutes = worldTimer;
	//player.money=123;
	temp = AutoArmorSave[player.pclass]
		+player.armorpoints[ARMOR_ARMOR]+player.armorpoints[ARMOR_SHIELD]
		+player.armorpoints[ARMOR_HELMET]+player.armorpoints[ARMOR_AMULET];
	
	tempMon=player.money/10;

	sprintf(tmp[0],"");
	sprintf(tmp[1],"TODAY IS %d DAY, %d:%d",days,hours,minutes);
	sprintf(tmp[2],"HEIGHT: %.1f FEET",height_float[player.pclass]);

	sprintf(tmp[3],"HEALTH POINTS: %d OF %d",player.plr->mo->health,player.maxhealth,player.health);
//	sprintf(tmp[1],"CURRENT HEALTH: %d",);
	sprintf(tmp[4],"STRENGTH: %d",player.strength);
	sprintf(tmp[5],"AGILITY: %d",player.agility);
	sprintf(tmp[6],"SPEED: %d",player.speed);
//	sprintf(tmp[7],"");//ATTACK: %d",0);
	sprintf(tmp[7],"DEFENSE: %d",FixedDiv(temp, 5*FRACUNIT)>>FRACBITS);
	sprintf(tmp[8],"YOU'RE LEVEL %d",player.exp_level);
	sprintf(tmp[9],"EXPERIENCE POINTS %d",player.experience);
	sprintf(tmp[10],"YOU NEED %d POINTS FOR LVL %d",player.next_level-player.experience,player.exp_level+1);
	sprintf(tmp[11],"MONEY: %d PH'ORIND %d FIL-AIRE",tempMon,player.money-(tempMon*10));
	//memset(statColls1[1],30,);
	for (i=0;i<12;i++)
	{
		statColls1[i]=tmp[i];
	}
	//*tmp=NULL;
	//Draw the Pages
	if (journalPages[currentJournal].pages!=NULL)
	{
		MN_DrTextAYellow(journalPages[currentJournal].pages[currentJournalPage].title,200-(MN_TextAWidth(journalPages[currentJournal].pages[currentJournalPage].title)/2),18);
		if (strcmp(journalPages[currentJournal].pages[currentJournalPage].imageName,"")!=0) GCanvas->DrawPatch(journalPages[currentJournal].pages[currentJournalPage].x,journalPages[currentJournal].pages[currentJournalPage].y,gi.W_GetNumForName(journalPages[currentJournal].pages[currentJournalPage].imageName));
		for (i=0;i<journalPages[currentJournal].pages[currentJournalPage].num_collss;i++)
			MN_DrTextA(journalPages[currentJournal].pages[currentJournalPage].colls_side[i],198,i*13+35);	
		for (i=0;i<journalPages[currentJournal].pages[currentJournalPage].num_colls;i++)
			MN_DrTextA(journalPages[currentJournal].pages[currentJournalPage].colls[i],85,i*13+journalPages[currentJournal].pages[currentJournalPage].collOffset);	
	}

	//Draw Mouse cursor
#ifdef USE_JOURNAL_MOUSE
	GCanvas->DrawPatch(mousesx,mousesy,gi.W_GetNumForName("armslot1"));
#endif
}


