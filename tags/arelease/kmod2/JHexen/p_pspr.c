
//**************************************************************************
//**
//** p_pspr.c : Heretic 2 : Raven Software, Corp.
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
#include "soundst.h"
//#include "ogl_def.h"

// MACROS ------------------------------------------------------------------

#define LOWERSPEED FRACUNIT*6
#define RAISESPEED FRACUNIT*6
#define WEAPONBOTTOM 128*FRACUNIT
#define WEAPONTOP 32*FRACUNIT
#define MAGIC_JUNK 1234

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern void P_ExplodeMissile(mobj_t *mo);
extern void A_UnHideThing(mobj_t *actor);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern fixed_t FloatBobOffsets[64];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

fixed_t bulletslope;


//New Weapon Info Table
//Remember that all possessed weaponry needs names
newweaponinfo_t NewWeaponInfo[NUMACTUALWEAPONS] =
{
{ // Fighter First Weapon - Punch - WP_FIGHTER_PUNCH
	MANA_NONE,0,		// mana
	S_PUNCHUP,S_PUNCHDOWN,S_PUNCHREADY,
	S_PUNCHATK1_1,S_PUNCHATK1_1,S_NULL,
	PCLASS_FIGHTER,0,
	0
	
},	
{ // Cleric First Weapon - Mace - WP_CLERIC_MACE
	MANA_NONE,0,			// mana
	S_CMACEUP,S_CMACEDOWN,S_CMACEREADY,
	S_CMACEATK_1,S_CMACEATK_1,S_NULL,
	PCLASS_CLERIC,0,
	-8*FRACUNIT
},
{ // Mage First Weapon - Red Wand - WP_MAGE_WAND3
	MANA_NONE,0,
	S_MWAND3UP,S_MWAND3DOWN,S_MWAND3READY,
	S_MWAND3ATK_1,S_MWAND3ATK_1,S_NULL,
	PCLASS_MAGE,0,
	9*FRACUNIT
},
{ // Mage First Weapon - Black Wand - WP_MAGE_WAND2
	MANA_NONE,0,
	S_MWAND2UP,S_MWAND2DOWN,S_MWAND2READY,
	S_MWAND2ATK_1,S_MWAND2ATK_1,S_NULL,
	PCLASS_MAGE,0,
	9*FRACUNIT
},
{ // Mage First Weapon - Wand - WP_MAGE_WAND
	MANA_NONE,0,
	S_MWANDUP,S_MWANDDOWN,S_MWANDREADY,
	S_MWANDATK_1,S_MWANDATK_1,S_NULL,
	PCLASS_MAGE,0,
	9*FRACUNIT
},
{ // Mage First Weapon - Yellow Wand - WP_MAGE_WAND4
	MANA_NONE,0,
	S_MWAND4UP,S_MWAND4DOWN,S_MWAND4READY,
	S_MWAND4ATK_1,S_MWAND4ATK_1,S_NULL,
	PCLASS_MAGE,0,
	9*FRACUNIT
},
{ // Corvus First Weapon - Gauntlets - WP_CORVUS_GAUNTLETS
	MANA_NONE,0,
	S_GAUNTLETUP,S_GAUNTLETDOWN,S_GAUNTLETREADY,
	S_GAUNTLETATK1_1,S_GAUNTLETATK1_3,S_NULL,
	PCLASS_CORVUS,0,
	15*FRACUNIT
},
{ // Corvus First Weapon - Powered Gauntlets - WP_CORVUS_PGAUNTLETS
	MANA_NONE,0,
	S_GAUNTLETUP2,S_GAUNTLETDOWN2,S_GAUNTLETREADY2_1,
	S_GAUNTLETATK2_1,S_GAUNTLETATK2_3,S_NULL,
	PCLASS_CORVUS,20,
	15*FRACUNIT
},
{ // Corvus First Weapon - Staff - WP_CORVUS_STAFF
	MANA_NONE,0,
	S_STAFFUP,S_STAFFDOWN,S_STAFFREADY,		
	S_STAFFATK1_1,S_STAFFATK1_1,S_NULL,
	PCLASS_CORVUS,0,
	0
},
{ // Corvus First Weapon - Powered Staff - WP_CORVUS_PSTAFF
	MANA_NONE,0,
	S_STAFFUP2,S_STAFFDOWN2,S_STAFFREADY2_1,		
	S_STAFFATK2_1,S_STAFFATK2_1,S_NULL,
	PCLASS_CORVUS,20,
	0
},
{ // Pig - Snout - WP_PIG_SNOUT
	MANA_NONE,0,			// mana
	S_SNOUTUP,S_SNOUTDOWN,S_SNOUTREADY,
	S_SNOUTATK1,S_SNOUTATK1,S_NULL,
	PCLASS_PIG,0,
	10*FRACUNIT
},
{ // Poss_Ettin First Weapon - Mace - WP_ETTIN_MACE
	MANA_NONE,0,
	S_POSS_1ETTINUP,S_POSS_1ETTINDOWN,S_POSS_1ETTINREADY,
	S_POSS_1ETTINATK_1,S_POSS_1ETTINATK_1,S_NULL,
	PCLASS_ETTIN,0,
	0,
	"MACE"
},
{ // Poss_Afrit First Weapon - Triple Fireball - WP_AFRIT_FIREBALL
	MANA_NONE,0,
	S_POSS_1AFRITUP,S_POSS_1AFRITDOWN,S_POSS_1AFRITREADY,
	S_POSS_1AFRITATK_1,S_POSS_1AFRITATK_1,S_NULL,
	PCLASS_AFRIT,0,
	0,
	"FIREBALLS"
},
{ // Poss_Centaur First Weapon - Sword - WP_CENTAUR_SWORD
	MANA_NONE,0,
	S_POSS_1CENTAURUP,S_POSS_1CENTAURDOWN,S_POSS_1CENTAURREADY,
	S_POSS_1CENTAURATK_1,S_POSS_1CENTAURATK_1,S_NULL,
	PCLASS_CENTAUR,0,
	0,
	"SWORD"
},
{ // Poss_Centaur Leader First Weapon - Sword - WP_CENTAURL_SWORD
	MANA_NONE,0,
	S_POSS_1CENTAURUP,S_POSS_1CENTAURDOWN,S_POSS_1CENTAURREADY,
	S_POSS_1CENTAURATK_1,S_POSS_1CENTAURATK_1,S_NULL,
	PCLASS_CENTAURL,0,
	0,
	"SWORD"
},
{ // Poss_Iceguy First Weapon - Missile - WP_ICEGUY_MISSILE
	MANA_NONE,0,
	S_POSS_1ICEGUYUP,S_POSS_1ICEGUYDOWN,S_POSS_1ICEGUYREADY,
	S_POSS_1ICEGUYATK_1,S_POSS_1ICEGUYATK_1,S_NULL,
	PCLASS_ICEGUY,0,
	0,
	"MISSILE"
},
{ // Poss_Serpent First Weapon - Bite - WP_SERPENT_BITE
	MANA_NONE,0,
	S_POSS_1SERPENTUP,S_POSS_1SERPENTDOWN,S_POSS_1SERPENTREADY,
	S_POSS_1SERPENTATK_1,S_POSS_1SERPENTATK_1,S_NULL,
	PCLASS_SERPENT,0,
	0,
	"BITE"
},
{ // Poss_Brown Serpent First Weapon - Bite - WP_BSERPENT_BITE
	MANA_NONE,0,
	S_POSS_1SERPENTUP,S_POSS_1SERPENTDOWN,S_POSS_1SERPENTREADY,
	S_POSS_1SERPENTATK_1,S_POSS_1SERPENTATK_1,S_NULL,
	PCLASS_BSERPENT,0,
	0,
	"BITE"
},
{ // Poss_Reiver First Weapon - Melee - WP_REIVER_MELEE
	MANA_NONE,0,
	S_POSS_1WRAITHUP,S_POSS_1WRAITHDOWN,S_POSS_1WRAITHREADY,
	S_POSS_1WRAITHATK_1,S_POSS_1WRAITHATK_1,S_NULL,
	PCLASS_REIVER,0,
	0,
	"MELEE"
},
{ // Poss_Bishop First Weapon - Melee - WP_BISHOP_MELEE
	MANA_NONE,0,
	S_POSS_1BISHOPUP,S_POSS_1BISHOPDOWN,S_POSS_1BISHOPREADY,
	S_POSS_1BISHOPATK_1,S_POSS_1BISHOPATK_1,S_NULL,
	PCLASS_BISHOP,0,
	0,
	"MELEE"
},
{ // Fighter - Powered Axe - WP_FIGHTER_AXE_G
	MANA_1,2,
	S_FAXEUP_G,S_FAXEDOWN_G,S_FAXEREADY_G,
	S_FAXEATK_G1,S_FAXEATK_G1,S_NULL,
	PCLASS_FIGHTER,1,
	-10*FRACUNIT
},
{ // Fighter - Axe - WP_FIGHTER_AXE
	MANA_NONE,0,
	S_FAXEUP,S_FAXEDOWN,S_FAXEREADY,
	S_FAXEATK_1,S_FAXEATK_1,S_NULL,
	PCLASS_FIGHTER,1,
	-10*FRACUNIT
},
{ // Cleric - Serpent Staff - WP_CLERIC_STAFF
	MANA_1,1,
	S_CSTAFFUP,S_CSTAFFDOWN,S_CSTAFFREADY,
	S_CSTAFFATK_1,S_CSTAFFATK_1,S_NULL,
	PCLASS_CLERIC,1,
	10*FRACUNIT
},
{ // Mage - Cone of shards - WP_MAGE_CONE
	MANA_1,3,
	S_CONEUP,S_CONEDOWN,S_CONEREADY,
	S_CONEATK1_1,S_CONEATK1_3,S_NULL,
	PCLASS_MAGE,1,
	20*FRACUNIT
},
{ // Corvus - Gold wand - WP_CORVUS_GWAND
	AM_GOLDWAND,1,
	S_GOLDWANDUP,S_GOLDWANDDOWN,S_GOLDWANDREADY,
	S_GOLDWANDATK1_1,S_GOLDWANDATK1_1,S_NULL,
	PCLASS_CORVUS,1,
	5*FRACUNIT
},
{ // Corvus - Powered Gold wand - WP_CORVUS_PGWAND
	AM_GOLDWAND,1,
	S_GOLDWANDUP,S_GOLDWANDDOWN,S_GOLDWANDREADY,
	S_GOLDWANDATK2_1,S_GOLDWANDATK2_1,S_NULL,
	PCLASS_CORVUS,21,
	5*FRACUNIT
},
{ // Poss_Centaur Second Weapon - Shield - WP_CENTAUR_SHIELD
	MANA_NONE,0,		// mana
	S_POSS_2CENTAURUP,S_POSS_2CENTAURDOWN,S_POSS_2CENTAURREADY,
	S_POSS_2CENTAURATK1,S_POSS_2CENTAURATK1,S_NULL,
	PCLASS_CENTAUR,1,
	0,
	"SHIELD"
},
{ // Poss_Centaur Leader Second Weapon - Shield - WP_CENTAURL_SHIELD
	MANA_NONE,0,
	S_POSS_2CENTAURUP,S_POSS_2CENTAURDOWN,S_POSS_2CENTAURREADY,
	S_POSS_2CENTAURATK1,S_POSS_2CENTAURATK1,S_NULL,
	PCLASS_CENTAURL,1,
	0,
	"SHIELD"
},		
{ // Poss_Serpent Second Weapon - Fireball - WP_SERPENT_FIREBALL
	MANA_NONE,0,
	S_POSS_2SERPENTUP,S_POSS_2SERPENTDOWN,S_POSS_2SERPENTREADY,
	S_POSS_2SERPENTATK_1,S_POSS_2SERPENTATK_1,S_NULL,
	PCLASS_SERPENT,1,
	0,
	"FIREBALL"
},
{ // Poss_Brown Serpent Second Weapon - Missile - WP_BSERPENT_MISSILE
	MANA_NONE,0,
	S_POSS_2SERPENTUP,S_POSS_2SERPENTDOWN,S_POSS_2SERPENTREADY,
	S_POSS_2SERPENTATK_1,S_POSS_2SERPENTATK_1,S_NULL,
	PCLASS_BSERPENT,1,
	0,
	"MISSILE"
},
{ // Poss_Reiver Second Weapon - Fireball - WP_REIVER_FIREBALL
	MANA_NONE,0,
	S_POSS_2WRAITHUP,S_POSS_2WRAITHDOWN,S_POSS_2WRAITHREADY,
	S_POSS_2WRAITHATK_1,S_POSS_2WRAITHATK_1,S_NULL,
	PCLASS_REIVER,1,
	0,
	"FIREBALL"
},
{ // Poss_Bishop Second Weapon - Swirly Missiles - WP_BISHOP_MISSILES
	MANA_NONE,0,
	S_POSS_2BISHOPUP,S_POSS_2BISHOPDOWN,S_POSS_2BISHOPREADY,
	S_POSS_2BISHOPATK_1,S_POSS_2BISHOPATK_1,S_NULL,
	PCLASS_BISHOP,1,
	0,
	"MISSILES"
},
{ // Fighter - Hammer - WP_FIGHTER_HAMMER
	MANA_NONE,3,
	S_FHAMMERUP,S_FHAMMERDOWN,S_FHAMMERREADY,
	S_FHAMMERATK_1,S_FHAMMERATK_1,S_NULL,
	PCLASS_FIGHTER,2,
	10*FRACUNIT
},
{ // Cleric - Flame Strike - WP_CLERIC_FLAME
	MANA_2,4,
	S_CFLAMEUP,S_CFLAMEDOWN,S_CFLAMEREADY1,
	S_CFLAMEATK_1,S_CFLAMEATK_1,S_NULL,
	PCLASS_CLERIC,2,
	10*FRACUNIT
},
{ // Mage - Lightning - WP_MAGE_LIGHTNING
	MANA_2,5,
	S_MLIGHTNINGUP,S_MLIGHTNINGDOWN,S_MLIGHTNINGREADY,
	S_MLIGHTNINGATK_1,S_MLIGHTNINGATK_1,S_NULL,
	PCLASS_MAGE,2,
	20*FRACUNIT
},
{ // Corvus - Crossbow - WP_CORVUS_CROSSBOW
	AM_CROSSBOW,1,
	S_CRBOWUP,S_CRBOWDOWN,S_CRBOW1,
	S_CRBOWATK1_1,S_CRBOWATK1_1,S_NULL,
	PCLASS_CORVUS,2,
	15*FRACUNIT
},
{ // Corvus - Powered Crossbow - WP_CORVUS_PCROSSBOW
	AM_CROSSBOW,1,
	S_CRBOWUP,S_CRBOWDOWN,S_CRBOW1,
	S_CRBOWATK2_1,S_CRBOWATK2_1,S_NULL,
	PCLASS_CORVUS,22,
	15*FRACUNIT
},
{ // Poss_Centaur Leader Third Weapon - Missile - WP_CENTAURL_MISSILE
	MANA_NONE,0,
	S_POSS_3CENTAURUP,S_POSS_3CENTAURDOWN,S_POSS_3CENTAURREADY,
	S_POSS_3CENTAURATK_1,S_POSS_3CENTAURATK_1,S_NULL,
	PCLASS_CENTAURL,2,
	0,
	"MISSILE"
},
{ // Fighter - Rune Sword - WP_FIGHTER_SWORD_G
	MANA_BOTH,14,
	S_FSWORDUP,S_FSWORDDOWN,S_FSWORDREADY,
	S_FSWORDATK_1,S_FSWORDATK_1,S_NULL,
	PCLASS_FIGHTER,3,
	10*FRACUNIT
},
{ // Fighter - Rune Sword - WP_FIGHTER_SWORD
	MANA_NONE,0,
	S_FSWORD2UP,S_FSWORD2DOWN,S_FSWORD2READY,
	S_FSWORD2ATK_1,S_FSWORD2ATK_1,S_NULL,
	PCLASS_FIGHTER,3,
	10*FRACUNIT
},
{ // Cleric - Holy Symbol - WP_CLERIC_HOLY
	MANA_BOTH,18,
	S_CHOLYUP,S_CHOLYDOWN,S_CHOLYREADY,
	S_CHOLYATK_1,S_CHOLYATK_1,S_NULL,
	PCLASS_CLERIC,3,
	0
},
{ // Mage - Staff - WP_MAGE_STAFF
	MANA_BOTH,15,
	S_MSTAFFUP,S_MSTAFFDOWN,S_MSTAFFREADY,
	S_MSTAFFATK_1,S_MSTAFFATK_1,S_NULL,
	PCLASS_MAGE,3,
	20*FRACUNIT
},
{ // Corvus - Mace - WP_CORVUS_MACE
	AM_MACE,1,
	S_MACEUP,S_MACEDOWN,S_MACEREADY,
	S_MACEATK1_1,S_MACEATK1_2,S_NULL,
	PCLASS_CORVUS,6,
	15*FRACUNIT
},
{ // Corvus - Powered Mace - WP_CORVUS_PMACE
	AM_MACE,5,
	S_MACEUP,S_MACEDOWN,S_MACEREADY,
	S_MACEATK2_1,S_MACEATK2_2,S_NULL,
	PCLASS_CORVUS,26,
	15*FRACUNIT
},
};


