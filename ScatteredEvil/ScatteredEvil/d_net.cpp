
//**************************************************************************
//**
//** d_net.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//** This version has the fixed ticdup code.
//**
//**************************************************************************

#include "h2def.h"
#include "i_win32.h"
#include "i_timer.h"
#include "i_net.h"
#include "console.h"
#include <stdlib.h> // for atoi()

#define NCMD_EXIT       0x80000000
#define NCMD_RETRANSMIT 0x40000000
#define NCMD_SETUP      0x20000000
#define NCMD_KILL       0x10000000              // kill game
#define NCMD_CHECKSUM   0x0fffffff

// SendPacket player masks
#define DDPM_ALL_PLAYERS	0xffffffff

// Net events.
enum
{
	NE_START_GAME,
	NE_END_GAME,
	NE_SERVER_CLOSED,
	NE_PLAYER_READY,
	NE_END_SYNC,
	NE_CHAT,
	NE_CHAT_BROADCAST
};


doomcom_t               *doomcom;
doomdata_t              *netbuffer;             // points inside doomcom


/*
==============================================================================

							NETWORKING

gametic is the tic about to (or currently being) run
maketic is the tick that hasn't had control made for it yet
nettics[] has the maketics for all players

a gametic cannot be run until nettics[] > gametic for all players

==============================================================================
*/

#define RESENDCOUNT     10

ticcmd_t		localcmds[BACKUPTICS];

ticcmd_t		netcmds[MAXPLAYERS][BACKUPTICS];

int             nettics[MAXNETNODES];
boolean         nodeingame[MAXNETNODES];			// set false as nodes leave game
boolean         remoteresend[MAXNETNODES];			// set when local needs tics
int             resendto[MAXNETNODES];              // set when remote needs our tics
int				svResendTo[MAXNETNODES][MAXNETNODES]; // [server] set when remote (1) needs tics of (2) 
int             resendcount[MAXNETNODES];

int             nodeforplayer[MAXPLAYERS];

boolean			map_rendered = false;
boolean			allow_net_traffic = true;	// Should net traffic be allowed?
int				netgame; // only true if >1 player
int				server;		// true if this computer is an open server.
int				limbo;		// true if the server is in limbo mode.
int				limboplayer;
int				consoleplayer;
int				displayplayer;
int				serverplayer;	// who is the server player? (set when game begins)

int				netModel = NETM_MULTICAST;		// The networking model.

int             gametic, maketic;

int             lastnettic, skiptics;
int             ticdup;
int             maxsend;        // BACKUPTICS/(2*ticdup)-1

int		gametime;
int     frametics[4], frameon;
int     frameskip[4];
int		oldnettics;
int		oldentertics;


//void H2_ProcessEvents (void);
//void G_BuildTiccmd (ticcmd_t *cmd);
//void H2_DoAdvanceDemo (void);
extern void ST_NetProgress(void);
extern void ST_NetDone(void);

boolean                 reboundpacket;
doomdata_t              reboundstore;


int NetbufferSize (void)
{
	guardSlow(NetbufferSize);
	return (int)&(((doomdata_t *)0)->cmds[netbuffer->numtics]);
	unguardSlow;
}

unsigned NetbufferChecksum (void)
{
	guardSlow(NetbufferChecksum);
	unsigned                c;
	int             i,l;

	c = 0x1234567;

#if defined(NeXT) || defined(NORMALUNIX)
	return 0;                       // byte order problems
#endif

	l = (NetbufferSize () - (int)&(((doomdata_t *)0)->retransmitfrom))/4;
	for (i=0 ; i<l ; i++)
		c += ((unsigned *)&netbuffer->retransmitfrom)[i] * (i+1);

	return c & NCMD_CHECKSUM;
	unguardSlow;
}

int ExpandTics (int low)
{
	guardSlow(ExpandTics);
	int     delta;

	delta = low - (maketic&0xff);

	if (delta >= -64 && delta <= 64)
		return (maketic&~0xff) + low;
	if (delta > 64)
		return (maketic&~0xff) - 256 + low;
	if (delta < -64)
		return (maketic&~0xff) + 256 + low;

	I_Error ("ExpandTics: strange value %i at maketic %i",low,maketic);
	return 0;
	unguardSlow;
}


//============================================================================

/*
==============
=
= HSendPacket
=
==============
*/

void HSendPacket (int node, int flags)
{
	guard(HSendPacket);
	netbuffer->checksum = NetbufferChecksum () | flags;

	if (!node)
	{
		reboundstore = *netbuffer;
		reboundpacket = true;
		return;
	}

	if (!allow_net_traffic)
		return;

	if (!netgame && !limbo)
		I_Error ("Tried to transmit to another node");

	doomcom->command = CMD_SEND;
	doomcom->remotenode = node;
	doomcom->datalength = NetbufferSize ();

if (debugfile)
{
	int             i;
	int             realretrans;
	if (netbuffer->checksum & NCMD_RETRANSMIT)
		realretrans = ExpandTics (netbuffer->retransmitfrom);
	else
		realretrans = -1;
	fprintf (debugfile,"send (%i + %i, R %i) [%i] ", 
		ExpandTics(netbuffer->starttic),netbuffer->numtics, realretrans, doomcom->datalength);
	for (i=0 ; i<doomcom->datalength ; i++)
		fprintf (debugfile,"%i ",((byte *)netbuffer)[i]);
	fprintf (debugfile,"\n");
}

	I_NetCmd ();
	unguard;
}

