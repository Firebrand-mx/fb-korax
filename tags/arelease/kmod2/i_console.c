
//**************************************************************************
//**
//** I_CONSOLE.C
//**
//** Win32 console window handling.
//** Used in dedicated mode.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "dd_def.h"
#include "i_win32.h"
#include "i_console.h"
#include "console.h"

// MACROS ------------------------------------------------------------------

#define MAXRECS			128
#define LINELEN			80
#define TEXT_ATTRIB		(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define CMDLINE_ATTRIB	(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static HANDLE hcInput;
static HANDLE hcScreen;
static CONSOLE_SCREEN_BUFFER_INFO cbInfo;
static int cx, cy;
static WORD attrib;
static int needNewLine = false;

// CODE --------------------------------------------------------------------

void ICon_Init()
{
	//MessageBox(NULL, "starting: freeconsole", "ICon_Init", MB_OK);
	FreeConsole();
	//MessageBox(NULL, "calling allocconsole", "ICon_Init", MB_OK);
	if(!AllocConsole()) 
	{
		//MessageBox(NULL, "allocconsole FAILED", "ICon_Init", MB_OK);
		I_Error("couldn't allocate a console! error %i\n", 
		GetLastError());
	}
	//MessageBox(NULL, "getting input handle", "ICon_Init", MB_OK);
	hcInput = GetStdHandle(STD_INPUT_HANDLE);
	if(hcInput == INVALID_HANDLE_VALUE) I_Error("bad input handle\n");

	//MessageBox(NULL, "setting title", "ICon_Init", MB_OK);
	if(!SetConsoleTitle("Korax "DOOMSDAY_VERSION_TEXT" / Dedicated"))
		I_Error("setting console title: error %i\n", GetLastError());

	//MessageBox(NULL, "getting output handle", "ICon_Init", MB_OK);
	hcScreen = GetStdHandle(STD_OUTPUT_HANDLE);
	if(hcScreen == INVALID_HANDLE_VALUE) I_Error("bad output handle\n");

	//MessageBox(NULL, "getting sbinfo", "ICon_Init", MB_OK);
	GetConsoleScreenBufferInfo(hcScreen, &cbInfo);

	// This is the location of the print cursor.
	cx = cy = 0;

	//MessageBox(NULL, "updating cmd line", "ICon_Init", MB_OK);

	ICon_UpdateCmdLine("");

	//MessageBox(NULL, "done", "ICon_Init", MB_OK);
}

void ICon_Shutdown()
{
}

void ICon_PostEvents()
{
	event_t ev;
	DWORD num, read;
	INPUT_RECORD rec[MAXRECS], *ptr;
	KEY_EVENT_RECORD *key;

	if(!GetNumberOfConsoleInputEvents(hcInput, &num)) 
		I_Error("ICon_PostEvents: error %i\n", GetLastError());
	if(!num) return;
	ReadConsoleInput(hcInput, rec, MAXRECS, &read);
	for(ptr=rec; read > 0; read--, ptr++)
	{
		if(ptr->EventType != KEY_EVENT) continue;
		key = &ptr->Event.KeyEvent;
		ev.type = key->bKeyDown? ev_keydown : ev_keyup;
		if(key->wVirtualKeyCode == VK_UP) 
			ev.data1 = DDKEY_UPARROW;
		else if(key->wVirtualKeyCode == VK_DOWN)
			ev.data1 = DDKEY_DOWNARROW;
		else
			ev.data1 = I_ScanToKey(key->wVirtualScanCode);
		DD_PostEvent(&ev);
	}
}

void ICon_SetCursor(int x, int y)
{
	COORD pos;

	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(hcScreen, pos);
}

void ICon_ScrollLine()
{
	SMALL_RECT src;
	COORD dest;
	CHAR_INFO fill;

	src.Left = 0;
	src.Right = 79;
	src.Top = 1;
	src.Bottom = cbInfo.dwSize.Y-2;
	dest.X = 0;
	dest.Y = 0;
	fill.Attributes = TEXT_ATTRIB;
	fill.Char.AsciiChar = ' ';
	ScrollConsoleScreenBuffer(hcScreen, &src, &cbInfo.srWindow, dest, &fill);
}