// CODE --------------------------------------------------------------------

//---------------------------------------------------------------------------
//
// PROC P_SetPsprite
//
//---------------------------------------------------------------------------

void P_SetPsprite(player_t *player, int position, statenum_t stnum)
{
	pspdef_t *psp;
	state_t *state;

	psp = &player->psprites[position];
	do
	{
		if(!stnum)
		{ // Object removed itself.
			psp->state = NULL;
			break;
		}
		state = &states[stnum];
		psp->state = state;
		psp->tics = state->tics; // could be 0

		//Some weaponry is based on speed rating
		if (psp->tics != 0 && (player->readyweapon == WP_FIGHTER_PUNCH ||
			player->readyweapon == WP_CLERIC_MACE ||
			player->readyweapon == WP_FIGHTER_AXE_G ||
			player->readyweapon == WP_FIGHTER_AXE ||
			player->readyweapon == WP_FIGHTER_HAMMER ||
			player->readyweapon == WP_FIGHTER_SWORD ||
			player->readyweapon == WP_FIGHTER_SWORD_G))
		{
			if (player->speed <6)
				psp->tics += 1;
			else if (player->speed >34)
				psp->tics -= 3;
			else if (player->speed >24)
				psp->tics -= 2;
			else if (player->speed >14)
				psp->tics -= 1;

			if (psp->tics < 1)
				psp->tics = 1;
		}

		if(state->misc1)
		{ // Set coordinates.
			psp->sx = state->misc1<<FRACBITS;
		}
		if(state->misc2)
		{
			psp->sy = state->misc2<<FRACBITS;
		}
		if(state->action)
		{ // Call action routine.
			state->action(player, psp);
			if(!psp->state)
			{
				break;
			}
		}
		stnum = psp->state->nextstate;
	} while(!psp->tics); // An initial state of 0 could cycle through.
}

//---------------------------------------------------------------------------
//
// PROC P_SetPspriteNF
//
// Identical to P_SetPsprite, without calling the action function
//---------------------------------------------------------------------------

void P_SetPspriteNF(player_t *player, int position, statenum_t stnum)
{
	pspdef_t *psp;
	state_t *state;

	psp = &player->psprites[position];
	do
	{
		if(!stnum)
		{ // Object removed itself.
			psp->state = NULL;
			break;
		}
		state = &states[stnum];
		psp->state = state;
		psp->tics = state->tics; // could be 0
		
		//Some weaponry is based on speed rating
		if (psp->tics != 0 && (player->readyweapon == WP_FIGHTER_PUNCH ||
			player->readyweapon == WP_CLERIC_MACE ||
			player->readyweapon == WP_FIGHTER_AXE ||
			player->readyweapon == WP_FIGHTER_HAMMER ||
			player->readyweapon == WP_FIGHTER_SWORD))
		{
			if (player->speed <6)
				psp->tics += 1;
			else if (player->speed >34)
				psp->tics -= 3;
			else if (player->speed >24)
				psp->tics -= 2;
			else if (player->speed >14)
				psp->tics -= 1;

			if (psp->tics < 1)
				psp->tics = 1;
		}

		if(state->misc1)
		{ // Set coordinates.
			psp->sx = state->misc1<<FRACBITS;
		}
		if(state->misc2)
		{
			psp->sy = state->misc2<<FRACBITS;
		}
		stnum = psp->state->nextstate;
	} while(!psp->tics); // An initial state of 0 could cycle through.
}

/*
=================
=
= P_CalcSwing
=
=================
*/

/*
fixed_t	swingx, swingy;
void P_CalcSwing (player_t *player)
{
	fixed_t	swing;
	int		angle;

// OPTIMIZE: tablify this

	swing = player->bob;

	angle = (FINEANGLES/70*leveltime)&FINEMASK;
	swingx = FixedMul ( swing, finesine[angle]);

	angle = (FINEANGLES/70*leveltime+FINEANGLES/2)&FINEMASK;
	swingy = -FixedMul ( swingx, finesine[angle]);
}
*/

//---------------------------------------------------------------------------
//
// PROC P_GetWeaponNum
//
//---------------------------------------------------------------------------

int P_GetWeaponNum(player_t *player, newweapontype_t weapontype)
{
	int i;
	int weaponnumber = 50;

	for (i = 0; i < NUMACTUALWEAPONS; i++) //First find the highest weaponnumber
	{		
		if (NewWeaponInfo[i].classtype == player->class)
		{
			if (weaponnumber == 50)
				weaponnumber = 0;
			else weaponnumber++;
		}
		if (i == weapontype)
		{ //Found the weapon the code was looking for		
			return (weaponnumber);
		}
	}
	return (0);
}

//---------------------------------------------------------------------------
//
// PROC P_NewPendingWeapon
//
//---------------------------------------------------------------------------

void P_NewPendingWeapon(player_t *player, int key)
{
	int i;
	int weaponnumber = 50; //Used for checking if the player has weapon
	int j = 0;
	//0=Nothing, 1=First or only, 2=Last, 3 = Not first or last, 4=Not in line
	pclass_t class=player->class;

	if (key == 50)
	{ //Special case, go for the best weapon possible and check mana
		for (i = 0; i < NUMACTUALWEAPONS; i++) //First find the highest weaponnumber
		{
			if (NewWeaponInfo[i].classtype == class)
			{
				if (weaponnumber == 50)
					weaponnumber = 0;
				else weaponnumber++;
			}
		}
		for (i = NUMACTUALWEAPONS-1; i > 0; i--)
		{
			if (NewWeaponInfo[i].classtype == class)
			{ //Found possible weapon
				if ((class >= PCLASS_ETTIN || player->weaponowned[weaponnumber])
					&& i != player->readyweapon)
				{ //Player has the weapon and don't have it up, check for mana
					// -JL- Check the actual mana count this weapon will use
					int manaCount = NewWeaponInfo[i].ammouse*10/player->agility;
					if (NewWeaponInfo[i].mana == MANA_NONE)
					{ //Weapon requires no mana, use it
						player->pendingweapon = i;
						if (player->powers[pw_weaponlevel2] && NewWeaponInfo[i+1].bindkey
							== NewWeaponInfo[i].bindkey+20)
							player->pendingweapon = i+1;
						return;
					} 
					else if (NewWeaponInfo[i].mana == MANA_BOTH)
					{ //Special case, check both mana pots
						if(player->mana[MANA_1] >= manaCount
						   && player->mana[MANA_2] >= manaCount)
						{
							player->pendingweapon = i;
							if (player->powers[pw_weaponlevel2] && NewWeaponInfo[i+1].bindkey
								== NewWeaponInfo[i].bindkey+20)
							player->pendingweapon = i+1;
							return;
						}
					}
					else if (player->mana[NewWeaponInfo[i].mana] >= manaCount)					
					{
						player->pendingweapon = i;
						if (player->powers[pw_weaponlevel2] && NewWeaponInfo[i+1].bindkey
							== NewWeaponInfo[i].bindkey+20)
							player->pendingweapon = i+1;
						return;
					}
				}
				weaponnumber--;
			}
		}
		return;
	}
	
	for (i = 0; i < NUMACTUALWEAPONS; i++) //First find the highest weaponnumber
	{
		if (NewWeaponInfo[i].classtype == class && NewWeaponInfo[i].bindkey == key)
		{
			if (j == 0 && i == player->readyweapon)
			{ //It's either the only or only the first, so don't bother more
				j = 1;
				break;
			} else if (j != 1 && i == player->readyweapon)
			{ //It *may* be the last of the line, so set it so
				j = 2;
			} else if (j == 0)
			{ //Well, it wasn't first, so don't try to think so anymore
				j = 4;
			} else if (j == 2 && i != player->readyweapon)
			{ //Whoops, it isn't the last afterall, so bail out
				j = 3;
				break;
			}
		}
		if (NewWeaponInfo[i].bindkey != key && j != 0)
		{ //Has set the j value correctly, don't bother looking for more
			break;
		}
	}

	for (i = 0; i < NUMACTUALWEAPONS; i++)
	{
		if (NewWeaponInfo[i].classtype == class)
		{ //Found a weapon that class has
			if (weaponnumber == 50)
				weaponnumber = 0;
			else weaponnumber++; 
		}
		if (NewWeaponInfo[i].classtype == class && NewWeaponInfo[i].bindkey == key)
		{ //Found weapon
			// -JL- Check the actual mana count this weapon will use
			int manaCount = NewWeaponInfo[i].ammouse*10/player->agility;
			if ((class >= PCLASS_ETTIN || player->weaponowned[weaponnumber])
				&& i != player->readyweapon && j != 1 && j!= 3
				&& (NewWeaponInfo[i].mana == MANA_NONE
					|| (NewWeaponInfo[i].mana == MANA_BOTH 
						&& player->mana[MANA_1] >= manaCount
						&& player->mana[MANA_2] >= manaCount)
					|| (NewWeaponInfo[i].mana != MANA_BOTH 
						&& player->mana[NewWeaponInfo[i].mana] >= manaCount)))
			{ //Player has the weapon, has enough mana and don't have it up, do the change
				player->pendingweapon = i;
				if (player->powers[pw_weaponlevel2] && NewWeaponInfo[i+1].bindkey
						== NewWeaponInfo[i].bindkey+20)
						player->pendingweapon = i+1;
				return;
			} else if (i == player->readyweapon) j = 0; //Choose the next in line
		}
	}
}


//---------------------------------------------------------------------------
//
// PROC P_ActivateMorphWeapon
//Also used for possession weapon change
//
//---------------------------------------------------------------------------