/*
==============
=
= HGetPacket
=
= Returns false if no packet is waiting
=
==============
*/

boolean HGetPacket (void)
{
	guard(HGetPacket);
	if (reboundpacket)
	{
		*netbuffer = reboundstore;
		doomcom->remotenode = 0;
		reboundpacket = false;
		return true;
	}

	if (!netgame && !limbo)
		return false;
	if (!allow_net_traffic)
		return false;

	doomcom->command = CMD_GET;
	I_NetCmd ();
	if (doomcom->remotenode == -1)
		return false;

	if (doomcom->datalength != NetbufferSize ())
	{
		if (debugfile)
			fprintf (debugfile,"bad packet length %i\n",doomcom->datalength);
		return false;
	}

	if (NetbufferChecksum () != (netbuffer->checksum&NCMD_CHECKSUM) )
	{
		if (debugfile)
			fprintf (debugfile,"bad packet checksum\n");
		return false;
	}

if (debugfile)
{
	int             realretrans;
			int     i;

	if (netbuffer->checksum & NCMD_SETUP)
		fprintf (debugfile,"setup packet\n");
	else
	{
		if (netbuffer->checksum & NCMD_RETRANSMIT)
			realretrans = ExpandTics (netbuffer->retransmitfrom);
		else
			realretrans = -1;
		fprintf (debugfile,"get %i = (%i + %i, R %i)[%i] ",doomcom->remotenode,
		ExpandTics(netbuffer->starttic),netbuffer->numtics, realretrans, doomcom->datalength);
		for (i=0 ; i<doomcom->datalength ; i++)
			fprintf (debugfile,"%i ",((byte *)netbuffer)[i]);
		fprintf (debugfile,"\n");
	}
}
	return true;
	unguard;
}

// Type is NE_CHAT.
void D_SendPacket(unsigned int playermask, int type, void *data, int length)
{
	guard(D_SendPacket);
	// If the type is wrong, no packet is sent.
	if (type != NE_CHAT) return;

	// Use the limbo player number if a netgame is not in progress.
	netbuffer->player = limbo? limboplayer : consoleplayer;

	// Calculate the length of the message.
	netbuffer->numtics = (length + 2) / sizeof(ticcmd_t) + 1;	// How many ticcmds?
	
	// The first two bytes are the length of the message.
	*(unsigned short*)netbuffer->cmds = length;
	
	// The actual data follows.
	memcpy(((byte *)netbuffer->cmds) + 2, data, length);
	netbuffer->starttic = type;

	// Send to the appropriate players according to the networking model.
	if(netModel == NETM_PEER_TO_PEER || (server && netModel == NETM_MULTICAST))
	{
		if(playermask == DDPM_ALL_PLAYERS)
			HSendPacket(-1, NCMD_SETUP); // Broadcast.
		else
		{
			int i;
			// Send to the players specified in the mask.
			for(i=0; i<MAXPLAYERS; i++)
				if(playermask & (1<<i))
					HSendPacket(I_DCNodeForPlayer(i), NCMD_SETUP);
		}
	}
	else if(netModel == NETM_MULTICAST && !server)
	{
		if(playermask == DDPM_ALL_PLAYERS)
		{
			// Ask the server to broadcast it.
			netbuffer->starttic = NE_CHAT_BROADCAST;
			HSendPacket(-2, NCMD_SETUP); // -2 is the server.
		}
		else
		{
			int	i;
			// It might be faster to send directly to the destination,
			// but we ARE using the multicast server model.
			for(i=0; i<MAXPLAYERS; i++)
				if(playermask & (1<<i))
				{
					netbuffer->targetplayer = i;
					HSendPacket(-2, NCMD_SETUP);
				}
		}
	}
	unguard;
}

// Prints the message in the console.
void ShowChatMessage()
{
	guard(ShowChatMessage);
	// The current packet in the netbuffer is a chat message,
	// let's unwrap and show it.
	CON_FPrintf(CBLF_GREEN, "%s: %s\n", 
		I_NetGetPlayerName(netbuffer->player), ((byte *)netbuffer->cmds) + 2);
	unguard;
}

