// jtNet.h : main header file for the JTNET DLL
//

#ifndef __JTNETDLL_H__
#define __JTNETDLL_H__

#include <dplay.h>
#include <dplobby.h>
#include "jtNetEx.h"

struct jtnetcon_t
{
	GUID	guid;
	int		type;			// JTNET_SERVICE_*
	void	*connection;	// The connection data.
	int		size;			// Size of the connection data.
	char	name[100];		
};

struct jtnetsession_t
{
	DPSESSIONDESC2		desc;
	char				name[64];
	char				info[128];
};

#ifndef JTNET_DX3
extern LPDIRECTPLAY4A	dPlay;
#else
extern LPDIRECTPLAY2A	dPlay;
#endif

extern jtnetcon_t		*connections;
extern int				numConnections;
extern jtnetsession_t	*sessions;
extern int				numSessions;
extern jtnetplayer_t	*players;
extern int				numPlayers;

int jtEnumerateSessions();
int jtEnumeratePlayers();

// Version information.
#define JTNET_VERSION_NUM			202
#define JTNET_VERSION_STR			"2.02"

#ifndef JTNET_DX3
#define JTNET_VER_ID				"DX6"
#else
#define JTNET_VER_ID				"DX3 BETA"
#endif

#define JTNET_VERSION_FULL			"jtNet Version 2.02 "__DATE__" ("JTNET_VER_ID")"

// jtNet messages.
#define JTNET_MSG_BEGIN_SESSION		0xffff0000
#define JTNET_MSG_OK				0xffff1234
#define JTNET_MSG_CANCEL			0xffff4321


#endif __JTNETDLL_H__
