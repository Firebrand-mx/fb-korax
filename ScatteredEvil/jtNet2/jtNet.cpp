// Only DirectX 6 compatible (needs DirectPlay4).

#include "jtNet.h"
#include "jtNetEx.h"

#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// {7DDFA9A0-84EA-11d3-B689-E29406BD95EC}
static const GUID GUID_jtNet = 
{ 0x7ddfa9a0, 0x84ea, 0x11d3, { 0xb6, 0x89, 0xe2, 0x94, 0x6, 0xbd, 0x95, 0xec } };


int jtValidateConnection();


// Here we go...

// Session data.
typedef struct
{
	void	*value;			// Pointer to the value.
	int		bitLen;			// Number of bits to store the value.
	int		bitOff;			// Bit offset in the storage dword.
//	DWORD	*storage;		// The storage dword.
	int		storageNum;		// The number of the storage dword.
} jtnetsd_t;


#define NUMSTORAGE	4

int				appMaxPlayers=8;		// Application-specific max plrs.
char			appGameName[100];		// App-specific default game name.
jtnetcon_t		*connections;
int				numConnections, selcon;
jtnetsession_t	*sessions;
int				numSessions;
bool			initOk = false;

// Is the connection init OK? Can be used to either force a new connection
// or just tell DP that the connection hasn't yet been initialized.
bool			connectionInitOk = false; 

LPDIRECTPLAY4A		dPlay = NULL;
LPDIRECTPLAYLOBBY3A	dPLobby = NULL;

HRESULT			hres;
DPID			thisPlrId;	// Id of the local player.
jtnetplayer_t	*players;
int				numPlayers;
jtnetsd_t		sesData[MAXSESSIONDATA];
int				numSesData;

FILE *debugfile = NULL;

int					numModems = 0;
char				**modemList = NULL;		// The list of modem names.
char				**serverNameList = NULL;// Num = numSessions
char				**serverInfoList = NULL;// Num = numSessions
char				**playerNameList = NULL;// Num = numPlayers

// Service provider configuration (for DP addresses).
char				tcpIpAddress[128];		// An address for TCP/IP.
WORD				tcpIpPort;
char				modemPhoneNum[80];		// The phone number for the modem.
int					modemWhich;				// The selected modem to use (index to the modem list).
DPCOMPORTADDRESS	serialPort;				// Data for the serial port.

// Server config.
char				sessionNameBuffer[512];	// Room for name + description.
int					maxPlayers = 8;
char				serverNameStr[100];
char				serverInfoStr[100];
DPSESSIONDESC2		serverSession;
char				playerName[100];		// Name of the local player (host or client).
int					serverData[3];			// Extra data for the server.

// Callback functions:
// This is called when a player is created in the session.
void (*PlayerCreatedCallback)(int newnode) = NULL;
// This is called when a player is destroyed in the session.
void (*PlayerDestroyedCallback)(jtnetplayer_t *plrdata) = NULL;

// This has been been CopyPasted from the DP examples...
char * GetDirectPlayErrStr(HRESULT hr)
{
	static char		szTempStr[12];

	switch (hr)
	{
	case DP_OK: return ("DP_OK");
	case DPERR_ALREADYINITIALIZED: return ("DPERR_ALREADYINITIALIZED");
	case DPERR_ACCESSDENIED: return ("DPERR_ACCESSDENIED");
	case DPERR_ACTIVEPLAYERS: return ("DPERR_ACTIVEPLAYERS");
	case DPERR_BUFFERTOOSMALL: return ("DPERR_BUFFERTOOSMALL");
	case DPERR_CANTADDPLAYER: return ("DPERR_CANTADDPLAYER");
	case DPERR_CANTCREATEGROUP: return ("DPERR_CANTCREATEGROUP");
	case DPERR_CANTCREATEPLAYER: return ("DPERR_CANTCREATEPLAYER");
	case DPERR_CANTCREATESESSION: return ("DPERR_CANTCREATESESSION");
	case DPERR_CAPSNOTAVAILABLEYET: return ("DPERR_CAPSNOTAVAILABLEYET");
	case DPERR_EXCEPTION: return ("DPERR_EXCEPTION");
	case DPERR_GENERIC: return ("DPERR_GENERIC");
	case DPERR_INVALIDFLAGS: return ("DPERR_INVALIDFLAGS");
	case DPERR_INVALIDOBJECT: return ("DPERR_INVALIDOBJECT");
//	case DPERR_INVALIDPARAM: return ("DPERR_INVALIDPARAM");	 dup value
	case DPERR_INVALIDPARAMS: return ("DPERR_INVALIDPARAMS");
	case DPERR_INVALIDPLAYER: return ("DPERR_INVALIDPLAYER");
	case DPERR_INVALIDGROUP: return ("DPERR_INVALIDGROUP");
	case DPERR_NOCAPS: return ("DPERR_NOCAPS");
	case DPERR_NOCONNECTION: return ("DPERR_NOCONNECTION");
//	case DPERR_NOMEMORY: return ("DPERR_NOMEMORY");		dup value
	case DPERR_OUTOFMEMORY: return ("DPERR_OUTOFMEMORY");
	case DPERR_NOMESSAGES: return ("DPERR_NOMESSAGES");
	case DPERR_NONAMESERVERFOUND: return ("DPERR_NONAMESERVERFOUND");
	case DPERR_NOPLAYERS: return ("DPERR_NOPLAYERS");
	case DPERR_NOSESSIONS: return ("DPERR_NOSESSIONS");
	case DPERR_PENDING: return ("DPERR_PENDING");
	case DPERR_SENDTOOBIG: return ("DPERR_SENDTOOBIG");
	case DPERR_TIMEOUT: return ("DPERR_TIMEOUT");
	case DPERR_UNAVAILABLE: return ("DPERR_UNAVAILABLE");
	case DPERR_UNSUPPORTED: return ("DPERR_UNSUPPORTED");
	case DPERR_BUSY: return ("DPERR_BUSY");
	case DPERR_USERCANCEL: return ("DPERR_USERCANCEL");
	case DPERR_NOINTERFACE: return ("DPERR_NOINTERFACE");
	case DPERR_CANNOTCREATESERVER: return ("DPERR_CANNOTCREATESERVER");
	case DPERR_PLAYERLOST: return ("DPERR_PLAYERLOST");
	case DPERR_SESSIONLOST: return ("DPERR_SESSIONLOST");
	case DPERR_UNINITIALIZED: return ("DPERR_UNINITIALIZED");
	case DPERR_NONEWPLAYERS: return ("DPERR_NONEWPLAYERS");
	case DPERR_INVALIDPASSWORD: return ("DPERR_INVALIDPASSWORD");
	case DPERR_CONNECTING: return ("DPERR_CONNECTING");
	case DPERR_CONNECTIONLOST: return ("DPERR_CONNECTIONLOST");
	case DPERR_UNKNOWNMESSAGE: return ("DPERR_UNKNOWNMESSAGE");
	case DPERR_CANCELFAILED: return ("DPERR_CANCELFAILED");
	case DPERR_INVALIDPRIORITY: return ("DPERR_INVALIDPRIORITY");
	case DPERR_NOTHANDLED: return ("DPERR_NOTHANDLED");
	case DPERR_CANCELLED: return ("DPERR_CANCELLED");
	case DPERR_ABORTED: return ("DPERR_ABORTED");
	case DPERR_BUFFERTOOLARGE: return ("DPERR_BUFFERTOOLARGE");
	case DPERR_CANTCREATEPROCESS: return ("DPERR_CANTCREATEPROCESS");
	case DPERR_APPNOTSTARTED: return ("DPERR_APPNOTSTARTED");
	case DPERR_INVALIDINTERFACE: return ("DPERR_INVALIDINTERFACE");
	case DPERR_NOSERVICEPROVIDER: return ("DPERR_NOSERVICEPROVIDER");
	case DPERR_UNKNOWNAPPLICATION: return ("DPERR_UNKNOWNAPPLICATION");
	case DPERR_NOTLOBBIED: return ("DPERR_NOTLOBBIED");
	case DPERR_SERVICEPROVIDERLOADED: return ("DPERR_SERVICEPROVIDERLOADED");
	case DPERR_ALREADYREGISTERED: return ("DPERR_ALREADYREGISTERED");
	case DPERR_NOTREGISTERED: return ("DPERR_NOTREGISTERED");
	case DPERR_AUTHENTICATIONFAILED: return ("DPERR_AUTHENTICATIONFAILED");
	case DPERR_CANTLOADSSPI: return ("DPERR_CANTLOADSSPI");
	case DPERR_ENCRYPTIONFAILED: return ("DPERR_ENCRYPTIONFAILED");
	case DPERR_SIGNFAILED: return ("DPERR_SIGNFAILED");
	case DPERR_CANTLOADSECURITYPACKAGE: return ("DPERR_CANTLOADSECURITYPACKAGE");
	case DPERR_ENCRYPTIONNOTSUPPORTED: return ("DPERR_ENCRYPTIONNOTSUPPORTED");
	case DPERR_CANTLOADCAPI: return ("DPERR_CANTLOADCAPI");
	case DPERR_NOTLOGGEDIN: return ("DPERR_NOTLOGGEDIN");
	case DPERR_LOGONDENIED: return ("DPERR_LOGONDENIED");
	}

	// For errors not in the list, return HRESULT string
	wsprintf(szTempStr, "0x%08X", hr);
	return (szTempStr);
}


