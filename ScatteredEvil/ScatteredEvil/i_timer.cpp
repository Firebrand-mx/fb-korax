
//**************************************************************************
//**
//** I_TIMER.C
//**
//** Version:		1.0
//** Last Build:	-?-
//** Author:		jk
//**
//** Timer routines.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include "h2def.h"
#include "i_timer.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

//int timerID;
//volatile int	ticcount = 0;	// 35 Hz
//volatile int	highResTic = 0;	// 140 Hz

// PRIVATE DATA DEFINITIONS ------------------------------------------------

//static HANDLE	hTimer;
static boolean	timerInstalled = false;
//static int		subtic = 0;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// I_TimerThread
//
//==========================================================================

//void CALLBACK I_TimerISR(UINT id, UINT msg, DWORD user, DWORD dw1, DWORD dw2)

/*void I_TimerThread(void *data)
{
	while(timerInstalled)
	{
		highResTic++;
		// Call the song player.
		MUS_SongPlayer();
		// Is it time to advance to the next tick?
		if(--subtic <= 0)
		{
			ticcount++;
			subtic = TIMER_SUB_TICS;
		}
		Sleep(TIMER_MS);
	}
}*/

//==========================================================================
//
// I_ShutdownTimer
//
//==========================================================================

void I_ShutdownTimer (void)
{
	timerInstalled = false;
	timeEndPeriod(1);
}

//==========================================================================
//
// I_StartupTimer
//
//==========================================================================

void I_StartupTimer (void)
{
//#ifndef NOTIMER
	ST_Message("  I_StartupTimer...\n");
/*	timeBeginPeriod(TIMER_RES);
	timerID = timeSetEvent(1000/140, TIMER_RES, I_TimerISR, 0, TIME_PERIODIC);
	if(!timerID) I_Error("Failed to start timer.\n");
	timerInstalled = true;*/
	
	//atexit(I_ShutdownTimer);

/*	timerInstalled = true;
	hTimer = (HANDLE) _beginthread(I_TimerThread, 2048, NULL);
	if(hTimer == (HANDLE) -1)
		I_Error("I_StartupTimer: Can't start timer!\n");
	SetThreadPriority(hTimer, THREAD_PRIORITY_TIME_CRITICAL);	
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);*/

	timeBeginPeriod(1);
//#endif
}

//==========================================================================
//
// I_GetTime
//
// Returns time in 1/35th second tics.
//
//==========================================================================

int I_GetTime (void)
{
	return (int) (I_GetRealTime() / 1000.0 * 35);
}

// Returns the timer value in seconds.
double I_GetSeconds (void)
{
	return (double) I_GetRealTime() / 1000.0;
}

unsigned int I_GetRealTime (void)
{
	static boolean first = true;
	static DWORD start;
	DWORD now = timeGetTime();

	if(first)
	{
		first = false;
		start = now;
		return 0;
	}
	
	// Wrapped around? (Every 50 days...)
	if(now < start) return 0xffffffff - start + now + 1;

	return now - start;
}