void P_ActivateMorphWeapon(player_t *player)
{
	P_NewPendingWeapon(player,0);
	player->readyweapon = player->pendingweapon;
	player->pendingweapon = WP_NOCHANGE;
	player->psprites[ps_weapon].sy = WEAPONTOP;
	P_SetPsprite(player, ps_weapon, NewWeaponInfo[player->readyweapon].readystate);
}


//---------------------------------------------------------------------------
//
// PROC P_PostMorphWeapon
//Also used for post-possession weapon change
//
//---------------------------------------------------------------------------
void P_PostMorphWeapon(player_t *player, newweapontype_t weapon)
{	
	player->pendingweapon = WP_NOCHANGE;
	player->readyweapon = weapon;
	player->psprites[ps_weapon].sy = WEAPONBOTTOM;
	P_SetPsprite(player, ps_weapon, NewWeaponInfo[weapon].upstate);
}

//---------------------------------------------------------------------------
//
// PROC P_BringUpWeapon
//
// Starts bringing the pending weapon up from the bottom of the screen.
//
//---------------------------------------------------------------------------

void P_BringUpWeapon(player_t *player)
{
	statenum_t new;
	
	if(player->pendingweapon == WP_NOCHANGE)
	{
		player->pendingweapon = player->readyweapon;
	}
	if(player->pendingweapon == WP_CORVUS_GAUNTLETS
		|| player->pendingweapon == WP_CORVUS_PGAUNTLETS)
	{
		S_StartSound(player->plr->mo, sfx_gntact);
	}	
	new = NewWeaponInfo[player->pendingweapon].upstate;
	
	player->pendingweapon = WP_NOCHANGE;
	player->psprites[ps_weapon].sy = WEAPONBOTTOM;
	P_SetPsprite(player, ps_weapon, new);
}

//---------------------------------------------------------------------------
//
// FUNC P_CheckMana
//
// Returns true if there is enough mana to shoot.  If not, selects the
// next weapon to use.
//
//---------------------------------------------------------------------------
boolean P_CheckMana(player_t *player)
{
	manatype_t mana;
	int count;

	mana = NewWeaponInfo[player->readyweapon].mana;
	count = NewWeaponInfo[player->readyweapon].ammouse*10/player->agility;
	if(mana == MANA_BOTH)
	{
		if(player->mana[MANA_1] >= count && player->mana[MANA_2] >= count)
		{
			return true;
		}
	}
	else if(mana == MANA_NONE || player->mana[mana] >= count)
	{
		return(true);
	}
	// out of mana, automatically choose the best weapon
	P_NewPendingWeapon(player, 50);
	
	P_SetPsprite(player, ps_weapon,
		NewWeaponInfo[player->readyweapon].downstate);
	return(false);
}

//---------------------------------------------------------------------------
//
// PROC P_FireWeapon
//
//---------------------------------------------------------------------------

void P_FireWeapon(player_t *player)
{
	statenum_t attackState;

	if(!P_CheckMana(player))
	{
		return;
	}
	//Remi: Yuerch, this should be done another way, but since there's only one poss creature
	//which needs it...
	if (player->class == PCLASS_CENTAURL && player->readyweapon == WP_CENTAURL_MISSILE)
		P_SetMobjState(player->plr->mo, S_POSS_CENTAUR_MISSILE1);
	else
		P_SetMobjState(player->plr->mo, PStateAttack[player->class]); // S_PLAY_ATK1);

	attackState = player->refire ? 
		NewWeaponInfo[player->readyweapon].holdatkstate
		: NewWeaponInfo[player->readyweapon].atkstate;

	if((player->readyweapon == WP_CORVUS_GAUNTLETS 
		|| player->readyweapon == WP_CORVUS_PGAUNTLETS) && !player->refire)
	{ // Play the sound for the initial gauntlet attack
		S_StartSound(player->plr->mo, sfx_gntuse);
	}
	P_SetPsprite(player, ps_weapon, attackState);
	P_NoiseAlert(player->plr->mo, player->plr->mo);
}

//---------------------------------------------------------------------------
//
// PROC P_DropWeapon
//
// The player died, so put the weapon away.
//
//---------------------------------------------------------------------------

void P_DropWeapon(player_t *player)
{
	P_SetPsprite(player, ps_weapon,
		NewWeaponInfo[player->readyweapon].downstate);
}

//---------------------------------------------------------------------------
//
// PROC A_WeaponReady
//
// The player can fire the weapon or change to another weapon at this time.
//
//---------------------------------------------------------------------------

void A_WeaponReady(player_t *player, pspdef_t *psp)
{
	int angle;


	// Remi: possessed centaurs blocking shall remain still
	// I would normally do this a better way, but since it was the most obvious and I couldn't
	// just include it in the frame table or something I just did it like this
	if (player->plr->mo->reactiontime <20 && 
		(player->class == PCLASS_CENTAUR || player->class == PCLASS_CENTAURL) &&
		(player->readyweapon == WP_CENTAUR_SHIELD || player->readyweapon == WP_CENTAURL_SHIELD))
		player->plr->mo->reactiontime = 2000;

	// Change player from attack state
	if(player->plr->mo->state >= &states[PStateAttack[player->class]]
		&& player->plr->mo->state <= &states[PStateAttackEnd[player->class]])
	{
		P_SetMobjState(player->plr->mo, PStateNormal[player->class]);
	}
	// Put the weapon away if the player has a pending weapon or has
	// died.
	if(player->pendingweapon != WP_NOCHANGE || !player->health)
	{
		P_SetPsprite(player, ps_weapon,
			NewWeaponInfo[player->readyweapon].downstate);
		return;
	}

	// Check for fire. 
	if(player->cmd.buttons&BT_ATTACK)
	{
		player->attackdown = true;
		P_FireWeapon(player);
		return;
	}
	else
	{
		player->attackdown = false;
	}

	if(!player->morphTics)
	{
		// Bob the weapon based on movement speed.
		angle = (128*leveltime)&FINEMASK;
		psp->sx = FRACUNIT+FixedMul(player->bob, finecosine[angle]);
		angle &= FINEANGLES/2-1;
		psp->sy = WEAPONTOP+FixedMul(player->bob, finesine[angle]);
	}
}

//---------------------------------------------------------------------------
//
// PROC A_ReFire
//
// The player can re fire the weapon without lowering it entirely.
//
//---------------------------------------------------------------------------

void A_ReFire(player_t *player, pspdef_t *psp)
{
	if((player->cmd.buttons&BT_ATTACK)
		&& player->pendingweapon == WP_NOCHANGE && player->health)
	{
		player->refire++;
		P_FireWeapon(player);
	}
	else
	{
		player->refire = 0;
		P_CheckMana(player);
	}
}

//---------------------------------------------------------------------------
//
// PROC A_Lower
//
//---------------------------------------------------------------------------

void A_Lower(player_t *player, pspdef_t *psp)
{
	if(player->morphTics)
	{
		psp->sy = WEAPONBOTTOM;
	}
	else
	{
		psp->sy += LOWERSPEED;
	}
	if(psp->sy < WEAPONBOTTOM)
	{ // Not lowered all the way yet
		return;
	}
	if(player->playerstate == PST_DEAD)
	{ // Player is dead, so don't bring up a pending weapon
		psp->sy = WEAPONBOTTOM;
		return;
	}
	if(!player->health)
	{ // Player is dead, so keep the weapon off screen
		P_SetPsprite(player,  ps_weapon, S_NULL);
		return;
	}
	player->readyweapon = player->pendingweapon;
	P_BringUpWeapon(player);
}

//---------------------------------------------------------------------------
//
// PROC A_Raise
//
//---------------------------------------------------------------------------

void A_Raise(player_t *player, pspdef_t *psp)
{
	psp->sy -= RAISESPEED;
	if(psp->sy > WEAPONTOP)
	{ // Not raised all the way yet
		return;
	}
	psp->sy = WEAPONTOP;
	P_SetPsprite(player, ps_weapon,
		NewWeaponInfo[player->readyweapon].readystate);	
}

/*
===============
=
= P_BulletSlope
=
= Sets a slope so a near miss is at aproximately the height of the
= intended target
=
===============
*/


void P_BulletSlope (mobj_t *mo)
{
	angle_t		an;

//
// see which target is to be aimed at
//
	an = mo->angle;
	bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
	if (!linetarget)
	{
		an += 1<<26;
		bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
		if (!linetarget)
		{
			an -= 2<<26;
			bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
		}
		if (!linetarget)
		{
			an += 1<<26;
			bulletslope = (mo->angle<<FRACBITS)/173;
			//bulletslope = (mo->player->lookdir<<FRACBITS)/173;
		}
	}
}


//****************************************************************************
//
// WEAPON ATTACKS
//
//****************************************************************************

//============================================================================
//
//	AdjustPlayerAngle
//
//============================================================================

#define MAX_ANGLE_ADJUST (5*ANGLE_1)

void AdjustPlayerAngle(mobj_t *pmo)
{
	angle_t angle;
	int difference;

	angle = R_PointToAngle2(pmo->x, pmo->y, linetarget->x, linetarget->y);
	difference = (int)angle-(int)pmo->angle;
	if(abs(difference) > MAX_ANGLE_ADJUST)
	{
		pmo->angle += difference > 0 ? MAX_ANGLE_ADJUST : -MAX_ANGLE_ADJUST;
	}
	else
	{
		pmo->angle = angle;
	}
}

//============================================================================
//
// A_SnoutAttack
//
//============================================================================

void A_SnoutAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;

	damage = 3+(P_Random()&3);
	angle = player->plr->mo->angle;
	slope = P_AimLineAttack(player->plr->mo, angle, MELEERANGE);
	PuffType = MT_SNOUTPUFF;
	PuffSpawned = NULL;
	P_LineAttack(player->plr->mo, angle, MELEERANGE, slope, damage);
	S_StartSound(player->plr->mo, SFX_PIG_ACTIVE1+(P_Random()&1));
	if(linetarget)
	{
		AdjustPlayerAngle(player->plr->mo);
//		player->plr->mo->angle = R_PointToAngle2(player->plr->mo->x,
//			player->plr->mo->y, linetarget->x, linetarget->y);
		if(PuffSpawned)
		{ // Bit something
			S_StartSound(player->plr->mo, SFX_PIG_ATTACK);
		}
	}
}

//============================================================================
//
// A_FHammerAttack
//
//============================================================================

#define HAMMER_RANGE	(MELEERANGE+MELEERANGE/2)

void A_FHammerAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	mobj_t *pmo=player->plr->mo;
	int damage;
	fixed_t power;
	int slope;
	int i;

	damage = (60+(P_Random()&63))*player->strength/10;
	power = 10*FRACUNIT;
	PuffType = MT_HAMMERPUFF;
	for(i = 0; i < 16; i++)
	{
		angle = pmo->angle+i*(ANG45/32);
		slope = P_AimLineAttack(pmo, angle, HAMMER_RANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, HAMMER_RANGE, slope, damage);
			AdjustPlayerAngle(pmo);
			if (linetarget->flags&MF_COUNTKILL || linetarget->player)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			pmo->special1 = false; // Don't throw a hammer
			goto hammerdone;
		}
		angle = pmo->angle-i*(ANG45/32);
		slope = P_AimLineAttack(pmo, angle, HAMMER_RANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, HAMMER_RANGE, slope, damage);
			AdjustPlayerAngle(pmo);
			if (linetarget->flags&MF_COUNTKILL || linetarget->player)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			pmo->special1 = false; // Don't throw a hammer
			goto hammerdone;
		}
	}
	// didn't find any targets in meleerange, so set to throw out a hammer
	PuffSpawned = NULL;
	angle = pmo->angle;
	slope = P_AimLineAttack(pmo, angle, HAMMER_RANGE);
	P_LineAttack(pmo, angle, HAMMER_RANGE, slope, damage);
	if(PuffSpawned)
	{
		pmo->special1 = false;
	}
	else
	{
		pmo->special1 = true;
	}
hammerdone:
	if(player->mana[MANA_2] < 
		(int)(NewWeaponInfo[player->readyweapon].ammouse*10/player->agility))
	{ // Don't spawn a hammer if the player doesn't have enough mana
		pmo->special1 = false;
	}
	return;		
}

//============================================================================
//
// A_FHammerThrow
//
//============================================================================

void A_FHammerThrow(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;

	if(!player->plr->mo->special1)
	{
		return;
	}
	player->mana[MANA_2] -= (NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
	mo = P_SpawnPlayerMissile(player->plr->mo, MT_HAMMER_MISSILE); 
	if(mo)
	{
		mo->special1 = 0;
	}	
}

//============================================================================
//
// A_FSwordAttack
//
//============================================================================

void A_FSwordAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *pmo;

	player->mana[MANA_1] -= (NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
	player->mana[MANA_2] -= (NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
	pmo = player->plr->mo;
	P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z-10*FRACUNIT, MT_FSWORD_MISSILE, 
		pmo->angle+ANG45/4);
	P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z-5*FRACUNIT, MT_FSWORD_MISSILE, 
		pmo->angle+ANG45/8);
	P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z, MT_FSWORD_MISSILE, pmo->angle);
	P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z+5*FRACUNIT, MT_FSWORD_MISSILE, 
		pmo->angle-ANG45/8);
	P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z+10*FRACUNIT, MT_FSWORD_MISSILE, 
		pmo->angle-ANG45/4);
	S_StartSound(pmo, SFX_FIGHTER_SWORD_FIRE);
}

//============================================================================
//
// A_FSwordAttack2
//
//============================================================================