BOOL FAR PASCAL EnumModemAddress(REFGUID lpguidDataType, DWORD dwDataSize,
								 LPCVOID lpData, LPVOID lpContext)
{
	LPSTR	lpszStr = (LPSTR) lpData;

	// modem
	if(!memcmp(&lpguidDataType, &DPAID_Modem, sizeof(GUID)))
	{
		// loop over all strings in list
		while(strlen(lpszStr))
		{
			// Store modem names in the modem list.
			modemList = (char**) realloc(modemList, sizeof(char*) * ++numModems);
			*(modemList+numModems-1) = (char*) malloc(strlen(lpszStr)+1);
			strcpy(*(modemList+numModems-1), lpszStr);

			// skip to next string
			lpszStr += strlen(lpszStr) + 1;
		}
	}

	return (TRUE);
}

void updateModemList()
{
	int					i;
	LPDIRECTPLAY		lpDPlay1 = NULL;
	LPDIRECTPLAY4A		lpDPlay4A = NULL;
	LPVOID				lpAddress = NULL;
	DWORD				dwAddressSize = 0;
	GUID				guidServiceProvider = DPSPGUID_MODEM;
	HRESULT				hr;

	// Update the list of modems.
	if(modemList) // Get rid of the old list.
	{
		for(i=0; i<numModems; i++) free(modemList[i]);
		free(modemList);
	}
	modemList = NULL;
	numModems = 0;
	modemWhich = -1;

	if FAILED(hr=DirectPlayCreate(&guidServiceProvider, &lpDPlay1, NULL))
	{
		/*if(debugfile) 
		{
			fprintf(debugfile, "errors: unavailable %x\n", DPERR_UNAVAILABLE);
			fprintf(debugfile, "directplaycreate failed (hr=%x)\n", hr);
		}*/
		goto FAILURE;
	}

	// Query for an ANSI DirectPlay4 interface.
	if FAILED(hr = lpDPlay1->QueryInterface(IID_IDirectPlay4A, (void**) &lpDPlay4A))
	{
		//if(debugfile) fprintf(debugfile, "queryinterface failed\n");
		goto FAILURE;
	}

	// Get size of player address for player zero.
	hr = lpDPlay4A->GetPlayerAddress(DPID_ALLPLAYERS, NULL, &dwAddressSize);
	if (hr != DPERR_BUFFERTOOSMALL)
	{
		//if(debugfile) fprintf(debugfile, "not too small buffer\n");
		goto FAILURE;
	}

	// Make room for it.
	lpAddress = malloc(dwAddressSize);
	if(lpAddress == NULL)
	{
		//if(debugfile) fprintf(debugfile, "could allow %d bytes of memory!\n", dwAddressSize);
		goto FAILURE;
	}

	// Get the address.
	if FAILED(lpDPlay4A->GetPlayerAddress(DPID_ALLPLAYERS, lpAddress, &dwAddressSize))
	{
		//if(debugfile) fprintf(debugfile, "getplayeraddress failed\n");
		goto FAILURE;
	}
	
	// Get modem strings from address and put them in the modem list.
	hr = dPLobby->EnumAddress(EnumModemAddress, lpAddress, dwAddressSize, NULL);
	if FAILED(hr)
		goto FAILURE;

	// Select the first modem.
	if(numModems) modemWhich = 0;

FAILURE:
	if(lpDPlay1) lpDPlay1->Release();
	if(lpDPlay4A) lpDPlay4A->Release();
	if(lpAddress) free(lpAddress);
}


BOOL FAR PASCAL connectionEnumerator(LPCGUID lpguidSP, LPVOID lpConnection,
									 DWORD dwConnectionSize, LPCDPNAME lpName,
									 DWORD dwFlags, LPVOID lpContext)
{
	// A new connection. Allocate memory for it.
	connections = (jtnetcon_t*) realloc(connections, sizeof(jtnetcon_t) * ++numConnections);
	jtnetcon_t *con = connections + numConnections-1;
	memcpy(&con->guid, lpguidSP, sizeof(con->guid));
	// The connection data.
	con->connection = malloc(dwConnectionSize);
	memcpy(con->connection, lpConnection, dwConnectionSize);
	con->size = dwConnectionSize;
	strncpy(con->name, lpName->lpszShortNameA, 100);

	// Identify the connection type.
	if(!memcmp(&con->guid, &DPSPGUID_TCPIP, sizeof(GUID)))
		con->type = JTNET_SERVICE_TCPIP;
	else if(!memcmp(&con->guid, &DPSPGUID_IPX, sizeof(GUID)))
		con->type = JTNET_SERVICE_IPX;
	else if(!memcmp(&con->guid, &DPSPGUID_SERIAL, sizeof(GUID)))
		con->type = JTNET_SERVICE_SERIAL;
	else if(!memcmp(&con->guid, &DPSPGUID_MODEM, sizeof(GUID)))
		con->type = JTNET_SERVICE_MODEM;
	else 
		con->type = JTNET_SERVICE_UNKNOWN;

	// Continue enumeration.
	return TRUE;
}

