
//**************************************************************************
//**
//** H2_NET.C ** JHexen networking
//** -Jaakko Keränen
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "p_local.h"
#include "settings.h"
#include "h2_net.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// servdata_t can be *max* 12 bytes long (3 ints).
typedef struct
{
	unsigned char	nomonsters : 1;
	unsigned char	respawn : 1;
	unsigned char	randclass : 1;
	unsigned char	skill : 4;
	unsigned char	map : 7;
	unsigned char	deathmatch : 1;
	unsigned char	slot : 3;
    /*float           nmdamage; // netMobDamageModifier
    float           nmhealth; // netMobHealthModifier*/
	unsigned char	nmdamage, nmhealth; // netMobDamage/HealthModifier
} servdata_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

boolean G_CheckDemoStatus();

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean advancedemo;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

void UpdateServerData(int set)
{
	servdata_t svd;

	if(set)
	{
		// Pack the data into the server data word.
		svd.deathmatch = netDeathmatch;
		svd.map = netMap;
		svd.nomonsters = netNomonsters;
		svd.randclass = netRandomclass;
		svd.respawn = netRespawn;
		svd.skill = netSkill;
		svd.slot = netSlot;
        svd.nmdamage = netMobDamageModifier;
        svd.nmhealth = netMobHealthModifier;
		I_NetSetServerData(&svd, sizeof(svd));
	}
	else
	{
		I_NetGetServerData(&svd, sizeof(svd));
		// Unpack the data.
		netDeathmatch = svd.deathmatch;
		netMap = svd.map;
		netNomonsters = svd.nomonsters;
		netRandomclass = svd.randclass;
		netRespawn = svd.respawn;
		netSkill = svd.skill;
		netSlot = svd.slot;
        netMobDamageModifier = svd.nmdamage; 
        netMobHealthModifier = svd.nmhealth; 
	}
}

int H2_NetServerOpen(int before)
{
	if(before) 
		UpdateServerData(true);
	else
	{
		plrdata_t pd = { netClass, netColor };
		I_NetSetPlayerData(&pd, sizeof(pd));
	}
	return true;
}

int H2_NetServerClose(int before)
{
	if(!before)
	{
		// Restore normal game state.
		deathmatch = false;
		nomonsters = false;

		P_SetMessage(&players[consoleplayer], "NETGAME ENDS", true);
		S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
	}
	return true;
}

int H2_NetServerStarted(int before)
{
	int			i;
	plrdata_t	pd;

	if(before)
	{
		// If we are the server, we must update the game data.
		if(server) UpdateServerData(true);
		
		// Update our player data, for the last time.
		pd.pclass = netClass;
		pd.color = netColor;
		I_NetSetPlayerData(&pd, sizeof(pd));
		return true;
	}
	// For clients: read the final server data.
	if(!server)	UpdateServerData(false);

	// Let's read the data of each player.
	for(i=0; i<MAXPLAYERS; i++)
	{
		if(players[i].ingame)
		{
			I_NetGetPlayerData(i, &pd, sizeof(pd));
			PlayerClass[i] = (pclass_t)(pd.pclass);
			PlayerColor[i] = pd.color;
		}
	}

	// Set the game parameters.
	deathmatch = netDeathmatch;
	nomonsters = netNomonsters;
	randomclass = netRandomclass;
	respawnparm = netRespawn;

	G_CheckDemoStatus();
	advancedemo = false;

	// Start the game.	
	if(!netSlot)
	{
		G_StartNewInit();
		G_InitNew((skill_t)netSkill, 1, P_TranslateMap(netMap), 1.0);
	}
	else
	{
		// Load the game from the slot.
		G_LoadGame(netSlot-1);
	}
	// Close the menu, the game begins!!
	MN_DeactivateMenu();
	return true;
}

int H2_NetConnect(int before)
{
	plrdata_t	pd = { netClass, netColor };

	// We do nothing before the actual connection is made.
	if(before) return true;
	
	// After connecting we get the game data from the server.
	UpdateServerData(false);

	// Set the player data.
	I_NetSetPlayerData(&pd, sizeof(pd));

	// Some diagnostics.
	ST_Message("Netgame: map%i, skill%i, dm%i, rs%i, rnd%i, !mnst%i, slot%i,\nnmDamage%i, nmHealth%i\n",
		netMap, netSkill, netDeathmatch, netRespawn, netRandomclass, netNomonsters, 
		netSlot, netMobDamageModifier, netMobHealthModifier);
	return true;
}

int H2_NetDisconnect(int before)
{
	if(before) return true;

	// Restore normal game state.
	deathmatch = false;
	nomonsters = false;
	return true;
}

int H2_NetPlayerEvent(int plrNumber, int peType, void *data)
{
	char	msgbuff[256];

	// If this isn't a netgame, we won't react.
	if(!netgame) return true;

	if(peType == DDPE_EXIT)
	{
		// A player has exited a netgame?
		strcpy(msgbuff, "PLAYER 1 LEFT THE GAME");
		msgbuff[7] += plrNumber;
		P_SetMessage(&players[consoleplayer], msgbuff, true);
		S_StartSound(NULL, SFX_CHAT);
	}
	// Don't show local chat messages.
	else if(peType == DDPE_CHAT_MESSAGE && plrNumber != consoleplayer)
	{
		int i, num, oldecho = echoMsg;
		// Count the number of players.
		for(i=num=0; i<MAXPLAYERS; i++)
			if(players[i].ingame) num++;
		// If there are more than two players, include the name of
		// the player who sent this.
		if(num > 2)
			sprintf(msgbuff, "%s: %s", I_NetGetPlayerName(plrNumber), data);
		else
			strcpy(msgbuff, (char *)data);
		// The chat message is already echoed by the console.
		echoMsg = false;
		P_SetMessage(&players[consoleplayer], msgbuff, true);
		echoMsg = oldecho;
		S_StartSound(NULL, SFX_CHAT);
	}
	return true;
}

