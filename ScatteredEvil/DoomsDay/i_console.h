#ifndef __DOOMSDAY_WINCONSOLE_H__
#define __DOOMSDAY_WINCONSOLE_H__

void ICon_Init();
void ICon_Shutdown();
void ICon_PostEvents();
void ICon_Print(int clflags, char *text);
void ICon_UpdateCmdLine(char *text);

#endif