// Initialize the network. Returns 0 if everything goes all right.
__declspec(dllexport) int jtNetInit(int service)
{
	int		i;

	if(initOk) return JTNET_ERROR_ALREADY_INITIALIZED;

	// Initialize the COM.
	CoInitialize(NULL);

	// First we need to get a DirectPlay object.
	if(FAILED(CoCreateInstance(CLSID_DirectPlay, NULL,
		CLSCTX_INPROC_SERVER, IID_IDirectPlay4A, (void**) &dPlay)))
		return JTNET_ERROR_GENERIC;

	// Then create a lobby for DP address construction.
	if(FAILED(CoCreateInstance(CLSID_DirectPlayLobby, NULL, 
		CLSCTX_INPROC_SERVER, IID_IDirectPlayLobby3A, (void**) &dPLobby)))
		goto INITFAILURE;

	// Enumerate the service providers and init the requested one.
	connections = 0;
	numConnections = 0;
	selcon = -1;
	if(FAILED(dPlay->EnumConnections(&GUID_jtNet, connectionEnumerator, 0, 0)))
		return JTNET_ERROR_INIT_SERVICES;

	// Try to find the correct service type.		
	for(i=0; i<numConnections; i++)
		if(connections[i].type == service)
		{
			selcon = i;
			break;
		}
	// Not found?
	if(selcon == -1) return JTNET_ERROR_SERVICE_NOT_FOUND;

	strcpy(tcpIpAddress, "");
	tcpIpPort = 0;
	strcpy(modemPhoneNum, "");

	//debugfile = fopen("jtNet.out", "w");
	//if(!debugfile) return JTNET_ERROR_GENERIC;

	// Get the modem list, if necessary.
	if(service == JTNET_SERVICE_MODEM) 
	{
		//fprintf(debugfile, "Service: Modem => updating modem list\n");
		updateModemList();
	}

	//fprintf(debugfile, "This file contains jtNet debug run information:\n\n");

	// Set the COM port defaults.
	serialPort.dwComPort = 1;
	serialPort.dwBaudRate = CBR_57600;
	serialPort.dwStopBits = ONESTOPBIT;
	serialPort.dwParity = NOPARITY;
	serialPort.dwFlowControl = DPCPA_RTSDTRFLOW;

	strcpy(playerName, "");
	strcpy(sessionNameBuffer, "");
	strcpy(serverNameStr, "");
	strcpy(serverInfoStr, "");
	memset(serverData, 0, sizeof(serverData));

	connectionInitOk = false;

	// Callback functions.
	PlayerCreatedCallback = NULL;
	PlayerDestroyedCallback = NULL;

	// No sessions yet.
	sessions = 0;
	numSessions = 0;

	strcpy(appGameName, "Multiplayer Game");
	appMaxPlayers = 8;

	numSesData = 0;

	// We're done.
	initOk = true;

	// If we are using the IPX protocol we can start enumerating
	// sessions right away. No extra config needed, you see.
	if(service == JTNET_SERVICE_IPX)
	{
		jtValidateConnection();
		jtEnumerateSessions();
	}
	return JTNET_ERROR_OK;

INITFAILURE:
	dPlay->Release();
	dPlay = NULL;
	return JTNET_ERROR_GENERIC;
}

static void clearConnections()
{
	// Free the connection data.
	for(int i=0; i<numConnections; i++)
		free(connections[i].connection);
	free(connections);
	connections = 0;
	numConnections = 0;
}

static void clearSessions()
{
	free(sessions);
	sessions = NULL;
	numSessions = 0;
}

static void clearPlayers()
{
	free(players);
	players = NULL;
	numPlayers = 0;
}

__declspec(dllexport) void jtNetShutdown()
{
	int		i;

	if(!initOk) return;

	if(debugfile) fclose(debugfile);

	if(dPLobby) dPLobby->Release();
	if(dPlay) 
	{
		dPlay->Close();
		dPlay->Release();
	}

	dPLobby = NULL;
	dPlay = NULL;

	CoUninitialize();

	initOk = false;

	// The modem list.
	for(i=0; i<numModems; i++) free(modemList[i]);
	free(modemList);
	modemList = NULL;
	numModems = 0;

	// The server lists.
/*	for(i=0; i<numSessions; i++)
	{
		free(serverNameList[i]);
		free(serverInfoList[i]);
	}*/
	free(serverNameList);
	free(serverInfoList);
	free(playerNameList);
	serverNameList = NULL;
	serverInfoList = NULL;
	playerNameList = NULL;

	clearConnections();
	clearSessions();
	clearPlayers();
}

/*// The DirectX 3 compatible version.
BOOL FAR PASCAL connectionEnumerator(LPGUID lpSPGuid, LPTSTR lpszSPName, DWORD dwMajorVersion,
									 DWORD dwMinorVersion, LPVOID lpContext)
{
	// A new connection. Allocate memory for it.
	connections = (jtnetcon_t*) realloc(connections, sizeof(jtnetcon_t) * ++numConnections);
	jtnetcon_t *con = connections + numConnections-1;
	memcpy(&con->guid, lpSPGuid, sizeof(con->guid));
	// The connection data.
	con->connection = 0;//malloc(dwConnectionSize);
	//memcpy(con->connection, lpConnection, dwConnectionSize);
	//con->size = dwConnectionSize;
	strncpy(con->name, lpszSPName, 100);
	// Continue enumeration.
	return TRUE;
}
*/

BOOL FAR PASCAL sessionEnumerator(LPCDPSESSIONDESC2 lpThisSD, LPDWORD lpdwTimeOut, 
								  DWORD dwFlags, LPVOID lpContext)
{
	int		nameLen;

	if(dwFlags & DPESC_TIMEDOUT)
		return FALSE;	// Timed out...

	// A new sessions. Add it to the list.
	sessions = (jtnetsession_t*) realloc(sessions, sizeof(jtnetsession_t)* ++numSessions);
	jtnetsession_t *ses = sessions + numSessions-1;
	memcpy(&ses->desc, lpThisSD, sizeof(*lpThisSD));
	// 'Unpack' the name and info.
	nameLen = lpThisSD->dwUser1;	
	memcpy(ses->name, lpThisSD->lpszSessionNameA, nameLen);
	ses->name[nameLen] = 0;
	strcpy(ses->info, lpThisSD->lpszSessionNameA+nameLen);
	ses->desc.lpszSessionNameA = ses->name;
	return TRUE;
}

BOOL FAR PASCAL playerEnumerator(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, 
								 DWORD dwFlags, LPVOID lpContext)
{
	if(dwPlayerType == DPPLAYERTYPE_PLAYER)
	{
		players = (jtnetplayer_t*) realloc(players, sizeof(jtnetplayer_t) * ++numPlayers);
		jtnetplayer_t *plr = players + numPlayers-1;
		memcpy(&plr->id, &dpId, sizeof(dpId));	
		strcpy(plr->name, lpName->lpszShortNameA);
	}
	return TRUE;
}

void copyBits(void *src, int srcOff, void *dst, int dstOff, int bits)
{
	int		srcmaxbits = srcOff + bits, dstmaxbits = dstOff + bits;
	int		i, data, mask;

	// Construct the bit mask.
	for(i=0, mask=0; i<bits; i++) mask |= 1 << (i+srcOff);
	if(srcmaxbits <= 8)
		data = *(char*)src & mask >> srcOff;
	else if(srcmaxbits <= 16)
		data = *(short*)src & mask >> srcOff;
	else
		data = *(int*)src & mask >> srcOff;
	// Now we have the data.
	// Construct the destination mask.
	for(i=0, mask=0; i<bits; i++) mask |= 1 << (i+dstOff);
	if(dstmaxbits <= 8)
	{
		*(char*)dst &= ~mask;
		*(char*)dst |= data << dstOff;
	}
	else if(dstmaxbits <= 16)
	{
		*(short*)dst &= ~mask;
		*(short*)dst |= data << dstOff;
	}
	else
	{
		*(int*)dst &= ~mask;
		*(int*)dst |= data << dstOff;
	}
	//printf( "copyBits (%p -> %p) : %d (%x)\n", src, dst, data, data);
}


