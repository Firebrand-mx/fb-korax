#ifndef __DOOMSDAY_WIN32_H__
#define __DOOMSDAY_WIN32_H__

#include "dd_share.h"

extern int systics;

// Initialization
void I_Init (void);
void I_InitNetwork (void);

void I_Quit(void);

// Messages
void I_Error (char *error, ...);

// Networking
void I_NetCmd (void);

// Events
void I_ClearKeyRepeaters(void);

// Updates
void I_Update (void);
void I_WaitVBL (int n);

void I_StartTic (void);
void I_StartFrame (void);
void I_EndFrame (void);
int I_GetTime (void);
byte *I_ZoneBase (int *size);
void I_NameForControlKey(int h2key, char *buff);
int I_GetFrameRate(void);

#endif