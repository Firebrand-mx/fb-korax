//**************************************************************************
//** 
//** dd_think.c
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

thinker_t thinkercap; // The head and tail of the thinker list

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// RunThinkers
//
//==========================================================================

void RunThinkers(void)
{
	thinker_t *currentthinker;

	currentthinker = thinkercap.next;
	while(currentthinker != &thinkercap)
	{
		if(currentthinker->function == (think_t)-1)
		{ // Time to remove it
			currentthinker->next->prev = currentthinker->prev;
			currentthinker->prev->next = currentthinker->next;
			Z_Free(currentthinker);
		}
		else if(currentthinker->function)
		{
			currentthinker->function(currentthinker);
		}
		currentthinker = currentthinker->next;
	}
}

//==========================================================================
//
// P_InitThinkers
//
//==========================================================================

void P_InitThinkers(void)
{
	thinkercap.prev = thinkercap.next = &thinkercap;
}

//==========================================================================
//
// P_AddThinker
//
// Adds a new thinker at the end of the list.
//
//==========================================================================

void P_AddThinker(thinker_t *thinker)
{
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;
}

//==========================================================================
//
// P_RemoveThinker
//
// Deallocation is lazy -- it will not actually be freed until its
// thinking turn comes up.
//
//==========================================================================

void P_RemoveThinker(thinker_t *thinker)
{
	thinker->function = (think_t) -1;
}
