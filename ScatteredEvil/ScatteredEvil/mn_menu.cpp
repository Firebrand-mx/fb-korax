
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

class KMenuUIButton;

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

class KMenuItem_t:public KMenuWindow
{
	DECLARE_CLASS(KMenuItem_t, KMenuWindow, 0);

	KMenuItem_t()
	{
		Height = ITEM_HEIGHT;
	}
};

class KMenuScreen:public KModalWindow
{
	DECLARE_CLASS(KMenuScreen, KModalWindow, 0);

	struct FButtonDefault
	{
		char *Name;
		int Action;
		MenuType_t Invoke;
		int Key;

		FButtonDefault(void) { }
		FButtonDefault(char *InName, int InAction,
			MenuType_t InInvoke = MENU_NONE, int InKey = 0)
			: Name(InName), Action(InAction), Invoke(InInvoke), Key(InKey)
		{ }
	};

	enum { MAX_BUTTONS = 10 };

	KMenuItem_t *Items[128];
	int NumItems;
	int CursorPos;
	//int CursorPrev;

	int ChoiceStartX;
	int ChoiceStartY;
	int	itemHeight;

	bool bUseSelector;

	FButtonDefault ButtonDefaults[MAX_BUTTONS];
	KMenuUIButton *WinButtons[MAX_BUTTONS];

	//	Constructor
	KMenuScreen(void)
	{
		Width = 640;
		Height = 480;
		itemHeight = ITEM_HEIGHT;
		Font = KCanvas::BigFont;
		bUseSelector = true;
	}

	void InitWindow(void);
	virtual void CreateChoices(void);
	virtual void CreateButtons(void);
	virtual void PostDrawWindow(KGC *gc);
	virtual bool RawInputEvent(event_t *event);
	virtual bool KeyPressed(int key);
	void CyclePrevChoice(void);
	void CycleNextChoice(void);
	bool ButtonActivated(KWindow *button);
	void ProcessMenuAction(int Action, MenuType_t Invoke, int Key);
	virtual void ProcessCustomMenuAction(int Key) { }
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

IMPLEMENT_CLASS(KMenuWindow);
IMPLEMENT_CLASS(KMenuItem_t);
IMPLEMENT_CLASS(KMenuScreen);

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

#define MAX_MENU_STACK		32

static KMenuScreen *MenuStack[MAX_MENU_STACK];
static int MenuSP;

boolean askforquit;
boolean typeofask;
static boolean slottextloaded;
static char _SlotText[6][SLOTTEXTLEN+2];
static char oldSlotText[SLOTTEXTLEN+2];
static int _SlotStatus[6];
static int slotptr;
static int currentSlot;
static int quicksave;
static int quickload;

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

//---------------------------------------------------------------------------
//
// PROC MN_Init
//
//---------------------------------------------------------------------------

void MN_Init(void)
{
	KCanvas::StaticInit();
	MenuActive = false;
	MauloBaseLump = gi.W_GetNumForName("FBULA0"); // ("M_SKL00");

	GRootWindow = Spawn<KRootWindow>();
	GRootWindow->Init();
}

//---------------------------------------------------------------------------
//
//	KMenuScreen::InitWindow
//
//---------------------------------------------------------------------------

void KMenuScreen::InitWindow(void)
{
	Super::InitWindow();
	CreateChoices();
	CreateButtons();
	if (NumItems)
	{
		GRootWindow->SetFocus(Items[CursorPos]);
	}
	else
	{
		GRootWindow->SetFocus(this);
	}
}

//---------------------------------------------------------------------------
//
//	KMenuScreen::CreateChoices
//
//---------------------------------------------------------------------------

void KMenuScreen::CreateChoices(void)
{
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
		if (askforquit)  //Draw questioning
		{
			GCanvas->SetOrigin(160, 120);
			MN_DrTextA(QuitEndMsg[typeofask-1], 160-
				MN_TextAWidth(QuitEndMsg[typeofask-1])/2, 80);
			if (typeofask == 3)
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
			}
			gi.Update(DDUF_FULLSCREEN);
			GCanvas->SetOrigin(0, 0);
		}
	}
	if (MenuActive)
	{
		gi.Update(DDUF_FULLSCREEN | DDUF_BORDER);

		// Draw a dark background. It makes it easier to read the menus.
		gi.GL_SetNoTexture();
		GCanvas->DrawRect(0, 0, 640, 480, 0, 0, 0, 0.5);
		gl.Color4f(1, 1, 1, 1);

		GRootWindow->PaintWindows(GCanvas);
	}
}

