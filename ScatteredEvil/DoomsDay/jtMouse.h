// Jake Tools: Mouse input using DirectInput

#ifndef __JAKETOOLS_MOUSE_H__
#define __JAKETOOLS_MOUSE_H__

// The buttons:
#define JTMB_LEFT	0x1
#define JTMB_RIGHT	0x2
#define JTMB_MIDDLE	0x4

typedef struct
{
	int x, y, z;	// Relative X and Y mickeys since last call.
	int buttons;	// The buttons bitfield.
} jtmousestate_t;

// Returns 0 if successful.
int jtMouseInit(HINSTANCE hInst, HWND hWnd); 

void jtMouseShutdown();

// Returns the number of events read.
void jtMouseGetState(jtmousestate_t *state);

#endif // __JAKETOOLS_MOUSE_H__