// All arguments are sent out as a chat message.
int CCmdChat(int argc, char **argv)
{
	guard(CCmdChat);
	char	buffer[100];
	int		i, mode = !stricmp(argv[0], "chat")? 0 : !stricmp(argv[0], "chatNum")? 1 : 2;
	unsigned int mask = 0;

	if(argc == 1)
	{
		CON_Printf( "Usage: %s (text)\n", 
			!mode? "chat" : mode==1? "chatNum (plr#)" : "chatTo (name)");
		CON_Printf( "Chat messages are max. 80 characters long.\n");
		CON_Printf( "Use quotes to get around arg processing.\n");
		return true;
	}

	// Chatting is only possible when connected.
	if(!netgame && !limbo) return false;

	// Too few arguments?
	if(mode && argc < 3) return false;

	// Assemble the chat message.
	strcpy(buffer, argv[!mode? 1 : 2]);
	for(i = (!mode? 2 : 3); i<argc; i++)
	{
		strcat(buffer, " ");
		strncat(buffer, argv[i], 80 - (strlen(buffer)+strlen(argv[i])+1));
	}
	buffer[80] = 0;

	// Send the message.
	switch(mode)
	{
	case 0: // chat
		mask = DDPM_ALL_PLAYERS;
		break;

	case 1: // chatNum
		mask = 1 << atoi(argv[1]);
		break;

	case 2: // chatTo
		for(i=0; i<MAXPLAYERS; i++)
			if(!stricmp(I_NetGetPlayerName(i), argv[1]))
			{
				mask = 1 << i;
				break;
			}
	}
	D_SendPacket(mask, NE_CHAT, buffer, strlen(buffer)+1);
	
	// Show the message locally, too.
	ShowChatMessage();

	// Inform the game, too.
	H2_NetPlayerEvent(netbuffer->player, DDPE_CHAT_MESSAGE,
		((byte *)netbuffer->cmds) + 2);
	return true;
	unguard;
}

static void HandlePacket()
{
	guard(HandlePacket);
	int type = netbuffer->starttic;

	// A chat broadcast?
	if(type == NE_CHAT_BROADCAST || type == NE_CHAT)
	{
		ShowChatMessage();
		H2_NetPlayerEvent(netbuffer->player, DDPE_CHAT_MESSAGE,
				((byte *)netbuffer->cmds) + 2);
	}
	unguard;
}

/*
===================
=
= GetPackets
=
===================
*/

char    exitmsg[80];