void ICon_SetAttrib(int flags)
{
	attrib = 0;
	if(flags & CBLF_WHITE) attrib = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	if(flags & CBLF_BLUE) attrib = FOREGROUND_BLUE;
	if(flags & CBLF_GREEN) attrib = FOREGROUND_GREEN;
	if(flags & CBLF_CYAN) attrib = FOREGROUND_BLUE | FOREGROUND_GREEN;
	if(flags & CBLF_RED) attrib = FOREGROUND_RED;
	if(flags & CBLF_MAGENTA) attrib = FOREGROUND_RED | FOREGROUND_BLUE;
	if(flags & CBLF_YELLOW) attrib = FOREGROUND_RED | FOREGROUND_GREEN;
	if(flags & CBLF_LIGHT) attrib |= FOREGROUND_INTENSITY;
	if((flags & CBLF_WHITE) != CBLF_WHITE) attrib |= FOREGROUND_INTENSITY;
	SetConsoleTextAttribute(hcScreen, attrib);
}

// Writes the text at the (cx,cy).
void ICon_WriteText(CHAR_INFO *line, int len)
{
	COORD linesize = { len, 1 };
	COORD from = { 0, 0 };
	SMALL_RECT rect;

	rect.Left = cx;
	rect.Right = cx+len;
	rect.Top = cy;
	rect.Bottom = cy;
	WriteConsoleOutput(hcScreen, line, linesize, from, &rect);
}

void ICon_Print(int clflags, char *text)
{
	CHAR_INFO line[LINELEN];
	int count = strlen(text), linestart, bpos;
	char *ptr = text, ch;

	if(needNewLine)
	{
		// Need to make some room.
		cx = 0;
		cy++;
		if(cy == cbInfo.dwSize.Y-1)
		{
			cy--;
			ICon_ScrollLine();
		}
		needNewLine = false;
	}
	bpos = linestart = cx;
	ICon_SetAttrib(clflags);
	for(; count>0; count--, ptr++)
	{
		ch = *ptr;
		if(ch != '\n' && bpos < LINELEN)
		{
			line[bpos].Attributes = attrib;
			line[bpos].Char.AsciiChar = ch;
			bpos++;
		}
		// Time for newline?
		if(ch == '\n' || bpos == LINELEN)
		{
			ICon_WriteText(line+linestart, bpos-linestart);
			cx += bpos-linestart;
			bpos = 0;
			linestart = 0;
			if(count > 1)	// Not the last character?
			{
				needNewLine = false;
				cx = 0;
				cy++;
				if(cy == cbInfo.dwSize.Y-1)
				{
					ICon_ScrollLine();
					cy--;
				}
			}
			else needNewLine = true;
		}
	}
	// Something in the buffer?
	if(bpos-linestart)
	{
		ICon_WriteText(line+linestart, bpos-linestart);
		cx += bpos-linestart;
	}
}

void ICon_UpdateCmdLine(char *text)
{
	CHAR_INFO line[LINELEN], *ch;
	unsigned int i;
	COORD linesize = { LINELEN, 1 };
	COORD from = { 0, 0 };
	SMALL_RECT rect;

	line[0].Char.AsciiChar = '>';
	line[0].Attributes = CMDLINE_ATTRIB;
	for(i=0, ch=line+1; i<LINELEN-1; i++, ch++)
	{
		if(i<strlen(text))
			ch->Char.AsciiChar = text[i];
		else
			ch->Char.AsciiChar = ' ';
		// Gray color.
		ch->Attributes = CMDLINE_ATTRIB;
	}
	rect.Left = 0;
	rect.Right = LINELEN-1;
	rect.Top = cbInfo.dwSize.Y-1;
	rect.Bottom = cbInfo.dwSize.Y-1;
	WriteConsoleOutput(hcScreen, line, linesize, from, &rect);
	ICon_SetCursor(strlen(text)+1, cbInfo.dwSize.Y-1);
}	