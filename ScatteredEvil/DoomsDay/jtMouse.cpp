// Jake Tools: Mouse input using DirectInput

#define DIRECTINPUT_VERSION 0x0300

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dinput.h>
#include <stdio.h>
#include "jtMouse.h"

static int initOk = 0;

static HRESULT hres;
static LPDIRECTINPUT dInput;
static LPDIRECTINPUTDEVICE didMouse;

int jtMouseInit(HINSTANCE hInst, HWND hWnd)
{
	// We aren't going to reinitialize.
	if(initOk) return 0;

	// Create a direct input object.
	if(FAILED(hres=DirectInputCreate(hInst, DIRECTINPUT_VERSION, &dInput, NULL)))
	{
		return -1;
	}
	
	// Create the mouse device.
	if(FAILED(hres=dInput->CreateDevice(GUID_SysMouse, &didMouse, NULL)))
	{
		return -1;
	}		

	// Set the data format.
	if(FAILED(hres=didMouse->SetDataFormat(&c_dfDIMouse)))
	{
		return -1;
	}

	// The cooperative level.
	if(FAILED(hres=didMouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND)))
	{
		return -1;
	}
	
	// Finally, acquire the mouse.
	didMouse->Acquire();

	// The initialization was evidently a success.
	initOk = 1;
	return 0;
}

#define SAFE_RELEASE(x) if(x) {(x)->Release(); (x)=0;}

void jtMouseShutdown()
{
	if(!initOk) return;

	// Release the created objects.
	SAFE_RELEASE(didMouse);
	SAFE_RELEASE(dInput);

	initOk = 0;	// We're not initialized any more.
}

// X and Y are relative mickeys. Buttons is a bitfield.
void jtMouseGetState(jtmousestate_t *state)
{
	DIMOUSESTATE mstate;

	memset(state, 0, sizeof(jtmousestate_t));
	if(FAILED(hres=didMouse->GetDeviceState(sizeof(mstate), &mstate)))
	{
		// Reacquire.
		hres = didMouse->Acquire();
		// Try again.
		if(FAILED(hres=didMouse->GetDeviceState(sizeof(mstate), &mstate))) return;
	}
	if(hres != DI_OK) 
	{
		printf( "Mouse/GetState failed.\n");
		return;
	}
	// Fill in the state structure.
	state->x = mstate.lX;
	state->y = mstate.lY;
	state->z = mstate.lZ;
	state->buttons = 0;
	if(mstate.rgbButtons[0] & 0x80) state->buttons |= JTMB_LEFT;
	if(mstate.rgbButtons[1] & 0x80) state->buttons |= JTMB_RIGHT;
	if(mstate.rgbButtons[2] & 0x80) state->buttons |= JTMB_MIDDLE;
}