void GetPackets (void)
{
	guard(GetPackets);
	int             netconsole;
	int             netnode;
	ticcmd_t		*src;
	int             realend;
	int             realstart;
	int				start, i;

	while (HGetPacket ())
	{
		if (netbuffer->checksum & NCMD_SETUP)
		{
			int type = netbuffer->starttic;
			switch(type)
			{
			case NE_START_GAME:
				// Set the serverplayer. Only the server can send a
				// start game event.
				serverplayer = netbuffer->player;
				I_NetServerStarted();
				break;

			case NE_END_GAME:
				if(netgame)
				{
					I_NetServerStopping(true);	// Before...
					I_NetServerStopping(false);	// ...and after. :-)
				}
				break;

			case NE_SERVER_CLOSED:	// We should quit?
				if(netgame) // Stop the server first.
				{
					I_NetServerStopping(true);	// Before...
					I_NetServerStopping(false);	// ...and after. :-)
				}
				CON_Execute("net disconnect", true);		
				break;

			case NE_CHAT_BROADCAST:		// |
				if(server)
				{
					for(i=0; i<MAXPLAYERS; i++)
					{
						int destnode = I_DCNodeForPlayer(i);
						// Send this to all players, except the sender
						// and ourselves.
						if(destnode == -1 || i == netbuffer->player) continue;
						if(i == (limbo? limboplayer : consoleplayer)) continue;
						HSendPacket(destnode, NCMD_SETUP);					
					}	
				}
				HandlePacket();
				break;

			case NE_CHAT:	// Chat message.
				if(netModel == NETM_MULTICAST && server)
				{
					// We are the server in multicast networking.
					// This packet may not even be for us.
					if(netbuffer->targetplayer != (limbo? limboplayer : consoleplayer))
					{
						// Send it where it belongs.
						HSendPacket(I_DCNodeForPlayer(netbuffer->targetplayer), NCMD_SETUP);
						// We can't process it ourselves.
						break;
					}
				}
				// The packet is (also) for us, handle it.
				HandlePacket();
				break;
			}
			// This packet has been dealt with.
			continue;               
		}
		
		// Determine who sent this packet.
		if(netModel == NETM_PEER_TO_PEER || !netgame)
		{
			netconsole = netbuffer->player;
			netnode = doomcom->remotenode;
		}
		else if(netModel == NETM_MULTICAST)
		{
			if(server)
			{
				netconsole = netbuffer->player;
				netnode = doomcom->remotenode;
			}
			else
			{
				// This packet is from the server.
				netconsole = netbuffer->player;
				// Check the doomcom nodes for the node number.
				// The packet is from the server, not the player
				// specified in netconsole.
				netnode = I_DCNodeForPlayer(netconsole);
			}
		}

		//
		// to save bytes, only the low byte of tic numbers are sent
		// Figure out what the rest of the bytes are
		//
		realstart = ExpandTics(netbuffer->starttic);
		realend = realstart + netbuffer->numtics;

		//
		// check for exiting the game
		//
		if (netbuffer->checksum & NCMD_EXIT)
		{
			if (!nodeingame[netnode])
				continue;
			nodeingame[netnode] = false;
			players[netconsole].ingame = false;

//			players[consoleplayer].message = exitmsg;
//                      if (demorecording)
//                              G_CheckDemoStatus ();
			continue;
		}

		//
		// check for a remote game kill
		//
		if (netbuffer->checksum & NCMD_KILL)
			I_Error ("Killed by network driver");

		nodeforplayer[netconsole] = netnode;

		//
		// check for retransmit request
		//
		if(resendcount[netnode] <= 0 && (netbuffer->checksum & NCMD_RETRANSMIT))
		{
			if(netModel == NETM_PEER_TO_PEER)
			{
				resendto[netnode] = ExpandTics(netbuffer->retransmitfrom);

				if (debugfile)
					fprintf (debugfile,"retransmit from %i\n", resendto[netnode]);
				
				resendcount[netnode] = RESENDCOUNT;
			}
			else if(netModel == NETM_MULTICAST)
			{
				if(server)
				{
					// We are the server. Resistance is futile.
					// We have the authority to answer these requests.
					if(netbuffer->targetplayer == consoleplayer)
					{
						// The client requests our own, local ticcmds.
						resendto[netnode] = ExpandTics(netbuffer->retransmitfrom);
						resendcount[netnode] = RESENDCOUNT;
	
						if(debugfile)
						{
							fprintf(debugfile,"retransmit from %i\n", resendto[netnode]);
						}
					}
					else
					{
						// The client wants ticcmds that belong to some other client.
						int reqNode = I_DCNodeForPlayer(netbuffer->targetplayer);
						if(reqNode >= 0)	// Just to be on the safe side..
						{
							svResendTo[netnode][reqNode] = ExpandTics(netbuffer->retransmitfrom);
						}
						resendcount[netnode] = RESENDCOUNT;

						if(debugfile)
						{
							fprintf(debugfile,"retransmit (%i requests %i) from %i\n", 
								netnode, reqNode, svResendTo[netnode][reqNode]);
						}
					}
				}
				else // A client?
				{
					if(netconsole != serverplayer)
					{
						I_Error("Console %i (node %i) sent this client a retransmit request!\n",
							netconsole, netnode);
					}
					resendto[netnode] = ExpandTics(netbuffer->retransmitfrom);

					if (debugfile)
						fprintf (debugfile,"retransmit from %i\n", resendto[netnode]);
				
					resendcount[netnode] = RESENDCOUNT;
				}
			}
		}
		else
			resendcount[netnode]--;

		//
		// check for empty / out of order / duplicated packet
		//
		if (netbuffer->numtics == 0 || realend == nettics[netnode])
			continue;

		if (realend < nettics[netnode])
		{
			if (debugfile)
				fprintf (debugfile,"out of order packet (%i + %i)\n" ,realstart,netbuffer->numtics);
			continue;
		}

		//
		// check for a missed packet
		//
		if (realstart > nettics[netnode])
		{
		// stop processing until the other system resends the missed tics
			if (debugfile)
				fprintf (debugfile,"missed tics from %i (%i - %i)\n", netnode, realstart, nettics[netnode]);

			remoteresend[netnode] = true;
			continue;
		}

//
// update command store from the packet
//
		remoteresend[netnode] = false;

		start = nettics[netnode] - realstart;
		src = &netbuffer->cmds[start];

		while (nettics[netnode] < realend)
		{
			// Copy the ticcmd.
			netcmds[netconsole][nettics[netnode] % BACKUPTICS] = *src;
			nettics[netnode]++;
			src++;
		}
	}
	unguard;
}

/*
=============
=
= NetUpdate
=
= Builds ticcmds for console player
= sends out a packet
=============
*/


