
//**************************************************************************
//**
//** st_start.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************


// HEADER FILES ------------------------------------------------------------

#include <math.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <io.h>
#include <fcntl.h>
#include <stdarg.h>				// Needed for next as well as dos
#include "h2def.h"
#include "i_win32.h"
#include "st_start.h"
#include "console.h"
#include "gl_def.h"
#include "gl_font.h"
#include "settings.h"

// MACROS ------------------------------------------------------------------
#define ST_MAX_NOTCHES		32
#define ST_NOTCH_WIDTH		16
#define ST_NOTCH_HEIGHT		23
#define ST_PROGRESS_X		64			// Start of notches x screen pos.
#define ST_PROGRESS_Y		441			// Start of notches y screen pos.

#define ST_NETPROGRESS_X		288
#define ST_NETPROGRESS_Y		32
#define ST_NETNOTCH_WIDTH		8
#define ST_NETNOTCH_HEIGHT		16
#define ST_MAX_NETNOTCHES		8

#define LOGO_WIDTH			256
#define LOGO_HEIGHT			51
#define LOGO_SCR_WIDTH		(512)
#define LOGO_SCR_HEIGHT		(102)

#define BUFFER_LEN			8000

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------
extern void SetVideoModeHR(void);
extern void ClearScreenHR(void);
extern void SlamHR(char *buffer);
extern void SlamBlockHR(int x, int y, int w, int h, char *src);
extern void InitPaletteHR(void);
extern void SetPaletteHR(byte *palette);
extern void GetPaletteHR(byte *palette);
extern void FadeToPaletteHR(byte *palette);
extern void FadeToBlackHR(void);
extern void BlackPaletteHR(void);
extern void I_StartupReadKeys(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------
char *ST_LoadScreen(void);
void ST_UpdateNotches(int notchPosition);
void ST_UpdateNetNotches(int notchPosition);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int bufferLines;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean startupScreen = false;

// PRIVATE DATA DEFINITIONS ------------------------------------------------
static fonthgt = 8;	// Height of the font.
static DGLuint bgflat, startupLogo;
char *bitmap = NULL;

// CODE --------------------------------------------------------------------



//--------------------------------------------------------------------------
//
// Startup Screen Functions
//
//--------------------------------------------------------------------------


//==========================================================================
//
// ST_Init - Do the startup screen
//
//==========================================================================

void ST_Init(void)
{
	startupScreen = true;
	gl.MatrixMode(DGL_PROJECTION);
	gl.PushMatrix();
	gl.LoadIdentity();
	gl.Ortho(0, 0, screenWidth, screenHeight, -1, 1);

	// Prepare the font.

	gl.Color3f(1, 1, 1);
	bgflat = 0;

	fonthgt = FR_TextHeight("Doomsday!");

	startupLogo = 0;
	if(W_CheckNumForName("ddlogo") != -1)
	{
		byte *image = (byte *)W_CacheLumpName("ddlogo", PU_CACHE);
		startupLogo = gl.NewTexture();
		gl.TexImage(DGL_LUMINANCE, 256, 64, 0, image);
		gl.TexParameter(DGL_MIN_FILTER, DGL_NEAREST);
		gl.TexParameter(DGL_MAG_FILTER, DGL_LINEAR);
		gl.TexParameter(DGL_WRAP_S, DGL_CLAMP);
		gl.TexParameter(DGL_WRAP_T, DGL_CLAMP);
	}
}

void ST_SetBgFlat(int lump)
{
	bgflat = GL_BindTexFlat(lump);	
}

void ST_Done(void)
{
	startupScreen = false;

	gl.DeleteTextures(1, &startupLogo);

	gl.MatrixMode(DGL_PROJECTION);
	gl.PopMatrix();
}


//==========================================================================
//
// ST_UpdateNotches
//
//==========================================================================

void ST_UpdateNotches(int notchPosition)
{
}


//==========================================================================
//
// ST_UpdateNetNotches - indicates network progress
//
//==========================================================================

void ST_UpdateNetNotches(int notchPosition)
{
}


//==========================================================================
//
// ST_Progress - increments progress indicator
//
//==========================================================================

void ST_Progress(void)
{
	ST_Message(".");
}


//==========================================================================
//
// ST_NetProgress - indicates network progress
//
//==========================================================================

void ST_NetProgress(void)
{
	ST_Message("*");
}


//==========================================================================
//
// ST_NetDone - net progress complete
//
//==========================================================================
void ST_NetDone(void)
{
// FIXME: Play a sound here.
//	S_StartSound(NULL, SFX_PICKUP_WEAPON);
}


//==========================================================================
//
// ST_Message - gives debug message
//
//==========================================================================

void ST_Message(char *message, ...)
{
	va_list argptr;
	char buffer[BUFFER_LEN];

	va_start(argptr, message);
	vsprintf(buffer, message, argptr);
	va_end(argptr);

	if ( strlen(buffer) >= BUFFER_LEN )
	{
		I_Error("Long debug message has overwritten memory");
	}

	// These messages are always dumped. If consoleDump is set,
	// CON_Printf() will dump the message for us.
	if(!consoleDump) printf(buffer);

	// Also print in the console.
	CON_Printf(buffer);

	// Print the messages in the console.
	if(startupScreen)
	{
		int i, vislines=screenHeight/fonthgt, y=0, st;
		gl.Clear(DGL_COLOR_BUFFER_BIT);
		if(startupLogo)
		{
			gl.Bind(startupLogo);
			GL_DrawRect((screenWidth-LOGO_SCR_WIDTH)/2, 
				(screenHeight-LOGO_SCR_HEIGHT)/2, 
				LOGO_SCR_WIDTH, LOGO_SCR_HEIGHT,
				.1f, .1f, .1f, 1);
		}
		st = bufferLines - vislines;
		// Show the last line, too, if there's something.
		if(CON_GetBufferLine(bufferLines-1)->len) st++;
		if(st < 0) st = 0;
		gl.Color3f(1, 1, 1);
		for(i=0; i<vislines && st+i < bufferLines; i++)
		{
			cbline_t *line = CON_GetBufferLine(st+i);
			if(!line) break;
			FR_TextOut(line->text, 0, y);			
			y += fonthgt;
		}
		gl.OnScreen();
	}
}

//==========================================================================
//
// ST_RealMessage - gives user message
//
//==========================================================================

void ST_RealMessage(char *message, ...)
{
	va_list argptr;
	char buffer[BUFFER_LEN];

	va_start(argptr, message);
	vsprintf(buffer, message, argptr);
	va_end(argptr);

	if ( strlen(buffer) >= BUFFER_LEN )
	{
		I_Error("Long debug message has overwritten memory\n");
	}

	printf(buffer);		// Always print these messages
}



//==========================================================================
//
// ST_LoadScreen - loads startup graphic
//
//==========================================================================


char *ST_LoadScreen(void)
{
	int length,lump;
	char *buffer;

	lump = W_GetNumForName("STARTUP");
	length = W_LumpLength(lump);
	buffer = (char *)Z_Malloc(length, PU_STATIC, NULL);
	W_ReadLump(lump, buffer);
	return(buffer);
}
