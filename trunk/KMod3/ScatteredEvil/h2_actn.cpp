
//**************************************************************************
//**
//** H2_ACTN.C
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "h2_actn.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

action_t actions[NUM_ACTIONS+1] =
{
	"left", 0,
	"right", 0,
	"forward", 0,
	"backward", 0,
	"strafel", 0,
	"strafer", 0,
	"jump", 0,
	"fire", 0,
	"use", 0,
	"strafe", 0,
	"speed", 0,
	"flyup", 0,
	"flydown", 0,
	"falldown", 0,
	"lookup", 0,
	"lookdown", 0,
	"lookcntr", 0,
	"usearti", 0,
	"mlook", 0,
	"jlook", 0,
	"nextwpn", 0,
	"prevwpn", 0,
	"weapon1", 0,
	"weapon2", 0,
	"weapon3", 0,
	"weapon4", 0,
	"panic", 0,
	"torch", 0,
	"health", 0,
	"mystic", 0,
	"krater", 0,
	"spdboots", 0,
	"blast", 0,
	"teleport", 0,
	"teleothr", 0,
	"poison", 0,
	"cantdie", 0,
	"servant", 0,
	"egg", 0,
	"demostop", 0,
	"duck", 0,
	"spell1", 0,
	"spell2", 0,
	"spell3", 0,
	"spell4", 0,
	"spell5", 0,
	"spell6", 0,
	"spell7", 0,
	"spell8", 0,
/*	"sell", 0,
	"reply", 0,*/
	"", 0
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

void DD_ClearActions(void)
{
	guard(DD_ClearActions);
	action_t *act;
	for(act=actions; act->name[0]; act++) act->on = false;
	unguard;
}

// The command begins with a '+' or a '-'.
// Returns true if the action was changed successfully.
// If has_prefix is false, the state of the action is negated.
int DD_ActionCommand(char *cmd, boolean has_prefix)
{
	guard(DD_ActionCommand);
	char		prefix = cmd[0];
	int			v1, v2;
	char		name8[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	action_t	*act;

	if(actions == NULL) return false;
		
	strncpy(name8, cmd + (has_prefix? 1 : 0), 8);
	v1 = *(int*) name8;
	v2 = *(int*) &name8[4];

	// Try to find a matching action by searching through the list.
	for(act=actions; act->name[0]; act++)
	{
		if(v1 == *(int*) act->name && v2 == *(int*) &act->name[4])
		{
			// This is a match!
			if(has_prefix)
				act->on = prefix=='+'? true : false;
			else
				act->on = !act->on;
			return true;
		}
	}
	return false;
	unguard;
}

int CCmdListActs(int argc, char **argv)
{
	guard(CCmdListActs);
	action_t	*act;

	ST_Message( "Actions commands registered by the game DLL:\n");
	for(act=actions; act->name[0]; act++)
		ST_Message("  %s\n", act->name);
	return true;
	unguard;
}

