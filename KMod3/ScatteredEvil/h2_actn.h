#ifndef __JHEXEN_ACTIONS_H__
#define __JHEXEN_ACTIONS_H__

#include "h2def.h"

// These must correspond the action_t's in the actions array!
// Things that are needed in building ticcmds should be here.
typedef enum 
{
	// Game controls.
	H2A_TURNLEFT,
	H2A_TURNRIGHT,
	H2A_FORWARD,
	H2A_BACKWARD,
	H2A_STRAFELEFT,
	H2A_STRAFERIGHT,
	H2A_JUMP,
	H2A_FIRE,
	H2A_USE,
	H2A_STRAFE,
	H2A_SPEED,
	H2A_FLYUP,
	H2A_FLYDOWN,
	H2A_FLYCENTER,
	H2A_LOOKUP,
	H2A_LOOKDOWN,
	H2A_LOOKCENTER,
	H2A_USEARTIFACT,
	H2A_MLOOK,
	H2A_JLOOK,
	H2A_NEXTWEAPON,
	H2A_PREVIOUSWEAPON,
	H2A_WEAPON1,
	H2A_WEAPON2,
	H2A_WEAPON3,
	H2A_WEAPON4,

	// Item hotkeys.
	H2A_PANIC,
	H2A_TORCH,
	H2A_HEALTH,
	H2A_MYSTICURN,
	H2A_KRATER,
	H2A_SPEEDBOOTS,
	H2A_BLASTRADIUS,
	H2A_TELEPORT,
	H2A_TELEPORTOTHER,
	H2A_POISONBAG,
	H2A_INVULNERABILITY,
	H2A_DARKSERVANT,
	H2A_EGG,

	// Game system actions.
	H2A_STOPDEMO,		
	H2A_DUCK,		
	H2A_SPELL1,		
	H2A_SPELL2,		
	H2A_SPELL3,
	H2A_SPELL4,		
	H2A_SPELL5,		
	H2A_SPELL6,		
	H2A_SPELL7,		
	H2A_SPELL8,		
	H2A_USESPELL,

	NUM_ACTIONS
} h2action_t;

// This is the actions array.
extern action_t actions[NUM_ACTIONS+1];

#endif