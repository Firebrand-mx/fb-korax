
//**************************************************************************
//**
//** DD_ACTN.C
//**
//** The Action Manager.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "dd_def.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

action_t *actions = NULL;		// Pointer to the actions list.

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

void DD_DefineActions(action_t *acts)
{
	guard(DD_DefineActions);
	// Store a pointer to the list of actions.
	actions = acts;
	unguard;
}

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

