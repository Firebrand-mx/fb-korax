
//**************************************************************************
//**
//** I_NET.C
//**
//** The actual network driver. Uses jtNet.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

//#include "H2def.h"
#include "dd_def.h"
#include "i_win32.h"
#include "i_timer.h"
#include "console.h"
#include "jtNetEx.h" // Exported functions from jtNet2.dll.

// MACROS ------------------------------------------------------------------

#define DCN_MISSING_NODE	-10

// TYPES -------------------------------------------------------------------

// A doomcom node.
typedef struct
{
	int	consoleplayer;		// The consoleplayer for this node.
	unsigned int nodeID;	// jtNet node ID.
	int jtNetNode;			// The corresponding jtNet node number (-1 if none).
} doomcomnode_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// These are the nodes that d_net.c refers to when it talks about nodes.
doomcomnode_t dcNodes[MAXPLAYERS];

char *serverName = "Korax";
char *serverInfo = "Multiplayer game server";
char *playerName = "Player";

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int	localtime;
static char *protocolNames[5] = { NULL, "IPX", "TCP/IP", "Serial Link", "Modem" };

// CODE --------------------------------------------------------------------

int __cdecl nodesorter(const void *e1, const void *e2)
{
	unsigned int a = *(unsigned int*)e1, b = *(unsigned int*)e2;
	return a-b;
}

int I_NetUpdateNodes()
{
	int				i, k, numnodes;
	unsigned int	sortedIDs[MAXPLAYERS], originals[MAXPLAYERS],
					myId = jtNetGetInteger(JTNET_MY_PLAYER_ID);

	// Setup doomcom.
	numnodes = jtNetGetPlayerIDs(originals);

	memcpy(sortedIDs, originals, sizeof(originals));

	// Assign player numbers according to a sorted list of
	// player (node) IDs.
	qsort(sortedIDs, numnodes, sizeof(int), nodesorter);

	if(!netgame)
	{
		// If we are not in a netgame we can change the doomcom
		// nodes as much as we want. They aren't yet used by
		// the play simulation.
		memset(dcNodes, 0, sizeof(dcNodes));
		for(i=0; i<numnodes; i++)
		{
			dcNodes[i].jtNetNode = i;
			dcNodes[i].nodeID = originals[i];
			// Find the right consoleplayer number.
			for(k=0; k<numnodes; k++)
			{
				if(dcNodes[i].nodeID == sortedIDs[k])
					dcNodes[i].consoleplayer = k;
			}
		}
		// Consoleplayer is always node 0.
		limboplayer = dcNodes[0].consoleplayer;
	}
	else
	{
		// We are in a netgame. D_net.c assumes that the doomcom nodes
		// will not change under any circumstances. The only change can
		// be that the jtNet nodes are modified because nodes are disconnected.
		// All we must do is to make sure each doomcom node has the correct
		// jtNet node. This can be done by remapping the node numbers using
		// the ID numbers. The IDs won't change even if nodes leave the
		// game.
		for(i=0; i<MAXPLAYERS; i++)
		{
			// Find the right jtNet node.
			dcNodes[i].jtNetNode = DCN_MISSING_NODE;	// Assume it's gone.
			for(k=0; k<numnodes; k++)
				if(originals[k] == dcNodes[i].nodeID)
				{
					dcNodes[i].jtNetNode = k;
					break;
				}
		}
	}

	if(dcNodes[0].jtNetNode != 0)
		I_Error("I_NetUpdateNodes: Consoleplayer is not node 0.\n");

	/*CON_Printf( "NetUpdateNodes: %d nodes\n", numnodes);
	for(i=0; i<numnodes; i++)
	{
		CON_Printf( "  player %i: node %i\n", i, nodeForPlayer[i]);
	}*/
	return numnodes;
}

int I_DCNodeForPlayer(int consolePlrNum)
{
	int		i;

	for(i=0; i<MAXPLAYERS; i++)
		if(dcNodes[i].consoleplayer == consolePlrNum)
			return i;
	return -1;
}

