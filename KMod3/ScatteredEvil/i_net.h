#ifndef __H2_NETWORK_DRIVER_H__
#define __H2_NETWORK_DRIVER_H__

//int I_NetInit();
void I_NetShutdown();

int I_DCNodeForPlayer(int consolePlrNum);
int I_PlayerForDCNode(int doomcomNode);
char *I_GetProtocolName(void);

void I_NetServerStarted();
void I_NetServerStopping(boolean before);

char *I_NetGetPlayerName(int player);
int I_NetSetPlayerData(void *data, int size);
int I_NetGetPlayerData(int playerNum, void *data, int size);
int I_NetSetServerData(void *data, int size);
int I_NetGetServerData(void *data, int size);

#endif // __H2_NETWORK_DRIVER_H__