// Returns true for successful transmission. 'to' is a plr #.
//--// NOTICE: Currently sending is synchronous: the function doesn't return
//--// until the message has been sent.
__declspec(dllexport) int jtNetSend(int to, void *buffer, int size)
{
	DPID dest;

	if(dPlay == NULL) return 0;

	// Check the destination.
	switch(to)
	{
	case JTNET_BROADCAST_NODE:	// Broadcast to everybody?
		if(debugfile) fprintf(debugfile, "Broadcast.\n");
		dest = DPID_ALLPLAYERS;
		break;

	case JTNET_SERVER_NODE:	// Send to the serverplayer?
		if(debugfile) fprintf(debugfile, "Server packet.\n");
		dest = DPID_SERVERPLAYER;
		break;

	default:
		if(debugfile) fprintf(debugfile, "Message to node %d.\n", to);
		dest = players[to].id;
		break;
	}

	return SUCCEEDED(dPlay->Send(thisPlrId, dest, 0, buffer, size));
	/*if((hres=dPlay->SendEx(thisPlrId, dest, DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG,
		buffer, size, 0, 0, NULL, NULL)) != DPERR_PENDING)*/
//		return 0;
	//return 1;
}

__declspec(dllexport) int jtNetSendToID(int id, void *buffer, int size)
{
	return SUCCEEDED(dPlay->Send(thisPlrId, id, 0, buffer, size));
}

__declspec(dllexport) int jtPlrNum(DPID id)
{
	for(int i=0; i<numPlayers; i++)
		if(players[i].id == id) return i;
	return -1;
}

__declspec(dllexport) void jtSysMsgHandler(DPMSG_GENERIC *msg)
{
	DPMSG_CREATEPLAYERORGROUP	*msgNew;
	DPMSG_DESTROYPLAYERORGROUP	*msgDel;
	DPMSG_SETSESSIONDESC		*msgDesc;
//	char						buffer[100];
	jtnetplayer_t				dummy;

	switch(msg->dwType)
	{
	case DPSYS_CREATEPLAYERORGROUP:		// A new player has joined?
		msgNew = (DPMSG_CREATEPLAYERORGROUP*) msg;
		
		jtEnumeratePlayers();
		
		// Call the callback, if one is set.
		if(PlayerCreatedCallback)
			PlayerCreatedCallback(jtPlrNum(msgNew->dpId));
		break;

	case DPSYS_DESTROYPLAYERORGROUP:	// A player has disconnected?
		msgDel = (DPMSG_DESTROYPLAYERORGROUP*) msg;

		// Call the callback, if one is set.
		memcpy(&dummy, players + jtPlrNum(msgDel->dpId), sizeof(jtnetplayer_t));
		if(PlayerDestroyedCallback)
			PlayerDestroyedCallback(&dummy);
		
		// Get the old index first.
//		strcpy(buffer, players[jtPlrNum(msgDel->dpId)].name);

		// Update the list of players.
		jtEnumeratePlayers();
		break;

	case DPSYS_SETPLAYERORGROUPNAME:	// A player has been renamed?
		break;

	case DPSYS_SETPLAYERORGROUPDATA:	// A player's data has been changed?
		break;

	case DPSYS_SETSESSIONDESC:			// The session's parameters have changed.
		msgDesc = (DPMSG_SETSESSIONDESC*) msg;
		memcpy(&serverSession, &msgDesc->dpDesc, sizeof(serverSession));
		serverData[0] = serverSession.dwUser2;
		serverData[1] = serverSession.dwUser3;
		serverData[2] = serverSession.dwUser4;
		break;

	case DPSYS_SESSIONLOST:				// The session has been lost?!
		break;

	default:	
		// Other messages are just ignored (they shouldn't be sent,
		// though).
		break;
	}
}

// Returns the number of bytes written to the buffer (zero if there
// was no message in the receive queue). The buffer must be big enough.
// From will be filled with the player number (-1 if a system message).
__declspec(dllexport) int jtNetGet(int *from, void *buffer, unsigned long bufSize)
{
	DPID	fromId, toId;

	if(dPlay == NULL) return 0;

	if(FAILED(hres=dPlay->Receive(&fromId, &toId, 0, buffer, &bufSize)))
		return 0;

	// Is it a system message?
	if(fromId == DPID_SYSMSG) 
	{
		jtSysMsgHandler( (DPMSG_GENERIC*) buffer);
		*from = -1;
		return 0; // The message isn't given to the application.
	}

	*from = jtPlrNum(fromId);
	return bufSize;
}

// Return true for 'all clear'.
/*int jtBeginSession()
{
	char	msg[1024];	// The message buffer.
	bool	*gotAck = new bool[numPlayers];
	int		i, done = false, msgFrom;
	int		*msgInt;

	// We haven't gotten acknowledgements from anybody yet.
	memset(gotAck, 0, numPlayers);
	// We of course acknowledge ourselves right away.
	gotAck[jtPlrNum(thisPlrId)] = true;

	while(!done)
	{
		// Send the 'begin session' message to all players.
		for(i=0; i<numPlayers; i++)
		{
			if(gotAck[i]) continue; // This player has already answered.
			msgInt = (int*) msg;
			*msgInt = JTNET_MSG_BEGIN_SESSION;
			jtNetSend(i, msgInt, 4); // Int is 4 bytes.
		}
		// Check the received messages.
		while(i=jtNetGet(&msgFrom, msg, 1024))
		{
			if(i == 4)
			{
				if(msgFrom == -1) return 0;	// What what whaaat!?
				msgInt = (int*) msg;
				if(*msgInt == JTNET_MSG_OK) gotAck[msgFrom] = true;
			}
		}
		
		// Check are we done.
		for(done=true, i=0; i<numPlayers && done; i++)
			if(!gotAck[i]) done = false;
	}
	delete [] gotAck;
	// Now we are all clear.
	return 1;
}*/

// We will wait until we receive a message that says the session begins.
// Then we will reply.
/*int jtAcknowledgeBegin()
{
	char	msg[1024];	// The message buffer.
	int		i, *msgInt, msgFrom;

	for(;;)
	{
		// While we wait, we can enumerate the players.
		jtEnumeratePlayers();
		
		// Check the messages.
		if(jtNetGet(&msgFrom, msg, 1024) == 4)
		{
			msgInt = (int*) msg;
			if(msgFrom == -1) continue;
			if(*msgInt == JTNET_MSG_BEGIN_SESSION)
			{
				*msgInt = JTNET_MSG_OK;
				// Send three times to be sure.
				for(i=0; i<3; i++) jtNetSend(msgFrom, msg, 4);
				break;
			}
			if(*msgInt == JTNET_MSG_CANCEL) // Oh?
			{
				// Hot damn!
				return 0;
			}
		}
	}
	return 1;
}*/