int I_PlayerForDCNode(int doomcomNode)
{
	return dcNodes[doomcomNode].consoleplayer;
}

void I_NetPlayerArrives(int newnode)
{
	int		i;

	if(netgame)
	{
		I_Error("I_NetPlayerArrives: No new players accepted while in netgame.\n");
	}

	if(limbo)
	{
		char **nameList = jtNetGetStringList(JTNET_PLAYER_NAME_LIST, NULL);
		CON_Printf( "%s has arrived.\n", nameList[newnode]);
		I_NetUpdateNodes();
	}
	// We need to find the right player number.
	for(i=0; i<MAXPLAYERS; i++)
		if(dcNodes[i].jtNetNode == newnode)
		{
			// Inform the DLL of the event.
			if(gx.NetPlayerEvent) gx.NetPlayerEvent(dcNodes[i].consoleplayer, DDPE_ARRIVAL, NULL);
			break;
		}
}

void I_NetPlayerLeaves(jtnetplayer_t *plrdata)
{
	int i, pNumber = -1;

	// First let's find out the player number.
	for(i=0; i<MAXPLAYERS; i++)
		if(dcNodes[i].nodeID == plrdata->id)
		{
			pNumber = dcNodes[i].consoleplayer;
			break;
		}

	// Print something in the console.
	CON_Printf( "%s has left.\n", plrdata->name);

	I_NetUpdateNodes();
	
	// Inform the DLL about this.
	if(gx.NetPlayerEvent) gx.NetPlayerEvent(pNumber, DDPE_EXIT, NULL);
}

// Returns the name of the specified player.
char *I_NetGetPlayerName(int player)
{
	int		i, num;
	char	**nameList = jtNetGetStringList(JTNET_PLAYER_NAME_LIST, &num);

	for(i=0; i<MAXPLAYERS; i++)
		if(dcNodes[i].consoleplayer == player)
			return nameList[dcNodes[i].jtNetNode];
	return "";
}

// Returns true if the operation is successful.
int I_NetSetPlayerData(void *data, int size)
{
	return jtNetSetData(JTNET_PLAYER_DATA, data, size);
}

// Returns true if the operation is successful.
int I_NetGetPlayerData(int playerNum, void *data, int size)
{
	int		i, node=-1;

	// Find the node for this player.
	for(i=0; i<MAXPLAYERS; i++)
		if(dcNodes[i].consoleplayer == playerNum)
		{
			node = dcNodes[i].jtNetNode;
			break;
		}
	if(node == -1) return false;

	return jtNetGetData(JTNET_PLAYER_DATA, node, data, size);
}

int I_NetSetServerData(void *data, int size)
{
	return jtNetSetData(JTNET_SERVER_DATA, data, size);
}

int I_NetGetServerData(void *data, int size)
{
	return jtNetGetData(JTNET_SERVER_DATA, 0, data, size);
}


// Initialize the driver. Returns true if successful.
int I_NetInit(int sp)
{
	int		i;

	// Clear the node list.
	memset(dcNodes, 0, sizeof(dcNodes));

	i = jtNetInit(sp);
	if(i == JTNET_ERROR_ALREADY_INITIALIZED) 
	{
		ST_Message("I_NetInit: Network driver already initialized.\n");
		return true;
	}
	if(i != JTNET_ERROR_OK)
		//I_Error("I_NetInit: Initialization failed.\n");
		return false;

	// Set some callback functions.
	jtNetSetCallback(JTNET_PLAYER_CREATED_CALLBACK, I_NetPlayerArrives);
	jtNetSetCallback(JTNET_PLAYER_DESTROYED_CALLBACK, I_NetPlayerLeaves);

	localtime = 0;

	// Doomcom initialization.
	doomcom->deathmatch = false;

	// Tic duplication.
	if((i=M_CheckParm("-ticdup")) && i < myargc-1)
	{
		doomcom->ticdup = strtol(myargv[i+1], NULL, 0);
		ST_Message( "I_NetInit: Ticdup set to %d.\n", doomcom->ticdup);
	}
	else
		doomcom->ticdup = 1;

	// Backup tics in each packet?
	if(M_CheckParm("-extratic"))
	{
		ST_Message( "I_NetInit: Sending backup tics in each packet.\n");
		doomcom->extratics = 1;
	}
	else
		doomcom->extratics = 0;

	return true;
}

