#ifndef __H2CONSOLE_H__
#define __H2CONSOLE_H__

#include "dd_share.h"

// A console buffer line.
typedef struct
{
	int len;					// This is the length of the line (no term).
	char *text;					// This is the text.
	int flags;
} cbline_t;

// Console commands can set this when they need to return a custom value
// e.g. for the game dll.
extern int CmdReturnValue;

void CON_Init();
void CON_Shutdown();
void CON_Open(int yes);
void CON_AddCommand(ccmd_t *cmd);
void CON_AddVariable(cvar_t *var);
void CON_Ticker(void);
boolean CON_Responder(event_t *event);
void CON_Drawer(void);
void CON_Printf(char *format, ...);
void CON_FPrintf(int flags, char *format, ...); // Flagged printf.
cbline_t *CON_GetBufferLine(int num);
int CON_Execute(char *command, int silent);

cvar_t* CvarGet(char *name);

#endif