#ifndef __H2TIMER__
#define __H2TIMER__

void I_StartupTimer (void);
void I_ShutdownTimer (void);
int I_GetTime (void);
double I_GetSeconds (void);
unsigned int I_GetRealTime (void);

#endif