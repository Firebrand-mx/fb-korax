// Jake Tools: Keyboard input using DirectInput

#ifndef __JAKETOOLS_KEYBOARD_H__
#define __JAKETOOLS_KEYBOARD_H__

#ifdef __cplusplus
extern "C" {	// We want to use these functions in C as well.
#endif

// The key event types.
#define JTKE_NONE			0
#define JTKE_KEY_DOWN		0x1
#define JTKE_KEY_UP			0x2

typedef struct JTKEYEVENT
{
	char			event;	// Type of the event.		
	unsigned char	code;	// The scancode (extended, corresponds DI keys).
} jtkeyevent_t;

// Returns 0 if successful.
int jtKeyInit(HINSTANCE hInst, HWND hWnd); 

void jtKeyShutdown();

// Returns the number of events read.
int jtKeyGetEvents(jtkeyevent_t *evbuf, int bufsize);

#ifdef __cplusplus
}
#endif

#endif // __JAKETOOLS_KEYBOARD_H__