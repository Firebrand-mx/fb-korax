// Jake Tools: Keyboard using DirectInput

#define DIRECTINPUT_VERSION 0x0300

#define WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <windows.h>
#include "jtkey.h"
#include <dinput.h>
#include <assert.h>

#define JTKINBUFSIZE 32		// The buffer can hold 32 events.

static int initOk = 0;

static HRESULT hres;
static LPDIRECTINPUT dInput;
static LPDIRECTINPUTDEVICE didKeyb;
static DIDEVICEOBJECTDATA devdat[JTKINBUFSIZE];

// Initialize DirectInput and the keyboard input device.
// Returns 0 if successful.
int jtKeyInit(HINSTANCE hInst, HWND hWnd)
{
	if(initOk) return 0;

	if((hres=DirectInputCreate(hInst, DIRECTINPUT_VERSION, &dInput, NULL)) != DI_OK)
	{
		return -1;
	}

	// Create the device for the keyboard.
	if((hres=dInput->CreateDevice(GUID_SysKeyboard, &didKeyb, NULL)) != DI_OK)
	{
		return -1;
	}
	
	// Set the cooperative level.		
	if((hres=didKeyb->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)) != DI_OK)
	{
		return -1;
	}

	// The data format.
	if((hres=didKeyb->SetDataFormat(&c_dfDIKeyboard)) != DI_OK)
	{
		return -1;
	}

	// The input buffer size.
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(dipdw);
	dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = JTKINBUFSIZE;
	if((hres=didKeyb->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)) != DI_OK)
	{
		return -1;
	}
	
	// Finally, acquire the device.
	didKeyb->Acquire();
	
	// Initialization was successful.
	initOk = 1;
	return 0;
}

#define SAFE_RELEASE(x) if(x) {(x)->Release(); (x)=0;}

void jtKeyShutdown()
{
	if(!initOk) return;

	// Get rid of the DI objects and devices.
	SAFE_RELEASE(didKeyb);
	SAFE_RELEASE(dInput);
	
	initOk = 0;	// Not any more.
}

// Get the keyboard events.
int jtKeyGetEvents(jtkeyevent_t *evbuf, int bufsize)
{
	DWORD num = JTKINBUFSIZE;
	
	if(FAILED(hres = didKeyb->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), devdat, &num, 0)))
	{
		// Try to reacquire.
		didKeyb->Acquire();
		num = JTKINBUFSIZE;
		hres = didKeyb->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), devdat, &num, 0);
		// If we fail again, what is there to do? 
		// We'll wait till next time.
		if(FAILED(hres)) return 0;
	}

	// Let's put the data into the caller's buffer.
	// Num is unsigned, so we cast it to int to shut up the compiler warning.
	for(int i=0; i<(int)num && i<bufsize; i++)
	{
		evbuf[i].event = (devdat[i].dwData & 0x80)? JTKE_KEY_DOWN : JTKE_KEY_UP;
		evbuf[i].code = (unsigned char) devdat[i].dwOfs;
	}
	return i;
}