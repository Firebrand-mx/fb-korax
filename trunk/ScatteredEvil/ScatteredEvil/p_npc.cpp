
//**************************************************************************
//**
//** p_npc.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "p_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

boolean P_TryWalk(mobj_t *actor);
boolean P_Move(mobj_t *actor);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
//	A_NPCStand
//
//==========================================================================

void A_NPCStand(mobj_t *actor)
{
	if (!actor->movecount)
	{
		//	Initialize standing time
		actor->movecount = 10 + (P_Random() & 15);
		return;
	}

	//	Wait
	actor->movecount--;
	if (actor->movecount)
	{
		return;
	}

	int basedir = P_Random() & 7;
	for (int i = 0; i < 4; i++)
	{
		//	Try to walk
		actor->movedir = (basedir + i) & 7;
		if (P_TryWalk(actor))
		{
			P_SetMobjState(actor, actor->info->seestate);
			return;
		}
		actor->movedir = (basedir - i) & 7;
		if (P_TryWalk(actor))
		{
			P_SetMobjState(actor, actor->info->seestate);
			return;
		}
	}
	actor->movedir = DI_NODIR;              // can't move
	actor->movecount = 2 + (P_Random() & 15);
}

//==========================================================================
//
//	A_NPCWalk
//
//==========================================================================

void A_NPCWalk(mobj_t *actor)
{
	if (!actor->movecount)
	{
		//	Initialize walking time
		actor->movecount = (P_Random() & 7) + 1;
		return;
	}

	// turn towards movement direction if not there yet
	if (actor->movedir < 8)
	{
		actor->angle &= (7 << 29);
		int delta = actor->angle - (actor->movedir << 29);
		if (delta > 0)
		{
			actor->angle -= ANG90 / 2;
		}
		else if (delta < 0)
		{
			actor->angle += ANG90 / 2;
		}
	}

	// walk around
	if (--actor->movecount < 0 || !P_Move(actor))
	{
		P_SetMobjState(actor, actor->info->spawnstate);
	}

	// make active sound
	if (actor->info->activesound && P_Random() < 3)
	{
		S_StartSound(actor, actor->info->activesound);
	}
}
