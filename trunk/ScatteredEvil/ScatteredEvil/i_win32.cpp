
//**************************************************************************
//**
//** I_WIN32.C : JHexen
//**
//** Version:		1.0
//** Last Build:	-?-
//** Author:		jk
//**
//** Win32-specific and other miscellaneous routines.
//** Internal stuff.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "h2def.h"
#include "gl_def.h"
#include "i_net.h"
#include "console.h"
#include "i_sound.h"
#include "i_timer.h"

#include "jtKey.h"		// JaKe Tools Keyboard handler.
#include "jtMouse.h"	// JaKe Tools Mouse handler.
#include "jtJoy.h"		// JaKe Tools Joystick handler.

// MACROS ------------------------------------------------------------------

#define KBDQUESIZE		32
#define MAX_DOWNKEYS	16		// Most keyboard support 6 or 7.

#define SC_RSHIFT       0x36
#define SC_LSHIFT       0x2a

// TYPES -------------------------------------------------------------------

typedef struct
{
	int key;			// The H2 key code (0 if not in use).
	int	timer;			// How's the time?
	int count;			// How many times has been repeated?
} repeater_t;

class DoomsdayError
{
public:
	char Message[1024];

	DoomsdayError(const char *InMessage)
	{
		strcpy(Message, InMessage);
	}
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void I_StartupKeyboard (void);
void I_ShutdownKeyboard (void);
void I_StartupMouse (void);
void I_ShutdownMouse (void);
void I_ReadMouse (void);
void I_StartupJoystick (void);
void I_ShutdownJoystick (void);
void I_JoystickEvents (void);
void I_ShutdownNetwork();

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern HWND			hWndMain;
extern HINSTANCE	hInstApp; // From JHexenMain.c

// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean		mousepresent;
boolean		joystickpresent;

int			systics=0;		// System tics (every game tic).

int			usejoystick=0;

int			mouseInverseY = false;
int			joySensitivity=5;		// Joystick sensitivity (dead zone).
int			mouseWheelSensi = 10;	// I'm shooting in the dark here.

int repWait1=15, repWait2=4; // The initial and secondary repeater delays (tics).

boolean	novideo; // if true, stay in text mode for debugging
boolean useexterndriver=false;	// I don't even know what this means. -jk

int			mouseDisableX = false, mouseDisableY = false;


byte scantokey[256] =	
{
//	0				1			2				3				4			5					6				7
//	8				9			A				B				C			D					E				F
// 0
	0  ,			27, 		'1',			'2',			'3',		'4',				'5',			'6',
	'7',			'8',		'9',			'0',			'-',		'=',				DDKEY_BACKSPACE,9,			// 0
// 1
	'q',			'w',		'e',			'r',			't',		'y',				'u',			'i',
	'o',			'p',		'[',			']',			13 ,		DDKEY_RCTRL,		'a',			's',		// 1
// 2
	'd',			'f',		'g',			'h',			'j',		'k',				'l',			';',
	39 ,			'`',		DDKEY_RSHIFT,	92,				'z',		'x',				'c',			'v',		// 2
// 3
	'b',			'n',		'm',			',',			'.',		'/',				DDKEY_RSHIFT,	'*',
	DDKEY_RALT,		' ',		0  ,			DDKEY_F1,		DDKEY_F2,	DDKEY_F3,			DDKEY_F4,		DDKEY_F5,   // 3
// 4
	DDKEY_F6,		DDKEY_F7,	DDKEY_F8,		DDKEY_F9,		DDKEY_F10,	DDKEY_NUMLOCK,		DDKEY_SCROLL,	DDKEY_NUMPAD7,
	DDKEY_NUMPAD8,	DDKEY_NUMPAD9, '-',			DDKEY_NUMPAD4,	DDKEY_NUMPAD5, DDKEY_NUMPAD6,	'+',			DDKEY_NUMPAD1, // 4
// 5
	DDKEY_NUMPAD2,	DDKEY_NUMPAD3, DDKEY_NUMPAD0, DDKEY_DECIMAL,0,			0, 					0,				DDKEY_F11,
	DDKEY_F12,		0  ,		0  ,			0  ,			DDKEY_BACKSLASH, 0,				0  ,			0,			// 5
// 6
	0  ,			0  ,		0  ,			0  ,			0  ,		0  ,				0  ,			0,
	0  ,			0  ,		0  ,			0  ,			0,			0  ,				0  ,			0,			// 6
// 7
	0  ,			0  ,		0  ,			0  ,			0,			0  ,				0  ,			0,
	0  ,			0  ,		0  ,			0,				0  ,		0  ,				0  ,			0,			// 7
// 8
	0  ,			0  ,		0  ,			0  ,			0,			0  ,				0  ,			0,
	0,				0  ,		0  ,			0,				0  ,		0  ,				0  ,			0,			// 8
// 9
	0  ,			0  ,		0  ,			0  ,			0,			0  ,				0  ,			0,
	0  ,			0  ,		0  ,			0,				DDKEY_ENTER, DDKEY_RCTRL,		0  ,			0,			// 9
// A
	0  ,			0  ,		0  ,			0  ,			0,			0  ,				0  ,			0,
	0  ,			0  ,		0  ,			0,				0  ,		0  ,				0  ,			0,			// A
// B
	0  ,			0  ,		0  ,			0  ,			0,			'/',				0  ,			0,
	DDKEY_RALT,		0  ,		0  ,			0,				0  ,		0  ,				0  ,			0,			// B
// C
	0  ,			0  ,		0  ,			0  ,			0,			DDKEY_PAUSE,		0  ,			DDKEY_HOME,
	DDKEY_UPARROW,	DDKEY_PGUP,	0  ,			DDKEY_LEFTARROW,0  ,		DDKEY_RIGHTARROW,	0  ,			DDKEY_END,	// C
// D
	DDKEY_DOWNARROW,DDKEY_PGDN, DDKEY_INS,		DDKEY_DEL,		0,			0  ,				0  ,			0,
	0  ,			0  ,		0  ,			0,				0  ,		0  ,				0  ,			0,			// D
// E	
	0  ,			0  ,		0  ,			0  ,			0,			0  ,				0  ,			0,
	0  ,			0  ,		0  ,			0,				0  ,		0  ,				0  ,			0,			// E
// F
	0  ,			0  ,		0  ,			0  ,			0,			0  ,				0  ,			0,
	0  ,			0  ,		0  ,			0,				0  ,		0  ,				0  ,			0			// F
//	0				1			2				3				4			5					6				7
//	8				9			A				B				C			D					E				F
};


// PRIVATE DATA DEFINITIONS ------------------------------------------------

static repeater_t keyReps[MAX_DOWNKEYS];
static int lastfpstic = 0, fpsnum = 0, lastfc = 0;

// CODE --------------------------------------------------------------------

byte I_ScanToKey(byte scan)
{
	return scantokey[scan];
}

byte I_KeyToScan(byte key)
{
	int	i;
	for(i=0; i<256; i++) if(scantokey[i] == key) return i;
	return 0;
}

// If buff is "" upon returning, the key is not valid for controls.
/*void I_NameForControlKey(int h2key, char *buff)
{
	strcpy(buff, "");
	switch(h2key)
	{
	case 0:
		strcpy(buff, "NONE");
		break;
	case DDKEY_RIGHTARROW:
		strcpy(buff, "RIGHT");
		break;
	case DDKEY_LEFTARROW:
		strcpy(buff, "LEFT");
		break;
	case DDKEY_UPARROW:
		strcpy(buff, "UP");
		break;
	case DDKEY_DOWNARROW:
		strcpy(buff, "DOWN");
		break;
	case DDKEY_ENTER:
		strcpy(buff, "ENTER");
		break;
	case DDKEY_RSHIFT:
		strcpy(buff, "SHIFT");
		break;
	case DDKEY_RCTRL:
		strcpy(buff, "CTRL");
		break;
	case DDKEY_RALT:
		strcpy(buff, "ALT");
		break;
	case DDKEY_INS:
		strcpy(buff, "INSERT");
		break;
	case DDKEY_DEL:
		strcpy(buff, "DELETE");
		break;
	case DDKEY_PGUP:
		strcpy(buff, "PAGE UP");
		break;
	case DDKEY_PGDN:
		strcpy(buff, "PAGE DOWN");
		break;
	case DDKEY_HOME:
		strcpy(buff, "HOME");
		break;
	case DDKEY_END:
		strcpy(buff, "END");
		break;
	case DDKEY_BACKSPACE:
		strcpy(buff, "BACKSPACE");
		break;
	case ' ':
		strcpy(buff, "SPACE");
		break;
	case '[':
		strcpy(buff, "SQBRKT OPEN");
		break;
	case ']':
		strcpy(buff, "SQBRKT CLOSE");
		break;
	case DDKEY_BACKSLASH:
		strcpy(buff, "BACKSLASH");
		break;
	default:
		h2key = toupper(h2key);
		if(h2key > 32 && h2key <= 90)
		{
			buff[0] = h2key;
			buff[1] = 0;
		}
	}
}*/

//==========================================================================
//
// I_Init
//
// Initialize machine state
//
//==========================================================================

void I_Init (void)
{
	ST_Message("  I_StartupKeyboard...\n");
	I_StartupKeyboard();
	ST_Message("  I_StartupMouse... ");
	I_StartupMouse();
	ST_Message("  I_StartupJoystick... ");
	I_StartupJoystick();
	//ST_Message("  S_Init...\n");
	//S_Init();
	ST_Message("  I_StartupSound...\n");
	I_StartupSound();
//	S_Start();
}

//==========================================================================
//
// I_Shutdown
//
// Return to default system state
//
//==========================================================================

void I_Shutdown (void)
{
	H2_Shutdown();

	I_ShutdownNetwork();
	// Let's shut down sound first, so Windows' HD-hogging doesn't jam
	// the MUS player (would produce horrible bursts of notes). -jk
	I_ShutdownSound();
	I_ShutdownGraphics();
	I_ShutdownMouse();
	I_ShutdownJoystick();
	I_ShutdownKeyboard();
}

//==========================================================================
//
// I_Error
//
//==========================================================================

void I_Error (char *error, ...)
{
	char	buff[200];
	va_list argptr;

	va_start (argptr,error);
	vsprintf (buff, error, argptr);
	va_end (argptr);
	printf ("%s\n", buff);

	throw DoomsdayError(buff);
}


void I_ClearKeyRepeaters(void)
{
	memset(keyReps, 0, sizeof(keyReps));
}

#define SC_UPARROW      0x48
#define SC_DOWNARROW    0x50
#define SC_LEFTARROW    0x4b
#define SC_RIGHTARROW   0x4d

void I_KeyboardEvents()
{
#ifndef NOKBD	
	event_t			ev;
	jtkeyevent_t	keyevs[KBDQUESIZE];
	int				i, k, numkeyevs;

	// Check the repeaters.
	ev.type = ev_keyrepeat;
	k = systics;	// The current time.
	for(i=0; i<MAX_DOWNKEYS; i++)
	{
		repeater_t *rep = keyReps + i;
		if(!rep->key) continue;
		ev.data1 = rep->key;
		if(!rep->count && k - rep->timer >= repWait1)
		{
			// The first time.
			rep->count++;
			rep->timer += repWait1;
			DD_PostEvent(&ev);
		}
		if(rep->count)
		{
			while(k - rep->timer >= repWait2)
			{
				rep->count++;
				rep->timer += repWait2;
				DD_PostEvent(&ev);				
			}
		}
	}

	// Read the keyboard events.
	numkeyevs = jtKeyGetEvents(keyevs, KBDQUESIZE);

	// Translate them to H2 keys.
	for(i=0; i<numkeyevs; i++)
	{
		jtkeyevent_t *jtke = keyevs+i;
		
		// Check the type of the event.
		if(jtke->event == JTKE_KEY_DOWN) // Key pressed?
			ev.type = ev_keydown;
		else if(jtke->event == JTKE_KEY_UP) // Key released?
			ev.type = ev_keyup;
		
		// Use the table to translate the scancode to a h2key.		
		ev.data1 = scantokey[jtke->code];

		// Maintain the repeater table.
		if(ev.type == ev_keydown)
		{
			// Find an empty repeater.
			for(k=0; k<MAX_DOWNKEYS; k++)
				if(!keyReps[k].key)
				{
					keyReps[k].key = ev.data1;
					keyReps[k].timer = systics;
					keyReps[k].count = 0;
					break;
				}
		}
		else if(ev.type == ev_keyup)
		{
			// Clear any repeaters with this key.
			for(k=0; k<MAX_DOWNKEYS; k++)
				if(keyReps[k].key == ev.data1)
					keyReps[k].key = 0;
		}

		// Post the event.
		DD_PostEvent(&ev);
	}
#endif
}

//==========================================================================
//
// I_StartTic
//
// called by D_DoomLoop
// called before processing each tic in a frame
// can call D_PostEvent
// asyncronous interrupt functions should maintain private ques that are
//     read by the syncronous functions to be converted into events
//
//==========================================================================

void I_StartTic (void)
{
	//int				k;
//	unsigned char	down;	// Is the key down?


	I_ReadMouse ();

//
// keyboard events
//
	I_KeyboardEvents();
	
}

//==========================================================================
//
// I_StartFrame
//
//==========================================================================

void I_StartFrame (void)
{
	MSG msg;

	// Start by checking the messages. This is the message pump.
	// Could be in a separate thread?
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(use_jtSound) I2_BeginSoundFrame(); else I3_BeginSoundFrame();
	I_JoystickEvents ();
}

/*//extern int framecount, ticcount;

void MN_Ticker(void)
{
	// Count the frames.
	if(gi.GetTime()-35 > lastfpstic)
	{
		lastfpstic = gi.GetTime();
		fpsnum = *gi.framecount-lastfc;
		lastfc = *gi.framecount;
	}
*/

int I_GetFrameRate(void)
{
	return fpsnum;
}


void I_EndFrame (void)
{
	// Increment the frame counter.
	framecount++;

	// Count the frames.
	if(I_GetTime()-35 >= lastfpstic)
	{
		lastfpstic = I_GetTime();
		fpsnum = framecount - lastfc;
		lastfc = framecount;
	}

	H2_EndFrame();
	if(use_jtSound) I2_EndSoundFrame(); else I3_EndSoundFrame();
}

/*
============================================================================

						KEYBOARD

============================================================================
*/

/*
===============
=
= I_StartupKeyboard
=
===============
*/

void I_StartupKeyboard (void)
{
#ifndef NOKBD
	if(jtKeyInit(hInstApp, hWndMain))
		I_Error("I_StartupKeyboard: Initialization failed.\n");

	// Clear the repeater table.
	memset(keyReps, 0, sizeof(keyReps));
	//repWait1 = 15;
	//repWait2 = 4;
#endif
}


void I_ShutdownKeyboard (void)
{
#ifndef NOKBD
	jtKeyShutdown();
#endif
}



/*
============================================================================

							MOUSE

============================================================================
*/


/*int I_ResetMouse (void)
{
	regs.w.ax = 0;                  // reset
	int386 (0x33, &regs, &regs);
	return regs.w.ax;
}*/



/*
================
=
= StartupMouse
=
================
*/

//void I_StartupCyberMan(void);

void I_StartupMouse (void)
{
	mousepresent = 0;
	if ( M_CheckParm ("-nomouse"))// || !usemouse )
	{
		ST_Message("Mouse: disabled\n");
		return;
	}

	if(jtMouseInit(hInstApp, hWndMain))
	{
		ST_Message("Mouse: initialization failed\n");
		return;
	}
	ST_Message("Mouse: ok\n");

	mousepresent = 1;

//	I_StartupCyberMan();
}


/*
================
=
= ShutdownMouse
=
================
*/

void I_ShutdownMouse (void)
{
	if (!mousepresent)
	  return;

	jtMouseShutdown();
}


void I_HideMouse (void)
{
	ShowCursor(FALSE);
}


/*
================
=
= I_ReadMouse
=
================
*/

int oldMouseButtons = 0;

void I_ReadMouse (void)
{
	event_t ev;
	jtmousestate_t mouse;
	int change;

//
// mouse events
//
	if (!mousepresent)
		return;

	// Get the mouse state.
	jtMouseGetState(&mouse);

	ev.type = ev_mouse;
	ev.data1 = mouseDisableX? 0 : mouse.x;
	ev.data2 = mouseDisableY? 0 : mouse.y;
	ev.data3 = mouse.z;
	if(!mouseInverseY)			// Inversing when NOT inversing...?
		ev.data2 = -ev.data2;	// An apparent negation is made.
	DD_PostEvent (&ev);

	// Insert the possible mouse Z axis into the button flags.
	if(abs(ev.data3) >= mouseWheelSensi)
	{
		mouse.buttons |= ev.data3 > 0? DDMB_MWHEELUP : DDMB_MWHEELDOWN;
	}

	// Check the buttons and send the appropriate events.
	change = oldMouseButtons ^ mouse.buttons; // The change mask.
	// Send the relevant events.
	if((ev.data1 = mouse.buttons & change))
	{
		ev.type = ev_mousebdown;
		DD_PostEvent(&ev);
	}
	if((ev.data1 = oldMouseButtons & change))
	{
		ev.type = ev_mousebup;
		DD_PostEvent(&ev);
	}
	oldMouseButtons = mouse.buttons;
}

/*
============================================================================

					JOYSTICK

============================================================================
*/


/*
===============
=
= I_StartupJoystick
=
===============
*/

int             basejoyx, basejoyy;

void I_StartupJoystick (void)
{
//	int     centerx, centery;

	joystickpresent = 0;
	if(M_CheckParm("-nojoy") || !usejoystick)
	{
		ST_Message("Joystick: disabled\n");
		return;
	}

	// Init jtJoy.
	if(jtJoyInit(hInstApp, hWndMain))
//	if (!I_ReadJoystick ())
	{
		joystickpresent = false;
		ST_Message("Joystick: init failed\n");
		return;
	}
	ST_Message("Joystick: ok\n");
	joystickpresent = true;

	// We don't need to calibrate, thank God.
}

void I_ShutdownJoystick()
{
	jtJoyShutdown();
}

/*
===============
=
= I_JoystickEvents
=
===============
*/

int oldJoyBState = 0;
float oldPOV = JTJOY_POV_CENTER;

void I_JoystickEvents (void)
{
	event_t			ev;
	jtjoystate_t	state;
	int				i, sensi = 10-joySensitivity, bstate;
	int				minNeg = JTJOY_AXISMIN*sensi/10,
					minPos = JTJOY_AXISMAX*sensi/10;

//
// joystick events
//
	if (!joystickpresent)
		return;

	jtJoyGetState(&state);

	bstate = 0;
	// Check the buttons.
	for(i=0; i<JTJOY_MAXBUTTONS; i++) 
		if(state.buttons[i]) bstate |= 1<<i; // Set the bits.

/*	// The POV takes care of the rest of the buttons.
	if(state.pov == JTJOY_POV_FORWARD) bstate |= 0x10000000;
	else if(state.pov == JTJOY_POV_RIGHT) bstate |= 0x20000000;
	else if(state.pov == JTJOY_POV_BACKWARD) bstate |= 0x40000000;
	else if(state.pov == JTJOY_POV_LEFT) bstate |= 0x80000000;*/

	// Check for button state changes. 
	i = oldJoyBState ^ bstate; // The change mask.
	// Send the relevant events.
	if((ev.data1 = bstate & i))
	{
		ev.type = ev_joybdown;
		DD_PostEvent(&ev);
	}
	if((ev.data1 = oldJoyBState & i))
	{
		ev.type = ev_joybup;
		DD_PostEvent(&ev);
	}
	oldJoyBState = bstate;

	// Check for a POV change.
	if(state.povAngle != oldPOV)
	{
		if(oldPOV != JTJOY_POV_CENTER)
		{
			// Send a notification that the existing POV angle is no
			// longer active.
			ev.type = ev_povup;
			ev.data1 = (int) (oldPOV/45 + .5);	// Round off correctly w/.5.
			DD_PostEvent(&ev);
		}
		if(state.povAngle != JTJOY_POV_CENTER)
		{
			// The new angle becomes active.
			ev.type = ev_povdown;
			ev.data1 = (int) (state.povAngle/45 + .5);
			DD_PostEvent(&ev);
		}
		oldPOV = state.povAngle;
	}

	// Send the joystick movement event.
	ev.type = ev_joystick;

	// The X axis.
	if(state.x < minNeg)
		ev.data1 = (state.x-minNeg) * JTJOY_AXISMIN / (JTJOY_AXISMIN-minNeg);
	else if(state.x > minPos)
		ev.data1 = (state.x-minPos) * JTJOY_AXISMAX / (JTJOY_AXISMAX-minPos);
	else
		ev.data1 = 0;

	// The Y axis.
	if(state.y < minNeg)
		ev.data2 = (state.y-minNeg) * JTJOY_AXISMIN / (JTJOY_AXISMIN-minNeg);
	else if(state.y > minPos)
		ev.data2 = (state.y-minPos) * JTJOY_AXISMAX / (JTJOY_AXISMAX-minPos);
	else
		ev.data2 = 0;

	DD_PostEvent(&ev);
}

//==========================================================================
//
// I_Quit
//
// Shuts down net game, saves defaults, prints the exit text message,
// goes to text mode, and exits.
//
//==========================================================================

void I_Quit(void)
{
	// Quit netgame if one is in progress.
	if(netgame || limbo)
		CON_Execute(server? "net server close" : "net disconnect", true);

	M_SaveDefaults();
	I_Shutdown();
	B_Shutdown();
	CON_Shutdown();
	DD_Shutdown();
	exit(0);
}

//--------------------------------------------------------------------------
//
// PROC I_WaitVBL
//
//--------------------------------------------------------------------------

void I_WaitVBL(int vbls)
{
/*	int stat;

	if(novideo)
	{
		return;
	}
	while(vbls--)
	{
		do
		{
			stat = inp(STATUS_REGISTER_1);
			if(stat&8)
			{
				break;
			}
		} while(1);
		do
		{
			stat = inp(STATUS_REGISTER_1);
			if((stat&8) == 0)
			{
				break;
			}
		} while(1);
	}*/
	// We're assuming that vertical refreshes are done at 60 Hz.
	//rest((int)(vbls * 16.667));
}

/*
====================
=
= I_InitNetwork
=
====================
*/

void I_InitNetwork (void)
{
//	int             i;

	// Doomcom will be the interface between the Doom network code
	// and the actual driver (in i_net.c).
	doomcom = (doomcom_t *)malloc (sizeof (*doomcom) );
	// -JL- Paranoia
	if (!doomcom)
		I_Error("I_InitNetwork: malloc failed");
	memset (doomcom, 0, sizeof(*doomcom) );
	doomcom->id = DOOMCOM_ID;

	//i = M_CheckParm ("-net");
	//if(!i)
//	{
	// The game is always started in single-player mode.
	// Doomcom is really the net driver's business, but since this 
	// isn't a net game yet...
	netgame = false;
	doomcom->numplayers = doomcom->numnodes = 1;
	doomcom->deathmatch = false;
	doomcom->consoleplayer = 0;
	doomcom->ticdup = 1;
	doomcom->extratics = 0;
//	return;
//	}

	// Multiplayer game.
//	if(!I_NetInit()) I_Error("I_NetInit failed.\n");

//	netgame = true;
}

void I_ShutdownNetwork()
{
	netgame = false;
	//free(doomcom->data.ticcmds);
	I_NetShutdown();
}

//=========================================================================
//
// I_CheckExternDriver
//
//		Checks to see if a vector, and an address for an external driver
//			have been passed.
//=========================================================================

#if 0
void I_CheckExternDriver(void)
{
/*	int i;

	if(!(i = M_CheckParm("-externdriver")))
	{
		return;
	}
	i_ExternData = (externdata_t *)atoi(myargv[i+1]);
	i_Vector = i_ExternData->vector;
*/
	useexterndriver = false;
}
#endif

//==========================================================================
//
// I_NetCmd
//
//==========================================================================

/*void I_NetCmd (void)
{
	if (!netgame)
		I_Error ("I_NetCmd when not in netgame");
	//DPMIInt (doomcom->intnum);

	// Now we need to simulate doomcom. Nice.

}*/

//==========================================================================
//
// I_Tactile
//
// Originated from the i_cyber.c, and has evidently something to do with
// the CyberMan. Doesn't do anything, see the original implementation.
//
//==========================================================================

/*void I_Tactile (int on, int off, int total)
{
}*/

//==========================================================================
//
// I_ZoneBase
//
//==========================================================================

byte *I_ZoneBase (int *size)
{
	int heap;
	byte *ptr;

	heap = maxzone;
	if(heap < MINIMUM_HEAP_SIZE) heap = MINIMUM_HEAP_SIZE;
	if(heap > MAXIMUM_HEAP_SIZE) heap = MAXIMUM_HEAP_SIZE;
	heap += 0x10000;
	
	ST_Message ("  Maxzone: 0x%x, ", maxzone);
	do // until we get the memory
	{
		heap -= 0x10000;                // leave 64k alone
		if (heap > maxzone)	heap = maxzone;
		ptr = (byte *)malloc (heap);
	} while(!ptr);

	ST_Message ("0x%x allocated for zone.\n", heap);
	ST_Message ("  ZoneBase: 0x%X.\n", (int)ptr);

	if (heap < 0x180000)
		I_Error ("  Insufficient memory!");

	*size = heap;
	return ptr;
}


//==========================================================================
//
// I_AllocLow
//
// Allocated memory. Original implementation allocated low DOS memory,
// but under Win32 we allocate memory as normal.
//
//==========================================================================

byte *I_AllocLow (int length)
{
	byte    *mem = (byte *)malloc(length);

	if(!mem) I_Error("I_AllocLow: Memory alloc of %i bytes failed",length);

	memset (mem,0,length);
	return mem;
}

//==========================================================================
//
//	I_DisableWindowsKeys
//
//==========================================================================

void I_DisableWindowsKeys(void)
{
	// Disable Alt-Tab, Alt-Esc, Ctrl-Alt-Del.
	// A bit of a hack, I'm afraid...
	SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, TRUE, 0, 0);
}