/*#ifdef JTNET_DX3
// This is the DX3 compatible way to create a DPlay2 interface.
HRESULT CreateDirectPlayInterface(LPGUID lpguidServiceProvider,
								  LPDIRECTPLAY2A *lplpDirectPlay2A)
{
	LPDIRECTPLAY   lpDirectPlay1 = NULL;
	LPDIRECTPLAY2A lpDirectPlay2A = NULL;
	HRESULT        hr;

	// Retrieve a DirectPlay 1.0 interface.
	hr = DirectPlayCreate(lpguidServiceProvider, &lpDirectPlay1, NULL);
	if(FAILED(hr)) goto FAILURE;

	// Query for an ANSI DirectPlay2 interface.
	hr = lpDirectPlay1->QueryInterface(IID_IDirectPlay2A, (LPVOID *) &lpDirectPlay2A);
	if(FAILED(hr)) goto FAILURE;

	// Return the created interface.
	*lplpDirectPlay2A = lpDirectPlay2A;

FAILURE:
	if(lpDirectPlay1) lpDirectPlay1->Release();
	return hr;
}
#endif*/

// Opens up a dialog box listing all the available DirectPlay service 
// providers. Returns non-zero if successful.
/*int jtNetChooseService()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState()); // Must please MFC.

	CServiceDlg dlg;

	selcon = -1;
	clearConnections();

	// Enumerate the connections.
#ifndef JTNET_DX3
	if(FAILED(dPlay->EnumConnections(&GUID_jtNet, connectionEnumerator, 0, 0)))
#else
	if(DirectPlayEnumerate(connectionEnumerator, NULL) != DP_OK)
#endif
		return 0;

	if(dlg.DoModal() == IDOK)
	{
		// We have a selection.
		selcon = dlg.m_listSel;

		if(selcon < 0) return 0;	// Hey?

		// Initialize the connection.
#ifndef JTNET_DX3
		hres = dPlay->InitializeConnection(connections[selcon].connection, 0);
#else
		hres = CreateDirectPlayInterface(&connections[selcon].guid, &dPlay);
#endif
		
		if(FAILED(hres))// && hres != DPERR_ALREADYINITIALIZED)
		{
			char buff[100];
			strcpy(buff, "Error initializing service: ");
			if(hres == DPERR_INVALIDFLAGS) strcat(buff, "Invalid flags");
			else if(hres == DPERR_INVALIDPARAMS) strcat(buff, "Invalid params");
			else if(hres == DPERR_UNAVAILABLE) strcat(buff, "Unavailable");
			else sprintf(buff, "Error initializing service: Error 0x%x", hres);
			MessageBox(AfxGetMainWnd()->m_hWnd, buff, "Error", MB_OK|MB_ICONERROR);
			return 0;
		}
		// We're successful.
		return 1;
	}
	return 0;	// Uh?
}

void sesDataTransfer(DPSESSIONDESC2 *sd, bool write)
{
	DWORD	*stPtr[NUMSTORAGE] = { &sd->dwUser1, &sd->dwUser2, &sd->dwUser3, &sd->dwUser4 };
	int		i;

//	printf( "sesDataTransfer (write: %d):\n", write);
	for(i=0; i<numSesData; i++)
	{
		jtnetsd_t *data = sesData + i;
		if(write)
			copyBits(data->value, 0, stPtr[data->storageNum], data->bitOff, data->bitLen);
		else
			copyBits(stPtr[data->storageNum], data->bitOff, data->value, 0, data->bitLen);
	}
}
*/

// Return true if successful.
int jtEnumerateSessions()
{
	DPSESSIONDESC2 sd;

	clearSessions();
	memset(&sd, 0, sizeof(sd));
	sd.dwSize = sizeof(sd);
	sd.guidApplication = GUID_jtNet;
	
	hres = dPlay->EnumSessions(&sd, 0, sessionEnumerator, 0, 
		DPENUMSESSIONS_ALL | DPENUMSESSIONS_ASYNC |
		DPENUMSESSIONS_RETURNSTATUS);

	if(debugfile) fprintf(debugfile, "jtEnumerateSessions: %s\n", GetDirectPlayErrStr(hres));

	if(hres == DPERR_CONNECTING) return JTNET_ERROR_CONNECTING;

	if FAILED(hres) 
		return JTNET_ERROR_GENERIC;

	return JTNET_ERROR_OK;
}

int playerSorter(const void *e1, const void *e2)
{
	jtnetplayer_t *plr1 = (jtnetplayer_t*) e1, *plr2 = (jtnetplayer_t*) e2;
	if(plr1->id < plr2->id) return -1;
	if(plr1->id > plr2->id) return 1;
	return 0;
}

// Don't call this after the session has been started!
// Doesn't necessarily preserve player order.
int jtEnumeratePlayers()
{
	clearPlayers();

	if(FAILED(hres=dPlay->EnumPlayers(NULL, playerEnumerator, 0, 0)))
		return 0;

	// Sort the list so that thisPlrId is first. 
	// Console player is always node #0, remember?
	for(int i=0; i<numPlayers; i++)
		if(players[i].id == thisPlrId)
		{
			jtnetplayer_t temp;
			memcpy(&temp, players, sizeof(jtnetplayer_t));
			memcpy(players, players+i, sizeof(jtnetplayer_t));
			memcpy(players+i, &temp, sizeof(jtnetplayer_t));
			break;
		}
	return 1;
}

__declspec(dllexport) int jtNetNumPlayers()
{
	return numPlayers;
}

__declspec(dllexport) unsigned int jtNetGetMyID()
{
	return thisPlrId;
}

// Returns the pointer to a copy of the player id list.
__declspec(dllexport) int jtNetGetPlayerIDs(unsigned int *list)
{
	if(dPlay == NULL) return 0;

	/*unsigned int *list = (unsigned int*) malloc(numPlayers*sizeof(int));
	for(int i=0; i<numPlayers; i++) list[i] = players[i].id;
	return list;*/
	jtEnumeratePlayers();
	
	for(int i=0; i<numPlayers; i++) list[i] = players[i].id;
	return numPlayers;
}

__declspec(dllexport) void jtNetSetMaxPlayers(int number)
{
	appMaxPlayers = number;
}

__declspec(dllexport) void jtNetSetServerName(const char *txt)
{
	/*strncpy(appGameName, txt, 100);
	appGameName[99] = 0;*/
}

// Won't return NULL.
__declspec(dllexport) const char *jtNetGetString(int strid)
{
	switch(strid)
	{
	case JTNET_VERSION:
		return JTNET_VERSION_FULL;
	case JTNET_TCPIP_ADDRESS:
		return tcpIpAddress;
	case JTNET_PHONE_NUMBER:
		return modemPhoneNum;
	case JTNET_SERVER_INFO:
		return serverInfoStr;
	case JTNET_NAME:
		return playerName;
	case JTNET_MODEM:
		// Return the name of the modem from the modem name list.
		if(modemWhich < 0 || modemWhich > numModems-1) return ""; // Not available.
		return modemList[modemWhich];
	}
	return "";
}

__declspec(dllexport) int jtNetSetString(int strid, char *value)
{
	switch(strid)
	{
	case JTNET_TCPIP_ADDRESS:
		strcpy(tcpIpAddress, value);
		break;
	case JTNET_PHONE_NUMBER:
		strcpy(modemPhoneNum, value);
		break;
	case JTNET_SERVER_INFO:
		strcpy(serverInfoStr, value);
		break;
	case JTNET_NAME:
		strcpy(playerName, value);
		break;
	default:
		return false;
	}
	return true;
}