void NetUpdate (void)
{
	guard(NetUpdate);
	int nowtime;
	int newtics;
	int	i,j;
	int realstart;
	int	gameticdiv;

//
// check time
//
	nowtime = I_GetTime ()/ticdup;
	newtics = nowtime - gametime;
	gametime = nowtime;

	if (newtics <= 0)                       // nothing new to update
		goto listen;

	if (skiptics <= newtics)
	{
		newtics -= skiptics;
		skiptics = 0;
	}
	else
	{
		skiptics -= newtics;
		newtics = 0;
	}


	netbuffer->player = consoleplayer;

//
// build new ticcmds for console player
//
	gameticdiv = gametic/ticdup;
	for (i=0 ; i<newtics ; i++)
	{
		I_StartTic ();
		DD_ProcessEvents ();

		if(debugfile) 
			fprintf(debugfile, "mktic:%i gtd:%i newtics:%i >> %i\n",maketic, gameticdiv, newtics, maketic-gameticdiv);
		
		if (maketic - gameticdiv >= BACKUPTICS/2-1)
			break;          // can't hold any more
		G_BuildTiccmd(&localcmds[maketic % BACKUPTICS]);
		maketic++;
	}


	if (singletics)
		return;         // singletic update is syncronous

//
// send the packet to the other nodes
//
	if(netModel == NETM_PEER_TO_PEER || !netgame)
	{
		for(i=0; i<doomcom->numnodes; i++)
			if(nodeingame[i])
			{
				netbuffer->starttic = realstart = resendto[i];
				netbuffer->numtics = maketic - realstart;
				if (netbuffer->numtics > BACKUPTICS)
					I_Error ("NetUpdate: netbuffer->numtics > BACKUPTICS\n");

				//if(debugfile) fprintf(debugfile, "Sending to %i: 

				resendto[i] = maketic - doomcom->extratics;
				for (j=0 ; j< netbuffer->numtics ; j++)
				{	
					netbuffer->cmds[j] = localcmds[(realstart + j) % BACKUPTICS];
				}
				if (remoteresend[i])
				{
					netbuffer->retransmitfrom = nettics[i];
					HSendPacket (i, NCMD_RETRANSMIT);
				}
				else /*if(netbuffer->numtics)*/ // Don't send empty packets for nothing.
				{
					netbuffer->retransmitfrom = 0;
					HSendPacket (i, 0);
				}
			}
	}
	else if(netModel == NETM_MULTICAST)
	{
		if(server)
		{
			// We are the server. We'll send our local ticcmds to all
			// players, including ourselves (rebounds back to us).
			for(i=0; i<doomcom->numplayers; i++)
				if(players[i].ingame)
				{
					int plrnode = I_DCNodeForPlayer(i);

					// These two have a special meaning when sent by
					// the server. The clients interpret them as follows:
					netbuffer->player = consoleplayer;	// source console
					//netbuffer->targetplayer = I_DCNodeForPlayer(consoleplayer);	// source node

					netbuffer->starttic = realstart = resendto[plrnode];
					netbuffer->numtics = maketic - realstart;
					if(netbuffer->numtics > BACKUPTICS)
						I_Error("NetUpdate: netbuffer->numtics > BACKUPTICS\n");

					resendto[plrnode] = maketic - doomcom->extratics;
					for(j=0; j<netbuffer->numtics; j++)
					{
						netbuffer->cmds[j] = localcmds[(realstart+j)%BACKUPTICS];
					}
					if(remoteresend[plrnode])
					{
						// This player must retransmit us some packets,
						// there was something missing.
						netbuffer->retransmitfrom = nettics[plrnode];
						HSendPacket(plrnode, NCMD_RETRANSMIT);
					}
					else if(netbuffer->numtics) // Don't send empty packets for nothing.
					{
						// Just send the ticcmds.
						netbuffer->retransmitfrom = 0;
						HSendPacket(plrnode, 0);
					}

					// As the server we must send the ticcmds of all clients
					// to all the other clients.
					for(j=0; j<doomcom->numplayers; j++)
					{
						int targetNode, k;

						if(i == j || !players[j].ingame || j == consoleplayer) 
							continue;
				
						// We are sending the ticcmds of console i to console j.
						targetNode = I_DCNodeForPlayer(j);

						// These two have a special meaning when sent by
						// the server. The clients interpret them as follows:
						netbuffer->player = i;				// source console
						//netbuffer->targetplayer = plrnode;	// source node

						// Now the destination client thinks it's receiving them
						// from the source console (really from us, of course).

						netbuffer->starttic = realstart = svResendTo[targetNode][plrnode];
						netbuffer->numtics = nettics[plrnode] - realstart;

						if(netbuffer->numtics > BACKUPTICS)
							I_Error("NetUpdate: (sv) netbuffer->numtics > BACKUPTICS\n");

						svResendTo[targetNode][plrnode] = nettics[plrnode] - doomcom->extratics;
						
						for(k=0; k<netbuffer->numtics; k++)
						{
							// Copy from the ticcmd buffer of player i.
							netbuffer->cmds[k] = netcmds[i][(realstart + k) % BACKUPTICS];
						}
						// Don't send empty packets for nothing.
						if(netbuffer->numtics)
						{
							netbuffer->retransmitfrom = 0;
							HSendPacket(targetNode, 0);
						}
					}
				}
		}
		else 
		{
			int svnode = I_DCNodeForPlayer(serverplayer);
			// We're a client. Send ticcmds only to the server, and locally,
			// of course.
			for(i=0; i<2; i++)
			{			
				int destNode = i? svnode : 0;

				netbuffer->targetplayer = serverplayer;
				netbuffer->starttic = realstart = resendto[destNode];
				netbuffer->numtics = maketic - realstart;
				if(netbuffer->numtics > BACKUPTICS)
					I_Error("NetUpdate: netbuffer->numtics > BACKUPTICS\n");

				resendto[destNode] = maketic - doomcom->extratics;
				for(j=0; j<netbuffer->numtics; j++)
				{
					netbuffer->cmds[j] = localcmds[(realstart+j)%BACKUPTICS];
				}
				// Don't send empty packets for nothing.
				if(netbuffer->numtics)
				{
					netbuffer->retransmitfrom = 0;
					HSendPacket(destNode, 0);
				}
			}

			// Retransmit requests will go also to the server.
			// We need to check these for each player in turn.
			netbuffer->starttic = maketic;
			netbuffer->numtics = 0;
			for(i=0; i<doomcom->numplayers; i++)
				if(players[i].ingame)
				{	
					int plrnode = I_DCNodeForPlayer(i);
					if(remoteresend[plrnode])
					{
						// The server answers these requests, but it can't know
						// what to reply if it doesn't know the target player.
						netbuffer->targetplayer = i;
						netbuffer->retransmitfrom = nettics[plrnode];
						HSendPacket(svnode, NCMD_RETRANSMIT);
					}
				}
		}
	}

//
// listen for other packets
//
listen:

	GetPackets ();
	unguard;
}