void A_FSwordAttack2(mobj_t *actor)
{
	angle_t angle = actor->angle;

	P_SpawnMissileAngle(actor, MT_FSWORD_MISSILE,angle+ANG45/4, 0);
	P_SpawnMissileAngle(actor, MT_FSWORD_MISSILE,angle+ANG45/8, 0);
	P_SpawnMissileAngle(actor, MT_FSWORD_MISSILE,angle,         0);
	P_SpawnMissileAngle(actor, MT_FSWORD_MISSILE,angle-ANG45/8, 0);
	P_SpawnMissileAngle(actor, MT_FSWORD_MISSILE,angle-ANG45/4, 0);
	S_StartSound(actor, SFX_FIGHTER_SWORD_FIRE);
}

//============================================================================
//
// A_FSwordFlames
//
//============================================================================

void A_FSwordFlames(mobj_t *actor)
{
	int i;

	for(i = 1+(P_Random()&3); i; i--)
	{
		P_SpawnMobj(actor->x+((P_Random()-128)<<12), actor->y
			+((P_Random()-128)<<12), actor->z+((P_Random()-128)<<11),
			MT_FSWORD_FLAME);
	}
}

//============================================================================
//
// A_MWandAttack
//
//============================================================================

void A_MWandAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;

	switch (player->readyweapon)
	{
	case WP_MAGE_WAND:
		mo = P_SpawnPlayerMissile(player->plr->mo, MT_MWAND_MISSILE);
		break;
	case WP_MAGE_WAND2:
		mo = P_SpawnPlayerMissile(player->plr->mo, MT_MWAND2_MISSILE);
		break;
	case WP_MAGE_WAND3:
		mo = P_SpawnPlayerMissile(player->plr->mo, MT_MWAND3_MISSILE);
		break;
	case WP_MAGE_WAND4:
		mo = P_SpawnPlayerMissile(player->plr->mo, MT_MWAND4_MISSILE);
		break;
	}
	if(mo)
	{
		mo->thinker.function = P_BlasterMobjThinker;
	}
	S_StartSound(player->plr->mo, SFX_MAGE_WAND_FIRE);
}

// ===== Mage Lightning Weapon =====

//============================================================================
//
// A_LightningReady
//
//============================================================================

void A_LightningReady(player_t *player, pspdef_t *psp)
{
	A_WeaponReady(player, psp);
	if(P_Random() < 160)
	{
		S_StartSound(player->plr->mo, SFX_MAGE_LIGHTNING_READY);
	}
}

//============================================================================
//
// A_LightningClip
//
//============================================================================

#define ZAGSPEED	FRACUNIT

void A_LightningClip(mobj_t *actor)
{
	mobj_t *cMo;
	mobj_t *target;
	int zigZag;

	if(actor->type == MT_LIGHTNING_FLOOR)
	{
		actor->z = actor->floorz;
		target = (mobj_t *)((mobj_t *)actor->special2)->special1;
	}
	else if(actor->type == MT_LIGHTNING_CEILING)
	{
		actor->z = actor->ceilingz-actor->height;
		target = (mobj_t *)actor->special1;
	}
	if(actor->type == MT_LIGHTNING_FLOOR)
	{ // floor lightning zig-zags, and forces the ceiling lightning to mimic
		cMo = (mobj_t *)actor->special2;
		zigZag = P_Random();
		if((zigZag > 128 && actor->special1 < 2) || actor->special1 < -2)
		{
			P_ThrustMobj(actor, actor->angle+ANG90, ZAGSPEED);
			if(cMo)
			{
				P_ThrustMobj(cMo, actor->angle+ANG90, ZAGSPEED);
			}
			actor->special1++;
		}
		else
		{
			P_ThrustMobj(actor, actor->angle-ANG90, ZAGSPEED);
			if(cMo)
			{
				P_ThrustMobj(cMo, cMo->angle-ANG90, ZAGSPEED);
			}
			actor->special1--;
		}
	}
	if(target)
	{
		if(target->health <= 0)
		{
			P_ExplodeMissile(actor);
		}
		else
		{
			actor->angle = R_PointToAngle2(actor->x, actor->y, target->x,
				target->y);
			actor->momx = 0;
			actor->momy = 0;
			P_ThrustMobj(actor, actor->angle, actor->info->speed>>1);
		}
	}
}

//============================================================================
//
// A_LightningZap
//
//============================================================================

void A_LightningZap(mobj_t *actor)
{
	mobj_t *mo;
	fixed_t deltaZ;

	A_LightningClip(actor);

	actor->health -= 8;
	if(actor->health <= 0)
	{
		P_SetMobjState(actor, actor->info->deathstate);
		return;
	}
	if(actor->type == MT_LIGHTNING_FLOOR)
	{
		deltaZ = 10*FRACUNIT;
	}
	else
	{
		deltaZ = -10*FRACUNIT;
	}
	mo = P_SpawnMobj(actor->x+((P_Random()-128)*actor->radius/256), 
		actor->y+((P_Random()-128)*actor->radius/256), 
		actor->z+deltaZ, MT_LIGHTNING_ZAP);
	if(mo)
	{
		mo->special2 = (int)actor;
		mo->momx = actor->momx;
		mo->momy = actor->momy;
		mo->target = actor->target;
		if(actor->type == MT_LIGHTNING_FLOOR)
		{
			mo->momz = 20*FRACUNIT;
		}
		else 
		{
			mo->momz = -20*FRACUNIT;
		}
	}
/*
	mo = P_SpawnMobj(actor->x+((P_Random()-128)*actor->radius/256), 
		actor->y+((P_Random()-128)*actor->radius/256), 
		actor->z+deltaZ, MT_LIGHTNING_ZAP);
	if(mo)
	{
		mo->special2 = (int)actor;
		mo->momx = actor->momx;
		mo->momy = actor->momy;
		mo->target = actor->target;
		if(actor->type == MT_LIGHTNING_FLOOR)
		{
			mo->momz = 16*FRACUNIT;
		}
		else 
		{
			mo->momz = -16*FRACUNIT;
		}
	}
*/
	if(actor->type == MT_LIGHTNING_FLOOR && P_Random() < 160)
	{
		S_StartSound(actor, SFX_MAGE_LIGHTNING_CONTINUOUS);
	}
}

//============================================================================
//
// A_MLightningAttack2
//
//============================================================================

void A_MLightningAttack2(mobj_t *actor)
{
	mobj_t *fmo, *cmo;

	fmo = P_SpawnPlayerMissile(actor, MT_LIGHTNING_FLOOR);
	cmo = P_SpawnPlayerMissile(actor, MT_LIGHTNING_CEILING);
	if(fmo)
	{
		fmo->special1 = 0;
		fmo->special2 = (int)cmo;
		A_LightningZap(fmo);	
	}
	if(cmo)
	{
		cmo->special1 = 0;	// mobj that it will track
		cmo->special2 = (int)fmo;
		A_LightningZap(cmo);	
	}
	S_StartSound(actor, SFX_MAGE_LIGHTNING_FIRE);
}

//============================================================================
//
// A_MLightningAttack
//
//============================================================================

void A_MLightningAttack(player_t *player, pspdef_t *psp)
{
	A_MLightningAttack2(player->plr->mo);
	player->mana[MANA_2] -= (NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
}

//============================================================================
//
// A_ZapMimic
//
//============================================================================

void A_ZapMimic(mobj_t *actor)
{
	mobj_t *mo;

	mo = (mobj_t *)actor->special2;
	if(mo)
	{
		if(mo->state >= &states[mo->info->deathstate]
			|| mo->state == &states[S_FREETARGMOBJ])
		{
			P_ExplodeMissile(actor);
		}
		else
		{
			actor->momx = mo->momx;
			actor->momy = mo->momy;
		}
	}
}

//============================================================================
//
// A_LastZap
//
//============================================================================

void A_LastZap(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_LIGHTNING_ZAP);
	if(mo)
	{
		P_SetMobjState(mo, S_LIGHTNING_ZAP_X1);
		mo->momz = 40*FRACUNIT;
	}
}

//============================================================================
//
// A_LightningRemove
//
//============================================================================

void A_LightningRemove(mobj_t *actor)
{
	mobj_t *mo;

	mo = (mobj_t *)actor->special2;
	if(mo)
	{
		mo->special2 = 0;
		P_ExplodeMissile(mo);
	}
}


//============================================================================
//
// MStaffSpawn
//
//============================================================================
void MStaffSpawn(mobj_t *pmo, angle_t angle)
{
	mobj_t *mo;

	mo = P_SPMAngle(pmo, MT_MSTAFF_FX2, angle);
	if (mo)
	{
		mo->target = pmo;
		mo->special1 = (int)P_RoughMonsterSearch(mo, 10);
	}
}

//============================================================================
//
// A_MStaffAttack
//
//============================================================================

void A_MStaffAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	mobj_t *pmo;

	player->mana[MANA_1] -= (NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
	player->mana[MANA_2] -= (NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
	pmo = player->plr->mo;
	angle = pmo->angle;
	
	MStaffSpawn(pmo, angle);
	MStaffSpawn(pmo, angle-ANGLE_1*5);
	MStaffSpawn(pmo, angle+ANGLE_1*5);
	S_StartSound(player->plr->mo, SFX_MAGE_STAFF_FIRE);
	if(player == &players[consoleplayer])
	{
		player->damagecount = 0;
		player->bonuscount = 0;
		/*I_SetPalette((byte *)W_CacheLumpNum(W_GetNumForName("playpal"),
			PU_CACHE)+STARTSCOURGEPAL*768);*/
		
		H2_SetFilter(STARTSCOURGEPAL);
	}
}

//============================================================================
//
// A_MStaffPalette
//
//============================================================================

void A_MStaffPalette(player_t *player, pspdef_t *psp)
{
	int pal;

	if(player == &players[consoleplayer])
	{
		pal = STARTSCOURGEPAL+psp->state-(&states[S_MSTAFFATK_2]);
		if(pal == STARTSCOURGEPAL+3)
		{ // reset back to original playpal
			pal = 0;
		}
		/*I_SetPalette((byte *)W_CacheLumpNum(W_GetNumForName("playpal"),
			PU_CACHE)+pal*768);*/
		H2_SetFilter(pal);
	}
}

//============================================================================
//
// A_MStaffWeave
//
//============================================================================

void A_MStaffWeave(mobj_t *actor)
{
	fixed_t newX, newY;
	int weaveXY, weaveZ;
	int angle;

	weaveXY = actor->special2>>16;
	weaveZ = actor->special2&0xFFFF;
	angle = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	newX = actor->x-FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	newY = actor->y-FixedMul(finesine[angle],
		FloatBobOffsets[weaveXY]<<2);
	weaveXY = (weaveXY+6)&63;
	newX += FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	newY += FixedMul(finesine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	P_TryMove(actor, newX, newY);
	actor->z -= FloatBobOffsets[weaveZ]<<1;
	weaveZ = (weaveZ+3)&63;
	actor->z += FloatBobOffsets[weaveZ]<<1;
	if(actor->z <= actor->floorz)
	{
		actor->z = actor->floorz+FRACUNIT;
	}
	actor->special2 = weaveZ+(weaveXY<<16);
}


//============================================================================
//
// A_MStaffTrack
//
//============================================================================

void A_MStaffTrack(mobj_t *actor)
{
	if ((actor->special1 == 0) && (P_Random()<50))
	{
		actor->special1 = (int)P_RoughMonsterSearch(actor, 10);
	}
	P_SeekerMissile(actor, ANGLE_1*2, ANGLE_1*10);
}


//============================================================================
//
// MStaffSpawn2 - for use by mage class boss
//
//============================================================================

void MStaffSpawn2(mobj_t *actor, angle_t angle)
{
	mobj_t *mo;

	mo = P_SpawnMissileAngle(actor, MT_MSTAFF_FX2, angle, 0);
	if (mo)
	{
		mo->target = actor;
		mo->special1 = (int)P_RoughMonsterSearch(mo, 10);
	}
}

//============================================================================
//
// A_MStaffAttack2 - for use by mage class boss
//
//============================================================================

void A_MStaffAttack2(mobj_t *actor)
{
	angle_t angle;
	angle = actor->angle;
	MStaffSpawn2(actor, angle);
	MStaffSpawn2(actor, angle-ANGLE_1*5);
	MStaffSpawn2(actor, angle+ANGLE_1*5);
	S_StartSound(actor, SFX_MAGE_STAFF_FIRE);
}

//============================================================================
//
// A_FPunchAttack
//
//============================================================================

void A_FPunchAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;
	mobj_t *pmo = player->plr->mo;
	fixed_t power;
	int i;

	damage = (40+(P_Random()&15))*player->strength/10;
	power = 2*FRACUNIT;
	PuffType = MT_PUNCHPUFF;
	for(i = 0; i < 16; i++)
	{
		angle = pmo->angle+i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, 2*MELEERANGE);
		if(linetarget)
		{
			player->plr->mo->special1++;
			if(pmo->special1 == 3)
			{
				damage <<= 1;
				power = 6*FRACUNIT;
				PuffType = MT_HAMMERPUFF;
			}
			P_LineAttack(pmo, angle, 2*MELEERANGE, slope, damage);
			if (linetarget->flags&MF_COUNTKILL || linetarget->player)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			AdjustPlayerAngle(pmo);
			goto punchdone;
		}
		angle = pmo->angle-i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, 2*MELEERANGE);
		if(linetarget)
		{
			pmo->special1++;
			if(pmo->special1 == 3)
			{
				damage <<= 1;
				power = 6*FRACUNIT;
				PuffType = MT_HAMMERPUFF;
			}
			P_LineAttack(pmo, angle, 2*MELEERANGE, slope, damage);
			if (linetarget->flags&MF_COUNTKILL || linetarget->player)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			AdjustPlayerAngle(pmo);
			goto punchdone;
		}
	}
	// didn't find any creatures, so try to strike any walls
	pmo->special1 = 0;

	angle = pmo->angle;
	slope = P_AimLineAttack(pmo, angle, MELEERANGE);
	P_LineAttack(pmo, angle, MELEERANGE, slope, damage);