__declspec(dllexport) int jtNetSetInteger(int intid, int value)
{
	int stopBits[3] = { ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS };
	int parity[4] = { NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY };
	int flowCtrl[5] = { DPCPA_NOFLOW, DPCPA_XONXOFFFLOW, DPCPA_RTSFLOW, 
		DPCPA_DTRFLOW, DPCPA_RTSDTRFLOW };

	switch(intid)
	{
	case JTNET_TCPIP_PORT:
		tcpIpPort = value;
		break;
	case JTNET_MODEM:
		if(value < 0 || value > numModems-1) return false;
		modemWhich = value;
		break;
	case JTNET_COMPORT:
		serialPort.dwComPort = value;
		break;
	case JTNET_BAUDRATE:
		// No checks are made as to the validity of the value.
		serialPort.dwBaudRate = value;
		break;
	case JTNET_STOPBITS:
		if(value < 0 || value > 2) return false;
		serialPort.dwStopBits = stopBits[value];
		break;
	case JTNET_PARITY:
		if(value < 0 || value > 3) return false;
		serialPort.dwParity = parity[value];
		break;
	case JTNET_FLOWCONTROL:
		if(value < 0 || value > 4) return false;
		serialPort.dwFlowControl = flowCtrl[value];
		break;
	case JTNET_MAX_PLAYERS:
		if(value <= 0) return false;
		maxPlayers = value;
		break;
	case JTNET_SERVER_DATA1:
		serverData[0] = value;
		break;
	case JTNET_SERVER_DATA2:
		serverData[1] = value;
		break;
	case JTNET_SERVER_DATA3:
		serverData[2] = value;
		break;
	default:
		return false;
	}
	return true;
}

__declspec(dllexport) int jtNetGetInteger(int id)
{
	switch(id)
	{
	case JTNET_SERVICE:
		if(!initOk || selcon == -1) return JTNET_SERVICE_UNKNOWN;
		return connections[selcon].type;
	case JTNET_TCPIP_PORT:
		return tcpIpPort;
	case JTNET_MODEM:
		return modemWhich;
	case JTNET_COMPORT:
		return serialPort.dwComPort;
	case JTNET_BAUDRATE:
		return serialPort.dwBaudRate;
	case JTNET_STOPBITS:
		return serialPort.dwStopBits;
	case JTNET_PARITY:
		return serialPort.dwParity;
	case JTNET_FLOWCONTROL:
		return serialPort.dwFlowControl;
	case JTNET_PLAYERS:
		return numPlayers;
	case JTNET_MAX_PLAYERS:
		return maxPlayers;
	case JTNET_SERVER_DATA1:
	case JTNET_SERVER_DATA2:
	case JTNET_SERVER_DATA3:
		return serverData[id-JTNET_SERVER_DATA1];

	case JTNET_MY_PLAYER_NUMBER:
		return jtPlrNum(thisPlrId);

	case JTNET_MY_PLAYER_ID:
		return thisPlrId;

	default:
		return false;
	}
	return 0;
}

__declspec(dllexport) char **jtNetGetServerNameList(int *num)
{
	int		i;	

	if(dPlay == NULL) return NULL;

	// We need to first enumerate the servers!
	if(jtEnumerateSessions() == JTNET_ERROR_CONNECTING)
	{
		if(num) *num = -1;	// To denote that the list is being received.
		return NULL;
	}

	// Maintain the name list.
	/*if(serverNameList)
	{
		//for(i=0; i<numSessions; i++) free(serverNameList[i]);
		free(serverNameList);
	}
	serverNameList = (char**) malloc(sizeof(char*) * numSessions);*/

	serverNameList = (char**) realloc(serverNameList, sizeof(char*) * numSessions);

	for(i=0; i<numSessions; i++)
	{
		/*serverNameList[i] = (char*) malloc(strlen(sessions[i].name)+1);
		strcpy(serverNameList[i], sessions[i].name);*/
		serverNameList[i] = sessions[i].name;
	}
	if(num) *num = numSessions;
	return serverNameList;
}

__declspec(dllexport) char **jtNetGetServerInfoList(int *num)
{
	int		i;

	if(dPlay == NULL) return NULL;

	// We need to first enumerate the servers!
	if(jtEnumerateSessions() == JTNET_ERROR_CONNECTING)
	{
		if(num) *num = -1;	// To denote that the list is being received.
		return NULL;
	}

	// Maintain the name list.
	/*if(serverInfoList)
	{
		//for(i=0; i<numSessions; i++) free(serverInfoList[i]);
		free(serverInfoList);
	}*/
	//serverInfoList = (char**) malloc(sizeof(char*) * numSessions);
	serverInfoList = (char**) realloc(serverInfoList, sizeof(char*) * numSessions);
	for(i=0; i<numSessions; i++)
	{
		/*serverInfoList[i] = (char*) malloc(strlen(sessions[i].info)+1);
		strcpy(serverInfoList[i], sessions[i].info);*/

		serverInfoList[i] = sessions[i].info;
	}
	if(num) *num = numSessions;
	return serverInfoList;
}

__declspec(dllexport) char **jtNetGetPlayerNameList(int *num)
{
	int		i;

	if(dPlay == NULL) return NULL;

	// We need to first enumerate the players!
	jtEnumeratePlayers();

	// Maintain the name list.
/*	if(playerNameList) free(playerNameList);
	playerNameList = (char**) malloc(sizeof(char*) * numPlayers);*/

	playerNameList = (char**) realloc(playerNameList, sizeof(char*) * numPlayers);
	
	for(i=0; i<numPlayers; i++)
		playerNameList[i] = players[i].name;

	if(num) *num = numPlayers;
	return playerNameList;
}

__declspec(dllexport) char **jtNetGetStringList(int id, int *num)
{
	switch(id)
	{
	case JTNET_MODEM_LIST:
		if(num) *num = numModems;
		return modemList;

	case JTNET_SERVER_NAME_LIST:
		return jtNetGetServerNameList(num);

	case JTNET_SERVER_INFO_LIST:
		return jtNetGetServerInfoList(num);

	case JTNET_PLAYER_NAME_LIST:
		return jtNetGetPlayerNameList(num);

	default:
		break;
	}
	return NULL;
}