//==========================================================================
//
// main
//
// Called by WinMain
//
//==========================================================================

void main(int argc, char **argv)
{
	try
	{
		myargc = argc;
		myargv = argv;
		DD_Main();
	}
	catch (DoomsdayError &e)
	{
		D_QuitNetGame ();
		I_Shutdown ();
		B_Shutdown();
		CON_Shutdown();
		ChangeDisplaySettings(0, 0); // Restore original mode, just in case.

		// Be a bit more graphic.
		ShowCursor(TRUE);

		char *tmp_msg = new char[strlen(e.Message) + strlen(DD_GetCoreDump()) + 4];
		sprintf(tmp_msg, "%s\n\n%s", e.Message, DD_GetCoreDump());
		MessageBox(hWndMain, tmp_msg, "Korax "VERSIONTEXT, MB_OK|MB_ICONERROR);
		delete tmp_msg;

		DD_Shutdown();

		// Get outta here.
		exit (1);
	}
	catch (...)
	{
		D_QuitNetGame ();
		I_Shutdown ();
		B_Shutdown();
		CON_Shutdown();
		ChangeDisplaySettings(0, 0); // Restore original mode, just in case.

		printf ("Received external exception\n");

		// Be a bit more graphic.
		ShowCursor(TRUE);
		char *tmp_msg = new char[strlen(DD_GetCoreDump()) + 32];
		sprintf(tmp_msg, "Received external exception\n\n%s", DD_GetCoreDump());
		MessageBox(hWndMain, tmp_msg, "Korax "VERSIONTEXT, MB_OK|MB_ICONERROR);
		delete tmp_msg;

		DD_Shutdown();

		// Get outta here.
		throw;
	}
}