punchdone:
	if(pmo->special1 == 3)
	{
		pmo->special1 = 0;
		P_SetPsprite(player, ps_weapon, S_PUNCHATK2_1);
		S_StartSound(pmo, SFX_FIGHTER_GRUNT);
	}
	return;		
}

//============================================================================
//
// A_FAxeAttack
//
//============================================================================

#define AXERANGE	2.25*MELEERANGE

void A_FAxeAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	mobj_t *pmo=player->plr->mo;
	fixed_t power;
	int damage;
	int slope;
	int i;
	int useMana;

	damage = (40+(P_Random()&15)+(P_Random()&7))*player->strength/10;
	power = 0;
	if(player->readyweapon == WP_FIGHTER_AXE_G)
	{
		damage <<= 1;
		power = 6*FRACUNIT;
		PuffType = MT_AXEPUFF_GLOW;
		useMana = 1;
	}
	else
	{
		PuffType = MT_AXEPUFF;
		useMana = 0;
	}
	for(i = 0; i < 16; i++)
	{
		angle = pmo->angle+i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, AXERANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, AXERANGE, slope, damage);
			if (linetarget->flags&MF_COUNTKILL || linetarget->player)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			AdjustPlayerAngle(pmo);
			useMana++; 
			goto axedone;
		}
		angle = pmo->angle-i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, AXERANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, AXERANGE, slope, damage);
			if (linetarget->flags&MF_COUNTKILL)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			AdjustPlayerAngle(pmo);
			useMana++; 
			goto axedone;
		}
	}
	// didn't find any creatures, so try to strike any walls
	pmo->special1 = 0;

	angle = pmo->angle;
	slope = P_AimLineAttack(pmo, angle, MELEERANGE);
	P_LineAttack(pmo, angle, MELEERANGE, slope, damage);

axedone:
	if(useMana == 2)
	{
		player->mana[MANA_1] -= 
			(NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);		
	}
	return;		
}

//===========================================================================
//
// A_CMaceAttack
//
//===========================================================================

void A_CMaceAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;
	int i;
	int newLife;
	
	switch(player->class) //This void is used by some possessed monsters, so damage needs change
	{
	case PCLASS_ETTIN:
		damage = HITDICE(2);
		PuffType = MT_HAMMERPUFF;
		break;
	case PCLASS_CENTAUR:
	case PCLASS_CENTAURL:
		damage = P_Random()%7+3;
		PuffType = MT_HAMMERPUFF;
		break;
	case PCLASS_SERPENT:
	case PCLASS_BSERPENT:
		damage = HITDICE(4);
		PuffType = MT_SERPENTPUFF;
		break;
	case PCLASS_REIVER:
		damage = HITDICE(2);
		PuffType = MT_REIVERPUFF;
		break;
	case PCLASS_BISHOP:
		damage = HITDICE(4);
		PuffType = MT_BISHOPPUFF;
		break;
	default:
		damage = (25+(P_Random()&15))*player->strength/10;
		PuffType = MT_HAMMERPUFF;
	}

	for(i = 0; i < 16; i++)
	{
		angle = player->plr->mo->angle+i*(ANG45/16);
		slope = P_AimLineAttack(player->plr->mo, angle, 2*MELEERANGE);
		if(linetarget)
		{
			P_LineAttack(player->plr->mo, angle, 2*MELEERANGE, slope, 
				damage);
			AdjustPlayerAngle(player->plr->mo);
//			player->plr->mo->angle = R_PointToAngle2(player->plr->mo->x,
//				player->plr->mo->y, linetarget->x, linetarget->y);
			if((player->class == PCLASS_REIVER) &&
				(linetarget->player || linetarget->flags&MF_COUNTKILL)
				&& (!(linetarget->flags2&(MF2_DORMANT+MF2_INVULNERABLE))))
			{
				newLife = player->health+(damage);
				newLife = newLife > mobjinfo[player->plr->mo->type].spawnhealth
					? mobjinfo[player->plr->mo->type].spawnhealth : newLife;
				player->plr->mo->health = player->health = newLife;
			}
			goto macedone;
		}
		angle = player->plr->mo->angle-i*(ANG45/16);
		slope = P_AimLineAttack(player->plr->mo, angle, 2*MELEERANGE);
		if(linetarget)
		{
			P_LineAttack(player->plr->mo, angle, 2*MELEERANGE, slope, 
				damage);
			AdjustPlayerAngle(player->plr->mo);
//			player->plr->mo->angle = R_PointToAngle2(player->plr->mo->x,
//				player->plr->mo->y, linetarget->x, linetarget->y);
			if((player->class == PCLASS_REIVER) &&
				(linetarget->player || linetarget->flags&MF_COUNTKILL)
				&& (!(linetarget->flags2&(MF2_DORMANT+MF2_INVULNERABLE))))
			{
				newLife = player->health+(damage);
				newLife = newLife > mobjinfo[player->plr->mo->type].spawnhealth
					? mobjinfo[player->plr->mo->type].spawnhealth : newLife;
				player->plr->mo->health = player->health = newLife;
			}
			goto macedone;
		}
	}
	// didn't find any creatures, so try to strike any walls
	player->plr->mo->special1 = 0;

	angle = player->plr->mo->angle;
	slope = P_AimLineAttack(player->plr->mo, angle, MELEERANGE);
	P_LineAttack(player->plr->mo, angle, MELEERANGE, slope, 
		damage);
macedone:
	return;		
}

//============================================================================
//
// A_CStaffCheck
//
//============================================================================

void A_CStaffCheck(player_t *player, pspdef_t *psp)
{
	mobj_t *pmo;
	int damage;
	int newLife;
	angle_t angle;
	int slope;
	int i;

	pmo = player->plr->mo;
	damage = (20+(P_Random()&15))*player->strength/10;
	PuffType = MT_CSTAFFPUFF;
	for(i = 0; i < 3; i++)
	{
		angle = pmo->angle+i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, 1.5*MELEERANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, 1.5*MELEERANGE, slope, damage);
			pmo->angle = R_PointToAngle2(pmo->x, pmo->y, 
				linetarget->x, linetarget->y);
			if((linetarget->player || linetarget->flags&MF_COUNTKILL)
				&& (!(linetarget->flags2&(MF2_DORMANT+MF2_INVULNERABLE))))
			{
				newLife = player->health+(damage>>3);
				newLife = newLife > player->maxhealth ? player->maxhealth : newLife;
				pmo->health = player->health = newLife;
				P_SetPsprite(player, ps_weapon, S_CSTAFFATK2_1);
			}
			player->mana[MANA_1] -= 
				(NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
			break;
		}
		angle = pmo->angle-i*(ANG45/16);
		slope = P_AimLineAttack(player->plr->mo, angle, 1.5*MELEERANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, 1.5*MELEERANGE, slope, damage);
			pmo->angle = R_PointToAngle2(pmo->x, pmo->y, 
				linetarget->x, linetarget->y);
			if(linetarget->player || linetarget->flags&MF_COUNTKILL)
			{
				newLife = player->health+(damage>>4);
				newLife = newLife > player->maxhealth ? player->maxhealth : newLife;
				pmo->health = player->health = newLife;
				P_SetPsprite(player, ps_weapon, S_CSTAFFATK2_1);
			}
			player->mana[MANA_1] -= 
				(NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
			break;
		}
	}
}

//============================================================================
//
// A_CStaffAttack
//
//============================================================================

void A_CStaffAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;
	mobj_t *pmo;

	player->mana[MANA_1] -=	(NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
	pmo = player->plr->mo;
	mo = P_SPMAngle(pmo, MT_CSTAFF_MISSILE, pmo->angle-(ANG45/15));
	if(mo)
	{
		mo->special2 = 32;
	}
	mo = P_SPMAngle(pmo, MT_CSTAFF_MISSILE, pmo->angle+(ANG45/15));
	if(mo)
	{
		mo->special2 = 0;
	}
	S_StartSound(player->plr->mo, SFX_CLERIC_CSTAFF_FIRE);
}

//============================================================================
//
// A_CStaffMissileSlither
//
//============================================================================

void A_CStaffMissileSlither(mobj_t *actor)
{
	fixed_t newX, newY;
	int weaveXY;
	int angle;

	weaveXY = actor->special2;
	angle = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	newX = actor->x-FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]);
	newY = actor->y-FixedMul(finesine[angle],
		FloatBobOffsets[weaveXY]);
	weaveXY = (weaveXY+3)&63;
	newX += FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]);
	newY += FixedMul(finesine[angle], 
		FloatBobOffsets[weaveXY]);
	P_TryMove(actor, newX, newY);
	actor->special2 = weaveXY;	
}

//============================================================================
//
// A_CStaffInitBlink
//
//============================================================================

void A_CStaffInitBlink(player_t *player, pspdef_t *psp)
{
	player->plr->mo->special1 = (P_Random()>>1)+20;
}

//============================================================================
//
// A_CStaffCheckBlink
//
//============================================================================

void A_CStaffCheckBlink(player_t *player, pspdef_t *psp)
{
	if(!--player->plr->mo->special1)
	{
		P_SetPsprite(player, ps_weapon, S_CSTAFFBLINK1);
		player->plr->mo->special1 = (P_Random()+50)>>2;
	}
}

//============================================================================
//
// A_CFlameAttack
//
//============================================================================

#define FLAMESPEED	(0.45*FRACUNIT)
#define CFLAMERANGE	(12*64*FRACUNIT)

void A_CFlameAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;

	mo = P_SpawnPlayerMissile(player->plr->mo, MT_CFLAME_MISSILE);
	if(mo)
	{
		mo->thinker.function = P_BlasterMobjThinker;
		mo->special1 = 2;
	}

	player->mana[MANA_2] -= (NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
	S_StartSound(player->plr->mo, SFX_CLERIC_FLAME_FIRE);
}

//============================================================================
//
// A_CFlamePuff
//
//============================================================================

void A_CFlamePuff(mobj_t *actor)
{
	A_UnHideThing(actor);
	actor->momx = 0;
	actor->momy = 0;
	actor->momz = 0;
	S_StartSound(actor, SFX_CLERIC_FLAME_EXPLODE);
}

//============================================================================
//
// A_CFlameMissile
//
//============================================================================

void A_CFlameMissile(mobj_t *actor)
{
	int i;
	int an, an90;
	fixed_t dist;
	mobj_t *mo;
	
	A_UnHideThing(actor);
	S_StartSound(actor, SFX_CLERIC_FLAME_EXPLODE);
	if(BlockingMobj && BlockingMobj->flags&MF_SHOOTABLE)
	{ // Hit something, so spawn the flame circle around the thing
		dist = BlockingMobj->radius+18*FRACUNIT;
		for(i = 0; i < 4; i++)
		{
			an = (i*ANG45)>>ANGLETOFINESHIFT;
			an90 = (i*ANG45+ANG90)>>ANGLETOFINESHIFT;
			mo = P_SpawnMobj(BlockingMobj->x+FixedMul(dist, finecosine[an]),
				BlockingMobj->y+FixedMul(dist, finesine[an]), 
				BlockingMobj->z+5*FRACUNIT, MT_CIRCLEFLAME);
			if(mo)
			{
				mo->angle = an<<ANGLETOFINESHIFT;
				mo->target = actor->target;
				mo->momx = mo->special1 = FixedMul(FLAMESPEED, finecosine[an]);
				mo->momy = mo->special2 = FixedMul(FLAMESPEED, finesine[an]);
				mo->tics -= P_Random()&3;
			}
			mo = P_SpawnMobj(BlockingMobj->x-FixedMul(dist, finecosine[an]),
				BlockingMobj->y-FixedMul(dist, finesine[an]), 
				BlockingMobj->z+5*FRACUNIT, MT_CIRCLEFLAME);
			if(mo)
			{
				mo->angle = ANG180+(an<<ANGLETOFINESHIFT);
				mo->target = actor->target;
				mo->momx = mo->special1 = FixedMul(-FLAMESPEED, 
					finecosine[an]);
				mo->momy = mo->special2 = FixedMul(-FLAMESPEED, finesine[an]);
				mo->tics -= P_Random()&3;
			}
		}
		P_SetMobjState(actor, S_FLAMEPUFF2_1);
	}
}

