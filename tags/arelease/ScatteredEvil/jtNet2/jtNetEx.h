// The exported functions of the jtNet DLL + other stuff.

#ifndef __JNET_EXP_H__
#define __JNET_EXP_H__

#ifdef JTNET2_EXPORTS
#define JTNET_API		__declspec(dllexport)
#else
#define JTNET_API		__declspec(dllimport)
#endif

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
	JTNET_ERROR_SERVICE_UNAVAILABLE,
	JTNET_ERROR_OPEN_SERVER,
	JTNET_ERROR_CREATE_PLAYER,
	JTNET_ERROR_SET_SERVER_PARAMS,
	JTNET_ERROR_CONNECTING,
	JTNET_ERROR_CONNECT_FAILED
};

// IDs.
enum
{
	JTNET_VERSION,
	JTNET_TCPIP_ADDRESS,
	JTNET_TCPIP_PORT,
	JTNET_PHONE_NUMBER,
	JTNET_MODEM,
	JTNET_COMPORT,
	JTNET_BAUDRATE,
	JTNET_STOPBITS,
	JTNET_PARITY,
	JTNET_FLOWCONTROL,
	JTNET_PLAYERS,			// Number of players
	JTNET_MAX_PLAYERS,
	JTNET_NAME,				// Player name
	JTNET_SERVER_INFO,		// Game description
	JTNET_SERVER_DATA1,
	JTNET_SERVER_DATA2,
	JTNET_SERVER_DATA3,

	JTNET_MY_PLAYER_NUMBER,	// Index in the player list.
	JTNET_MY_PLAYER_ID,

	JTNET_MODEM_LIST,
	JTNET_SERVER_NAME_LIST,
	JTNET_SERVER_INFO_LIST,
	JTNET_PLAYER_NAME_LIST,
	JTNET_PLAYER_LIST,

	JTNET_SERVER_DATA,		// Game specific server data.
	JTNET_PLAYER_DATA,		// Game specific player data.

	JTNET_PLAYER_CREATED_CALLBACK,
	JTNET_PLAYER_DESTROYED_CALLBACK,

	JTNET_SERVICE,

	// Special nodes for jtNetSend.
	JTNET_BROADCAST_NODE = -1,
	JTNET_SERVER_NODE = -2
};


// Information about a server.
typedef struct
{
	char	name[64];
	char	description[128];
	int		players, maxPlayers;
	int		canJoin;
	int		data[3];
} jtnetserver_t;

typedef struct 
{
	unsigned int	id;
	char			name[100];
} jtnetplayer_t;


JTNET_API int jtNetInit(int service);
JTNET_API void jtNetShutdown();
JTNET_API int jtNetOpenServer(char *serverName);
JTNET_API int jtNetCloseServer();
JTNET_API int jtNetLockServer(int yes);

JTNET_API int jtNetConnect(char *serverName);
JTNET_API int jtNetDisconnect();

//int jtNetChooseService();
//void jtNetSetMaxPlayers(int number);
//void jtNetSetSessionName(const char *txt);
//int jtNetSetup();
JTNET_API int jtNetSend(int to, void *buffer, int size);
JTNET_API int jtNetSendToID(int id, void *buffer, int size);

// Returns the number of bytes written to the buffer (zero if there
// was no message in the receive queue). The buffer must be big enough.
// 'from' will be filled with the player number (-1 if a system message).
JTNET_API int jtNetGet(int *from, void *buffer, unsigned long bufSize);

//int jtNetNumPlayers();
//unsigned int *jtNetGetPlayerIDs();
//unsigned int jtNetGetMyID();

//int jtNetAddSessionProperty(void *ptr, int bits);
//const char *jtNetGetVersion();

JTNET_API const char *jtNetGetString(int id);
JTNET_API int jtNetGetInteger(int id);
JTNET_API char **jtNetGetStringList(int id, int *num);
JTNET_API int jtNetGetServerInfo(jtnetserver_t *buffer, int numitems);
JTNET_API int jtNetGetPlayerIDs(unsigned int *list);

JTNET_API int jtNetSetString(int id, char *value);
JTNET_API int jtNetSetInteger(int id, int value);
JTNET_API int jtNetSetCallback(int id, void *ptr);

JTNET_API int jtNetSetData(int id, void *ptr, int size);
JTNET_API int jtNetGetData(int id, int index, void *ptr, int size);


#endif