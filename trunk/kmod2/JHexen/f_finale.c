
//**************************************************************************
//**
//** f_finale.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "soundst.h"
#include "p_local.h"
#include <ctype.h>
//#include "ogl_def.h"

// MACROS ------------------------------------------------------------------

#define	TEXTSPEED	3
#define	TEXTWAIT	250

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void TextWrite(void);
static void DrawPic(void);
static void InitializeFade(boolean fadeIn);
static void DeInitializeFade(void);
static void FadePic(void);
static char *GetFinaleText(int sequence);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean automapactive;
extern boolean viewactive;

// PUBLIC DATA DECLARATIONS ------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int FinaleStage;
static int FinaleCount;
static int FinaleEndCount;
static int FinaleLumpNum;
static int FontABaseLump;
static char *FinaleText;

static fixed_t Palette;
static fixed_t PaletteDelta;
//static byte RealPalette;

// CODE --------------------------------------------------------------------

//===========================================================================
//
// F_StartFinale
//
//===========================================================================

void F_StartFinale (void)
{
	gameaction = ga_nothing;
	gamestate = GS_FINALE;
	viewactive = false;
	automapactive = false;
	P_ClearMessage(&players[consoleplayer]);

	FinaleStage = 0;
	FinaleCount = 0;
	FinaleText = GetFinaleText(0);
	FinaleEndCount = 70;
	FinaleLumpNum = gi.W_GetNumForName("FINALE1");
	FontABaseLump = gi.W_GetNumForName("FONTA_S")+1;
	InitializeFade(1);

//	S_ChangeMusic(mus_victor, true);
	S_StartSongName("hall", false); // don't loop the song
}

//===========================================================================
//
// F_Responder
//
//===========================================================================

boolean F_Responder(event_t *event)
{
	return false;
}

//===========================================================================
//
// F_Ticker
//
//===========================================================================

void F_Ticker (void)
{
	FinaleCount++;
	if(FinaleStage < 5 && FinaleCount >= FinaleEndCount)
	{
		FinaleCount = 0;
		FinaleStage++;
		switch(FinaleStage)
		{
			case 1: // Text 1
				FinaleEndCount = strlen(FinaleText)*TEXTSPEED+TEXTWAIT;
				break;
			case 2: // Pic 2, Text 2
				FinaleText = GetFinaleText(1);
				FinaleEndCount = strlen(FinaleText)*TEXTSPEED+TEXTWAIT;
				FinaleLumpNum = gi.W_GetNumForName("FINALE2");
				S_StartSongName("orb", false);
				break;
			case 3: // Pic 2 -- Fade out
				FinaleEndCount = 70;
				DeInitializeFade();
				InitializeFade(0);
				break;
			case 4: // Pic 3 -- Fade in
				FinaleLumpNum = gi.W_GetNumForName("FINALE3");
				FinaleEndCount = 71;
				DeInitializeFade();
				InitializeFade(1);
				S_StartSongName("chess", true);
				break;
			case 5: // Pic 3 , Text 3
				FinaleText = GetFinaleText(2);
				DeInitializeFade();
				break;
			default:
				 break;
		}
		return;
	}
	if(FinaleStage == 0 || FinaleStage == 3 || FinaleStage == 4)
	{
		FadePic();
	}
}

//===========================================================================
//
// TextWrite
//
//===========================================================================