/*
void A_CFlameAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *pmo;
	angle_t angle;
	int damage;
	int i;
	int an, an90;
	fixed_t dist;
	mobj_t *mo;

	pmo = player->plr->mo;
	P_BulletSlope(pmo);
	damage = 25+HITDICE(3);
	angle = pmo->angle;
	if(player->refire)
	{
		angle += (P_Random()-P_Random())<<17;
	}
	P_AimLineAttack(pmo, angle, CFLAMERANGE); // Correctly set linetarget
	if(!linetarget)
	{
		angle += ANGLE_1*2;
		P_AimLineAttack(pmo, angle, CFLAMERANGE);
		if(!linetarget)
		{
			angle -= ANGLE_1*4;
			P_AimLineAttack(pmo, angle, CFLAMERANGE);
			if(!linetarget)
			{
				angle += ANGLE_1*2;
			}
		}		
	}
	if(linetarget)
	{
		PuffType = MT_FLAMEPUFF2;
	}
	else
	{
		PuffType = MT_FLAMEPUFF;
	}
	P_LineAttack(pmo, angle, CFLAMERANGE, bulletslope, damage);
	if(linetarget)
	{ // Hit something, so spawn the flame circle around the thing
		dist = linetarget->radius+18*FRACUNIT;
		for(i = 0; i < 4; i++)
		{
			an = (i*ANG45)>>ANGLETOFINESHIFT;
			an90 = (i*ANG45+ANG90)>>ANGLETOFINESHIFT;
			mo = P_SpawnMobj(linetarget->x+FixedMul(dist, finecosine[an]),
				linetarget->y+FixedMul(dist, finesine[an]), 
				linetarget->z+5*FRACUNIT, MT_CIRCLEFLAME);
			if(mo)
			{
				mo->angle = an<<ANGLETOFINESHIFT;
				mo->target = pmo;
				mo->momx = mo->special1 = FixedMul(FLAMESPEED, finecosine[an]);
				mo->momy = mo->special2 = FixedMul(FLAMESPEED, finesine[an]);
				mo->tics -= P_Random()&3;
			}
			mo = P_SpawnMobj(linetarget->x-FixedMul(dist, finecosine[an]),
				linetarget->y-FixedMul(dist, finesine[an]), 
				linetarget->z+5*FRACUNIT, MT_CIRCLEFLAME);
			if(mo)
			{
				mo->angle = ANG180+(an<<ANGLETOFINESHIFT);
				mo->target = pmo;
				mo->momx = mo->special1 = FixedMul(-FLAMESPEED, 
					finecosine[an]);
				mo->momy = mo->special2 = FixedMul(-FLAMESPEED, finesine[an]);
				mo->tics -= P_Random()&3;
			}
		}
	}
// Create a line of flames from the player to the flame puff
	CFlameCreateFlames(player->plr->mo);

	player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
	S_StartSound(player->plr->mo, SFX_CLERIC_FLAME_FIRE);
}
*/

//============================================================================
//
// A_CFlameRotate
//
//============================================================================

#define FLAMEROTSPEED	2*FRACUNIT

void A_CFlameRotate(mobj_t *actor)
{
	int an;

	an = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	actor->momx = actor->special1+FixedMul(FLAMEROTSPEED, finecosine[an]);
	actor->momy = actor->special2+FixedMul(FLAMEROTSPEED, finesine[an]);
	actor->angle += ANG90/15;
}


//============================================================================
//
// A_CHolyAttack3
//
// 	Spawns the spirits
//============================================================================

void A_CHolyAttack3(mobj_t *actor)
{
	P_SpawnMissile(actor, actor->target, MT_HOLY_MISSILE);
	S_StartSound(actor, SFX_CHOLY_FIRE);
}


//============================================================================
//
// A_CHolyAttack2 
//
// 	Spawns the spirits
//============================================================================

void A_CHolyAttack2(mobj_t *actor)
{
	int j;
	int i;
	mobj_t *mo;
	mobj_t *tail, *next;

	for(j = 0; j < 4; j++)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_HOLY_FX);
		if(!mo)
		{
			continue;
		}
		switch(j)
		{ // float bob index
			case 0:
				mo->special2 = P_Random()&7; // upper-left
				break;
			case 1:
				mo->special2 = 32+(P_Random()&7); // upper-right
				break;
			case 2:
				mo->special2 = (32+(P_Random()&7))<<16; // lower-left
				break;
			case 3:
				mo->special2 = ((32+(P_Random()&7))<<16)+32+(P_Random()&7);
				break;
		}
		mo->z = actor->z;
		mo->angle = actor->angle+(ANGLE_45+ANGLE_45/2)-ANGLE_45*j;
		P_ThrustMobj(mo, mo->angle, mo->info->speed);
		mo->target = actor->target;
		mo->args[0] = 10; // initial turn value
		mo->args[1] = 0; // initial look angle
		if(deathmatch)
		{ // Ghosts last slightly less longer in DeathMatch
			mo->health = 85;
		}
		if(linetarget)
		{
			mo->special1 = (int)linetarget;
			mo->flags |= MF_NOCLIP|MF_SKULLFLY;
			mo->flags &= ~MF_MISSILE;
		}
		tail = P_SpawnMobj(mo->x, mo->y, mo->z, MT_HOLY_TAIL);
		tail->special2 = (int)mo; // parent
		for(i = 1; i < 3; i++)
		{
			next = P_SpawnMobj(mo->x, mo->y, mo->z, MT_HOLY_TAIL);
			P_SetMobjState(next, next->info->spawnstate+1);
			tail->special1 = (int)next;
			tail = next;
		}
		tail->special1 = 0; // last tail bit
	}
}

//============================================================================
//
// A_CHolyAttack
//
//============================================================================