void I_NetShutdown()
{
	if(netgame || limbo)
	{
		CON_Execute(server? "net server close" : "net disconnect", true);
	}
	netgame = false;
	server = false;
	limbo = false;
	jtNetShutdown();
}

// Send the data in the doomcom databuffer.
static void SendPacket(int destination)
{
	int	destnode = destination;
	
	if(destnode >= 0 && destnode < MAXPLAYERS)
	{
		destnode = dcNodes[destination].jtNetNode;
		if(destnode == DCN_MISSING_NODE)
			ST_Message("SendPacket: destnode missing!\n");
	}
	jtNetSend(destnode, &doomcom->data, doomcom->datalength);
}

static void GetPacket()
{
	int from, bytes, i;

	doomcom->remotenode = -1;
	bytes = jtNetGet(&from, &doomcom->data, sizeof(doomcom->data));
	if(!bytes || from < 0) return;	// No packet.

	// There was a packet!
	doomcom->datalength = bytes;
	
	// What is the corresponding doomcom node?
	for(i=0; i<MAXPLAYERS; i++)
		if(dcNodes[i].jtNetNode == from)
		{
			doomcom->remotenode = i;
			break;
		}
}

void I_NetCmd()
{
	if(doomcom->command == CMD_SEND)
	{
		localtime++;
        SendPacket(doomcom->remotenode);
	}
	else if(doomcom->command == CMD_GET)
	{
		GetPacket();
    }
}

char *I_GetProtocolName(void)
{
	return protocolNames[jtNetGetInteger(JTNET_SERVICE)];
}

//-----------------------------------------------------------------
//
// Network Events
//
void I_NetServerOpening(boolean before)
{
	if(before)
	{
		// Hmm?
	}
	else
	{
		server = limbo = true;
		I_NetUpdateNodes();
	}
	// Call game DLL's NetServerOpened.
	if(gx.NetServerOpen) gx.NetServerOpen(before);
}

void I_NetServerClosing(boolean before)
{
	if(before)
	{
		D_QuitNetGame();
		D_StopNetGame(true);
	}
	else
	{
		server = false;
	}
	// Call game DLL's NetServerClosed.
	if(gx.NetServerClose) gx.NetServerClose(before);
}

void I_NetServerStarted()
{
	if(netgame) return;	// Already in netgame, can't start again.

	doomcom->numplayers = doomcom->numnodes = I_NetUpdateNodes();
	doomcom->consoleplayer = limboplayer;

	// Setup the netgame.
	D_StartNetGame();

	// This is mainly for updating player data and other important stuff.
	D_SyncNetStart();

	// Synchronization ends; game begins on all computers.
	// Call the post-serverstart routine.
	if(gx.NetServerStart) gx.NetServerStart(false);
}

void I_NetServerStopping(boolean before)
{
	if(before)
	{
		if(!netgame) return;	// Server can't stop unless we're in netgame.

		// Stop the netgame.
		D_QuitNetGame();
		D_StopNetGame(false);
	}

	// Call game DLL's NetServerStopped.
	if(gx.NetServerStop) gx.NetServerStop(before);
}

void I_NetConnecting(boolean before)
{
	if(!before)
	{
		limbo = true;
		// Query jtNet for a player number.
		I_NetUpdateNodes();
	}

	// Call game DLL's NetConnected.
	if(gx.NetConnect) gx.NetConnect(before);
}

// before = true if the connection hasn't yet been closed.
void I_NetDisconnecting(boolean before)
{
	if(before)
	{
		// This lets us exit gracefully.
		D_QuitNetGame();
	}
	else
	{
		D_StopNetGame(true);
		limbo = false;
		netgame = false;	
		limboplayer = 0;
		// Call game DLL's NetDisconnected.
	}
	if(gx.NetDisconnect) gx.NetDisconnect(before);
}