HRESULT CreateServiceProviderAddress(LPVOID *lplpAddress, LPDWORD lpdwAddressSize)
{
	DPCOMPOUNDADDRESSELEMENT	addressElements[3];
	LPVOID						lpAddress = NULL;
	DWORD						dwAddressSize = 0;
	DWORD						dwElementCount;
	GUID						guidServiceProvider;
	HRESULT 					hr;
	int							service = connections[selcon].type;

	// get currently selected service provider
	memcpy(&guidServiceProvider, &connections[selcon].guid, sizeof(GUID));

	dwElementCount = 0;

	if(service == JTNET_SERVICE_MODEM)
	{
		// Modem needs a service provider, a phone number string and a modem string

		// service provider
		addressElements[dwElementCount].guidDataType = DPAID_ServiceProvider;
		addressElements[dwElementCount].dwDataSize = sizeof(GUID);
		addressElements[dwElementCount].lpData = (LPVOID) &DPSPGUID_MODEM;
		dwElementCount++;

		// add a modem string if available
		addressElements[dwElementCount].guidDataType = DPAID_Modem;
		addressElements[dwElementCount].dwDataSize = lstrlen(modemList[modemWhich]) + 1;
		addressElements[dwElementCount].lpData = modemList[modemWhich];
		dwElementCount++;

		// add phone number string
		addressElements[dwElementCount].guidDataType = DPAID_Phone;
		addressElements[dwElementCount].dwDataSize = lstrlen(modemPhoneNum) + 1;
		addressElements[dwElementCount].lpData = modemPhoneNum;
		dwElementCount++;
	}

	// internet TCP/IP service provider
	else if(service == JTNET_SERVICE_TCPIP)
	{
		// TCP/IP needs a service provider, an IP address, and optional port #

		// service provider
		addressElements[dwElementCount].guidDataType = DPAID_ServiceProvider;
		addressElements[dwElementCount].dwDataSize = sizeof(GUID);
		addressElements[dwElementCount].lpData = (LPVOID) &DPSPGUID_TCPIP;
		dwElementCount++;

		// IP address string
		addressElements[dwElementCount].guidDataType = DPAID_INet;
		addressElements[dwElementCount].dwDataSize = lstrlen(tcpIpAddress) + 1;
		addressElements[dwElementCount].lpData = tcpIpAddress;
		dwElementCount++;

		// Optional Port number
		if(tcpIpPort > 0)
		{
			addressElements[dwElementCount].guidDataType = DPAID_INetPort;
			addressElements[dwElementCount].dwDataSize = sizeof(WORD);
			addressElements[dwElementCount].lpData = &tcpIpPort;
			dwElementCount++;
		}
	}

	// IPX service provider
	else if(service == JTNET_SERVICE_IPX)
	{
		// IPX just needs a service provider

		// service provider
		addressElements[dwElementCount].guidDataType = DPAID_ServiceProvider;
		addressElements[dwElementCount].dwDataSize = sizeof(GUID);
		addressElements[dwElementCount].lpData = (LPVOID) &DPSPGUID_IPX;
		dwElementCount++;
	}

	// Serial link
	else if(service == JTNET_SERVICE_SERIAL)
	{
		// service provider
		addressElements[dwElementCount].guidDataType = DPAID_ServiceProvider;
		addressElements[dwElementCount].dwDataSize = sizeof(GUID);
		addressElements[dwElementCount].lpData = (LPVOID) &DPSPGUID_SERIAL;
		dwElementCount++;

		addressElements[dwElementCount].guidDataType = DPAID_ComPort;
		addressElements[dwElementCount].dwDataSize = sizeof(DPCOMPORTADDRESS);
		addressElements[dwElementCount].lpData = (LPVOID) &serialPort;
		dwElementCount++;
	}

	// anything else, let service provider collect settings, if any
	else
	{
		// service provider
		addressElements[dwElementCount].guidDataType = DPAID_ServiceProvider;
		addressElements[dwElementCount].dwDataSize = sizeof(GUID);
		addressElements[dwElementCount].lpData = (LPVOID) &guidServiceProvider;
		dwElementCount++;
	}

	// see how much room is needed to store this address
	hr = dPLobby->CreateCompoundAddress(addressElements, dwElementCount, 
		NULL, &dwAddressSize);
	if (hr != DPERR_BUFFERTOOSMALL)
		goto FAILURE;

	// allocate space
	lpAddress = malloc(dwAddressSize);
	if (lpAddress == NULL)
	{
		hr = DPERR_NOMEMORY;
		goto FAILURE;
	}

	// create the address
	hr = dPLobby->CreateCompoundAddress(addressElements, dwElementCount, 
		lpAddress, &dwAddressSize);
	if FAILED(hr)
		goto FAILURE;

	// return the address info
	*lplpAddress = lpAddress;
	*lpdwAddressSize = dwAddressSize;

	return (DP_OK);

FAILURE:
	if (lpAddress) free(lpAddress);

	return (hr);
}


int jtValidateConnection()
{
	DWORD			addressSize = 0;
	LPVOID			lpAddress = NULL;
	int				ret = JTNET_ERROR_OK;
	
	// If the interface already exists, destroy it.
	if(dPlay)
	{
		if(!connectionInitOk)
		{
			dPlay->Close();
			dPlay->Release();
			dPlay = NULL;
		}
		else return JTNET_ERROR_OK;
	}

	if(FAILED(CoCreateInstance(CLSID_DirectPlay, NULL, 
		CLSCTX_INPROC_SERVER, IID_IDirectPlay4A, (void**) &dPlay)))
		return JTNET_ERROR_GENERIC;

	// We need the DP address that defines our service provider.
	if(FAILED(CreateServiceProviderAddress(&lpAddress, &addressSize)))
		return JTNET_ERROR_GENERIC;

	// Initialize the connection with the DP address.
	if(FAILED(dPlay->InitializeConnection(lpAddress, 0)))
		ret = JTNET_ERROR_INIT_SERVICES;

	connectionInitOk = true;
	free(lpAddress);
	return ret;
}

// Open a session and create the server player.
__declspec(dllexport) int jtNetOpenServer(char *serverName)
{
	DPSESSIONDESC2	*sd;
	
	if(jtValidateConnection() != JTNET_ERROR_OK)
		return JTNET_ERROR_GENERIC;

	strcpy(serverNameStr, serverName);

	// Open a session and create the serverplayer.
	sd = &serverSession;
	memset(sd, 0, sizeof(*sd));
	sd->dwSize = sizeof(*sd);
	sd->dwFlags = DPSESSION_KEEPALIVE | DPSESSION_OPTIMIZELATENCY | DPSESSION_MULTICASTSERVER;
	sd->guidApplication = GUID_jtNet;
	sd->dwMaxPlayers = maxPlayers;
	sd->dwUser2 = serverData[0];
	sd->dwUser3 = serverData[1];
	sd->dwUser4 = serverData[2];
	// Let's compose the server name / description.
	sd->dwUser1 = strlen(serverNameStr);
	strcpy(sessionNameBuffer, serverNameStr);
	strcat(sessionNameBuffer, serverInfoStr);
	sd->lpszSessionNameA = sessionNameBuffer;
	
	// Open the session. Really need to check for connecting?
	while((hres=dPlay->Open(sd, DPOPEN_CREATE | DPOPEN_RETURNSTATUS)) == DPERR_CONNECTING);
	
	if(hres != DP_OK) return JTNET_ERROR_OPEN_SERVER;

	// Now we have a session. Create the serverplayer.
	DPNAME dpName;
	memset(&dpName, 0, sizeof(dpName));
	dpName.dwSize = sizeof(dpName);
	dpName.lpszShortNameA = dpName.lpszLongNameA = playerName;
	
	if(FAILED(dPlay->CreatePlayer(&thisPlrId, &dpName, NULL, NULL, 0, DPPLAYER_SERVERPLAYER)))
		return JTNET_ERROR_CREATE_PLAYER;

	// Now the server is open.
	return JTNET_ERROR_OK;			
}

__declspec(dllexport) int jtNetLockServer(int yes)
{
	if(dPlay == NULL) return JTNET_ERROR_GENERIC;

	if(yes) // Server locked!
		serverSession.dwFlags |= DPSESSION_JOINDISABLED;
	else // Open it.
		serverSession.dwFlags &= ~DPSESSION_JOINDISABLED;

	if(FAILED(dPlay->SetSessionDesc(&serverSession, 0)))
		return JTNET_ERROR_SET_SERVER_PARAMS;

	return JTNET_ERROR_OK;
}

