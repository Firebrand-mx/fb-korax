
//**************************************************************************
//**
//** GL_MAIN.C
//**
//** Version:		1.0
//** Last Build:	-?-
//** Author:		jk
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "h2def.h"
#include "gl_def.h"
#include "gl_font.h"
#include "i_win32.h"
#include "console.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// Prepare a GDI font. Select it as the current font.
int FR_PrepareGDIFont(HFONT hfont);

void GL_ResetData(void);	
void GL_ResetLumpTexData();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern volatile int ticcount;
extern boolean novideo;

extern int maxnumnodes;
extern boolean filloutlines;

//extern HWND	hWndMain;		// Handle to the main window.

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int UpdateState;

// The display mode and the default values.
// ScreenBits is ignored.
int screenWidth=640, screenHeight=480, screenBits=32;
int defResX=640, defResY=480, defBPP=0;	// The default resolution (config file).
int maxTexSize;
int ratioLimit = 0;		// Zero if none.
int test3dfx = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static boolean initOk = false;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// I_Update
//
//==========================================================================

void I_Update (void)
{
	if(UpdateState == I_NOUPDATE) return;
//
// blit screen to video
//
	gl.OnScreen();
	UpdateState = I_NOUPDATE; // clear out all draw types
}

//--------------------------------------------------------------------------
//
// PROC I_InitGraphics
//
//--------------------------------------------------------------------------


void I_InitGraphics(void)
{
	HFONT	hfont;
	int		p;

	if(initOk) return;	// Already initialized.

	novideo = M_CheckParm("-novideo");
	if(novideo)
	{
		return;
	}

	// By default, use the resolution defined in hexen.cfg.
	screenWidth = defResX;
	screenHeight = defResY;
	screenBits = defBPP;

	// See the resolution arguments.
	if((p=M_CheckParm("-width")) && p < myargc-1)
		screenWidth = atoi(myargv[p+1]);
	if((p=M_CheckParm("-height")) && p < myargc-1)
		screenHeight = atoi(myargv[p+1]);
	if((p=M_CheckParm("-bpp")) && p < myargc-1)
		screenBits = atoi(myargv[p+1]);

	ST_Message( "Display mode: %i x %i", screenWidth, screenHeight);
	if(screenBits) ST_Message( " x %i", screenBits);
	ST_Message( ".\n");
	gl.Init(screenWidth, screenHeight, screenBits, !nofullscreen);

	// Check the maximum texture size.
	gl.GetIntegerv(DGL_MAX_TEXTURE_SIZE, &maxTexSize);
	if(maxTexSize == 256) 
	{
		ST_Message("  Using restricted texture w/h ratio (1:8).\n");
		ratioLimit = 8;
		if(screenBits == 32)
			ST_Message("  Warning: Are you sure your video card accelerates a 32 bit mode?\n");
	}
	if(M_CheckParm("-outlines"))
	{
		filloutlines = false;
		ST_Message( "  Textures have outlines.\n");
	}
	if(M_CheckParm("-3dfxtest"))
	{
		test3dfx = 1;
		ST_Message("  3dfx test mode.\n");
	}

	GL_InitRenderer();

	FR_Init();
	hfont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
	if(hfont == NULL)
		I_Error("I_InitOpenGL: There is no SYSTEM_FIXED_FONT!\n");
	FR_PrepareGDIFont(hfont);

	// Set the console font.
	CON_Execute("font default", true);

	initOk = true;
}

//--------------------------------------------------------------------------
//
// PROC I_ShutdownGraphics
//
//--------------------------------------------------------------------------

void I_ShutdownDGL(void)
{
	GL_ResetData();
	GL_ResetLumpTexData();
	gl.Shutdown();
}

void I_ShutdownGraphics(void)
{
	if(!initOk) return;	// Not yet initialized fully.

	FR_Shutdown();
	R_SkyShutdown();
	I_ShutdownDGL();

	initOk = false;
}

//--------------------------------------------------------------------------
//
// PROC I_ChangeResolution
//
// Changes the resolution to the specified one. Returns true if the 
// operation is successful.
//
//--------------------------------------------------------------------------

int I_ChangeResolution(int w, int h)
{
	if(gl.ChangeMode(w, h, 0, !nofullscreen) == DGL_OK)
	{
		screenWidth = w;
		screenHeight = h;
		// We can't be in a 3D mode right now, so we can
		// adjust the viewport right away.
		gl.Viewport(0, 0, screenWidth, screenHeight);
		return true;
	}
	return false;	
}

//--------------------------------------------------------------------------
//
// PROC I_GrabScreen
//
// Copies the current contents of the frame buffer and returns a pointer
// to data containing 24-bit RGB triplets.
//
//--------------------------------------------------------------------------

unsigned char *GL_GrabScreen(void)
{
	unsigned char *buffer = (byte *)malloc(screenWidth*screenHeight*3);

	// -JL- Paranoia
	if (!buffer)
		I_Error("GL_GrabScreen: malloc failed");
	gl.Grab(0, 0, screenWidth, screenHeight, DGL_RGB, buffer);
	return buffer;
}