//-----------------------------------------------------------------
// 
// The NET Console Command
//
int CCmdNet(int argc, char **argv)
{
	int		i;
	boolean success = true;

	if(argc == 1)	// No args?
	{
		CON_Printf( "Usage: %s (cmd/args)\n", argv[0]);
		return true;
	}
	// We may be spending a fair number of tics here, so we better stop
	// the timer.
	//I_SaveTime();

	if(argc == 2)	// One argument?
	{
		if(!stricmp(argv[1], "shutdown"))
		{
			I_NetShutdown();
			CON_Printf( "Network shut down.\n");
		}
		else if(!stricmp(argv[1], "modems"))
		{
			int num;
			char **modemList = jtNetGetStringList(JTNET_MODEM_LIST, &num);
			if(modemList == NULL)
			{
				CON_Printf( "No modem list available.\n");
			}
			else for(i=0; i<num; i++)
			{
				CON_Printf( "%d: %s\n", i, modemList[i]);
			}
		}
		else if(!stricmp(argv[1], "servers"))
		{
			int num = jtNetGetServerInfo(NULL, 0);
			if(num < 0)
			{
				CON_Printf( "Server list is being retrieved. Try again in a moment.\n");
			}
			else if(num == 0)
			{
				CON_Printf( "No server list available.\n");
			}
			else 
			{
				jtnetserver_t *buffer = malloc(sizeof(jtnetserver_t) * num);
				// -JL- Paranoia
				if (!buffer)
					I_Error("CCmdNet: malloc failed");
				jtNetGetServerInfo(buffer, num);
				CON_Printf( "%d server%s found.\n", num, num!=1? "s were" : " was");
				if(num)
				{
					CON_Printf("%-20s P/M L Description:\n", "Name:");
					for(i=0; i<num; i++)
						CON_Printf("%-20s %d/%d %c %s\n", buffer[i].name,
						buffer[i].players, buffer[i].maxPlayers,
						buffer[i].canJoin? ' ':'*',
						buffer[i].description);
				}
				free(buffer);
			}			
		}
		else if(!stricmp(argv[1], "players"))
		{
			int num = 0;
			char **nameList = jtNetGetStringList(JTNET_PLAYER_NAME_LIST, &num);
			if(!num)
			{
				CON_Printf( "No player list available.\n");
			}
			else 
			{
				CON_Printf( "There %s %d player%s.\n", num==1? "is" : "are", num, num!=1? "s" : "");
				for(i=0; i<num; i++)
					CON_Printf( "%d: %s\n", i, nameList[i]);
			}
		}			
		else if(!stricmp(argv[1], "info"))
		{
			CON_Printf( "Doomcom nodes:\n");
			for(i=0; i<MAXPLAYERS; i++)
			{
				CON_Printf( "%i: player %i, node %i, node ID %x (ingame: %i)\n", 
					i, dcNodes[i].consoleplayer, dcNodes[i].jtNetNode,
					dcNodes[i].nodeID, players[i].ingame);
			}
			
			CON_Printf( "\nNetwork game: %s\n", netgame? "yes" : "no");
			CON_Printf( "Server: %s\n", server? "yes" : "no");
			CON_Printf( "Limbo: %s (you're player %d)\n", limbo? "yes" : "no", limboplayer);
			CON_Printf( "TCP/IP address: %s\n", jtNetGetString(JTNET_TCPIP_ADDRESS));
			CON_Printf( "TCP/IP port: %d (0x%x)\n", jtNetGetInteger(JTNET_TCPIP_PORT),
				jtNetGetInteger(JTNET_TCPIP_PORT));
			CON_Printf( "Modem: %d (%s)\n", jtNetGetInteger(JTNET_MODEM),
				jtNetGetString(JTNET_MODEM));
			CON_Printf( "Phone number: %s\n", jtNetGetString(JTNET_PHONE_NUMBER));
			CON_Printf( "Serial: COM %d, baud %d, stop %d, parity %d, flow %d\n",
				jtNetGetInteger(JTNET_COMPORT),
				jtNetGetInteger(JTNET_BAUDRATE),
				jtNetGetInteger(JTNET_STOPBITS),
				jtNetGetInteger(JTNET_PARITY),
				jtNetGetInteger(JTNET_FLOWCONTROL));
		}
		else if(!stricmp(argv[1], "disconnect"))
		{
			if(!netgame && !limbo)
			{
				CON_Printf( "This client is not connected to a server.\n");
				return false;
			}
			if(server)
			{
				CON_Printf( "You can't disconnect the server! Use \"net server close\".\n");
				return false;
			}

			// Things that have to be done before closing the connection.
			I_NetDisconnecting(true);

			success = !jtNetDisconnect();
			if(success)
			{
				CON_Printf( "Disconnected.\n");
				// Things that are done afterwards.
				I_NetDisconnecting(false);
			}
			else
				CON_Printf( "Error disconnecting.\n");
		}
		else 
		{
			CON_Printf( "Bad arguments.\n");
			return false; // Bad args.
		}
	}
	if(argc == 3)	// Two arguments?
	{
		if(!stricmp(argv[1], "init"))
		{
			int sp = (!stricmp(argv[2], "tcp/ip") || !stricmp(argv[2], "tcpip"))? JTNET_SERVICE_TCPIP 
				: !stricmp(argv[2], "ipx")? JTNET_SERVICE_IPX 
				: !stricmp(argv[2], "serial")? JTNET_SERVICE_SERIAL
				: !stricmp(argv[2], "modem")? JTNET_SERVICE_MODEM
				: JTNET_SERVICE_UNKNOWN;

			if(sp == JTNET_SERVICE_UNKNOWN)
			{
				ST_Message( "%s is not a supported service provider.\n", argv[2]);
				return false;
			}

			// Initialization.
			success = I_NetInit(sp);
			if(success)
				ST_Message( "Network initialization OK.\n");
			else
				ST_Message( "Network initialization failed!\n");

			// Let everybody know of this.
			CmdReturnValue = success;
		}
		else if(!stricmp(argv[1], "server"))
		{
			if(!stricmp(argv[2], "open"))
			{
				if(server) // Is the server already open?
				{
					CON_Printf( "Server already open.\n");
					return false;
				}
				// First set the info strings.
				jtNetSetString(JTNET_SERVER_INFO, serverInfo);
				jtNetSetString(JTNET_NAME, playerName);

				I_NetServerOpening(true);

				// Now we can try to open the server.
				success = !jtNetOpenServer(serverName);
				if(success)
				{
					CON_Printf( "Server \"%s\" opened.\n", serverName);
					I_NetServerOpening(false);
				}
				else
					CON_Printf( "Server opening failed.\n");
				CmdReturnValue = success;
			}
			else if(!stricmp(argv[2], "go") || !stricmp(argv[2], "start"))
			{
				if(!server)
				{
					CON_Printf( "This is not a server!\n");
					return false;
				}
				if(netgame)
				{
					CON_Printf( "Server already running.\n");
					return false;
				}
				// The game DLL may have something that needs doing
				// before we actually begin.
				//if(gx.NetServerStart) gx.NetServerStart(true);

				success = !jtNetLockServer(true);
				if(!success) 
				{
					CON_Printf("Couldn't lock the server.\n");
					return false;
				}
				I_NetServerStarted();
			}
			else if(!stricmp(argv[2], "stop"))
			{
				if(!server)
				{
					CON_Printf( "This is not a server!\n");
					return false;
				}
				if(limbo)
				{
					CON_Printf( "Server already in limbo mode.\n");
					return false;
				}
				I_NetServerStopping(true);
				success = !jtNetLockServer(false);
				if(!success)
				{
					CON_Printf( "Couldn't unlock the server.\n");
					return false;
				}
				CON_Printf( "Server stopped, back in limbo mode.\n");
				I_NetServerStopping(false);
			}
			else if(!stricmp(argv[2], "close"))
			{
				if(!server)
				{
					CON_Printf( "This is not a server!\n");
					return false;
				}
				// Close the server and kick everybody out.
				I_NetServerClosing(true);

				success = !jtNetCloseServer();
				if(success)
				{
					CON_Printf( "Server closed.\n");
					I_NetServerClosing(false);
				}
				else
					CON_Printf( "Server closing failed.\n");
			}
			else
			{
				CON_Printf( "Bad arguments.\n");
				return false;
			}
		}
		else if(!stricmp(argv[1], "connect"))
		{
			if(netgame || limbo)
			{
				CON_Printf( "This client is already connected.\n");
				return false;
			}

			// Set the name of the player.
			jtNetSetString(JTNET_NAME, playerName);

			I_NetConnecting(true);

			success = !jtNetConnect(argv[2]);
			if(success)
			{
				CON_Printf( "Connected to '%s'.\n", argv[2]);
				I_NetConnecting(false);				
			}
			else
				CON_Printf( "Connection failed.\n");

			CmdReturnValue = success;
		}
	}
	if(argc >= 3)
	{
		// TCP/IP settings.
		if(!stricmp(argv[1], "tcpip") || !stricmp(argv[1], "tcp/ip"))
		{
			if(!stricmp(argv[2], "address") && argc > 3)
			{
				success = jtNetSetString(JTNET_TCPIP_ADDRESS, argv[3]);
				CON_Printf( "TCP/IP address: %s\n", argv[3]);
			}
			else if(!stricmp(argv[2], "port") && argc > 3)
			{
				int val = strtol(argv[3], NULL, 0);
				success = jtNetSetInteger(JTNET_TCPIP_PORT, val);
				CON_Printf( "TCP/IP port: %d\n", val);
			}
		}
		// Modem settings.
		if(!stricmp(argv[1], "modem"))
		{
			if(!stricmp(argv[2], "phone"))
			{
				success = jtNetSetString(JTNET_PHONE_NUMBER, argv[3]);
				CON_Printf( "Modem phone number: %s\n", argv[3]);
			}
			else 
			{
				int val = strtol(argv[2], NULL, 0);
				char **modemList = jtNetGetStringList(JTNET_MODEM_LIST, NULL);
				success = jtNetSetInteger(JTNET_MODEM, val);
				if(success)
				{
					CON_Printf( "Selected modem: %d, %s\n", val, modemList[val]);
				}
			}
		}
		// Serial settings.
		else if(!stricmp(argv[1], "serial"))
		{
			if(!stricmp(argv[2], "com"))
			{
				int val = strtol(argv[3], NULL, 0);
				success = jtNetSetInteger(JTNET_COMPORT, val);
				if(success) CON_Printf( "COM port: %d\n", val);
			}
			else if(!stricmp(argv[2], "baud"))
			{
				int val = strtol(argv[3], NULL, 0);
				success = jtNetSetInteger(JTNET_BAUDRATE, val);
				if(success) CON_Printf( "Baud rate: %d\n", val);
			}
			else if(!stricmp(argv[2], "stop"))
			{
				int val = strtol(argv[3], NULL, 0);
				success = jtNetSetInteger(JTNET_STOPBITS, val);
				if(success) CON_Printf( "Stopbits: %d\n", val);
			}
			else if(!stricmp(argv[2], "parity"))
			{
				int val = strtol(argv[3], NULL, 0);
				success = jtNetSetInteger(JTNET_PARITY, val);
				if(success) CON_Printf( "Parity: %d\n", val);
			}
			else if(!stricmp(argv[2], "flow"))
			{
				int val = strtol(argv[3], NULL, 0);
				success = jtNetSetInteger(JTNET_FLOWCONTROL, val);
				if(success) CON_Printf( "Flow control: %d\n", val);
			}
		}
	}

	// We don't necessarily get this far.

	// Finish the time warp.
//	I_RestoreTime();

	return success;
}