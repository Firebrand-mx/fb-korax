
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

// MACROS ------------------------------------------------------------------

#define LEFT_DIR 0
#define RIGHT_DIR 1
#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SLOTTEXTLEN	16
#define ASCII_CURSOR '['

// TYPES -------------------------------------------------------------------

struct chr_val
{
	int val[7];
};

typedef enum
{
	MENU_MAIN,
	MENU_CLASS,
	MENU_SKILL,
	MENU_CHAR,
	MENU_LOAD,
	MENU_SAVE,
	MENU_OPTIONS,
	MENU_GAMEPLAY,
	MENU_GRAPHICS,
	MENU_EFFECTS,
	MENU_RESOLUTION,
	MENU_SOUND,
	MENU_CONTROLS,
	MENU_MOUSEOPTS,
	MENU_JOYCONFIG,
	MENU_INFO,

	//	Treat them as menus
	MENU_UPDATING,
	MENU_JOURNAL,
	MENU_SPELL,

	MENU_EXPMOD,

	MAX_MENU,

	MENU_NONE = MAX_MENU
} MenuType_t;

enum EMenuAction
{
	MA_None,
	MA_Menu,
	MA_MenuNC,
	MA_EndGame,
	MA_Quit,
	MA_Custom,
};

class KMenuScreen;

extern boolean shiftdown;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

boolean G_CheckDemoStatus(void);
boolean F_Responder(event_t *ev);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

chr_val Defaultroll(int Sclass);
chr_val Reroll(int Sclass);

void SetMenu(MenuType_t menu);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void InitFonts(void);
static void SCQuitGame(int option);
static boolean SCNetCheck(int option);
static void SCLoadGame(int option);
static void SCSaveGame(int option);

static void DrawFilesMenu(void);
void MN_LoadSlotText(void);