static void TextWrite (void)
{
	int		count;
	char	*ch;
	int		c;
	int		cx, cy;
	patch_t *w;

	gi.GL_DrawRawScreen(FinaleLumpNum);
	if(FinaleStage == 5)
	{ // Chess pic, draw the correct character graphic
		if(netgame)
		{
			gi.GL_DrawPatch(20, 0, gi.W_GetNumForName("chessall"));
		}
		else if(PlayerClass[consoleplayer])
		{
			gi.GL_DrawPatch(60, 0, gi.W_GetNumForName("chessc")+PlayerClass[consoleplayer]-1);
		}
	}
	// Draw the actual text
	if(FinaleStage == 5)
	{
		cy = 135;
	}
	else
	{
		cy = 5;
	}
	cx = 20;
	ch = FinaleText;
	count = (FinaleCount-10)/TEXTSPEED;
	if (count < 0)
	{
		count = 0;
	}
	for(; count; count--)
	{
		c = *ch++;
		if(!c)
		{
			break;
		}
		if(c == '\n')
		{
			cx = 20;
			cy += 9;
			continue;
		}
		if(c < 32)
		{
			continue;
		}
		c = toupper(c);
		if(c == 32)
		{
			cx += 5;
			continue;
		}
		w = gi.W_CacheLumpNum(FontABaseLump+c-33, PU_CACHE);
		if(cx+w->width > SCREENWIDTH)
		{
			break;
		}
		gi.GL_DrawPatch(cx, cy, FontABaseLump+c-33);
		cx += w->width;
	}
}

//===========================================================================
//
// InitializeFade
//
//===========================================================================

static void InitializeFade(boolean fadeIn)
{
	if(fadeIn)
	{
		Palette = 0;
		PaletteDelta = FixedDiv(0xff<<FRACBITS, 70*FRACUNIT);
	}
	else
	{
		Palette = 0xff<<FRACBITS;
		PaletteDelta = FixedDiv(Palette, -70*FRACUNIT);
	}
}

//===========================================================================
//
// DeInitializeFade
//
//===========================================================================

static void DeInitializeFade(void)
{
	Palette = 255<<FRACBITS;
	PaletteDelta = 0;
}

//===========================================================================
//
// FadePic
//
//===========================================================================

static void FadePic(void)
{
	Palette += PaletteDelta;
}

//===========================================================================
//
// DrawPic
//
//===========================================================================

static void DrawPic(void)
{
	gi.GL_DrawRawScreen(FinaleLumpNum);
	if(FinaleStage == 4 || FinaleStage == 5)
	{ // Chess pic, draw the correct character graphic
		if(netgame)
		{
			gi.GL_DrawPatch(20, 0, gi.W_GetNumForName("chessall"));
		}
		else if(PlayerClass[consoleplayer])
		{
			gi.GL_DrawPatch(60, 0, gi.W_GetNumForName("chessc")+PlayerClass[consoleplayer]-1);
		}
	}
}

//===========================================================================
//
// F_Drawer
//
//===========================================================================

void F_Drawer(void)
{
	switch(FinaleStage)
	{
		case 0: // Fade in initial finale screen
			DrawPic();
			break;
		case 1:
		case 2:
			TextWrite();
			break;
		case 3: // Fade screen out
			DrawPic();
			break;
		case 4: // Fade in chess screen
			DrawPic();
			break;
		case 5:
			TextWrite();
			break;
	}
	// The Dark Fader (Darth Vader?). The black filter, if you will.
	if(FinaleStage == 0 || FinaleStage == 3 || FinaleStage == 4)
	{
		gi.GL_SetNoTexture();
		gi.GL_DrawRect(0, 0, 320, 200, 0,0,0, (255-(Palette>>FRACBITS))/255.0);
	}
	//UpdateState |= I_FULLSCRN;	
	gi.Update(DDUF_FULLSCREEN);
}

//==========================================================================
//
// GetFinaleText
//
//==========================================================================

static char *GetFinaleText(int sequence)
{
	char *msgLumpName;
	int msgSize;
	int msgLump;
	static char *winMsgLumpNames[] =
	{
		"win1msg",
		"win2msg",
		"win3msg"
	};

	msgLumpName = winMsgLumpNames[sequence];
	msgLump = gi.W_GetNumForName(msgLumpName);
	msgSize = gi.W_LumpLength(msgLump);
	if(msgSize >= MAX_INTRMSN_MESSAGE_SIZE)
	{
		gi.Error("Finale message too long (%s)", msgLumpName);
	}
	gi.W_ReadLump(msgLump, ClusterMessage);
	ClusterMessage[msgSize] = 0; // Append terminator
	return ClusterMessage;
}