__declspec(dllexport) int jtNetCloseServer()
{
	if(!dPlay) return JTNET_ERROR_GENERIC;

	if(FAILED(dPlay->Close())) return JTNET_ERROR_GENERIC;

	// Force a new connection.
	connectionInitOk = false;

	return JTNET_ERROR_OK;
}

__declspec(dllexport) jtnetsession_t *jtGetSession(char *name)
{
	if(dPlay == NULL) return NULL;

	for(int i=0; i<numSessions; i++)
	{
		if(!stricmp(sessions[i].name, name))
			return sessions + i;
	}
	return NULL;
}

// Connects to the specified server as a client.
__declspec(dllexport) int jtNetConnect(char *serverName)
{
	jtnetsession_t *sdata; //= jtGetSession(serverName);

	if(jtValidateConnection() != JTNET_ERROR_OK)
		return JTNET_ERROR_GENERIC;

	sdata = jtGetSession(serverName);
	if(!sdata) return JTNET_ERROR_GENERIC;
	
	// Try to open the session (connect to the server).
	while( (hres=dPlay->Open(&sdata->desc, DPOPEN_JOIN | DPOPEN_RETURNSTATUS))
		== DPERR_CONNECTING);
	if(hres != DP_OK) return JTNET_ERROR_CONNECT_FAILED;

	// Take a copy of the session data.
	memcpy(&serverSession, &sdata->desc, sizeof(sdata->desc));
	serverData[0] = sdata->desc.dwUser2;
	serverData[1] = sdata->desc.dwUser3;
	serverData[2] = sdata->desc.dwUser4;
	
	// Try to create a player.
	DPNAME dpName;
	memset(&dpName, 0, sizeof(dpName));
	dpName.dwSize = sizeof(dpName);
	dpName.lpszShortNameA = dpName.lpszLongNameA = playerName;
	
	if(FAILED(dPlay->CreatePlayer(&thisPlrId, &dpName, NULL, NULL, 0, 0)))
	{
		// Close the opened session.
		dPlay->Close();
		return JTNET_ERROR_CREATE_PLAYER;
	}

	jtEnumeratePlayers();

	// We seem to be successful.
	return JTNET_ERROR_OK;
}

__declspec(dllexport) int jtNetDisconnect()
{
	if(!dPlay) return JTNET_ERROR_GENERIC;
	if(FAILED(dPlay->Close())) return JTNET_ERROR_GENERIC;

	dPlay->Release();
	dPlay = NULL;

	return JTNET_ERROR_OK;
}

__declspec(dllexport) int jtNetGetServerInfo(jtnetserver_t *buffer, int numitems)
{
	int		i;

	if(debugfile) fprintf(debugfile, "jtNetGetServerInfo:\n");

	if(jtValidateConnection() != JTNET_ERROR_OK) 
	{
		if(debugfile) fprintf(debugfile, "- connection wasn't validated\n");
		return 0;
	}

	if(debugfile) fprintf(debugfile, "- enumerating sessions\n");

	if((i=jtEnumerateSessions()) != JTNET_ERROR_OK)
	{
		if(i == JTNET_ERROR_CONNECTING) return -1;

		if(debugfile) fprintf(debugfile, "- enum error %d\n", i);
	}

	if(debugfile) fprintf(debugfile, "- %d sessions found\n", numSessions);

	if(buffer == NULL) return numSessions;

	for(i=0; i<numSessions && i<numitems; i++)
	{
		jtnetsession_t *ses = sessions + i;
		jtnetserver_t *sd = buffer + i;
		strcpy(sd->name, ses->name);
		strcpy(sd->description, ses->info);
		sd->canJoin = ses->desc.dwFlags & DPSESSION_JOINDISABLED? false : true;
		sd->players = ses->desc.dwCurrentPlayers;
		sd->maxPlayers = ses->desc.dwMaxPlayers;
		sd->data[0] = ses->desc.dwUser2;
		sd->data[1] = ses->desc.dwUser3;
		sd->data[2] = ses->desc.dwUser4;
	}
	return i;
}

__declspec(dllexport) int jtNetSetCallback(int id, void *ptr)
{
	switch(id)
	{
	case JTNET_PLAYER_CREATED_CALLBACK:
		PlayerCreatedCallback = (void(*)(int)) ptr;
		break;

	case JTNET_PLAYER_DESTROYED_CALLBACK:
		PlayerDestroyedCallback = (void(*)(jtnetplayer_t*)) ptr;
		break;

	default:
		// What was that?
		return false;
	}
	return true;
}


// Set a data block.
__declspec(dllexport) int jtNetSetData(int id, void *ptr, int size)
{
	DPSESSIONDESC2 *sd;

	if(dPlay == NULL) return false;

	switch(id)
	{
	case JTNET_SERVER_DATA:
		memcpy(serverData, ptr, (size>sizeof(serverData))? sizeof(serverData) : size);
		// Update the session data.
		sd = &serverSession;
		sd->dwUser2 = serverData[0];
		sd->dwUser3 = serverData[1];
		sd->dwUser4 = serverData[2];
		hres = dPlay->SetSessionDesc(sd, 0);
		if(FAILED(hres) && hres != DPERR_NOSESSIONS) break;
		return true;

	case JTNET_PLAYER_DATA:
		// Set the data of the local player.
		hres = dPlay->SetPlayerData(thisPlrId, ptr, size, DPSET_REMOTE | DPSET_GUARANTEED);
		if(FAILED(hres)) break;
		return true;
	}
	return false;
}

// If ptr == NULL, return the size of the data block.
__declspec(dllexport) int jtNetGetData(int id, int index, void *ptr, int ptrSize)
{
	DWORD		size;

	if(dPlay == NULL) return false;

	// What do we have here?
	switch(id)
	{
	case JTNET_SERVER_DATA:
		// The index is disregarded.
		memcpy(ptr, serverData, (ptrSize>sizeof(serverData))? sizeof(serverData) : ptrSize);
		return true;

	case JTNET_PLAYER_DATA:
		// Is the index a valid player number?
		if(index < 0 || index > numPlayers-1) break;				

		// Do we need to return the size?
		if(!ptr)
		{
			hres = dPlay->GetPlayerData(players[index].id, NULL, &size, 0);
			if(FAILED(hres)) return 0;
			return size;
		}
		// Get the data.
		size = ptrSize;
		hres = dPlay->GetPlayerData(players[index].id, ptr, &size, 0);
		if(FAILED(hres)) return 0;
		return true;
	}
	return false;
}

// Returns non-zero if the adding was successful.
/*int jtNetAddSessionProperty(void *ptr, int bits)
{
	jtnetsd_t	*sd, *prev;
	
	if(numSesData == MAXSESSIONDATA)
		return 0;
	sd = sesData + numSesData;
	// Write down the specified information.
	sd->bitLen = bits;
	sd->value = ptr;
	if(!numSesData)
	{
		sd->bitOff = 0;
		sd->storageNum = 0;
	}
	else	// Find a suitable storage dword.
	{
		// This previous one will tell us where the put the new property.
		prev = sesData + numSesData-1;
		if(prev->bitOff + prev->bitLen + sd->bitLen > 32)
		{
			// Doesn't fit.
			sd->bitOff = 0;
			sd->storageNum = prev->storageNum+1;
			if(sd->storageNum == NUMSTORAGE)
				return 0;
		}
		else
		{
			sd->bitOff = prev->bitOff + prev->bitLen;
			sd->storageNum = prev->storageNum;
		}
	}
	// We're successful.
	numSesData++;
	return 1;
}*/