/*
=====================
=
= CheckAbort
=
=====================
*/

void CheckAbort (void)
{
	guardSlow(CheckAbort);
	event_t *ev;
	int             stoptic;

	stoptic = I_GetTime () + 2;
	while (I_GetTime() < stoptic)
		I_StartTic ();

	I_StartTic ();
	for ( ; eventtail != eventhead
	; eventtail = (++eventtail)&(MAXEVENTS-1) )
	{
		ev = &events[eventtail];
		if (ev->type == ev_keydown && ev->data1 == DDKEY_ESCAPE)
			I_Error ("Network game synchronization aborted.");
	}
	unguardSlow;
}



/*
===================
=
= D_CheckNetGame
=
= Works out player numbers among the net participants
===================
*/

//extern  int                     viewangleoffset;

//
// This is the network one-time initialization.
//
void D_CheckNetGame (void)
{
	guard(D_CheckNetGame);
	int             i, j;
//	int pClass;

	for (i=0 ; i<MAXNETNODES ; i++)
	{
		nodeingame[i] = false;
		nettics[i] = 0;
		remoteresend[i] = false;        // set when local needs tics
		resendto[i] = 0;                // which tic to start sending
		for(j=0; j<MAXNETNODES; j++) 
			svResendTo[i][j] = 0;
	}

// I_InitNetwork sets doomcom and netgame
	// Already done in H2_Main().
	//I_InitNetwork (); 

	if (doomcom->id != DOOMCOM_ID)
		I_Error ("Doomcom buffer invalid!");
	netbuffer = &doomcom->data;
	consoleplayer = displayplayer = doomcom->consoleplayer;

//ST_Message ("startskill %i  deathmatch: %i  startmap: %i  startepisode: %i\n", startskill, deathmatch, startmap, startepisode);

// read values out of doomcom
	ticdup = doomcom->ticdup;
	maxsend = BACKUPTICS/(2*ticdup)-1;
	if (maxsend<1)
		maxsend = 1;

	for(i=0; i<doomcom->numplayers; i++) 
		players[i].ingame = true;
	for(i=0; i<doomcom->numnodes; i++) 
		nodeingame[i] = true;

//ST_Message ("player %i of %i (%i nodes)\n", consoleplayer+1, doomcom->numplayers, doomcom->numnodes);
	unguard;

}

void D_StartNetGame()
{
	guard(D_StartNetGame);
	int		i, j;

	// Reset all the counters and other data.
	// We must make it appear as if the program had just been started.
	for (i=0 ; i<MAXNETNODES ; i++)
	{
		nodeingame[i] = false;
		nettics[i] = 0;
		remoteresend[i] = false;		// set when local needs tics
		resendto[i] = 0;				// which tic to start sending
		resendcount[i] = 0;
		for(j=0; j<MAXNETNODES; j++) svResendTo[i][j] = 0;
	}
	gametic = maketic = 0;
	lastnettic = 0;
	skiptics = 0;
	gametime = 0;
	frameon = 0;
	memset(frametics, 0, sizeof(frametics)); 
	memset(frameskip, 0, sizeof(frameskip));
	oldnettics = 0;
	oldentertics = 0;

	if (doomcom->id != DOOMCOM_ID) I_Error ("Doomcom buffer invalid!");

	netbuffer = &doomcom->data;
	consoleplayer = displayplayer = doomcom->consoleplayer;
	limbo = false;
	netgame = true;

	// read values out of doomcom
	ticdup = doomcom->ticdup;
	maxsend = BACKUPTICS/(2*ticdup)-1;
	if(maxsend < 1) maxsend = 1;

	for(i=0; i<doomcom->numplayers; i++) 
		players[i].ingame = true;
	for(i=0; i<doomcom->numnodes; i++) 
		nodeingame[i] = true;

	if(server)
	{
		// We are the serverplayer.
		serverplayer = consoleplayer;

		// Let's tell everybody that the game should be started.
		netbuffer->numtics = 0;
		netbuffer->player = consoleplayer;		// This becomes serverplayer on other systems.
		netbuffer->starttic = NE_START_GAME;	
		HSendPacket(-1, NCMD_SETUP);
	}
	unguard;
}

