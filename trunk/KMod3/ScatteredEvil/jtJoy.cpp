// Jake Tools: Joystick input using the MMSYSTEM

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "jtJoy.h"

static int initOk = 0;

static MMRESULT hres;

int jtJoyInit(HINSTANCE hInst, HWND hWnd)
{
	int		i = 0, num = joyGetNumDevs();

	// We aren't going to reinitialize.
	if(initOk) return 0;

	if(!num)
	{
		return -1; // No joysticks supported.
	}
	
	while(!initOk && i<num)
	{
		JOYINFOEX jinfo;
		memset(&jinfo, 0, sizeof(jinfo));
		jinfo.dwSize = sizeof(jinfo);
		// Check that the joystick is plugged in.
		if(joyGetPosEx(i++,/*JOYSTICKID1, */&jinfo) == JOYERR_UNPLUGGED)
			continue;
			//return -1;

		// The initialization was evidently a success.
		initOk = 1;
	}
	if(!initOk) return -1;
	return 0;
}

#define SAFE_RELEASE(x) if(x) {(x)->Release(); (x)=0;}

void jtJoyShutdown()
{
	if(!initOk) return;

	initOk = 0;	// We're not initialized any more.
}

void jtJoyGetState(jtjoystate_t *state)
{
	JOYINFOEX jin;
	memset(&jin, 0, sizeof(jin));
	jin.dwSize = sizeof(jin);
	jin.dwFlags = JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ | JOY_RETURNR
		| JOY_RETURNBUTTONS | JOY_RETURNCENTERED | JOY_RETURNPOV;
	if(joyGetPosEx(JOYSTICKID1, &jin) != JOYERR_NOERROR) return;

	//printf( "%d, %d - %x\n", jin.dwXpos, jin.dwYpos, jin.dwButtons);

	// Fill in the state structure. 
	state->x = jin.dwXpos;
	state->y = jin.dwYpos;
	state->z = jin.dwZpos;
	state->r = jin.dwRpos;
	state->x = (state->x - 0x8000) * JTJOY_AXISMAX/0x8000;
	state->y = (state->y - 0x8000) * JTJOY_AXISMAX/0x8000;
	state->z = (state->z - 0x8000) * JTJOY_AXISMAX/0x8000;
	state->r = (state->r - 0x8000) * JTJOY_AXISMAX/0x8000;
	
	for(int i=0; i<JTJOY_MAXBUTTONS; i++)
		state->buttons[i] = char ((jin.dwButtons>>i) & 0x1);

	// Determine the state of the point-of-view hat.
/*	switch(jin.dwPOV)
	{
	case JOY_POVFORWARD:
		state->pov = JTJOY_POV_FORWARD;
		break;
	case JOY_POVBACKWARD:
		state->pov = JTJOY_POV_BACKWARD;
		break;
	case JOY_POVLEFT:
		state->pov = JTJOY_POV_LEFT;
		break;
	case JOY_POVRIGHT:
		state->pov = JTJOY_POV_RIGHT;
		break;
	default:
		state->pov = JTJOY_POV_CENTER;
	}*/
	if(jin.dwPOV == JOY_POVCENTERED)
		state->povAngle = JTJOY_POV_CENTER;
	else
		state->povAngle = jin.dwPOV/100.0f;
}
