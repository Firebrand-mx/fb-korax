// Jake Tools: Joystick input using DirectInput

#ifndef __JAKETOOLS_JOYSTICK_H__
#define __JAKETOOLS_JOYSTICK_H__

#define JTJOY_AXISMIN		-10000
#define JTJOY_AXISMAX		10000
#define JTJOY_MAXBUTTONS	32
#define JTJOY_POV_CENTER	-1

typedef struct
{
	int		x, y, z, r;		// The axes.
	char	buttons[JTJOY_MAXBUTTONS];
	float	povAngle;		// 0 - 359 degrees.
} jtjoystate_t;

// Returns 0 if no errors occur.
int jtJoyInit(HINSTANCE hInst, HWND hWnd);

void jtJoyShutdown();

void jtJoyGetState(jtjoystate_t *state);

#endif // __JAKETOOLS_JOYSTICK_H__