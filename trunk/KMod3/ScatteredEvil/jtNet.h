// The exported functions of the jtNet + other stuff.

#ifndef __JNET_H__
#define __JNET_H__

#define MAXSESSIONDATA	32

// The supported service providers.
enum
{
	JTNET_SERVICE_UNKNOWN,
	JTNET_SERVICE_IPX,
	JTNET_SERVICE_TCPIP,
	JTNET_SERVICE_SERIAL,
	JTNET_SERVICE_MODEM
};

// Error codes.
enum
{
	JTNET_ERROR_OK,
	JTNET_ERROR_ALREADY_INITIALIZED,
	JTNET_ERROR_GENERIC,
	JTNET_ERROR_INIT_SERVICES,
	JTNET_ERROR_SERVICE_NOT_FOUND,
	JTNET_ERROR_OPEN_SERVER,
	JTNET_ERROR_CREATE_PLAYER,
	JTNET_ERROR_SET_SERVER_PARAMS,
	JTNET_ERROR_CONNECTING,
	JTNET_ERROR_CONNECT_FAILED
};

// IDs.
enum
{
	JTNET_TCPIP_ADDRESS,
	JTNET_TCPIP_PORT,
	JTNET_PHONE_NUMBER,
	JTNET_MODEM,
	JTNET_COMPORT,
	JTNET_BAUDRATE,
	JTNET_STOPBITS,
	JTNET_PARITY,
	JTNET_FLOWCONTROL,
	JTNET_NAME,				// Player name
	JTNET_SERVER_INFO,		// Game description

	JTNET_MY_PLAYER_ID,

	JTNET_MODEM_LIST,
	JTNET_PLAYER_NAME_LIST,

	JTNET_SERVER_DATA,		// Game specific server data.
	JTNET_PLAYER_DATA,		// Game specific player data.

	JTNET_PLAYER_CREATED_CALLBACK,
	JTNET_PLAYER_DESTROYED_CALLBACK,

	JTNET_SERVICE,
};


// Information about a server.
struct jtnetserver_t
{
	char	name[64];
	char	description[128];
	int		players, maxPlayers;
	int		canJoin;
	int		data[3];
};

struct jtnetplayer_t
{
	unsigned int	id;
	char			name[100];
};


int jtNetInit(int service);
void jtNetShutdown();
int jtNetOpenServer(char *serverName);
int jtNetCloseServer();
int jtNetLockServer(int yes);

int jtNetConnect(char *serverName);
int jtNetDisconnect();

int jtNetSend(int to, void *buffer, int size);

// Returns the number of bytes written to the buffer (zero if there
// was no message in the receive queue). The buffer must be big enough.
// 'from' will be filled with the player number (-1 if a system message).
int jtNetGet(int *from, void *buffer, unsigned long bufSize);

const char *jtNetGetString(int id);
int jtNetGetInteger(int id);
char **jtNetGetStringList(int id, int *num);
int jtNetGetServerInfo(jtnetserver_t *buffer, int numitems);
int jtNetGetPlayerIDs(unsigned int *list);

int jtNetSetString(int id, char *value);
int jtNetSetInteger(int id, int value);
int jtNetSetCallback(int id, void *ptr);

int jtNetSetData(int id, void *ptr, int size);
int jtNetGetData(int id, int index, void *ptr, int size);


#endif