void D_SyncNetStart()
{
	guard(D_SyncNetStart);
	int		i;
	int		lastTime = 0;		// Make first update immediately.
	int		startTime = I_GetTime(), maxTime = 35 * 10;	// Timeout in 10 seconds.

if(debugfile) fprintf(debugfile, "D_SyncNetStart:\n");

	// Call the pre-serverstart routine.
	H2_NetServerStarted(true);

	if(server)
	{
		// All the players that have reported in as being ready.
		boolean plrReady[MAXPLAYERS];	

		memset(plrReady, 0, sizeof(plrReady));
		plrReady[consoleplayer] = true;	// The server is ready.
		for(i=0; i<MAXPLAYERS; i++)
			if(!players[i].ingame) plrReady[i] = true;

		if(debugfile)
		{
			fprintf(debugfile, "ready players: ");
			for(i=0; i<MAXPLAYERS; i++)
				fprintf(debugfile, "%i ", plrReady[i]);
			fprintf(debugfile, "\n");
		}

		// We'll wait until all the players are ready.
		do
		{
			if(I_GetTime() - startTime > maxTime)
			{
				ST_Message("Net sync timeout.\n");
				break;
			}

			// Get the packets.
			if(!HGetPacket()) continue;

			// We are not interested in other packets than setup ones.
			if(!(netbuffer->checksum & NCMD_SETUP)) continue;

			// Who sent this packet?
			if(netbuffer->starttic == NE_PLAYER_READY) // Ready?
			{
				plrReady[netbuffer->player] = true;
				if(debugfile) fprintf(debugfile, "player %d ready\n", netbuffer->player);
			}

			// Is it time to tell the players again?
			if(I_GetTime() > lastTime+15)
			{
				for(i=0; i<MAXPLAYERS; i++)
				{
					if(plrReady[i]) continue;

					netbuffer->numtics = 0;
					netbuffer->player = consoleplayer;
					netbuffer->starttic = NE_START_GAME;
					HSendPacket(i, NCMD_SETUP);

					if(debugfile) fprintf(debugfile, "sent packet to %i\n", i);
				}
				lastTime = I_GetTime();
			}
		
			// Check that everybody is set to go.
			for(i=0; i<MAXPLAYERS; i++) if(!plrReady[i]) break;
		} while(i < MAXPLAYERS);

if(debugfile) fprintf(debugfile, "* everybody ready, let's go!\n");

		// Now we can start, send a couple of start messages.
		for(i=0; i<3; i++)
		{
			netbuffer->numtics = 0;
			netbuffer->player = consoleplayer;
			netbuffer->starttic = NE_END_SYNC;
			HSendPacket(-1, NCMD_SETUP);
		}
	}
	else
	{
		// We'll send the server a message to tell we are ready.
		for(;;)
		{
			if(I_GetTime() - startTime > maxTime)
			{
				ST_Message("Net sync timeout.\n");
				break;
			}

			if(!HGetPacket()) continue;
			if(!(netbuffer->checksum & NCMD_SETUP)) continue;

			if(netbuffer->starttic == NE_END_SYNC) // Hey, let's go!
				break;
	
			// Is it time to send a reminder to the server?
			if(I_GetTime() > lastTime+15)
			{
				netbuffer->numtics = 0;
				netbuffer->player = consoleplayer;
				netbuffer->starttic = NE_PLAYER_READY; // Ready.
				HSendPacket(-2, NCMD_SETUP);

if(debugfile) fprintf(debugfile, "sending the server a packet\n");

				lastTime = I_GetTime();
			}
		}
if(debugfile) fprintf(debugfile, "* sync done\n");
	}
	unguard;
}

void D_StopNetGame(boolean closing)
{
	guard(D_StopNetGame);
	int		i;

	if(server)
	{
		// We are a server. This means we should inform all the 
		// connected clients that the server is about to stop.
		
		for(i=0; i<3; i++) // Send a bunch to be sure.
		{
			netbuffer->numtics = 0;
			netbuffer->player = consoleplayer;
			netbuffer->starttic = closing? NE_SERVER_CLOSED : NE_END_GAME;	
			HSendPacket(-1, NCMD_SETUP);
		}
	}

	// If we are just stopping, go to limbo mode.
	limbo = !closing;
	netgame = false;

	// All remote nodes are forgotten about.
	for(i=1; i<doomcom->numnodes; i++)
		nodeingame[i] = false;

	// The local player is here, of course.
	nodeingame[0] = true;

	// Only the consoleplayer remains.
	for(i=0; i<MAXPLAYERS; i++)
		if(i != consoleplayer)
			players[i].ingame = false;
	unguard;
}

/*
==================
=
= D_QuitNetGame
=
= Called before quitting to leave a net game without hanging the
= other players
=
==================
*/