void A_CHolyAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;

	player->mana[MANA_1] -= (NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
	player->mana[MANA_2] -= (NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
	mo = P_SpawnPlayerMissile(player->plr->mo, MT_HOLY_MISSILE);
	if(player == &players[consoleplayer])
	{
		player->damagecount = 0;
		player->bonuscount = 0;
/*		I_SetPalette((byte *)W_CacheLumpNum(W_GetNumForName("playpal"),
			PU_CACHE)+STARTHOLYPAL*768);*/
		H2_SetFilter(STARTHOLYPAL);
	}
	S_StartSound(player->plr->mo, SFX_CHOLY_FIRE);
}

//============================================================================
//
// A_CHolyPalette
//
//============================================================================

void A_CHolyPalette(player_t *player, pspdef_t *psp)
{
	int pal;

	if(player == &players[consoleplayer])
	{
		pal = STARTHOLYPAL+psp->state-(&states[S_CHOLYATK_6]);
		if(pal == STARTHOLYPAL+3)
		{ // reset back to original playpal
			pal = 0;
		}
/*		I_SetPalette((byte *)W_CacheLumpNum(W_GetNumForName("playpal"),
			PU_CACHE)+pal*768);*/
		H2_SetFilter(pal);
	}
}

//============================================================================
//
// CHolyFindTarget
//
//============================================================================

static void CHolyFindTarget(mobj_t *actor)
{
	mobj_t *target;

	if(target = P_RoughMonsterSearch(actor, 6))
	{
		actor->special1 = (int)target;
		actor->flags |= MF_NOCLIP|MF_SKULLFLY;
		actor->flags &= ~MF_MISSILE;
	}
}

//============================================================================
//
// CHolySeekerMissile
//
// 	 Similar to P_SeekerMissile, but seeks to a random Z on the target
//============================================================================

static void CHolySeekerMissile(mobj_t *actor, angle_t thresh, angle_t turnMax)
{
	int dir;
	int dist;
	angle_t delta;
	angle_t angle;
	mobj_t *target;
	fixed_t newZ;
	fixed_t deltaZ;

	target = (mobj_t *)actor->special1;
	if(target == NULL)
	{
		return;
	}
	if(!(target->flags&MF_SHOOTABLE) 
	|| (!(target->flags&MF_COUNTKILL) && !target->player))
	{ // Target died/target isn't a player or creature
		actor->special1 = 0;
		actor->flags &= ~(MF_NOCLIP|MF_SKULLFLY);
		actor->flags |= MF_MISSILE;
		CHolyFindTarget(actor);
		return;
	}
	dir = P_FaceMobj(actor, target, &delta);
	if(delta > thresh)
	{
		delta >>= 1;
		if(delta > turnMax)
		{
			delta = turnMax;
		}
	}
	if(dir)
	{ // Turn clockwise
		actor->angle += delta;
	}
	else
	{ // Turn counter clockwise
		actor->angle -= delta;
	}
	angle = actor->angle>>ANGLETOFINESHIFT;
	actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
	actor->momy = FixedMul(actor->info->speed, finesine[angle]);
	if(!(leveltime&15) 
		|| actor->z > target->z+(target->height)
		|| actor->z+actor->height < target->z)
	{
		newZ = target->z+((P_Random()*target->height)>>8);
		deltaZ = newZ-actor->z;
		if(abs(deltaZ) > 15*FRACUNIT)
		{
			if(deltaZ > 0)
			{
				deltaZ = 15*FRACUNIT;
			}
			else
			{
				deltaZ = -15*FRACUNIT;
			}
		}
		dist = P_AproxDistance(target->x-actor->x, target->y-actor->y);
		dist = dist/actor->info->speed;
		if(dist < 1)
		{
			dist = 1;
		}
		actor->momz = deltaZ/dist;
	}
	return;
}

//============================================================================
//
// A_CHolyWeave
//
//============================================================================

static void CHolyWeave(mobj_t *actor)
{
	fixed_t newX, newY;
	int weaveXY, weaveZ;
	int angle;

	weaveXY = actor->special2>>16;
	weaveZ = actor->special2&0xFFFF;
	angle = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	newX = actor->x-FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	newY = actor->y-FixedMul(finesine[angle],
		FloatBobOffsets[weaveXY]<<2);
	weaveXY = (weaveXY+(P_Random()%5))&63;
	newX += FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	newY += FixedMul(finesine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	P_TryMove(actor, newX, newY);
	actor->z -= FloatBobOffsets[weaveZ]<<1;
	weaveZ = (weaveZ+(P_Random()%5))&63;
	actor->z += FloatBobOffsets[weaveZ]<<1;	
	actor->special2 = weaveZ+(weaveXY<<16);
}

//============================================================================
//
// A_CHolySeek
//
//============================================================================

void A_CHolySeek(mobj_t *actor)
{
	actor->health--;
	if(actor->health <= 0)
	{
		actor->momx >>= 2;
		actor->momy >>= 2;
		actor->momz = 0;
		P_SetMobjState(actor, actor->info->deathstate);
		actor->tics -= P_Random()&3;
		return;
	}
	if(actor->special1)
	{
		CHolySeekerMissile(actor, actor->args[0]*ANGLE_1,
			actor->args[0]*ANGLE_1*2);
		if(!((leveltime+7)&15))
		{
			actor->args[0] = 5+(P_Random()/20);
		}
	}
	CHolyWeave(actor);
}

//============================================================================
//
// CHolyTailFollow
//
//============================================================================

static void CHolyTailFollow(mobj_t *actor, fixed_t dist)
{
	mobj_t *child;
	int an;
	fixed_t oldDistance, newDistance;

	child = (mobj_t *)actor->special1;
	if(child)
	{
		an = R_PointToAngle2(actor->x, actor->y, child->x, 
			child->y)>>ANGLETOFINESHIFT;
		oldDistance = P_AproxDistance(child->x-actor->x, child->y-actor->y);
		if(P_TryMove(child, actor->x+FixedMul(dist, finecosine[an]), 
			actor->y+FixedMul(dist, finesine[an])))
		{
			newDistance = P_AproxDistance(child->x-actor->x, 
				child->y-actor->y)-FRACUNIT;
			if(oldDistance < FRACUNIT)
			{
				if(child->z < actor->z)
				{
					child->z = actor->z-dist;
				}
				else
				{
					child->z = actor->z+dist;
				}
			}
			else
			{
				child->z = actor->z+FixedMul(FixedDiv(newDistance, 
					oldDistance), child->z-actor->z);
			}
		}
		CHolyTailFollow(child, dist-FRACUNIT);
	}
}

//============================================================================
//
// CHolyTailRemove
//
//============================================================================

static void CHolyTailRemove(mobj_t *actor)
{
	mobj_t *child;

	child = (mobj_t *)actor->special1;
	if(child)
	{
		CHolyTailRemove(child);
	}
	P_RemoveMobj(actor);
}

//============================================================================
//
// A_CHolyTail
//
//============================================================================

void A_CHolyTail(mobj_t *actor)
{
	mobj_t *parent;

	parent = (mobj_t *)actor->special2;

	if(parent)
	{
		if(parent->state >= &states[parent->info->deathstate])
		{ // Ghost removed, so remove all tail parts
			CHolyTailRemove(actor);
			return;
		}
		else if(P_TryMove(actor, parent->x-FixedMul(14*FRACUNIT,
			finecosine[parent->angle>>ANGLETOFINESHIFT]),
			parent->y-FixedMul(14*FRACUNIT, 
			finesine[parent->angle>>ANGLETOFINESHIFT])))
		{
			actor->z = parent->z-5*FRACUNIT;
		}
		CHolyTailFollow(actor, 10*FRACUNIT);
	}
}
//============================================================================
//
// A_CHolyCheckScream
//
//============================================================================

void A_CHolyCheckScream(mobj_t *actor)
{
	A_CHolySeek(actor);
	if(P_Random() < 20)
	{
		S_StartSound(actor, SFX_SPIRIT_ACTIVE);
	}
	if(!actor->special1)
	{
		CHolyFindTarget(actor);
	}
}

//============================================================================
//
// A_CHolySpawnPuff
//
//============================================================================

void A_CHolySpawnPuff(mobj_t *actor)
{
	P_SpawnMobj(actor->x, actor->y, actor->z, MT_HOLY_MISSILE_PUFF);
}

//----------------------------------------------------------------------------
//
// PROC A_FireConePL1
//
//----------------------------------------------------------------------------

#define SHARDSPAWN_LEFT		1
#define SHARDSPAWN_RIGHT	2
#define SHARDSPAWN_UP		4
#define SHARDSPAWN_DOWN		8

void A_FireConePL1(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;
	int i;
	mobj_t *pmo,*mo;
	int conedone=false;

	pmo = player->plr->mo;
	player->mana[MANA_1] -= (NewWeaponInfo[player->readyweapon].ammouse*10/player->agility);
	S_StartSound(pmo, SFX_MAGE_SHARDS_FIRE);

	damage = 90+(P_Random()&15);
	for(i = 0; i < 16; i++)
	{
		angle = pmo->angle+i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, MELEERANGE);
		if(linetarget)
		{
			pmo->flags2 |= MF2_ICEDAMAGE;
			P_DamageMobj(linetarget, pmo, pmo, damage);
			pmo->flags2 &= ~MF2_ICEDAMAGE;
			conedone = true;
			break;
		}
	}

	// didn't find any creatures, so fire projectiles
	if (!conedone)
	{
		mo = P_SpawnPlayerMissile(pmo, MT_SHARDFX1);
		if (mo)
		{
			mo->special1 = SHARDSPAWN_LEFT|SHARDSPAWN_DOWN|SHARDSPAWN_UP
				|SHARDSPAWN_RIGHT;
			mo->special2 = 3; // Set sperm count (levels of reproductivity)
			mo->target = pmo;
			mo->args[0] = 3;		// Mark Initial shard as super damage
		}
	}
}

void A_ShedShard(mobj_t *actor)
{
	mobj_t *mo;
	int spawndir = actor->special1;
	int spermcount = actor->special2;

	if (spermcount <= 0) return;				// No sperm left
	actor->special2 = 0;
	spermcount--;

	// every so many calls, spawn a new missile in it's set directions
	if (spawndir & SHARDSPAWN_LEFT)
	{
		mo=P_SpawnMissileAngleSpeed(actor, MT_SHARDFX1, actor->angle+(ANG45/9),
											 0, (20+2*spermcount)<<FRACBITS);
		if (mo)
		{
			mo->special1 = SHARDSPAWN_LEFT;
			mo->special2 = spermcount;
			mo->momz = actor->momz;
			mo->target = actor->target;
			mo->args[0] = (spermcount==3)?2:0;
		}
	}
	if (spawndir & SHARDSPAWN_RIGHT)
	{
		mo=P_SpawnMissileAngleSpeed(actor, MT_SHARDFX1, actor->angle-(ANG45/9),
											 0, (20+2*spermcount)<<FRACBITS);
		if (mo)
		{
			mo->special1 = SHARDSPAWN_RIGHT;
			mo->special2 = spermcount;
			mo->momz = actor->momz;
			mo->target = actor->target;
			mo->args[0] = (spermcount==3)?2:0;
		}
	}
	if (spawndir & SHARDSPAWN_UP)
	{
		mo=P_SpawnMissileAngleSpeed(actor, MT_SHARDFX1, actor->angle, 
											 0, (15+2*spermcount)<<FRACBITS);
		if (mo)
		{
			mo->momz = actor->momz;
			mo->z += 8*FRACUNIT;
			if (spermcount & 1)			// Every other reproduction
				mo->special1 = SHARDSPAWN_UP | SHARDSPAWN_LEFT | SHARDSPAWN_RIGHT;
			else
				mo->special1 = SHARDSPAWN_UP;
			mo->special2 = spermcount;
			mo->target = actor->target;
			mo->args[0] = (spermcount==3)?2:0;
		}
	}
	if (spawndir & SHARDSPAWN_DOWN)
	{
		mo=P_SpawnMissileAngleSpeed(actor, MT_SHARDFX1, actor->angle, 
											 0, (15+2*spermcount)<<FRACBITS);
		if (mo)
		{
			mo->momz = actor->momz;
			mo->z -= 4*FRACUNIT;
			if (spermcount & 1)			// Every other reproduction
				mo->special1 = SHARDSPAWN_DOWN | SHARDSPAWN_LEFT | SHARDSPAWN_RIGHT;
			else
				mo->special1 = SHARDSPAWN_DOWN;
			mo->special2 = spermcount;
			mo->target = actor->target;
			mo->args[0] = (spermcount==3)?2:0;
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_HideInCeiling
//
//----------------------------------------------------------------------------

/*
void A_HideInCeiling(mobj_t *actor)
{
	actor->z = actor->ceilingz+4*FRACUNIT;
}
*/

//----------------------------------------------------------------------------
//
// PROC A_FloatPuff
//
//----------------------------------------------------------------------------

/*
void A_FloatPuff(mobj_t *puff)
{
	puff->momz += 1.8*FRACUNIT;
}
*/

void A_Light0(player_t *player, pspdef_t *psp)
{
	player->plr->extralight = 0;
}

/*
void A_Light1(player_t *player, pspdef_t *psp)
{
	player->extralight = 1;
}
*/

/*
void A_Light2(player_t *player, pspdef_t *psp)
{
	player->extralight = 2;
}
*/

//------------------------------------------------------------------------
//
// PROC P_SetupPsprites
//
// Called at start of level for each player
//
//------------------------------------------------------------------------

void P_SetupPsprites(player_t *player)
{
	int i;

	// Remove all psprites
	for(i = 0; i < NUMPSPRITES; i++)
	{
		player->psprites[i].state = NULL;
	}
	// Spawn the ready weapon
	if (NewWeaponInfo[player->readyweapon].classtype != player->class)
	{ //Hey, what the hell? The player shouldn't have this weapon!
		P_NewPendingWeapon(player,0);
	} else
	player->pendingweapon = player->readyweapon;
	player->readyweapon = player->pendingweapon;
	P_BringUpWeapon(player);
}

//------------------------------------------------------------------------
//
// PROC P_MovePsprites
//
// Called every tic by player thinking routine
//
//------------------------------------------------------------------------

void P_MovePsprites(player_t *player)
{
	int i;
	pspdef_t *psp;
	state_t *state;

	psp = &player->psprites[0];
	for(i = 0; i < NUMPSPRITES; i++, psp++)
	{
		if((state = psp->state) != 0) // a null state means not active
		{
			// drop tic count and possibly change state
			if(psp->tics != -1)	// a -1 tic count never changes
			{
				psp->tics--;
				if(!psp->tics)
				{
					P_SetPsprite(player, i, psp->state->nextstate);
				}
			}
		}
	}
	player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
	player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}

//============================================================================
//
// PROC A_ProjFire
//Projectile firing, used by some Possessed monsters
//
//============================================================================

void A_ProjFire(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;
	mobj_t *pmo;
	fixed_t an;

	pmo = player->plr->mo;

	switch(player->class)
	{
	case PCLASS_AFRIT:	
		mo = P_SpawnPlayerMissile(pmo, MT_FIREDEMON_FX6);
		if (mo) S_StartSound(pmo, SFX_FIRED_ATTACK);
		break;
	case PCLASS_CENTAURL:
		mo = P_SpawnPlayerMissile(pmo, MT_CENTAUR_FX);
		if (mo) S_StartSound(pmo, SFX_CENTAURLEADER_ATTACK);
		break;
	case PCLASS_ICEGUY:
		an = (pmo->angle+ANG90)>>ANGLETOFINESHIFT;
		P_SPMAngleXYZ(pmo,pmo->x+FixedMul(pmo->radius>>1,
		finecosine[an]), pmo->y+FixedMul(pmo->radius>>1,
		finesine[an]), pmo->z+40*FRACUNIT, MT_ICEGUY_FX, pmo->angle);
		an = (pmo->angle-ANG90)>>ANGLETOFINESHIFT;
		mo = P_SPMAngleXYZ(pmo, pmo->x+FixedMul(pmo->radius>>1,
		finecosine[an]), pmo->y+FixedMul(pmo->radius>>1,
		finesine[an]), pmo->z+40*FRACUNIT, MT_ICEGUY_FX, pmo->angle);
		if (mo) S_StartSound(pmo, SFX_ICEGUY_ATTACK);
		break;
	case PCLASS_SERPENT:
		mo = P_SpawnPlayerMissile(pmo, MT_DEMONFX1);
		if (mo)
		{
			mo->z += 30*FRACUNIT;
			S_StartSound(pmo, SFX_DEMON_MISSILE_FIRE);
		}
		break;
	case PCLASS_BSERPENT:
		mo = P_SpawnPlayerMissile(pmo, MT_DEMON2FX1);
		if (mo)
		{
			mo->z += 30*FRACUNIT;
			S_StartSound(pmo, SFX_DEMON_MISSILE_FIRE);
		}
		break;
	case PCLASS_REIVER:
		mo = P_SpawnPlayerMissile(pmo, MT_WRAITHFX1);
		if (mo) S_StartSound(pmo, SFX_WRAITH_MISSILE_FIRE);
		break;
	case PCLASS_BISHOP:
		if(!pmo->special2)
		{
			pmo->special2 = 0;
			P_SetMobjState(pmo, S_POSS_BISHOP);
			P_SetPsprite(player, ps_weapon, S_POSS_2BISHOPATK_3);
			return;
		}
		mo = P_SpawnPlayerMissile(pmo, MT_BISH_FX);
		if(mo)
		{
			mo->special1 = (int)P_RoughMonsterSearch(mo, 10);
			mo->special2 = 16; // High word == x/y, Low word == z					
		}
		pmo->special2--;
		break;
	}
}

//============================================================================
//
// A_PSetReflective
//
//============================================================================

void A_PSetReflective(player_t *player, pspdef_t *psp)
{
	player->plr->mo->flags2 |= MF2_REFLECTIVE;
	P_SetMobjState(player->plr->mo, S_POSS_CENTAUR_BLOCK);
	A_SetInvulnerable(player->plr->mo);
	player->plr->mo->reactiontime = 2000;
}

//============================================================================
//
// A_PUnSetReflective
//
//============================================================================

void A_PUnSetReflective(player_t *player, pspdef_t *psp)
{
	player->plr->mo->flags2 &= ~MF2_REFLECTIVE;	
	P_SetMobjState(player->plr->mo, S_POSS_CENTAUR);
	A_UnSetInvulnerable(player->plr->mo);
	player->plr->mo->reactiontime = 0;

}

//============================================================================
//
// A_PossBishStart
//
//============================================================================

void A_PossBishStart(player_t *player, pspdef_t *psp)
{
	player->plr->mo->special2 = (P_Random()&3)+5;
	S_StartSound(player->plr->mo, SFX_BISHOP_ATTACK);	
}

//----------------------------------------------------------------------------
//
// PROC A_StaffAttackPL1
//
//----------------------------------------------------------------------------

void A_StaffAttackPL1(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;

	damage = 5+(P_Random()&15);
	angle = player->plr->mo->angle;
	angle += (P_Random()-P_Random())<<18;
	slope = P_AimLineAttack(player->plr->mo, angle, MELEERANGE);
	PuffType = MT_STAFFPUFF;
	P_LineAttack(player->plr->mo, angle, MELEERANGE, slope, damage);
	if(linetarget)
	{
		//S_StartSound(player->plr->mo, sfx_stfhit);
		// turn to face target
		player->plr->mo->angle = R_PointToAngle2(player->plr->mo->x,
			player->plr->mo->y, linetarget->x, linetarget->y);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_StaffAttackPL2
//
//----------------------------------------------------------------------------

void A_StaffAttackPL2(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;

	// P_inter.c:P_DamageMobj() handles target momentums
	damage = 18+(P_Random()&63);
	angle = player->plr->mo->angle;
	angle += (P_Random()-P_Random())<<18;
	slope = P_AimLineAttack(player->plr->mo, angle, MELEERANGE);
	PuffType = MT_STAFFPUFF2;
	P_LineAttack(player->plr->mo, angle, MELEERANGE, slope, damage);
	if(linetarget)
	{
		//S_StartSound(player->plr->mo, sfx_stfpow);
		// turn to face target
		player->plr->mo->angle = R_PointToAngle2(player->plr->mo->x,
			player->plr->mo->y, linetarget->x, linetarget->y);
	}
}

//---------------------------------------------------------------------------
//
// PROC A_GauntletAttack
//
//---------------------------------------------------------------------------

void A_GauntletAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;
	int randVal;
	fixed_t dist;

	psp->sx = ((P_Random()&3)-2)*FRACUNIT;
	psp->sy = WEAPONTOP+(P_Random()&3)*FRACUNIT;
	angle = player->plr->mo->angle;
	if(player->powers[pw_weaponlevel2])
	{
		damage = HITDICE(2);
		dist = 4*MELEERANGE;
		angle += (P_Random()-P_Random())<<17;
		PuffType = MT_GAUNTLETPUFF2;
	}
	else
	{
		damage = HITDICE(2);
		dist = MELEERANGE+1;
		angle += (P_Random()-P_Random())<<18;
		PuffType = MT_GAUNTLETPUFF1;
	}
	slope = P_AimLineAttack(player->plr->mo, angle, dist);
	P_LineAttack(player->plr->mo, angle, dist, slope, damage);
	if(!linetarget)
	{
		if(P_Random() > 64)
		{
			player->plr->extralight = !player->plr->extralight;
		}
		S_StartSound(player->plr->mo, sfx_gntful);
		return;
	}
	randVal = P_Random();
	if(randVal < 64)
	{
		player->plr->extralight = 0;
	}
	else if(randVal < 160)
	{
		player->plr->extralight = 1;
	}
	else
	{
		player->plr->extralight = 2;
	}
	if(player->powers[pw_weaponlevel2])
	{
		P_GiveBody(player, damage>>1);
		S_StartSound(player->plr->mo, sfx_gntpow);
	}
	else
	{
		S_StartSound(player->plr->mo, sfx_gnthit);
	}
	// turn to face target
	angle = R_PointToAngle2(player->plr->mo->x, player->plr->mo->y,
		linetarget->x, linetarget->y);
	if(angle-player->plr->mo->angle > ANG180)
	{
		if(angle-player->plr->mo->angle < -ANG90/20)
			player->plr->mo->angle = angle+ANG90/21;
		else
			player->plr->mo->angle -= ANG90/20;
	}
	else
	{
		if(angle-player->plr->mo->angle > ANG90/20)
			player->plr->mo->angle = angle-ANG90/21;
		else
			player->plr->mo->angle += ANG90/20;
	}
	player->plr->mo->flags |= MF_JUSTATTACKED;
}

//----------------------------------------------------------------------------
//
// PROC A_FireGoldWandPL1
//
//----------------------------------------------------------------------------

void A_FireGoldWandPL1(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;
	angle_t angle;
	int damage;

	mo = player->plr->mo;
	player->mana[NewWeaponInfo[player->readyweapon].mana]
		-= NewWeaponInfo[player->readyweapon].ammouse;
	P_BulletSlope(mo);
	damage = 7+(P_Random()&7);
	angle = mo->angle;
	if(player->refire)
	{
		angle += (P_Random()-P_Random())<<18;
	}
	PuffType = MT_GOLDWANDPUFF1;
	P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
	S_StartSound(player->plr->mo, sfx_gldhit);
}

//----------------------------------------------------------------------------
//
// PROC A_FireGoldWandPL2
//
//----------------------------------------------------------------------------

void A_FireGoldWandPL2(player_t *player, pspdef_t *psp)
{
	int i;
	mobj_t *mo;
	angle_t angle;
	int damage;
	fixed_t momz;

	mo = player->plr->mo;
	player->mana[NewWeaponInfo[player->readyweapon].mana] -=
		deathmatch ? NewWeaponInfo[player->readyweapon-1].ammouse 
		: NewWeaponInfo[player->readyweapon].ammouse;
	PuffType = MT_GOLDWANDPUFF2;
	P_BulletSlope(mo);
	momz = FixedMul(mobjinfo[MT_GOLDWANDFX2].speed, bulletslope);
	P_SpawnMissileAngle(mo, MT_GOLDWANDFX2, mo->angle-(ANG45/8), momz);
	P_SpawnMissileAngle(mo, MT_GOLDWANDFX2, mo->angle+(ANG45/8), momz);
	angle = mo->angle-(ANG45/8);
	for(i = 0; i < 5; i++)
	{
		damage = 1+(P_Random()&7);
		P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
		angle += ((ANG45/8)*2)/4;
	}
	S_StartSound(player->plr->mo, sfx_gldhit);
}

//----------------------------------------------------------------------------
//
// PROC A_FireMacePL1B
//
//----------------------------------------------------------------------------

void A_FireMacePL1B(player_t *player, pspdef_t *psp)
{
	mobj_t *pmo;
	mobj_t *ball;
	angle_t angle;

	if(player->mana[NewWeaponInfo[player->readyweapon].mana] < 
		NewWeaponInfo[player->readyweapon].ammouse)
	{
		return;
	}
	player->mana[NewWeaponInfo[player->readyweapon].mana] 
		-= NewWeaponInfo[player->readyweapon].ammouse;
	pmo = player->plr->mo;
	ball = P_SpawnMobj(pmo->x, pmo->y, pmo->z+28*FRACUNIT 
		//- FOOTCLIPSIZE*(pmo->flags2&MF2_FEETARECLIPPED != 0)
		, MT_MACEFX2);
	ball->momz = 2*FRACUNIT+(((int)player->plr->lookdir)<<(FRACBITS-5));
	angle = pmo->angle;
	ball->target = pmo;
	ball->angle = angle;
	ball->z += ((int)player->plr->lookdir)<<(FRACBITS-4);
	angle >>= ANGLETOFINESHIFT;
	ball->momx = (pmo->momx>>1)
		+FixedMul(ball->info->speed, finecosine[angle]);
	ball->momy = (pmo->momy>>1)
		+FixedMul(ball->info->speed, finesine[angle]);
	S_StartSound(ball, sfx_lobsht);
	P_CheckMissileSpawn(ball);
}

//----------------------------------------------------------------------------
//
// PROC A_FireMacePL1
//
//----------------------------------------------------------------------------

void A_FireMacePL1(player_t *player, pspdef_t *psp)
{
	mobj_t *ball;

	if(P_Random() < 28)
	{
		A_FireMacePL1B(player, psp);
		return;
	}
	if(player->mana[NewWeaponInfo[player->readyweapon].mana] 
		< NewWeaponInfo[player->readyweapon].ammouse)
	{
		return;
	}
	player->mana[NewWeaponInfo[player->readyweapon].mana] 
		-= NewWeaponInfo[player->readyweapon].ammouse;
	psp->sx = ((P_Random()&3)-2)*FRACUNIT;
	psp->sy = WEAPONTOP+(P_Random()&3)*FRACUNIT;
	ball = P_SPMAngle(player->plr->mo, MT_MACEFX1, player->plr->mo->angle
		+(((P_Random()&7)-4)<<24));
	if(ball)
	{
		ball->special1 = 16; // tics till dropoff
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MacePL1Check
//
//----------------------------------------------------------------------------

void A_MacePL1Check(mobj_t *ball)
{
	angle_t angle;

	if(ball->special1 == 0)
	{
		return;
	}
	ball->special1 -= 4;
	if(ball->special1 > 0)
	{
		return;
	}
	ball->special1 = 0;
	ball->flags2 |= MF2_LOGRAV;
	angle = ball->angle>>ANGLETOFINESHIFT;
	ball->momx = FixedMul(7*FRACUNIT, finecosine[angle]);
	ball->momy = FixedMul(7*FRACUNIT, finesine[angle]);
	ball->momz -= ball->momz>>1;
}

//----------------------------------------------------------------------------
//
// PROC A_MaceBallImpact
//
//----------------------------------------------------------------------------

void A_MaceBallImpact(mobj_t *ball)
{
	if((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
	{ // Landed in some sort of liquid
		P_RemoveMobj(ball);
		return;
	}
	if((ball->health != MAGIC_JUNK) && (ball->z <= ball->floorz)
		&& ball->momz)
	{ // Bounce
		ball->health = MAGIC_JUNK;
		ball->momz = (ball->momz*192)>>8;
		ball->flags2 &= ~MF2_FLOORBOUNCE;
		P_SetMobjState(ball, ball->info->spawnstate);
		S_StartSound(ball, sfx_bounce);
	}
	else
	{ // Explode
		ball->flags |= MF_NOGRAVITY;
		ball->flags2 &= ~MF2_LOGRAV;
		S_StartSound(ball, sfx_lobhit);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_MaceBallImpact2
//
//----------------------------------------------------------------------------

void A_MaceBallImpact2(mobj_t *ball)
{
	mobj_t *tiny;
	angle_t angle;

	if((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
	{ // Landed in some sort of liquid
		P_RemoveMobj(ball);
		return;
	}
	if((ball->z != ball->floorz) || (ball->momz < 2*FRACUNIT))
	{ // Explode
		ball->momx = ball->momy = ball->momz = 0;
		ball->flags |= MF_NOGRAVITY;
		ball->flags2 &= ~(MF2_LOGRAV|MF2_FLOORBOUNCE);
	}
	else
	{ // Bounce
		ball->momz = (ball->momz*192)>>8;
		P_SetMobjState(ball, ball->info->spawnstate);

		tiny = P_SpawnMobj(ball->x, ball->y, ball->z, MT_MACEFX3);
		angle = ball->angle+ANG90;
		tiny->target = ball->target;
		tiny->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		tiny->momx = (ball->momx>>1)+FixedMul(ball->momz-FRACUNIT,
			finecosine[angle]);
		tiny->momy = (ball->momy>>1)+FixedMul(ball->momz-FRACUNIT,
			finesine[angle]);
		tiny->momz = ball->momz;
		P_CheckMissileSpawn(tiny);

		tiny = P_SpawnMobj(ball->x, ball->y, ball->z, MT_MACEFX3);
		angle = ball->angle-ANG90;
		tiny->target = ball->target;
		tiny->angle = angle;
		angle >>= ANGLETOFINESHIFT;
		tiny->momx = (ball->momx>>1)+FixedMul(ball->momz-FRACUNIT,
			finecosine[angle]);
		tiny->momy = (ball->momy>>1)+FixedMul(ball->momz-FRACUNIT,
			finesine[angle]);
		tiny->momz = ball->momz;
		P_CheckMissileSpawn(tiny);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_FireMacePL2
//
//----------------------------------------------------------------------------

void A_FireMacePL2(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;

	player->mana[NewWeaponInfo[player->readyweapon].mana] -=
		deathmatch ? NewWeaponInfo[player->readyweapon-1].mana 
		: NewWeaponInfo[player->readyweapon].mana;
	mo = P_SpawnPlayerMissile(player->plr->mo, MT_MACEFX4);
	if(mo)
	{
		mo->momx += player->plr->mo->momx;
		mo->momy += player->plr->mo->momy;
		mo->momz = 2*FRACUNIT+(((int)player->plr->lookdir)<<(FRACBITS-5));
		if(linetarget)
		{
			mo->special1 = (int)linetarget;
		}
	}
	S_StartSound(player->plr->mo, sfx_lobsht);
}

//----------------------------------------------------------------------------
//
// PROC A_DeathBallImpact
//
//----------------------------------------------------------------------------

void A_DeathBallImpact(mobj_t *ball)
{
	int i;
	mobj_t *target;
	angle_t angle;
	boolean newAngle;

	if((ball->z <= ball->floorz) && (P_HitFloor(ball) != FLOOR_SOLID))
	{ // Landed in some sort of liquid
		P_RemoveMobj(ball);
		return;
	}
	if((ball->z <= ball->floorz) && ball->momz)
	{ // Bounce
		newAngle = false;
		target = (mobj_t *)ball->special1;
		if(target)
		{
			if(!(target->flags&MF_SHOOTABLE))
			{ // Target died
				ball->special1 = 0;
			}
			else
			{ // Seek
				angle = R_PointToAngle2(ball->x, ball->y,
					target->x, target->y);
				newAngle = true;
			}
		}
		else
		{ // Find new target
			angle = 0;
			for(i = 0; i < 16; i++)
			{
				P_AimLineAttack(ball, angle, 10*64*FRACUNIT);
				if(linetarget && ball->target != linetarget)
				{
					ball->special1 = (int)linetarget;
					angle = R_PointToAngle2(ball->x, ball->y,
						linetarget->x, linetarget->y);
					newAngle = true;
					break;
				}
				angle += ANGLE_45/2;
			}
		}
		if(newAngle)
		{
			ball->angle = angle;
			angle >>= ANGLETOFINESHIFT;
			ball->momx = FixedMul(ball->info->speed, finecosine[angle]);
			ball->momy = FixedMul(ball->info->speed, finesine[angle]);
		}
		P_SetMobjState(ball, ball->info->spawnstate);
		S_StartSound(ball, sfx_pstop);
	}
	else
	{ // Explode
		ball->flags |= MF_NOGRAVITY;
		ball->flags2 &= ~MF2_LOGRAV;
		S_StartSound(ball, sfx_phohit);
	}
}

//----------------------------------------------------------------------------
//
// PROC A_FireCrossbowPL1
//
//----------------------------------------------------------------------------

void A_FireCrossbowPL1(player_t *player, pspdef_t *psp)
{
	mobj_t *pmo;

	pmo = player->plr->mo;
	player->mana[NewWeaponInfo[player->readyweapon].mana] 
		-= NewWeaponInfo[player->readyweapon].ammouse;
	P_SpawnPlayerMissile(pmo, MT_CRBOWFX1);
	P_SPMAngle(pmo, MT_CRBOWFX3, pmo->angle-(ANG45/10));
	P_SPMAngle(pmo, MT_CRBOWFX3, pmo->angle+(ANG45/10));
	S_StartSound(pmo, sfx_bowsht);
}

//----------------------------------------------------------------------------
//
// PROC A_FireCrossbowPL2
//
//----------------------------------------------------------------------------

void A_FireCrossbowPL2(player_t *player, pspdef_t *psp)
{
	mobj_t *pmo;

	pmo = player->plr->mo;
	player->mana[NewWeaponInfo[player->readyweapon].mana] -=
		deathmatch ? NewWeaponInfo[player->readyweapon-1].ammouse 
		: NewWeaponInfo[player->readyweapon].ammouse;
	P_SpawnPlayerMissile(pmo, MT_CRBOWFX2);
	P_SPMAngle(pmo, MT_CRBOWFX2, pmo->angle-(ANG45/10));
	P_SPMAngle(pmo, MT_CRBOWFX2, pmo->angle+(ANG45/10));
	P_SPMAngle(pmo, MT_CRBOWFX3, pmo->angle-(ANG45/5));
	P_SPMAngle(pmo, MT_CRBOWFX3, pmo->angle+(ANG45/5));
	S_StartSound(player->plr->mo, sfx_bowsht);
}

//----------------------------------------------------------------------------
//
// PROC A_BoltSpark
//
//----------------------------------------------------------------------------

void A_BoltSpark(mobj_t *bolt)
{
	mobj_t *spark;

	if(P_Random() > 50)
	{
		spark = P_SpawnMobj(bolt->x, bolt->y, bolt->z, MT_CRBOWFX4);
		spark->x += (P_Random()-P_Random())<<10;
		spark->y += (P_Random()-P_Random())<<10;
	}
}