//---------------------------------------------------------------------------
//
//	KMenuScreen::PostDrawWindow
//
//---------------------------------------------------------------------------

void KMenuScreen::PostDrawWindow(KGC *gc)
{
	if (bUseSelector && Items[CursorPos])
	{
		gc->DrawIcon(Items[CursorPos]->X + SELECTOR_XOFFSET,
			Items[CursorPos]->Y + SELECTOR_YOFFSET - (10 - itemHeight / 2),
			FindTexture(MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2"));
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
	gi.OpenConsole(false);
	MenuActive = false;
	askforquit = true;
	typeofask = 1; //quit game
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
	if (!_SlotStatus[option])
	{ // Don't try to load from an empty slot
		return;
	}

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
	G_SaveGame(option, _SlotText[option]);
	MN_DeactivateMenu();
	//BorderNeedRefresh = true;
	gi.Update(DDUF_BORDER);
	if (quicksave == -1)
	{
		quicksave = option+1;
		P_ClearMessage(&players[consoleplayer]);
	}
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

//---------------------------------------------------------------------------
//
// FUNC H2_PrivilegedResponder
//
//---------------------------------------------------------------------------

int H2_PrivilegedResponder(event_t *event)
{
	if (MenuActive && CurrentMenu->RawInputEvent(event))
	{
		return true;
	}

	// Process the screen shot key right away.
	if(ravpic && event->data1 == DDKEY_F1)
	{
		if(event->type == ev_keydown) G_ScreenShot();
		// All F1 events are eaten.
		return true;
	}
	return false;
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
	if (!MenuActive && !chatmodeon)
	{
		switch(key)
		{
			case DDKEY_TAB:
				SetMenu(MENU_JOURNAL);
				MenuActive = true;
				if(!netgame && !demoplayback && !demorecording)
				{
					paused = true;
				}
				return true;  //kmod
			case 'u':
				SetMenu(MENU_UPDATING);
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
}

//---------------------------------------------------------------------------
//
//	KMenuScreen::RawInputEvent
//
//---------------------------------------------------------------------------

bool KMenuScreen::RawInputEvent(event_t *event)
{
	if (Items[CursorPos] && Items[CursorPos]->RawInputEvent(event))
	{
		return true;
	}
	return Super::RawInputEvent(event);
}

void KMenuScreen::CyclePrevChoice(void)
{
	do
	{
		if (CursorPos <= 0)
		{
			CursorPos = NumItems - 1;
		}
		else
		{
			CursorPos--;
		}
	} while (!Items[CursorPos]->IsSensitive());
	GRootWindow->SetFocus(Items[CursorPos]);
}

void KMenuScreen::CycleNextChoice(void)
{
	do
	{
		if (CursorPos + 1 > NumItems - 1)
		{
			CursorPos = 0;
		}
		else
		{
			CursorPos++;
		}
	} while (!Items[CursorPos]->IsSensitive());
	GRootWindow->SetFocus(Items[CursorPos]);
}

//---------------------------------------------------------------------------
//
//	KMenuScreen::KeyPressed
//
//---------------------------------------------------------------------------

bool KMenuScreen::KeyPressed(int key)
{
	switch (key)
	{
	case DDKEY_DOWNARROW:
		CycleNextChoice();
		S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
		return true;

	case DDKEY_UPARROW:
		CyclePrevChoice();
		S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
		return true;

	case DDKEY_ESCAPE:
		PopMenu();
		return true;
	}
	return Super::KeyPressed(key);
}

//==========================================================================
//
//	KMenuUIChoice
//
//==========================================================================

class KMenuUIChoice:public KMenuItem_t
{
	DECLARE_CLASS(KMenuUIChoice, KMenuItem_t, 0);
	
	char *ActionText;

	KMenuUIChoice(void)
	{
		Font = KCanvas::SmallFont;
		ActionText = "Choice";
	}

	void InitWindow(void)
	{
		Super::InitWindow();

		LoadSetting();
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawText(0, 0, ActionText);
	}

	virtual void LoadSetting(void)
	{
	}

	virtual void SaveSetting(void)
	{
	}
};
IMPLEMENT_CLASS(KMenuUIChoice);

//==========================================================================
//
//	KMenuUIChoiceSlider
//
//==========================================================================

class KMenuUIChoiceSlider:public KMenuUIChoice
{
	DECLARE_CLASS(KMenuUIChoiceSlider, KMenuUIChoice, 0);

	int TickPosition;
	int NumTicks;
	float StartValue;
	float EndValue;

	int *pInteger;
	float *pFloat;
	char *CVarName;
	cvar_t *pCVar;

	KTexture *TexMid1;
	KTexture *TexMid2;
	KTexture *TexLeft;
	KTexture *TexRight;
	KTexture *TexKnob;

	KMenuUIChoiceSlider(void)
	{
		ActionText = "Slider choice";
		NumTicks = 11;
		EndValue = 10.0;
	}

	void InitWindow(void)
	{
		if (CVarName)
		{
			pCVar = gi.GetCVar(CVarName);
			if (pCVar->type == CVT_INT)
			{
				pInteger = (int *)pCVar->ptr;
			}
			else if (pCVar->type == CVT_FLOAT)
			{
				pFloat = (float *)pCVar->ptr;
			}
			StartValue = pCVar->min;
			EndValue = pCVar->max;
		}

		Super::InitWindow();

		TexMid1 = FindTexture("M_SLDMD1");
		TexMid2 = FindTexture("M_SLDMD2");
		TexLeft = FindTexture("M_SLDLT");
		TexRight = FindTexture("M_SLDRT");
		TexKnob = FindTexture("M_SLDKB");
	}

	void CycleNextValue(void)
	{
		int NewValue;

		NewValue = GetTickPosition();
		if (NewValue < NumTicks - 1)
		{
			NewValue++;
		}
		SetTickPosition(NewValue);
	}

	void CyclePreviousValue(void)
	{
		int NewValue;

		NewValue = GetTickPosition();
		if (NewValue > 0)
		{
			NewValue--;
		}
		SetTickPosition(NewValue);
	}

	int GetTickPosition(void)
	{
		return TickPosition;
	}

	void SetTickPosition(int NewPosition)
	{
		TickPosition = NewPosition;
	}

	void SetValue(float NewValue)
	{
		SetTickPosition(int((NewValue - StartValue) * (NumTicks - 1) / 
			(EndValue - StartValue)));
	}

	float GetValue(void)
	{
		return StartValue + GetTickPosition() * (EndValue - StartValue) / 
			(NumTicks - 1);
	}

	void LoadSetting(void)
	{
		if (pInteger)
		{
			SetValue(float(*pInteger));
		}
		else if (pFloat)
		{
			SetValue(*pFloat);
		}
	}

	void SaveSetting(void)
	{
		if (pInteger)
		{
			*pInteger = int(GetValue());
		}
		else if (pFloat)
		{
			*pFloat = GetValue();
		}
	}

	void DrawWindow(KGC *gc)
	{
		int x;
		int y;
		int i;

		Super::DrawWindow(gc);

		x = 24 + 160;
		y = 0;
		for (i = 0; i < 12; i++)
		{
			gc->DrawIcon(x + i * 8, y, i & 1 ?  TexMid2 : TexMid1);
		}
		gc->DrawIcon(x - 32, y, TexLeft);
		gc->DrawIcon(x + 96, y, TexRight);
		gc->DrawIcon(x - 2 + TickPosition * 100 / (NumTicks - 1), y + 7, TexKnob);
	}

	bool KeyPressed(int key)
	{
		switch (key)
		{
		case DDKEY_LEFTARROW:
			CyclePreviousValue();
			SaveSetting();
			S_StartSound(NULL, SFX_PICKUP_KEY);
			return true;

		case DDKEY_RIGHTARROW:
		case DDKEY_ENTER:
			CycleNextValue();
			SaveSetting();
			S_StartSound(NULL, SFX_PICKUP_KEY);
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KMenuUIChoiceSlider);

//==========================================================================
//
//	KMenuUIChoiceEnum
//
//==========================================================================

class KMenuUIChoiceEnum:public KMenuUIChoice
{
	DECLARE_CLASS(KMenuUIChoiceEnum, KMenuUIChoice, 0);

	enum { MAX_ENUM_COUNT = 40 };

	const char *EnumText[MAX_ENUM_COUNT];

	int CurrentValue;

	int *pInteger;
	char *CVarName;

	KMenuUIChoiceEnum(void)
	{
	}

	void InitWindow(void)
	{
		if (CVarName)
		{
			cvar_t *pCVar;
			pCVar = gi.GetCVar(CVarName);
			pInteger = (int *)pCVar->ptr;
		}
		Super::InitWindow();
	}

	void CycleNextValue(void)
	{
		int NewValue;

		// Cycle to the next value, but make sure we don't exceed the 
		// bounds of the enumText array.  If we do, start back at the 
		// bottom.

		NewValue = GetValue() + 1;

		if (NewValue == MAX_ENUM_COUNT)
			NewValue = 0;
		else if (!EnumText[NewValue])
			NewValue = 0;

		SetValue(NewValue);
	}

	void CyclePreviousValue(void)
	{
		int NewValue;

		NewValue = GetValue() - 1;

		if (NewValue < 0)
		{
			NewValue = MAX_ENUM_COUNT - 1;

			while (!EnumText[NewValue] && NewValue > 0)
				NewValue--;	
		}

		SetValue(NewValue);
	}

	void SetValue(int NewValue)
	{
		CurrentValue = NewValue;
	}

	int GetValue(void)
	{
		return CurrentValue;
	}

	void LoadSetting(void)
	{
		if (pInteger)
		{
			SetValue(*pInteger);
		}
	}

	void SaveSetting(void)
	{
		if (pInteger)
		{
			*pInteger = GetValue();
		}
	}

	void DrawWindow(KGC *gc)
	{
		Super::DrawWindow(gc);

		gc->DrawText(160, 0, EnumText[CurrentValue]);
	}

	bool KeyPressed(int key)
	{
		switch (key)
		{
		case DDKEY_LEFTARROW:
			CyclePreviousValue();
			SaveSetting();
			S_StartSound(NULL, SFX_PICKUP_KEY);
			return true;

		case DDKEY_RIGHTARROW:
		case DDKEY_ENTER:
			CycleNextValue();
			SaveSetting();
			S_StartSound(NULL, SFX_PICKUP_KEY);
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KMenuUIChoiceEnum);

//==========================================================================
//
//	KMenuChoice_OnOff
//
//==========================================================================

class KMenuChoice_OnOff:public KMenuUIChoiceEnum
{
	DECLARE_CLASS(KMenuChoice_OnOff, KMenuUIChoiceEnum, 0);

	KMenuChoice_OnOff(void)
	{
		EnumText[0] = "OFF";
		EnumText[1] = "ON";
	}
};
IMPLEMENT_CLASS(KMenuChoice_OnOff);

//==========================================================================
//
//	KMenuUIFileSlot
//
//==========================================================================

class KMenuUIFileSlot:public KMenuItem_t
{
	DECLARE_CLASS(KMenuUIFileSlot, KMenuItem_t, 0);

	int SlotIndex;

	KMenuUIFileSlot(void)
	{
		Font = KCanvas::SmallFont;
	}

	void PrepareSlot(int InIndex)
	{
		SlotIndex = InIndex;

		if (!slottextloaded)
		{
			MN_LoadSlotText();
		}
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawIcon(0, 0, FindTexture("M_FSLOT"));
		if (_SlotStatus[SlotIndex])
		{
			gc->DrawText(5, 5, _SlotText[SlotIndex]);
		}
	}
};
IMPLEMENT_CLASS(KMenuUIFileSlot);

//==========================================================================
//
//	KMenuUIButton
//
//==========================================================================

class KMenuUIButton:public KMenuItem_t
{
	DECLARE_CLASS(KMenuUIButton, KMenuItem_t, 0);

	char *ButtonText;

	KMenuUIButton(void)
	{
		ButtonText = "Button";
		Font = KCanvas::BigFont;
	}

	void SetButtonText(char *NewText)
	{
		ButtonText = NewText;
	}

	void DrawWindow(KGC *gc)
	{
		gc->DrawText(0, 0, ButtonText);
	}

	bool KeyPressed(int key)
	{
		switch (key)
		{
		case DDKEY_ENTER:
			GetParent()->ButtonActivated(this);
			S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
			return true;
		}
		return Super::KeyPressed(key);
	}
};
IMPLEMENT_CLASS(KMenuUIButton);

//==========================================================================
//
//	KMenuScreen::CreateButtons
//
//==========================================================================

void KMenuScreen::CreateButtons(void)
{
	int ButtonIndex;

	for (ButtonIndex = 0; ButtonIndex < MAX_BUTTONS; ButtonIndex++)
	{
		if (ButtonDefaults[ButtonIndex].Name)
		{
			WinButtons[ButtonIndex] = NewWindow(KMenuUIButton, this);

			WinButtons[ButtonIndex]->SetButtonText(ButtonDefaults[ButtonIndex].Name);
			WinButtons[ButtonIndex]->SetPos(ChoiceStartX, ChoiceStartY + NumItems * itemHeight);
			Items[NumItems++] = WinButtons[ButtonIndex];
		}
		else
		{
			break;
		}
	}
}

//==========================================================================
//
//	KMenuScreen::ButtonActivated
//
//==========================================================================

bool KMenuScreen::ButtonActivated(KWindow *ButtonPressed)
{
	bool bHandled;
	int ButtonIndex;

	bHandled = false;

	Super::ButtonActivated(ButtonPressed);

	// Figure out which button was pressed
	for (ButtonIndex = 0; ButtonIndex < MAX_BUTTONS; ButtonIndex++)
	{
		if (ButtonPressed == WinButtons[ButtonIndex])
		{
			// Check to see if there's somewhere to go
			ProcessMenuAction(ButtonDefaults[ButtonIndex].Action, 
				ButtonDefaults[ButtonIndex].Invoke, 
				ButtonDefaults[ButtonIndex].Key);

			bHandled = true;
			break;
		}
	}

	return bHandled;
}

//==========================================================================
//
//	KMenuScreen::ProcessMenuAction
//
//==========================================================================

void KMenuScreen::ProcessMenuAction(int Action, MenuType_t Invoke, int Key)
{
	switch (Action)
	{
	case MA_None:
		break;

	case MA_Menu:
		SetMenu(Invoke);
		break;

	case MA_MenuNC:
		if (SCNetCheck(Key))
		{
			SetMenu(Invoke);
		}
		break;

	case MA_EndGame:
		if (!demoplayback && SCNetCheck(3))
		{
			ForceMenuOff();
			askforquit = true;
			typeofask = 2; //endgame
		}
		break;

	case MA_Quit:
		SCQuitGame(0);
		break;

	case MA_Custom:
		ProcessCustomMenuAction(Key);
		break;
	}
}

//==========================================================================
//
//
//
//==========================================================================

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

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

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
};

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
		gi.Error("PopMenu: Stack underflow");

	MenuStack[MenuSP]->Destroy();
	if (MenuSP)
	{
		CurrentMenu = MenuStack[MenuSP - 1];
		CurrentMenu->Show();
		GRootWindow->SetFocus(CurrentMenu->Items[CurrentMenu->CursorPos]);
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
	if (MenuActive)
	{
		return;
	}
	if (paused)
	{
		S_ResumeSound();
	}
	StartMenu(MENU_MAIN);
}

//---------------------------------------------------------------------------
//
// PROC MN_DeactivateMenu
//
//---------------------------------------------------------------------------

void MN_DeactivateMenu(void)
{
	if(!CurrentMenu) return;

	ForceMenuOff();
	S_StartSound(NULL, SFX_PLATFORM_STOP);
	P_ClearMessage(&players[consoleplayer]);
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
			}
		}
	}
	else if(!stricmp(argv[0], "quickload"))
	{
		if(SCNetCheck(2))
		{
			if(!quickload || quickload == -1)
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

//==========================================================================
//
//
//
//==========================================================================