static void PopMenu(void);
static void ForceMenuOff(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean gamekeydown[256]; // The NUMKEYS macro is local to g_game

extern int AutoArmorSave[NUMCLASSES];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean	MenuActive;
boolean messageson = true;
boolean mn_SuicideConsole;
KMenuScreen	*CurrentMenu;
int		MenuTime;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int MenuPClass;
static int MenuPSkill;
static int MenuPExpMod;
chr_val MenuPValues = { 10, 80, 8, 2, 10, 10, 10 };	// -JL- Added default values
static boolean soundchanged;

#define MAX_MENU_STACK		32

static KMenuScreen *MenuStack[MAX_MENU_STACK];
static int MenuSP;

//
// [CW] 2008-11-28
enum
{
	QuitEndMsgType_None = -1,
	QuitEndMsgType_Quit,
	QuitEndMsgType_EndGame,
	QuitEndMsgType_Suicide
};

boolean askforquit;
boolean typeofask;
static boolean slottextloaded;
static char _SlotText[6][SLOTTEXTLEN+2];
static char oldSlotText[SLOTTEXTLEN+2];
static int _SlotStatus[6];
static int slotptr;
static int currentSlot;
// [CW] 2008-11-28
//static int quicksave;
//static int quickload;

static char *GammaText[] = 
{
	TXT_GAMMA_LEVEL_OFF,
	TXT_GAMMA_LEVEL_1,
	TXT_GAMMA_LEVEL_2,
	TXT_GAMMA_LEVEL_3,
	TXT_GAMMA_LEVEL_4
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

#include "mn_base.h"

#include "mn_main.h"
#include "mn_class.h"
#include "mn_skill.h"
#include "mn_char.h"
#include "mn_load.h"
#include "mn_save.h"
#include "mn_options.h"
#include "mn_gameplay.h"
#include "mn_gfx.h"
#include "mn_effects.h"
#include "mn_resolution.h"
#include "mn_sound.h"
#include "mn_controls.h"
#include "mn_mouse.h"
#include "mn_joy.h"
#include "mn_info.h"
#include "mn_updating.h"
#include "mn_journal.h"
#include "mn_spell.h"
#include "mn_expmod.h"

//==========================================================================
//
//
//
//==========================================================================

static KClass *MenuClasses[MAX_MENU] = {
	KMenuScreenMain::StaticClass(),
	KMenuScreenClass::StaticClass(),
	KMenuScreenSkill::StaticClass(),
	KMenuScreenChar::StaticClass(),
	KMenuScreenLoadGame::StaticClass(),
	KMenuScreenSaveGame::StaticClass(),
	KMenuScreenOptions::StaticClass(),
	KMenuScreenGameplay::StaticClass(),
	KMenuScreenGraphics::StaticClass(),
	KMenuScreenEffects::StaticClass(),
	KMenuScreenResolution::StaticClass(),
	KMenuScreenSound::StaticClass(),
	KMenuScreenControls::StaticClass(),
	KMenuScreenMouseOptions::StaticClass(),
	KMenuScreenJoyConfig::StaticClass(),
	KMenuScreenInfo::StaticClass(),
	KUpdatingScreen::StaticClass(),
	KJournalScreen::StaticClass(),
	KSpellScreen::StaticClass(),
	KMenuScreenExpMod::StaticClass(),
};

//---------------------------------------------------------------------------
//
// PROC MN_Init
//
//---------------------------------------------------------------------------

void MN_Init(void)
{
	guard(MN_Init);
	KCanvas::StaticInit();
	MenuActive = false;

	GRootWindow = Spawn<KRootWindow>();
	GRootWindow->Init();
	unguard;
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
	GL_SetColorAndAlpha(1, 1, 1, 1);
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
	GL_SetColorAndAlpha(1, 1, 1, 1);
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
	GL_SetColorAndAlpha(1, 1, 1, 1);
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
	if (MenuActive)
	{
		MenuTime++;
	}
}

//==========================================================================
//
// DrawMessage
//
//==========================================================================

static void DrawMessage(void)
{
	guard(DrawMessage);
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
	unguard;
}

char *QuitEndMsg[] =
{
	"ARE YOU SURE YOU WANT TO QUIT?",
	"ARE YOU SURE YOU WANT TO END THE GAME?",
// [CW] 2008-11-28
	//"DO YOU WANT TO QUICKSAVE THE GAME NAMED",
	//"DO YOU WANT TO QUICKLOAD THE GAME NAMED",
	"ARE YOU SURE YOU WANT TO SUICIDE?"
};

//---------------------------------------------------------------------------
//
// PROC MN_Drawer
//
//---------------------------------------------------------------------------

void MN_Drawer(void)
{
	guard(MN_Drawer);
	GCanvas->SetOrigin(160, 0);
	DrawMessage();
	GCanvas->SetOrigin(0, 0);

	// FPS.
	if (showFPS)
	{
		char fpsbuff[80];
		sprintf(fpsbuff, "%d FPS", I_GetFrameRate());
		MN_DrTextA(fpsbuff, 640-MN_TextAWidth(fpsbuff), 0);
		DD_GameUpdate(DDUF_TOP);
	}
	
	if (MenuActive == false)
	{
		if (askforquit)  //Draw questioning
		{
			GCanvas->SetOrigin(160, 120);
			//
			// [CW] 2008-11-28: typeofask usage changed
			MN_DrTextA(QuitEndMsg[typeofask/*-1*/], 160-
				MN_TextAWidth(QuitEndMsg[typeofask/*-1*/])/2, 80);
			//
			// [CW] 2008-11-28: Quicksave and quickload questions disabled
			/*if (typeofask == 3)
			{
				MN_DrTextA(_SlotText[quicksave-1], 160-
					MN_TextAWidth(_SlotText[quicksave-1])/2, 90);
				MN_DrTextA("?", 160+
					MN_TextAWidth(_SlotText[quicksave-1])/2, 90);
			}
			if (typeofask == 4)
			{
				MN_DrTextA(_SlotText[quickload-1], 160-
					MN_TextAWidth(_SlotText[quickload-1])/2, 90);
				MN_DrTextA("?", 160+
					MN_TextAWidth(_SlotText[quicksave-1])/2, 90);
			}*/
			DD_GameUpdate(DDUF_FULLSCREEN);
			GCanvas->SetOrigin(0, 0);
		}
	}
	if (MenuActive)
	{
		DD_GameUpdate(DDUF_FULLSCREEN | DDUF_BORDER);

		// Draw a dark background. It makes it easier to read the menus.
		GL_SetNoTexture();
		GCanvas->DrawRect(0, 0, 640, 480, 0, 0, 0, 0.5);
		gl.Color4f(1, 1, 1, 1);
	}
	GRootWindow->PaintWindows(GCanvas);
	unguard;
}

//---------------------------------------------------------------------------
//
// PROC DrawFilesMenu
//
//---------------------------------------------------------------------------

static void DrawFilesMenu(void)
{
// clear out the quicksave/quickload stuff
	// [CW] 2008-11-28
	//quicksave = 0;
	//quickload = 0;
	P_ClearMessage(&players[consoleplayer]);
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
		if (found)
		{
			memcpy(_SlotText[slot], description, SLOTTEXTLEN);
			_SlotStatus[slot] = 1;
		}
		else
		{
			memset(_SlotText[slot], 0, SLOTTEXTLEN);
			_SlotStatus[slot] = 0;
		}
	}
	slottextloaded = true;
}

//---------------------------------------------------------------------------
//
// PROC SCQuitGame
//
//---------------------------------------------------------------------------

static void SCQuitGame(int option)
{
	CON_Open(false);
	MenuActive = false;
	ForceMenuOff();
	askforquit = true;
	// [CW] 2008-11-28
	typeofask = QuitEndMsgType_Quit;//1; //quit game
	if(!netgame && !demoplayback)
	{
		paused = true;
	}
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

//---------------------------------------------------------------------------
//
// PROC SCLoadGame
//
//---------------------------------------------------------------------------

static void SCLoadGame(int option)
{
	// [CW] 2008-11-28
	if (option != QUICK_SLOT &&
		!_SlotStatus[option])
	{ // Don't try to load from an empty slot
		return;
	}

	G_LoadGame(option);
	MN_DeactivateMenu();
	//BorderNeedRefresh = true;
	DD_GameUpdate(DDUF_BORDER);
	// [CW] 2008-11-28 CHECKME
	/*if(quickload == -1)
	{
		quickload = option+1;
		P_ClearMessage(&players[consoleplayer]);
	}*/
}

//---------------------------------------------------------------------------
//
// PROC SCSaveGame
//
//---------------------------------------------------------------------------

static void SCSaveGame(int option)
{
	// [CW] 2008-11-28:
	if (option == QUICK_SLOT)
		G_SaveGame(QUICK_SLOT, QUICK_DESCRIPTION);
	else
		G_SaveGame(option, _SlotText[option]);
	MN_DeactivateMenu();
	//BorderNeedRefresh = true;
	DD_GameUpdate(DDUF_BORDER);
	// [CW] 2008-11-28 CHECKME!!
	/*if (quicksave == -1)
	{
		quicksave = option+1;
		P_ClearMessage(&players[consoleplayer]);
	}*/
}

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

fixed_t player_vitality[NUMCLASSES]=
{
	{0x08},
	{0x05},
	{0x02},
	{0x06},
};

fixed_t player_intelligence[NUMCLASSES]=
{
	{0x02},
	{0x05},
	{0x08},
	{0x06},
};

fixed_t player_wisdom[NUMCLASSES]=
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
	values.val[3]=player_vitality[Sclass];
	values.val[4]=init_speed[Sclass];
	values.val[5]=player_intelligence[Sclass];
	values.val[6]=player_wisdom[Sclass];
	return values;
}


chr_val Reroll(int Sclass)
{
	chr_val values;
	switch(Sclass)
	{
	case PCLASS_FIGHTER:
		values.val[2]=6+(P_Random()&3);  //strength
		values.val[4]=6+(P_Random()&6);  //agility
		values.val[3]=1+(P_Random()&2);  //vitality
		values.val[5]=1+(P_Random()&2);  //inteligence
		values.val[6]=1+(P_Random()&2);  //wisdom
		values.val[0]=100-values.val[2]-values.val[3]-values.val[4];  //health
		values.val[1]=3+(P_Random()&2);  //spiritual power
		break;
	case PCLASS_CLERIC:
		values.val[2]=2+(P_Random()&4);  //strength
		values.val[4]=2+(P_Random()&6);  //agility
		values.val[3]=2+(P_Random()&4);  //vitality
		values.val[5]=2+(P_Random()&4);  //intelligence
		values.val[6]=2+(P_Random()&4);  //wisdom
		values.val[0]=66-values.val[2]-values.val[3]-values.val[4];  //health
		values.val[1]=80-values.val[0];               //spiritual power
		break;
	case PCLASS_MAGE:
		values.val[2]=1+(P_Random()&3);  //strength
		values.val[4]=1+(P_Random()&5);  //agility
		values.val[3]=6+(P_Random()&6);  //vitality
		values.val[5]=6+(P_Random()&6);  //intelligence
		values.val[6]=6+(P_Random()&6);  //wisdom
		values.val[0]=40-values.val[2]-values.val[3]-values.val[4];  //health
		values.val[1]=80-values.val[0];               //spiritual power
		break;
	case PCLASS_CORVUS:
		values.val[2]=2+(P_Random()&4);  //strength
		values.val[4]=2+(P_Random()&6);  //speed
		values.val[3]=2+(P_Random()&4);  //efficiency
		values.val[5]=6+(P_Random()&6);  //vitality
		values.val[6]=6+(P_Random()&6);  //vitality
		values.val[0]=66-values.val[2]-values.val[3]-values.val[4];  //health
		values.val[1]=80-values.val[0];               //spiritual power
		break;
	default:
		break;
	}
	return values;
}

//---------------------------------------------------------------------------
//
// FUNC H2_PrivilegedResponder
//
//---------------------------------------------------------------------------

int H2_PrivilegedResponder(event_t *event)
{
	guard(H2_PrivilegedResponder);
	for (KWindow *W = GRootWindow->FocusWindow; W; W = W->GetParent())
	{
		if (W->RawInputEvent(event))
		{
			return true;
		}
	}

	// Process the screen shot key right away.
	if(ravpic && event->data1 == DDKEY_F1)
	{
		if(event->type == ev_keydown) G_ScreenShot();
		// All F1 events are eaten.
		return true;
	}
	return false;
	unguard;
}

//---------------------------------------------------------------------------
//
// FUNC MN_Responder
//
//---------------------------------------------------------------------------

boolean MN_Responder(event_t *event)
{
	guard(MN_Responder);
	int key;
	extern boolean automapactive;
	extern void H2_StartTitle(void);

	if(event->data1 == DDKEY_RSHIFT)
	{
		shiftdown = (event->type == ev_keydown);
	}

	if(event->type != ev_keydown && event->type != ev_keyrepeat)
	{
		return(false);
	}
	key = event->data1;

	for (KWindow *W = GRootWindow->FocusWindow; W; W = W->GetParent())
	{
		if (W->KeyPressed(key))
		{
			return true;
		}
	}

	if(askforquit)
	{
		switch(key)
		{
			case 'y':
				if(askforquit)
				{
					//
					// [CW] 2008-11-28: typeofask constants
					switch(typeofask) //Switch questioning
					{
						case QuitEndMsgType_Quit/*1*/:
							G_CheckDemoStatus(); 
							I_Quit();
							break;
						case QuitEndMsgType_EndGame/*2*/:
							P_ClearMessage(&players[consoleplayer]);
							typeofask = QuitEndMsgType_None/*0*/;
							askforquit = false;
							paused = false;
							GL_SetFilter(0);
							H2_StartTitle(); // go to intro/demo mode.
							break;
						// [CW] 2008-11-28: quicksaving amd quickloading are now instant
						/*case 3:
							P_SetMessage(&players[consoleplayer], 
								"QUICKSAVING....", false);
							SCSaveGame(quicksave-1);
							askforquit = false;
							typeofask = 0;
							//BorderNeedRefresh = true;
							DD_GameUpdate(DDUF_BORDER);
							return true;
						case 4:
							P_SetMessage(&players[consoleplayer], 
								"QUICKLOADING....", false);
							SCLoadGame(quickload-1);
							askforquit = false;
							typeofask = 0;
							//BorderNeedRefresh = true;
							DD_GameUpdate(DDUF_BORDER);
							return true;*/
						case QuitEndMsgType_Suicide/*5*/:
							askforquit = false;
							typeofask = QuitEndMsgType_None/*0*/;
							//BorderNeedRefresh = true;
							DD_GameUpdate(DDUF_BORDER);
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
					typeofask = QuitEndMsgType_None/*0*/;
					paused = false;
					/*UpdateState |= I_FULLSCRN;
					BorderNeedRefresh = true;*/
					DD_GameUpdate(DDUF_FULLSCREEN | DDUF_BORDER);
					return true;
				}
				return false;
		}
		return false; // don't let the keys filter thru
	}
	if (!MenuActive && !chatmodeon)
	{
		switch(key)
		{
/*			case DDKEY_TAB:
				SetMenu(MENU_JOURNAL);
				MenuActive = true;
				if(!netgame && !demoplayback && !demorecording)
				{
					paused = true;
				}
				return true;  //kmod*/
			case 'u':
				SetMenu(MENU_UPDATING);
				MenuActive = true;
				if(!netgame && !demoplayback && !demorecording)
				{
					paused = true;
				}
				return true;
			case 'z':
				SetMenu(MENU_SPELL);
				MenuActive = true;
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
	return false;
	unguard;
}

//---------------------------------------------------------------------------
//
//	PushMenu
//
//---------------------------------------------------------------------------

static void PushMenu(MenuType_t menu)
{
	if (MenuSP)
	{
		CurrentMenu->Hide();
	}
	CurrentMenu = (KMenuScreen *)KWindow::StaticCreateWindow(MenuClasses[menu], GRootWindow);
	MenuStack[MenuSP++] = CurrentMenu;
}

//---------------------------------------------------------------------------
//
//	PopMenu
//
//---------------------------------------------------------------------------

static void PopMenu(void)
{
	MenuSP--;

	if (MenuSP < 0)
		I_Error("PopMenu: Stack underflow");

	MenuStack[MenuSP]->Destroy();
	if (MenuSP)
	{
		CurrentMenu = MenuStack[MenuSP - 1];
		CurrentMenu->Show();
		S_StartSound(NULL, SFX_PICKUP_KEY);
	}
	else
	{
		CurrentMenu = NULL;
		MenuActive = false;
		if (!netgame)
		{
			paused = false;
		}
		S_StartSound(NULL, SFX_PLATFORM_STOP);
		P_ClearMessage(&players[consoleplayer]);
	}
}

//---------------------------------------------------------------------------
//
//	ForceMenuOff
//
//---------------------------------------------------------------------------

static void ForceMenuOff(void)
{
	int i;

	for (i = 0; i < MenuSP; i++)
	{
		MenuStack[i]->Destroy();
	}
	MenuSP = 0;
	CurrentMenu = NULL;
	MenuActive = false;
	if (!netgame)
	{
		paused = false;
	}
}

//---------------------------------------------------------------------------
//
//	StartMenu
//
//---------------------------------------------------------------------------

static void StartMenu(MenuType_t menu)
{
	MenuActive = true;
	MenuTime = 0;
	PushMenu(menu);
	if (!netgame && !demoplayback)
	{
		paused = true;
	}
	//S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
	S_StartSound(NULL, SFX_PLATFORM_STOP);
	slottextloaded = false; //reload the slot text, when needed
}

//---------------------------------------------------------------------------
//
// PROC SetMenu
//
//---------------------------------------------------------------------------

void SetMenu(MenuType_t menu)
{
	PushMenu(menu);
}

//---------------------------------------------------------------------------
//
// PROC MN_ActivateMenu
//
//---------------------------------------------------------------------------

void MN_ActivateMenu(void)
{
	guard(MN_ActivateMenu);
	if (MenuActive)
	{
		return;
	}
	if (paused)
	{
		S_ResumeSound();
	}
	StartMenu(MENU_MAIN);
	unguard;
}

//---------------------------------------------------------------------------
//
// PROC MN_DeactivateMenu
//
//---------------------------------------------------------------------------

void MN_DeactivateMenu(void)
{
	guard(MN_DeactivateMenu);
	if(!CurrentMenu) return;

	ForceMenuOff();
	S_StartSound(NULL, SFX_PLATFORM_STOP);
	P_ClearMessage(&players[consoleplayer]);
	unguard;
}

//---------------------------------------------------------------------------
//
//	CCmdMenuAction
//
//---------------------------------------------------------------------------

int CCmdMenuAction(int argc, char **argv)
{
	guard(CCmdMenuAction);
	// Can we get out of here early?
	if(/*MenuActive == true || */chatmodeon) return true;

	if(!stricmp(argv[0], "infoscreen"))
	{
		// start up info screens
		StartMenu(MENU_INFO);
	}
	else if(!stricmp(argv[0], "savegame"))
	{
		if(gamestate == GS_LEVEL && !demoplayback)
		{
			StartMenu(MENU_SAVE);
		}
	}
	else if(!stricmp(argv[0], "loadgame"))
	{
		if(SCNetCheck(2))
		{
			StartMenu(MENU_LOAD);
		}
	}
	else if(!stricmp(argv[0], "soundmenu"))
	{
		StartMenu(MENU_SOUND);
	}
	else if(!stricmp(argv[0], "suicide"))
	{
		CON_Open(false);
		MenuActive = false;
		askforquit = true;
		// [CW] 2008-11-28
		typeofask = QuitEndMsgType_Suicide/*5*/; // suicide
		return true;
	}
	else if(!stricmp(argv[0], "quicksave"))
	{
		if(gamestate == GS_LEVEL && !demoplayback)
		{
			//
			// [CW] 2008-11-28 : instant Quicksaving
			//

			P_SetMessage(&players[consoleplayer], 
								"QUICKSAVING....", false);
			SCSaveGame(/*quicksave-1*/QUICK_SLOT);
			askforquit = false;
			//typeofask = QuitEndMsgType_None;
			//BorderNeedRefresh = true;
			DD_GameUpdate(DDUF_BORDER);

			/*if(!quicksave || quicksave == -1)
			{
				StartMenu(MENU_SAVE);
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
			}*/
		}
	}
	else if(!stricmp(argv[0], "quickload"))
	{
		if(SCNetCheck(2))
		{
			//
			// [CW] 2008-11-28 : instant Quickloading
			//

			P_SetMessage(&players[consoleplayer], 
				"QUICKLOADING....", false);
			SCLoadGame(/*quickload-1*/QUICK_SLOT);
			askforquit = false;
			//typeofask = 0;
			//BorderNeedRefresh = true;
			DD_GameUpdate(DDUF_BORDER);

			/*if(!quickload || quickload == -1)
			{
				StartMenu(MENU_LOAD);
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
			}*/
		}
	}
	else if(!stricmp(argv[0], "quit"))
	{
		SCQuitGame(0);
		S_StartSound(NULL, SFX_CHAT);
	}
	else if(!stricmp(argv[0], "toggleGamma"))
	{
		int gamma = usegamma + 1;
		char cmd[20];
		if(gamma > 4)
		{
			gamma = 0;
		}
		sprintf(cmd, "setgamma %d", gamma);
		CON_Execute(cmd, true);
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
	unguard;
}

//==========================================================================
//
//
//
//==========================================================================