void D_QuitNetGame (void)
{
	guard(D_QuitNetGame);
	int             i;

	if (debugfile)
		fclose (debugfile);

	if (!netgame /*|| !usergame */|| consoleplayer == -1 || !allow_net_traffic)
		return;

// send a bunch of packets for security
	if(netModel == NETM_PEER_TO_PEER)
	{
		netbuffer->player = consoleplayer;
		netbuffer->numtics = 0;
	}
	else if(netModel == NETM_MULTICAST)
	{
		netbuffer->player = consoleplayer;
		netbuffer->numtics = 0;
/*		if(server)
		{
			// This tells the clients the server's node.
			netbuffer->targetplayer = I_DCNodeForPlayer(consoleplayer);			
		}*/
	}
	for (i=0 ; i<4 ; i++)
	{
		/*for (j=1 ; j<doomcom->numnodes ; j++)
			if (nodeingame[j])
				HSendPacket (j, NCMD_EXIT);*/
		HSendPacket(-1, NCMD_EXIT);	// Broadcast.
		I_WaitVBL (1);
	}
	unguard;
}


void D_GetTicCmd(void *cmd, int player)
{
	guard(D_GetTicCmd);
	memcpy(cmd, &netcmds[player][(gametic/ticdup)%BACKUPTICS], sizeof(ticcmd_t));
	unguard;
}



/*
===============
=
= TryRunTics
=
===============
*/

extern  boolean advancedemo;

void TryRunTics (void)
{
	guard(TryRunTics);
	int 			i;
	int 			lowtic;
	int 			entertic;
	int 			realtics, availabletics;
	int 			counts;
	int 			numplaying;

//
// get real tics
//
	entertic = I_GetTime ()/ticdup;
	realtics = entertic - oldentertics;
	oldentertics = entertic;

//
// get available tics
//
	NetUpdate ();

	lowtic = DDMAXINT;
	numplaying = 0;
	for (i=0 ; i<doomcom->numnodes ; i++)
		if (nodeingame[i])
		{
			numplaying++;
			if (nettics[i] < lowtic)
				lowtic = nettics[i];
		}
	availabletics = lowtic - gametic/ticdup;


//
// decide how many tics to run
//
	if (realtics < availabletics-1)
		counts = realtics+1;
	else if (realtics < availabletics)
		counts = realtics;
	else
		counts = availabletics;
	if (counts < 1)
		counts = 1;

	frameon++;

if (debugfile)
	fprintf (debugfile,"=======real: %i  avail: %i  game: %i\n",realtics, availabletics,counts);

	if (allow_net_traffic)
	{
	//=============================================================================
	//
	//      ideally nettics[0] should be 1 - 3 tics above lowtic
	//      if we are consistantly slower, speed up time
	//
		for (i=0 ; i<MAXPLAYERS ; i++)
			if (players[i].ingame)
				break;
		if (consoleplayer == i)
		{       // the key player does not adapt
		}
		else
		{
			if (nettics[0] <= nettics[nodeforplayer[i]])
			{
				gametime--;
	//                      printf ("-");
			}
			frameskip[frameon&3] = (oldnettics > nettics[nodeforplayer[i]]);
			oldnettics = nettics[0];
			if (frameskip[0] && frameskip[1] && frameskip[2] && frameskip[3])
			{
				skiptics = 1;
	//                      printf ("+");
			}
		}
	//=============================================================================
	}       // demoplayback

	//
	// wait for new tics if needed
	//
		while (lowtic < gametic/ticdup + counts)
		{
/*if (debugfile)
	fprintf (debugfile,"wait: low %i < %i + %i\n", lowtic, gametic/ticdup, counts);*/

			NetUpdate ();
			lowtic = DDMAXINT;

			//if(debugfile) fprintf(debugfile, "nettics: ");

			for (i=0 ; i<doomcom->numnodes ; i++)
			{
				/*if(debugfile) fprintf(debugfile, "%i%s", nettics[i], 
					i==doomcom->numnodes-1? "\n" : " ");*/
				if (nodeingame[i] && nettics[i] < lowtic)
					lowtic = nettics[i];
			}

			if (lowtic < gametic/ticdup)
				I_Error ("TryRunTics: lowtic (%d) < gametic (%d)", lowtic, gametic/ticdup);

			// don't stay in here forever -- give the menu a chance to work
			if (I_GetTime ()/ticdup - entertic >= 20)
			{
				H2_Ticker();
				return;
			}
		}

//
// run the count * ticdup tics
//
	while (counts--)
	{
		for (i=0 ; i<ticdup ; i++)
		{
			if (gametic/ticdup > lowtic) I_Error ("gametic>lowtic");

			/*if (advancedemo)
				H2_DoAdvanceDemo ();*/
			H2_Ticker();
			G_Ticker();
			CON_Ticker();

			gametic++;
			systics++;
			//
			// modify command for duplicated tics
			//
			if (i != ticdup-1)
			{
				ticcmd_t	*cmd;
				int			buf;
				int         j;

				buf = (gametic/ticdup)%BACKUPTICS;
				for (j=0 ; j<MAXPLAYERS ; j++)
				{
					cmd = &netcmds[j][buf];
					
					if (cmd->buttons & BT_SPECIAL)
						cmd->buttons = 0;
				}
			}
		}
		NetUpdate ();                                   // check for new console commands
	}
	unguard;
}


