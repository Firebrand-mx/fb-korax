
//**************************************************************************
//**
//** p_inter.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

#include "h2def.h"
#include "p_local.h"
#include "soundst.h"
#include "settings.h"
//#include "console.h"

int echoMsg = 1;

#define BONUSADD 6

int ArmorIncrement[NUMCLASSES][NUMARMOR] =
{
	{ 25*FRACUNIT, 20*FRACUNIT, 15*FRACUNIT, 5*FRACUNIT },
	{ 10*FRACUNIT, 25*FRACUNIT, 5*FRACUNIT, 20*FRACUNIT },
	{ 5*FRACUNIT, 15*FRACUNIT, 10*FRACUNIT, 25*FRACUNIT },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }
};

int AutoArmorSave[NUMCLASSES] = { 15*FRACUNIT, 10*FRACUNIT, 5*FRACUNIT, 0 };

char *TextKeyMessages[] = 
{
	TXT_KEY_STEEL,
	TXT_KEY_CAVE,
	TXT_KEY_AXE,
	TXT_KEY_FIRE,
	TXT_KEY_EMERALD,
	TXT_KEY_DUNGEON,
	TXT_KEY_SILVER,
	TXT_KEY_RUSTED,
	TXT_KEY_HORN,
	TXT_KEY_SWAMP,
	TXT_KEY_CASTLE
};

void P_UndoPossessMonster(mobj_t *actor, player_t *player); // Remi
void Give_Level(player_t *player,unsigned int level);
static void SetDormantArtifact(mobj_t *arti);
static void TryPickupArtifact(player_t *player, artitype_t artifactType,
	mobj_t *artifact);
static void TryPickupWeapon(player_t *player,
	newweapontype_t weaponType, mobj_t *weapon, char *message);
static void TryPickupWeaponPiece(player_t *player, pclass_t matchClass,
	int pieceValue, mobj_t *pieceMobj);

#ifdef __NeXT__
extern void strupr(char *s);
#endif

//--------------------------------------------------------------------------
//
// PROC P_SetMessage
//
//--------------------------------------------------------------------------

void P_SetMessage(player_t *player, char *message, boolean ultmsg)
{
	extern boolean messageson;
	extern int echoMsg;
			
	if((player->ultimateMessage || !messageson) && !ultmsg)
	{
		return;
	}

	if(!player->messageTics <= 0 && player->message) //If there's already a message, move it to 2
	{
		if (player->yellowMessage) player->yellowMessage2 = true;
		else player->yellowMessage2 = false;
		player->ultimateMessage = false;
		strcpy(player->message2, player->message);
		player->messageTics2 = player->messageTics;
	}

	if(strlen(message) > 79)
	{
		memcpy(player->message, message, 80);
		player->message[80] = 0;
	}
	else
	{
		strcpy(player->message, message);
	}
	strupr(player->message);
	player->messageTics = MESSAGETICS;
	player->yellowMessage = false;
	if(ultmsg)
	{
		player->ultimateMessage = true;
	}
	if(player == &players[consoleplayer])
	{
		//BorderTopRefresh = true;
		gi.Update(DDUF_TOP);
	}
	// Also show the message in the console.
	if(echoMsg) gi.flconprintf(CBLF_CYAN, "%s\n", message);
}

//==========================================================================
//
// P_SetYellowMessage
//
//==========================================================================

void P_SetYellowMessage(player_t *player, char *message, boolean ultmsg)
{
	extern boolean messageson;
	extern int echoMsg;
	
	if((player->ultimateMessage || !messageson) && !ultmsg)
	{
		return;
	}

	if(!player->messageTics <= 0 && player->message) //If there's already a message, move it to 2
	{
		if (player->yellowMessage) player->yellowMessage2 = true;
		else player->yellowMessage2 = false;
		player->ultimateMessage = false;
		strcpy(player->message2, player->message);
		player->messageTics2 = player->messageTics;
	}

	if(strlen(message) > 79)
	{
		memcpy(player->message, message, 80);
		player->message[80] = 0;
	}
	else
	{
		strcpy(player->message, message);
	}
	player->messageTics = 5*MESSAGETICS; // Bold messages last longer
	player->yellowMessage = true;
	if(ultmsg)
	{
		player->ultimateMessage = true;
	}
	if(player == &players[consoleplayer])
	{
		//BorderTopRefresh = true;
		gi.Update(DDUF_TOP);
	}
	// Also show the message in the console.
	if(echoMsg) gi.flconprintf(CBLF_CYAN, "%s\n", message);
}

//==========================================================================
//
// P_ClearMessage
//
//==========================================================================

void P_ClearMessage(player_t *player)
{
	player->messageTics = 0;
	player->messageTics2 = 0;
	if(player == &players[consoleplayer])
	{
		//BorderTopRefresh = true;
		gi.Update(DDUF_TOP);
	}
}

//----------------------------------------------------------------------------
//
// PROC P_HideSpecialThing
//
//----------------------------------------------------------------------------

void P_HideSpecialThing(mobj_t *thing)
{
	thing->flags &= ~MF_SPECIAL;
	thing->flags2 |= MF2_DONTDRAW; 
	P_SetMobjState(thing, S_HIDESPECIAL1);
}

//--------------------------------------------------------------------------
//
// FUNC P_GiveMana
//
// Returns true if the player accepted the mana, false if it was
// refused (player has MAX_MANA).
//
//--------------------------------------------------------------------------

boolean P_GiveMana(player_t *player, manatype_t mana, int count)
{
	int prevMana;
	//weapontype_t changeWeapon;

	if(mana == MANA_NONE || mana == MANA_BOTH)
	{
		return(false);
	}
	if(mana < 0 || mana > NUMMANA)
	{
		gi.Error("P_GiveMana: bad type %i", mana);
	}
	if ((player->exp_level<3)&&(player->class==1))
	{
		if(boringmessage == 0)
		{
			P_SetMessage(player, "YOU NEED TO BE LEVEL 3 TO USE MANA", false);
			boringmessage = BORINGTICS;
		}
		return false;
	}
	if ((player->exp_level<5)&&(player->class==0))
	{
		if(boringmessage == 0)
		{
			P_SetMessage(player, "YOU NEED TO BE LEVEL 5 TO USE MANA", false);
			boringmessage = BORINGTICS;
		}
		return false;
	}
	if(player->mana[mana] == MAX_MANA)
	{
		return(false);
	}
	if(gameskill == sk_baby || gameskill == sk_nightmare)
	{ // extra mana in baby mode and nightmare mode
		count += count>>1;
	}
	prevMana = player->mana[mana];

	player->mana[mana] += count;
	if(player->mana[mana] > MAX_MANA)
	{
		player->mana[mana] = MAX_MANA;
	}	
	return(true);
}

//==========================================================================
//
// TryPickupWeapon
//
//==========================================================================

static void TryPickupWeapon(player_t *player,
	newweapontype_t weaponType, mobj_t *weapon, char *message)
{
	boolean remove;
	boolean gaveMana;
	boolean gaveWeapon;
	int weaponnum;

	weaponnum = P_GetWeaponNum(player, weaponType);
	
	remove = true;
	if(player->class != NewWeaponInfo[weaponType].classtype)
	{ // Wrong class, but try to pick up for mana
		if(netgame && !deathmatch)
		{ // Can't pick up weapons for other classes in coop netplay
			return;
		}
		if(NewWeaponInfo[weaponType].mana != MANA_BOTH
			|| NewWeaponInfo[weaponType].mana != MANA_NONE)
		{
			if(!P_GiveMana(player, NewWeaponInfo[weaponType].mana, 25))
			{
				return;
			}
		}		
	}
	else if(netgame && !deathmatch)
	{ // Cooperative net-game
		if(player->weaponowned[weaponnum])
		{
			return;
		}
		player->weaponowned[weaponnum] = true;
		if (weaponType == WP_FIGHTER_AXE)  //Remi: Special case, get both axes
				player->weaponowned[P_GetWeaponNum(player, WP_FIGHTER_AXE_G)] = true;

		if(NewWeaponInfo[weaponType].mana != MANA_BOTH
			|| NewWeaponInfo[weaponType].mana != MANA_NONE)
		{
			P_GiveMana(player, NewWeaponInfo[weaponType].mana, 25);
		}		
		//P_NewPendingWeapon(player,weaponType);
		player->pendingweapon = weaponType;
		remove = false;
	}
	else
	{ // Deathmatch or single player game
		if(NewWeaponInfo[weaponType].mana != MANA_BOTH
			|| NewWeaponInfo[weaponType].mana != MANA_NONE)
		{
			gaveMana = P_GiveMana(player, NewWeaponInfo[weaponType].mana, 25);
		}				
		if(player->weaponowned[weaponnum])
		{
			gaveWeapon = false;
		}
/*		else
		if ((player->exp_level<3)&&(weaponType == WP_SECOND))
		{
			P_SetMessage(player, "YOU NEED TO BE LEVEL 3 TO USE YOUR SECOND WEAPON", false);
			gaveWeapon = false;
		} 
		else if((player->exp_level<5)&&(weaponType == WP_THIRD))
		{
			P_SetMessage(player, "YOU NEED TO BE LEVEL 5 TO USE YOUR THIRD WEAPON", false);
			gaveWeapon = false;
		}*/   //kmod
		else
		{
			gaveWeapon = true;
			player->weaponowned[weaponnum] = true;
			if (weaponType == WP_FIGHTER_AXE)  //Remi: Special case, get both axes
				player->weaponowned[P_GetWeaponNum(player, WP_FIGHTER_AXE_G)] = true;
			if(NewWeaponInfo[weaponType].bindkey 
				> NewWeaponInfo[player->readyweapon].bindkey)
			{ // Only switch to more powerful weapons
				P_NewPendingWeapon(player,NewWeaponInfo[weaponType].bindkey); //Remi
				//player->pendingweapon = weaponType;
			}
		}
		if(!(gaveWeapon || gaveMana))
		{ // Player didn't need the weapon or any mana
			return;
		}
	}

	P_SetMessage(player, message, false);
	if(weapon->special)
	{
		P_ExecuteLineSpecial(weapon->special, weapon->args,
			NULL, 0, player->plr->mo);
		weapon->special = 0;
	}

	if(remove)
	{
		if(deathmatch && !(weapon->flags2&MF2_DROPPED))
		{
			P_HideSpecialThing(weapon);
		}
		else
		{
			P_RemoveMobj(weapon);
		}
	}

	player->bonuscount += BONUSADD;
	if(player == &players[consoleplayer])
	{
		S_StartSound(NULL, SFX_PICKUP_WEAPON);
		SB_PaletteFlash(false);
	}
}

//--------------------------------------------------------------------------
//
// FUNC P_GiveWeapon
//
// Returns true if the weapon or its mana was accepted.
//
//--------------------------------------------------------------------------

/*
boolean P_GiveWeapon(player_t *player, pclass_t class, weapontype_t weapon)
{
	boolean gaveMana;
	boolean gaveWeapon;

	if(player->class != class)
	{ // player cannot use this weapon, take it anyway, and get mana
		if(netgame && !deathmatch)
		{ // Can't pick up weapons for other classes in coop netplay
			return false;
		}
		if(weapon == WP_SECOND)
		{
			return P_GiveMana(player, MANA_1, 25);
		}
		else
		{
			return P_GiveMana(player, MANA_2, 25);
		}		
	}
	if(netgame && !deathmatch)
	{ // Cooperative net-game
		if(player->weaponowned[weapon])
		{
			return(false);
		}
		player->bonuscount += BONUSADD;
		player->weaponowned[weapon] = true;
		if(weapon == WP_SECOND)
		{
			P_GiveMana(player, MANA_1, 25);
		}
		else 
		{
			P_GiveMana(player, MANA_2, 25);
		}
		player->pendingweapon = weapon;
		if(player == &players[consoleplayer])
		{
			S_StartSound(NULL, SFX_PICKUP_WEAPON);
		}
		return(false);
	}
	if(weapon == WP_SECOND)
	{
		gaveMana = P_GiveMana(player, MANA_1, 25);
	}
	else 
	{
		gaveMana = P_GiveMana(player, MANA_2, 25);
	}
	if(player->weaponowned[weapon])
	{
		gaveWeapon = false;
	}
	else
	{
		gaveWeapon = true;
		player->weaponowned[weapon] = true;
		if(weapon > player->readyweapon)
		{ // Only switch to more powerful weapons
			player->pendingweapon = weapon;
		}
	}
	return(gaveWeapon || gaveMana);
}
*/

//===========================================================================
//
// P_GiveWeaponPiece
//
//===========================================================================

/*
boolean P_GiveWeaponPiece(player_t *player, pclass_t class, int piece)
{
	P_GiveMana(player, MANA_1, 20);
	P_GiveMana(player, MANA_2, 20);
	if(player->class != class)
	{
		return true;
	}
	else if(player->pieces&piece)
	{ // player already has that weapon piece
		return true;
	}
	player->pieces |= piece;
	if(player->pieces == 7)
	{ // player has built the fourth weapon!
		P_GiveWeapon(player, class, WP_FOURTH);
		S_StartSound(player->plr->mo, SFX_WEAPON_BUILD);
	}
	return true;
}
*/

//==========================================================================
//
// TryPickupWeaponPiece
//
//==========================================================================

static void TryPickupWeaponPiece(player_t *player, pclass_t matchClass,
	int pieceValue, mobj_t *pieceMobj)
{
	boolean remove;
	boolean checkAssembled;
	boolean gaveWeapon;
	int gaveMana;
	newweapontype_t weapontype;
	
	static char *fourthWeaponText[] =
	{
		TXT_WEAPON_F4,
		TXT_WEAPON_C4,
		TXT_WEAPON_M4
	};
	static char *weaponPieceText[] =
	{
		TXT_QUIETUS_PIECE,
		TXT_WRAITHVERGE_PIECE,
		TXT_BLOODSCOURGE_PIECE
	};
	static int pieceValueTrans[] =
	{
		0,							// 0: never
		WPIECE1|WPIECE2|WPIECE3,	// WPIECE1 (1)
		WPIECE2|WPIECE3,			// WPIECE2 (2)
		0,							// 3: never
		WPIECE3						// WPIECE3 (4)
	};

	remove = true;
	checkAssembled = true;
	gaveWeapon = false;
	switch(player->class)
	{
	case PCLASS_FIGHTER:
		weapontype = WP_FIGHTER_SWORD;
		break;
	case PCLASS_CLERIC:
		weapontype = WP_CLERIC_HOLY;
		break;
	case PCLASS_MAGE:
		weapontype = WP_MAGE_STAFF;
		break;
	}
		
	if(player->class != matchClass)
	{ // Wrong class, but try to pick up for mana
		if(netgame && !deathmatch)
		{ // Can't pick up wrong-class weapons in coop netplay
			return;
		}
		checkAssembled = false;
		gaveMana = P_GiveMana(player, MANA_1, 20)+
			P_GiveMana(player, MANA_2, 20);
		if(!gaveMana)
		{ // Didn't need the mana, so don't pick it up
			return;
		}
	}
	else if(netgame && !deathmatch)
	{ // Cooperative net-game
		if(player->pieces&pieceValue)
		{ // Already has the piece
			return;
		}
		pieceValue = pieceValueTrans[pieceValue];
		P_GiveMana(player, MANA_1, 20);
		P_GiveMana(player, MANA_2, 20);
		remove = false;
	}
	else
	{ // Deathmatch or single player game
		gaveMana = P_GiveMana(player, MANA_1, 20)+
			P_GiveMana(player, MANA_2, 20);
		if(player->pieces&pieceValue)
		{ // Already has the piece, check if mana needed
			if(!gaveMana)
			{ // Didn't need the mana, so don't pick it up
				return;
			}
			checkAssembled = false;
		}
	}

	// Pick up the weapon piece
	if(pieceMobj->special)
	{
		P_ExecuteLineSpecial(pieceMobj->special, pieceMobj->args,
			NULL, 0, player->plr->mo);
		pieceMobj->special = 0;
	}
	if(remove)
	{
		if(deathmatch && !(pieceMobj->flags2&MF2_DROPPED))
		{
			P_HideSpecialThing(pieceMobj);
		}
		else
		{
			P_RemoveMobj(pieceMobj);
		}
	}
	player->bonuscount += BONUSADD;
	if(player == &players[consoleplayer])
	{
		SB_PaletteFlash(false);
	}

	// Check if fourth weapon assembled
	if(checkAssembled)
	{
		player->pieces |= pieceValue;
		if(player->pieces == (WPIECE1|WPIECE2|WPIECE3))
		{
			gaveWeapon = true;
			player->weaponowned[P_GetWeaponNum(player, weapontype)] = true;
			if (weapontype == WP_FIGHTER_SWORD)
				player->weaponowned[P_GetWeaponNum(player, WP_FIGHTER_SWORD_G)] = true;
			P_NewPendingWeapon(player,NewWeaponInfo[weapontype].bindkey); //Remi
		}
	}

	if(gaveWeapon)
	{
		P_SetMessage(player, fourthWeaponText[matchClass], false);
		// Play the build-sound full volume for all players
		S_StartSound(NULL, SFX_WEAPON_BUILD);
	}
	else
	{
		P_SetMessage(player, weaponPieceText[matchClass], false);
		if(player == &players[consoleplayer])
		{
			S_StartSound(NULL, SFX_PICKUP_WEAPON);
		}
	}
}

//---------------------------------------------------------------------------
//
// FUNC P_GiveBody
//
// Returns false if the body isn't needed at all.
//
//---------------------------------------------------------------------------

boolean P_GiveBody(player_t *player, int num)
{
	int max;
	int tsp=0;
	unsigned int o_sp=player->sp_power;
	if (num == 25 && !player->morphTics)
	{
		switch (player->class)
		{
		case PCLASS_FIGHTER: 
			num=25+player->exp_level;
			tsp=5+player->exp_level;
			break;
		case PCLASS_CLERIC: 
			num=15+player->exp_level;
			tsp=10+player->exp_level;
			break;
		case PCLASS_MAGE: 
			num=8+player->exp_level;
			tsp=17+player->exp_level;
			break;
		default:
			break;
		}
	}

	if (num == 100 && !player->morphTics)
	{
		switch (player->class)
		{
		case PCLASS_FIGHTER: 
			num=player->maxhealth;;
			tsp=player->maxsp_power;
			break;
		case PCLASS_CLERIC: 
			num=player->maxhealth;;
			tsp=player->maxsp_power;
			break;
		case PCLASS_MAGE: 
			num=player->maxhealth;;
			tsp=player->maxsp_power;
			break;
		default:
			break;
		}
	}

	if (tsp+player->sp_power>player->maxsp_power)
		tsp=player->maxsp_power-player->sp_power;
	player->sp_power+=tsp;
	max = player->maxhealth;
	if(player->morphTics)
	{
		max = MAXMORPHHEALTH;
	}
	if (player->class >= PCLASS_ETTIN)
	{
		max = player->plr->mo->info->spawnhealth;
	}
	if(player->health >= max && player->sp_power==o_sp)
	{
		return(false);
	}
	if(player->health >= max)
	{
		return(true);
	}
	player->health += num;
	if(player->health > max)
	{
		player->health = max;
	}
	player->plr->mo->health = player->health;
	return(true);
}

//---------------------------------------------------------------------------
//
// FUNC P_GiveArmor
//
// Returns false if the armor is worse than the current armor.
//
//---------------------------------------------------------------------------

boolean P_GiveArmor(player_t *player, armortype_t armortype, int amount)
{
	int hits;
	int totalArmor;

	extern int ArmorMax[NUMCLASSES];

	if(amount == -1)
	{
		hits = ArmorIncrement[player->class][armortype];
		if(player->armorpoints[armortype] >= hits)
		{
			return false;
		}
		else
		{
			player->armorpoints[armortype] = hits;
		}
	}
	else
	{
		hits = amount*5*FRACUNIT;
		totalArmor = player->armorpoints[ARMOR_ARMOR]
			+player->armorpoints[ARMOR_SHIELD]
			+player->armorpoints[ARMOR_HELMET]
			+player->armorpoints[ARMOR_AMULET]
			+AutoArmorSave[player->class];
		if(totalArmor < ArmorMax[player->class]*5*FRACUNIT)
		{
			player->armorpoints[armortype] += hits;
		}
		else
		{
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------------
//
// PROC P_GiveKey
//
//---------------------------------------------------------------------------

int P_GiveKey(player_t *player, keytype_t key)
{
	if(player->keys&(1<<key))
	{
		return false;
	}
	player->bonuscount += BONUSADD;
	player->keys |= 1<<key;
	return true;
}

//---------------------------------------------------------------------------
//
// FUNC P_GivePower
//
// Returns true if power accepted.
//
//---------------------------------------------------------------------------

boolean P_GivePower(player_t *player, powertype_t power)
{
	if(power == pw_invulnerability)
	{
		if(player->powers[power] > BLINKTHRESHOLD)
		{ // Already have it
			return(false);
		}
		player->powers[power] = INVULNTICS;
		player->plr->mo->flags2 |= MF2_INVULNERABLE;
		if(player->class == PCLASS_MAGE)
		{
			player->plr->mo->flags2 |= MF2_REFLECTIVE;
		}
		return(true);
	}
	if(power == pw_flight)
	{
		if(player->powers[power] > BLINKTHRESHOLD)
		{ // Already have it
			return(false);
		}
		player->powers[power] = FLIGHTTICS;
		player->plr->mo->flags2 |= MF2_FLY;
		player->plr->mo->flags |= MF_NOGRAVITY;
		if(player->plr->mo->z <= player->plr->mo->floorz)
		{
			player->flyheight = 10; // thrust the player in the air a bit
		}
		return(true);
	}
	if(power == pw_infrared)
	{
		if(player->powers[power] > BLINKTHRESHOLD)
		{ // Already have it
			return(false);
		}
		player->powers[power] = INFRATICS;
		return(true);
	}
	if(power == pw_speed)
	{
		if(player->powers[power] > BLINKTHRESHOLD)
		{ // Already have it
			return(false);
		}
		player->powers[power] = SPEEDTICS;
		return(true);
	}
	if(power == pw_minotaur)
	{
		// Doesn't matter if already have power, renew ticker
		player->powers[power] = MAULATORTICS;
		return(true);
	}
/*
	if(power == pw_ironfeet)
	{
		player->powers[power] = IRONTICS;
		return(true);
	}
	if(power == pw_strength)
	{
		P_GiveBody(player, 100);
		player->powers[power] = 1;
		return(true);
	}
*/
	if(player->powers[power])
	{
		return(false); // already got it
	}
	player->powers[power] = 1;
	return(true);
}

//==========================================================================
//
// TryPickupArtifact
//
//==========================================================================

static void TryPickupArtifact(player_t *player, artitype_t artifactType,
	mobj_t *artifact)
{
	static char *artifactMessages[NUMARTIFACTS] =
	{
		NULL,
		TXT_ARTIINVULNERABILITY,
		TXT_ARTIHEALTH,
		TXT_ARTISUPERHEALTH,
		TXT_ARTIHEALINGRADIUS,
		TXT_ARTISUMMON,
		TXT_ARTITORCH,
		TXT_ARTIEGG,
		TXT_ARTIFLY,
		TXT_ARTIBLASTRADIUS,
		TXT_ARTIPOISONBAG,
		TXT_ARTITELEPORTOTHER,
		TXT_ARTISPEED,
		TXT_ARTIBOOSTMANA,
		TXT_ARTIBOOSTARMOR,
		TXT_ARTITELEPORT,
		TXT_ARTIPUZZSKULL,
		TXT_ARTIPUZZGEMBIG,
		TXT_ARTIPUZZGEMRED,
		TXT_ARTIPUZZGEMGREEN1,
		TXT_ARTIPUZZGEMGREEN2,
		TXT_ARTIPUZZGEMBLUE1,
		TXT_ARTIPUZZGEMBLUE2,
		TXT_ARTIPUZZBOOK1,
		TXT_ARTIPUZZBOOK2,
		TXT_ARTIPUZZSKULL2,
		TXT_ARTIPUZZFWEAPON,
		TXT_ARTIPUZZCWEAPON,
		TXT_ARTIPUZZMWEAPON,
		TXT_ARTIPUZZGEAR,	// All gear pickups use the same text
		TXT_ARTIPUZZGEAR,
		TXT_ARTIPUZZGEAR,
		TXT_ARTIPUZZGEAR
	};

	if(P_GiveArtifact(player, artifactType, artifact))
	{
		if(artifact->special)
		{
			P_ExecuteLineSpecial(artifact->special, artifact->args,
				NULL, 0, NULL);
			artifact->special = 0;
		}
		player->bonuscount += BONUSADD;
		if(artifactType < arti_firstpuzzitem)
		{
			SetDormantArtifact(artifact);
			S_StartSound(artifact, SFX_PICKUP_ARTIFACT);
			P_SetMessage(player, artifactMessages[artifactType], false);
		}
		else
		{ // Puzzle item
			S_StartSound(NULL, SFX_PICKUP_ITEM);
			P_SetMessage(player, artifactMessages[artifactType], true);
			if(!netgame || deathmatch)
			{ // Remove puzzle items if not cooperative netplay
				P_RemoveMobj(artifact);
			}
		}
	}
}

//---------------------------------------------------------------------------
//
// FUNC P_GiveArtifact
//
// Returns true if artifact accepted.
//
//---------------------------------------------------------------------------

boolean P_GiveArtifact(player_t *player, artitype_t arti, mobj_t *mo)
{
	int i;
	int j;
	boolean slidePointer;

	slidePointer = false;
	i = 0;
	while(player->inventory[i].type != arti && i < player->inventorySlotNum)
	{
		i++;
	}
	if(i == player->inventorySlotNum)
	{
		if(arti < arti_firstpuzzitem)
		{
			i = 0;
			while(player->inventory[i].type < arti_firstpuzzitem
			&& i < player->inventorySlotNum)
			{
				i++;
			}
			if(i != player->inventorySlotNum)
			{
				for(j = player->inventorySlotNum; j > i; j--)
				{
					player->inventory[j].count = player->inventory[j-1].count;
					player->inventory[j].type = player->inventory[j-1].type;
					slidePointer = true;
				}
			}
		}
		player->inventory[i].count = 1;
		player->inventory[i].type = arti;
		player->inventorySlotNum++;
	}
	else
	{
		if(arti >= arti_firstpuzzitem && netgame && !deathmatch)
		{ // Can't carry more than 1 puzzle item in coop netplay
			return false;
		}
		if(player->inventory[i].count >= 25)
		{ // Player already has 25 of this item
			return false;
		}
		player->inventory[i].count++;
	}
	if(!player->artifactCount)
	{
		player->readyArtifact = arti;
	}
	else if(player == &players[consoleplayer] && slidePointer
		&& i <= inv_ptr)
	{
		inv_ptr++;
		curpos++;
		if(curpos > 6)
		{
			curpos = 6;
		}
	}
	player->artifactCount++;
	return(true);
}

//==========================================================================
//
// SetDormantArtifact
//
// Removes the MF_SPECIAL flag and initiates the artifact pickup
// animation.
//
//==========================================================================

static void SetDormantArtifact(mobj_t *arti)
{
	arti->flags &= ~MF_SPECIAL;
	if(deathmatch && !(arti->flags2 & MF2_DROPPED))
	{
		if(arti->type == MT_ARTIINVULNERABILITY)
		{
			P_SetMobjState(arti, S_DORMANTARTI3_1);
		}
		else if(arti->type == MT_SUMMONMAULATOR
			|| arti->type == MT_ARTIFLY)
		{
			P_SetMobjState(arti, S_DORMANTARTI2_1);
		}
		else
		{
			P_SetMobjState(arti, S_DORMANTARTI1_1);
		}
	}
	else
	{ // Don't respawn
		P_SetMobjState(arti, S_DEADARTI1);
	}
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreArtifact
//
//---------------------------------------------------------------------------

void A_RestoreArtifact(mobj_t *arti)
{
	arti->flags |= MF_SPECIAL;
	P_SetMobjState(arti, arti->info->spawnstate);
	S_StartSound(arti, SFX_RESPAWN);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreSpecialThing1
//
// Make a special thing visible again.
//
//---------------------------------------------------------------------------

void A_RestoreSpecialThing1(mobj_t *thing)
{
	thing->flags2 &= ~MF2_DONTDRAW;
	S_StartSound(thing, SFX_RESPAWN);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreSpecialThing2
//
//---------------------------------------------------------------------------

void A_RestoreSpecialThing2(mobj_t *thing)
{
	thing->flags |= MF_SPECIAL;
	P_SetMobjState(thing, thing->info->spawnstate);
}

//---------------------------------------------------------------------------
//
//	P_CantUseItem
//
//---------------------------------------------------------------------------

void P_CantUseItem(mobj_t *item, player_t *player, char *message)
{
	if (boringmessage == 0)
	{
		P_SetMessage(player, message, false);
		boringmessage = BORINGTICS;
	}

	if (item->special)
	{
		P_ExecuteLineSpecial(item->special, item->args, NULL, 0, player->plr->mo);
		item->special = 0;
	}
}

//---------------------------------------------------------------------------
//
// PROC P_TouchSpecialThing
//
//---------------------------------------------------------------------------

void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher)
{
	player_t *player;
	fixed_t delta;
	int sound;
	boolean respawn;

	delta = special->z-toucher->z;
	if(delta > toucher->height || delta < -32*FRACUNIT)
	{ // Out of reach
		return;
	}
	if(toucher->health <= 0)
	{ // Toucher is dead
		return;
	}
	if (!(toucher->flags & MF_PICKUP))
	{
		//	Pigs can't pick up anything
		if (toucher->player->morphTics)
		{
			return;
		}
		//	Possessed creatures can pick up health vials
		if (special->type != MT_HEALINGBOTTLE)
		{
			return;
		}
	}
	sound = SFX_PICKUP_ITEM;
	player = toucher->player;
	if(player == NULL) return;
	respawn = true;
	switch(special->sprite)
	{
		// Items
		case SPR_PTN1: // Item_HealingPotion
			if(!P_GiveBody(player, 10))
			{
				return;
			}
			P_SetMessage(player, TXT_ITEMHEALTH, false);
			break;
		case SPR_ARM1:
/*			if ((player->exp_level<2)&&(player->class==0))
			{
				P_SetMessage(player, "YOU NEED TO BE LEVEL 2 TO USE THE ARMOR", false);
				return;
			}*/    //kmod
			if (/*(player->exp_level<7)&&*/(player->class==1))
			{
				P_CantUseItem(special, player, "YOU CAN'T USE THE MESH ARMOR");
				return;
			}
			if (/*(player->exp_level<7)&&*/(player->class==2))
			{
				P_CantUseItem(special, player, "YOU CAN'T USE THE MESH ARMOR");
				return;
			}
			if(!P_GiveArmor(player, ARMOR_ARMOR, -1))
			{
				return;
			}
			P_SetMessage(player, TXT_ARMOR1, false);
			break;
		case SPR_ARM2:
/*			if ((player->exp_level<5)&&(player->class==0))
			{
				P_SetMessage(player, "YOU NEED TO BE LEVEL 5 TO USE THE FALCON SHIELD", false);
				return;
			}*/   //kmod
			if (/*(player->exp_level<2)&&*/(player->class==2))
			{				
				P_CantUseItem(special, player, "YOU CAN'T USE THE FALCON SHIELD");
				return;
			}
/*			if ((player->exp_level<5)&&(player->class==2))
			{
				P_SetMessage(player, "YOU NEED TO BE LEVEL 5 TO USE THE FALCON SHIELD", false);
				return;
			}*/   //kmod
			if(!P_GiveArmor(player, ARMOR_SHIELD, -1))
			{
				return;
			}
			P_SetMessage(player, TXT_ARMOR2, false);
			break;
		case SPR_ARM3:
			if ((player->class==2))
			{				
				P_CantUseItem(special, player, "YOU CAN'T USE THE HELMET");
				return;
			}
			if(!P_GiveArmor(player, ARMOR_HELMET, -1))
			{
				return;
			}
			P_SetMessage(player, TXT_ARMOR3, false);
			break;
		case SPR_ARM4:
/*			if ((player->exp_level<7)&&(player->class==0))
			{
				P_SetMessage(player, "YOU NEED TO BE LEVEL 7 TO USE THE AMULET OF WARDING", false);
				return;
			}
			if ((player->exp_level<5)&&(player->class==1))
			{
				P_SetMessage(player, "YOU NEED TO BE LEVEL 5 TO USE THE AMULET OF WARDING", false);
				return;
			}
			if ((player->exp_level<2)&&(player->class==2))
			{
				P_SetMessage(player, "YOU NEED TO BE LEVEL 2 TO USE THE AMULET OF WARDING", false);
				return;
			}*/   //kmod
			if(!P_GiveArmor(player, ARMOR_AMULET, -1))
			{
				return;
			}
			P_SetMessage(player, TXT_ARMOR4, false);
			break;

		// Keys
		case SPR_KEY1:
		case SPR_KEY2:
		case SPR_KEY3:
		case SPR_KEY4:
		case SPR_KEY5:
		case SPR_KEY6:
		case SPR_KEY7:
		case SPR_KEY8:
		case SPR_KEY9:
		case SPR_KEYA:
		case SPR_KEYB:
			if(!P_GiveKey(player, special->sprite-SPR_KEY1))
			{
				return;
			}
			P_SetMessage(player, TextKeyMessages[special->sprite-SPR_KEY1],
				true);
			sound = SFX_PICKUP_KEY;

			// Check and process the special now in case the key doesn't
			// get removed for coop netplay
			if(special->special)
			{
				P_ExecuteLineSpecial(special->special, special->args,
					NULL, 0, toucher);
				special->special = 0;
			}

			if(!netgame)
			{ // Only remove keys in single player game
				break;
			}
			player->bonuscount += BONUSADD;
			if(player == &players[consoleplayer])
			{
				S_StartSound(NULL, sound);
				SB_PaletteFlash(false);
			}
			return;

		// Artifacts
		case SPR_PTN2:
			TryPickupArtifact(player, arti_health, special);
			return;
		case SPR_SOAR:
			TryPickupArtifact(player, arti_fly, special);
			return;
		case SPR_INVU:
			if (player->class==PCLASS_MAGE)
			{
				P_CantUseItem(special, player, "YOU CAN'T USE THE ICON OF THE DEFENDER");
				return;
			}
			TryPickupArtifact(player, arti_invulnerability, special);
			return;
		case SPR_SUMN:
			TryPickupArtifact(player, arti_summon, special);
			return;
		case SPR_PORK:
			TryPickupArtifact(player, arti_egg, special);
			return;
		case SPR_SPHL:
			TryPickupArtifact(player, arti_superhealth, special);
			return;
		case SPR_HRAD:
			TryPickupArtifact(player, arti_healingradius, special);
			return;
		case SPR_TRCH:
			TryPickupArtifact(player, arti_torch, special);
			return;
		case SPR_ATLP:
			TryPickupArtifact(player, arti_teleport, special);
			return;
		case SPR_TELO:
			if (player->class!=PCLASS_CLERIC)
			{
				P_CantUseItem(special, player, "YOU CAN'T USE THE BANISHMENT DEVICE");
				return;
			}
			TryPickupArtifact(player, arti_teleportother, special);
			return;
		case SPR_PSBG:
			TryPickupArtifact(player, arti_poisonbag, special);
			return;
		case SPR_SPED:
			TryPickupArtifact(player, arti_speed, special);
			return;
		case SPR_BMAN:
			TryPickupArtifact(player, arti_boostmana, special);
			return;
		case SPR_BRAC:
			if ((player->class>0))  //(player->exp_level<4)  //kmod
			{
				P_CantUseItem(special, player, "YOU CAN'T USE THE DRAGONSKIN BRACERS");
				return;
			}
			TryPickupArtifact(player, arti_boostarmor, special);
			return;
		case SPR_BLST:
			TryPickupArtifact(player, arti_blastradius, special);
			return;

		// Puzzle artifacts
		case SPR_ASKU:
			TryPickupArtifact(player, arti_puzzskull, special);
			return;
		case SPR_ABGM:
			TryPickupArtifact(player, arti_puzzgembig, special);
			Give_Level(player,(gameskill<2?5:(gameskill<3?4:3)));
			return;
		case SPR_AGMR:
			TryPickupArtifact(player, arti_puzzgemred, special);
			return;
		case SPR_AGMG:
			TryPickupArtifact(player, arti_puzzgemgreen1, special);
			return;
		case SPR_AGG2:
			TryPickupArtifact(player, arti_puzzgemgreen2, special);
			return;
		case SPR_AGMB:
			TryPickupArtifact(player, arti_puzzgemblue1, special);
			return;
		case SPR_AGB2:
			TryPickupArtifact(player, arti_puzzgemblue2, special);
			return;
		case SPR_ABK1:
			TryPickupArtifact(player, arti_puzzbook1, special);
			return;
		case SPR_ABK2:
			TryPickupArtifact(player, arti_puzzbook2, special);
			return;
		case SPR_ASK2:
			TryPickupArtifact(player, arti_puzzskull2, special);
			return;
		case SPR_AFWP:
			TryPickupArtifact(player, arti_puzzfweapon, special);
			return;
		case SPR_ACWP:
			TryPickupArtifact(player, arti_puzzcweapon, special);
			return;
		case SPR_AMWP:
			TryPickupArtifact(player, arti_puzzmweapon, special);
			return;
		case SPR_AGER:
			TryPickupArtifact(player, arti_puzzgear1, special);
			return;
		case SPR_AGR2:
			TryPickupArtifact(player, arti_puzzgear2, special);
			return;
		case SPR_AGR3:
			TryPickupArtifact(player, arti_puzzgear3, special);
			return;
		case SPR_AGR4:
			TryPickupArtifact(player, arti_puzzgear4, special);
			return;

		// Mana 
		case SPR_MAN1:
			if(!P_GiveMana(player, MANA_1, 15))
			{
				return;
			}
			P_SetMessage(player, TXT_MANA_1, false);
			break;
		case SPR_MAN2: 
			if(!P_GiveMana(player, MANA_2, 15))
			{
				return;
			}
			P_SetMessage(player, TXT_MANA_2, false);
			break;
		case SPR_MAN3: // Double Mana Dodecahedron
			if(!P_GiveMana(player, MANA_1, 20))
			{
				if(!P_GiveMana(player, MANA_2, 20))
				{
					return;
				}
			}
			else
			{
				P_GiveMana(player, MANA_2, 20);
			}
			P_SetMessage(player, TXT_MANA_BOTH, false);
			break;

		// 2nd and 3rd Mage Weapons
		case SPR_WMCS: // Frost Shards
			TryPickupWeapon(player, WP_MAGE_CONE,
				special, TXT_WEAPON_M2);
			return;
		case SPR_WMLG: // Arc of Death
			TryPickupWeapon(player, WP_MAGE_LIGHTNING,
				special, TXT_WEAPON_M3);
			return;

		// 2nd and 3rd Fighter Weapons
		case SPR_WFAX: // Timon's Axe
			TryPickupWeapon(player, WP_FIGHTER_AXE,
				special, TXT_WEAPON_F2);
			return;
		case SPR_WFHM: // Hammer of Retribution
			TryPickupWeapon(player, WP_FIGHTER_HAMMER,
				special, TXT_WEAPON_F3);
			return;

		// 2nd and 3rd Cleric Weapons
		case SPR_WCSS: // Serpent Staff
			TryPickupWeapon(player, WP_CLERIC_STAFF,
				special, TXT_WEAPON_C2);
			return;
		case SPR_WCFM: // Firestorm
			TryPickupWeapon(player, WP_CLERIC_FLAME,
				special, TXT_WEAPON_C3);
			return;

		// Fourth Weapon Pieces
		case SPR_WFR1:
			TryPickupWeaponPiece(player, PCLASS_FIGHTER, WPIECE1,
				special);
			return;
		case SPR_WFR2:
			TryPickupWeaponPiece(player, PCLASS_FIGHTER, WPIECE2,
				special);
			return;
		case SPR_WFR3:
			TryPickupWeaponPiece(player, PCLASS_FIGHTER, WPIECE3,
				special);
			return;
		case SPR_WCH1:
			TryPickupWeaponPiece(player, PCLASS_CLERIC, WPIECE1,
				special);
			return;
		case SPR_WCH2:
			TryPickupWeaponPiece(player, PCLASS_CLERIC, WPIECE2,
				special);
			return;
		case SPR_WCH3:
			TryPickupWeaponPiece(player, PCLASS_CLERIC, WPIECE3,
				special);
			return;
		case SPR_WMS1:
			TryPickupWeaponPiece(player, PCLASS_MAGE, WPIECE1,
				special);
			return;
		case SPR_WMS2:
			TryPickupWeaponPiece(player, PCLASS_MAGE, WPIECE2,
				special);
			return;
		case SPR_WMS3:
			TryPickupWeaponPiece(player, PCLASS_MAGE, WPIECE3,
				special);
			return;
		case SPR_SILV:
			player->money += 1;
			P_SetMessage(player, "SILVER", false);
			break;
		case SPR_GOLD:
			player->money += 5;
			P_SetMessage(player, "GOLD", false);
			break;
		default:
			gi.Error("P_SpecialThing: Unknown gettable thing");
	}
	if(special->special)
	{
		P_ExecuteLineSpecial(special->special, special->args, NULL,
			0, toucher);
		special->special = 0;
	}
	if(deathmatch && respawn && !(special->flags2&MF2_DROPPED))
	{
		P_HideSpecialThing(special);
	}
	else
	{
		P_RemoveMobj(special);
	}
	player->bonuscount += BONUSADD;
	if(player == &players[consoleplayer])
	{
		S_StartSound(NULL, sound);
		SB_PaletteFlash(false);
	}
}

// Search thinker list for minotaur
mobj_t *ActiveMinotaur(player_t *master)
{
	mobj_t *mo;
	player_t *plr;
	thinker_t *think;
	unsigned int *starttime;

	for(think = gi.thinkercap->next; think != gi.thinkercap; think = think->next)
	{
		if(think->function != P_MobjThinker) continue;
		mo = (mobj_t *)think;
		if(mo->type != MT_MINOTAUR) continue;
		if(mo->health <= 0) continue;
		if(!(mo->flags&MF_COUNTKILL)) continue;		// for morphed minotaurs
		if(mo->flags&MF_CORPSE) continue;
		starttime = (unsigned int *)mo->args;
		if ((leveltime - *starttime) >= MAULATORTICS) continue;
		plr = ((mobj_t *)mo->special1)->player;
		if(plr == master) return(mo);
	}
	return(NULL);
}

//Search thinker list for player dummy
mobj_t *ActivePlayerDummy(player_t *master)
{
	mobj_t *mo;
	player_t *plr;
	thinker_t *think;

	for(think = gi.thinkercap->next; think != gi.thinkercap; think = think->next)
	{
		if(think->function != P_MobjThinker) continue;
		mo = (mobj_t *)think;
		if(mo->type != MT_DUMMY_MAGE) continue;
		if(mo->health <= 0) continue;
		if(mo->flags&MF_CORPSE) continue;
		plr = ((mobj_t *)mo->special1)->player;
		if(plr == master) return(mo);
	}
	return(NULL);
}

//---------------------------------------------------------------------------
//
// PROC Give_Experience
//
//---------------------------------------------------------------------------
/*typedef struct {
	int addhealth;
	int addstrength;
	int addagility;
	int addspeed;
}next_level;


next_level levelIncrement[NUMCLASSES][5] =
{
	{{9,1,0,0},{9,1,0,0},{9,1,0,0},{9,0,1,0},{9,0,0,1}},
	{{6,1,0,0},{6,0,1,0},{6,1,0,0},{6,0,1,0},{6,0,0,1}},
	{{3,0,1,0},{3,0,1,0},{3,0,1,0},{3,1,0,0},{3,0,0,1}},
	{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
};*/

extern fixed_t health_table[NUMCLASSES][5];
extern fixed_t sppower_table[NUMCLASSES][5];
extern fixed_t skillp_table[5];

void Give_Level(player_t *player,unsigned int level)
{
	int cl,lvl;double asphealt;
	char tmpString[30];
	cl=player->class;
			player->exp_level+=level;
			lvl=player->exp_level;
			//player->experience-=player->next_level;
			player->av_points+=(skillp_table[gameskill]*level);
//			player->next_level=player->next_level*2;//player->prev_level;
			//player->prev_level=player->next_level-player->prev_level;
			asphealt=(double)player->health / (double)player->maxhealth;
			player->maxhealth_old=(player->maxhealth_old!=player->maxhealth?player->maxhealth_old:player->maxhealth);
			player->maxhealth+=(health_table[cl][gameskill]*level);//levelIncrement[cl][(lvl-2) % 5].addhealth;
			player->sp_power_old=(player->sp_power_old!=player->sp_power?player->sp_power_old:player->sp_power);
			player->maxsp_power+=(sppower_table[cl][gameskill]*level);
			player->sp_power=player->maxsp_power;
			player->plr->mo->health = player->health = (int)((double)player->maxhealth*asphealt);
			sprintf(tmpString,TXT_NEXTLEVEL,lvl);
			P_SetYellowMessage(player, tmpString, false);
			S_StartSound(NULL, SFX_CHAT);
	
	if (cl == PCLASS_MAGE)
	{
		if (player->exp_level >= 5)
			player->weaponowned[P_GetWeaponNum(player,WP_MAGE_WAND3)] = true;
		if (player->exp_level >= 3)
			player->weaponowned[P_GetWeaponNum(player,WP_MAGE_WAND2)] = true;
		if (player->exp_level >= 1)
			player->weaponowned[P_GetWeaponNum(player,WP_MAGE_WAND)] = true;
	}
}

void Give_Experience(player_t *player,int experience)
{
	int cl,lvl;double asphealt;
	char tmpString[30];
	cl=player->class;
	if (player->experience>=2100000666) return;
	if ((player->experience+experience)>=2100000666) 
	{
		player->experience=2100000666;
		player->next_level=2100000999;
	} 
	else 
	{
		player->experience+=experience;
		while (player->experience>=player->next_level && (cl<PCLASS_PIG))
		{
			
			
			player->exp_level++;
			lvl=player->exp_level;
			//player->experience-=player->next_level;
			player->next_level=player->next_level*2;//player->prev_level;
			//player->prev_level=player->next_level-player->prev_level;
			asphealt=(double)player->health / (double)player->maxhealth;
			player->maxhealth_old=(player->maxhealth_old!=player->maxhealth?player->maxhealth_old:player->maxhealth);
			player->maxhealth+=health_table[cl][gameskill];//levelIncrement[cl][(lvl-2) % 5].addhealth;
			player->sp_power_old=(player->sp_power_old!=player->sp_power?player->sp_power_old:player->sp_power);
			player->maxsp_power+=sppower_table[cl][gameskill];
			player->sp_power=player->maxsp_power;
			player->plr->mo->health = player->health = (int)((double)player->maxhealth*asphealt);
			sprintf(tmpString,TXT_NEXTLEVEL,lvl);
			P_SetYellowMessage(player, tmpString, false);

			if (cl == PCLASS_MAGE)
			{
				if (player->exp_level == 6)
					player->weaponowned[P_GetWeaponNum(player,WP_MAGE_WAND3)] = true;
				if (player->exp_level == 4)
					player->weaponowned[P_GetWeaponNum(player,WP_MAGE_WAND2)] = true;
				if (player->exp_level == 2)
					player->weaponowned[P_GetWeaponNum(player,WP_MAGE_WAND)] = true;	
			}

			switch (cl) //Messages about new stuff
			{
			case PCLASS_FIGHTER:
				switch (lvl)
				{
				case 5:
					P_SetYellowMessage(player, "YOU CAN NOW USE MANA!", false);
					break;
				}
				break;
			case PCLASS_CLERIC:
				switch (lvl)
				{
				case 3:
					P_SetYellowMessage(player, "YOU CAN NOW USE MANA!", false);
					break;
				case 5:
					P_SetYellowMessage(player, "YOU CAN NOW CAST SPIRITS WITHIN!", false);
					break;
				case 7:
					P_SetYellowMessage(player, "YOU CAN NOW CAST BANISHMENT!", false);
					break;
				case 9:
					P_SetYellowMessage(player, "YOU CAN NOW CAST WRATH OF THE GODS", false);
					break;
				}
				break;
			case PCLASS_MAGE:
				switch (lvl)
				{
				case 2:
					P_SetYellowMessage(player, "YOU NOW HAVE THE BLUE WAND!", false);
					break;
				case 3:
					P_SetYellowMessage(player, "YOU CAN NOW CAST MANA CREATION!", false);
					break;
				case 4:
					P_SetYellowMessage(player, "YOU CAN HAVE THE GREY WAND!", false);
					break;
				case 5:
					P_SetYellowMessage(player, "YOU CAN NOW CAST REPULSION!", false);
					break;
				case 6:
					P_SetYellowMessage(player, "YOU NOW HAVE THE RED WAND!", false);
					break;
				case 7:
					P_SetYellowMessage(player, "YOU CAN NOW CAST SPEED!", false);
					break;
				case 8:
					P_SetYellowMessage(player, "YOU CAN NOW CAST SUMMON MONSTER!", false);
					break;
				case 9:
					P_SetYellowMessage(player, "YOU CAN NOW CAST POSSESSION!", false);
					break;
				case 10:
					P_SetYellowMessage(player, "YOU CAN NOW CAST HORRIBLE PAIN!", false);
					break;
				}
				break;
			}


			S_StartSound(NULL, SFX_CHAT);
			if(!netgame && !demoplayback && !demorecording)
				player->av_points+=skillp_table[gameskill];
			else 
			{
				switch (cl)
				{
				case PCLASS_FIGHTER:
					player->strength+=skillp_table[gameskill];
					break;
				case PCLASS_CLERIC:
					player->strength+=skillp_table[gameskill]/2;
					player->agility+=skillp_table[gameskill]-(skillp_table[gameskill]/2);
					break;
				case PCLASS_MAGE:
					player->agility+=skillp_table[gameskill];
					break;
				}
			}
/*			player->strength+=levelIncrement[cl][(lvl-2) % 5].addstrength;
			player->agility+=levelIncrement[cl][(lvl-2) % 5].addagility;
			player->speed+=levelIncrement[cl][(lvl-2) % 5].addspeed;*/
		}
	}
}


//---------------------------------------------------------------------------
//
// PROC P_KillMobj
//
//---------------------------------------------------------------------------

void P_KillMobj(mobj_t *source, mobj_t *target)
{
	int dummy;
	mobj_t *master;
	
	
	target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY|MF_NOGRAVITY);
	target->flags |= MF_CORPSE|MF_DROPOFF;
	target->flags2 &= ~MF2_PASSMOBJ;
	target->height >>= 2;
	if((target->flags&MF_COUNTKILL || target->type == MT_ZBELL) 
		 && target->special)
	{ // Initiate monster death actions
		if(target->type == MT_SORCBOSS)
		{
			dummy = 0;
			P_StartACS(target->special, 0, (byte *)&dummy, target,
				NULL, 0);
		}
		else
		{
			P_ExecuteLineSpecial(target->special, target->args,
				NULL, 0, target);
		}
	}
	if(source && source->player)
	{ // Check for frag changes
		if(target->player && deathmatch) // make sure it's a deathmatch
		{
			if(target == source)
			{ // Self-frag
				target->player->plr->frags[target->player-players]--;
				
				if(cmdfrag && netgame && source->player == &players[consoleplayer])
				{ // Send out a frag count packet
					//gi.SendFrags(source->player->plr);
				}
			}
			else
			{
				source->player->plr->frags[target->player-players]++;
				source->player->experience+=1000;
				
				if(cmdfrag && netgame && source->player == &players[consoleplayer])
				{ // Send out a frag count packet
					//gi.SendFrags(source->player->plr);
				}
			}
		}
	}
	if(target->player)
	{ // Player death
		if(!source)
		{ // Self-frag
			target->player->plr->frags[target->player-players]--;
			
			if(cmdfrag && netgame && target->player == &players[consoleplayer])
			{ // Send out a frag count packet
				//gi.SendFrags(target->player->plr);
			}
		}
		target->flags &= ~MF_SOLID;
		target->flags2 &= ~MF2_FLY;
		target->player->powers[pw_flight] = 0;
		target->player->playerstate = PST_DEAD;
		P_DropWeapon(target->player);
		if(target->flags2&MF2_FIREDAMAGE)
		{ // Player flame death
			switch(target->player->class)
			{
				case PCLASS_FIGHTER:
					S_StartSound(target, SFX_PLAYER_FIGHTER_BURN_DEATH);
					P_SetMobjState(target, S_PLAY_F_FDTH1);
					return;
				case PCLASS_CLERIC:
					S_StartSound(target, SFX_PLAYER_CLERIC_BURN_DEATH);
					P_SetMobjState(target, S_PLAY_C_FDTH1);
					return;
				case PCLASS_MAGE:
					S_StartSound(target, SFX_PLAYER_MAGE_BURN_DEATH);
					P_SetMobjState(target, S_PLAY_M_FDTH1);
					return;
				default:
					break;
			}
		}
		if(target->flags2&MF2_ICEDAMAGE)
		{ // Player ice death
			target->flags &= ~(7<<MF_TRANSSHIFT); //no translation
			target->flags |= MF_ICECORPSE;
			switch(target->player->class)
			{
				case PCLASS_FIGHTER:
					P_SetMobjState(target, S_FPLAY_ICE);
					return;
				case PCLASS_CLERIC:
					P_SetMobjState(target, S_CPLAY_ICE);
					return;
				case PCLASS_MAGE:
					P_SetMobjState(target, S_MPLAY_ICE);
					return;
				case PCLASS_PIG:
					P_SetMobjState(target, S_PIG_ICE);
					return;
				default:
					break;
			}
		}
	}
    else {
        // target is some monster or an object
		// -JL- Give experience also if source is a summoned monster
        if (source && (source->player || (source->flags3 & MF3_FRIENDLY)) && !(target->flags3&MF3_FRIENDLY))
		{
			if (netgame)
			{
				int i,numPl=0;
				for (i=0; i<MAXPLAYERS; i++)
					if (players[i].plr->ingame) numPl++;
				for (i=0; i<MAXPLAYERS; i++)
					if (players[i].plr->ingame)
						// -JL- must use P_Random in gameplay code
						Give_Experience(&players[i],(target->experience*((90+(double)P_Random()/255*21)/100)*(gameskill<2?125:(gameskill<3?100:80))/100)/numPl);
			} 
			else
			{
				player_t *p;

				p = source->player;
				if (!p)
				{
					// -JL- Sumoned monster kills give experience to console player
					p = &players[consoleplayer];
				}
				// -JL- must use P_Random in gameplay code
				Give_Experience(p,target->experience*((90+(double)P_Random()/255*21)/100)*(gameskill<2?125:(gameskill<3?100:80))/100);
			}
		}
		else
		{
            if (source && !(source->flags3&MF3_FRIENDLY && target->flags3&MF3_FRIENDLY))
				// -JL- must use P_Random in gameplay code
				source->experience+=target->experience*((90+(double)P_Random()/255*21)/100);  //RPG
		}
        // mobj death, record as player's kill in netgame + coop
		// -JL- There is a flag MF_COUNTKILL indicating a monster
		if(netgame && !deathmatch && source && source->player && source->player->plr &&
			!(target->flags3&MF3_FRIENDLY) && (target->flags & MF_COUNTKILL)) 
		{
            source->player->plr->frags[0]++;
        }
    }
        
	if(target->flags2&MF2_FIREDAMAGE)
	{
		if(target->type == MT_FIGHTER_BOSS 
			|| target->type == MT_CLERIC_BOSS
			|| target->type == MT_MAGE_BOSS)
		{
 			switch(target->type)
			{
				case MT_FIGHTER_BOSS:
					S_StartSound(target, SFX_PLAYER_FIGHTER_BURN_DEATH);
					P_SetMobjState(target, S_PLAY_F_FDTH1);
					return;
				case MT_CLERIC_BOSS:
					S_StartSound(target, SFX_PLAYER_CLERIC_BURN_DEATH);
					P_SetMobjState(target, S_PLAY_C_FDTH1);
					return;
				case MT_MAGE_BOSS:
					S_StartSound(target, SFX_PLAYER_MAGE_BURN_DEATH);
					P_SetMobjState(target, S_PLAY_M_FDTH1);
					return;
				default:
					break;
			}
		}
		else if(target->type == MT_TREEDESTRUCTIBLE)
		{
			P_SetMobjState(target, S_ZTREEDES_X1);
			target->height = 24*FRACUNIT;
			S_StartSound(target, SFX_TREE_EXPLODE);
			return;
		}
	}
	if(target->flags2&MF2_ICEDAMAGE)
	{
		target->flags |= MF_ICECORPSE;
		switch(target->type)
		{
			case MT_BISHOP:
			case MT_POSS_BISHOP:
				P_SetMobjState(target, S_BISHOP_ICE);
				return;		
			case MT_CENTAUR:
			case MT_CENTAURLEADER:
			case MT_POSS_CENTAUR:
			case MT_POSS_CENTAURL:
				P_SetMobjState(target, S_CENTAUR_ICE);
				return;		
			case MT_DEMON:
			case MT_DEMON2:
			case MT_POSS_DEMON:
			case MT_POSS_DEMON2:
				P_SetMobjState(target, S_DEMON_ICE);
				return;		
			case MT_SERPENT:
			case MT_SERPENTLEADER:
				P_SetMobjState(target, S_SERPENT_ICE);
				return;		
			case MT_WRAITH:
			case MT_WRAITHB:
			case MT_POSS_WRAITH:
				P_SetMobjState(target, S_WRAITH_ICE);
				return;
			case MT_ETTIN:
			case MT_POSS_ETTIN:
				P_SetMobjState(target, S_ETTIN_ICE1);
				return;
			case MT_FIREDEMON:
			case MT_POSS_AFRIT:
				P_SetMobjState(target, S_FIRED_ICE1);
				return;
			case MT_FIGHTER_BOSS:
				P_SetMobjState(target, S_FIGHTER_ICE);
				return;
			case MT_CLERIC_BOSS:
				P_SetMobjState(target, S_CLERIC_ICE);
				return;
			case MT_MAGE_BOSS:
				P_SetMobjState(target, S_MAGE_ICE);
				return;
			case MT_PIG:
				P_SetMobjState(target, S_PIG_ICE);
				return;
			case MT_DUMMY_FIGHTER:
				P_SetMobjState(target, S_FPLAY_ICE);
				return;
			case MT_DUMMY_CLERIC:
				P_SetMobjState(target, S_CPLAY_ICE);
				return;
			case MT_DUMMY_MAGE:
				P_SetMobjState(target, S_MPLAY_ICE);
				return;
			default:
				target->flags &= ~MF_ICECORPSE;
				break;
		}
	}

	if(target->type == MT_MINOTAUR)
	{
		master = (mobj_t *)target->special1;
		if(master->health > 0)
		{
			if (!ActiveMinotaur(master->player))
			{
				master->player->powers[pw_minotaur] = 0;
			}
		}		
	}
	else if(target->type == MT_TREEDESTRUCTIBLE)
	{
		target->height = 24*FRACUNIT;
	}
	if(target->health < -(target->info->spawnhealth>>1)
		&& target->info->xdeathstate)
	{ // Extreme death
		P_SetMobjState(target, target->info->xdeathstate);
	}
	else
	{ // Normal death
		if ((target->type==MT_FIREDEMON) &&
			(target->z <= target->floorz + 2*FRACUNIT) &&
			(target->info->xdeathstate))
		{
			// This is to fix the imps' staying in fall state
			P_SetMobjState(target, target->info->xdeathstate);
		}
		else
		{
			P_SetMobjState(target, target->info->deathstate);
		}
	}
	target->tics -= P_Random()&3;
//	I_StartSound(&actor->r, actor->info->deathsound);
}

//---------------------------------------------------------------------------
//
// FUNC P_MinotaurSlam
//
//---------------------------------------------------------------------------

void P_MinotaurSlam(mobj_t *source, mobj_t *target)
{
	angle_t angle;
	fixed_t thrust;

	angle = R_PointToAngle2(source->x, source->y, target->x, target->y);
	angle >>= ANGLETOFINESHIFT;
	thrust = 16*FRACUNIT+(P_Random()<<10);
	target->momx += FixedMul(thrust, finecosine[angle]);
	target->momy += FixedMul(thrust, finesine[angle]);
	P_DamageMobj(target, NULL, source, HITDICE(4));
	if(target->player)
	{
		target->reactiontime = 14+(P_Random()&7);
	}
	source->args[0] = 0;			// Stop charging
}

//---------------------------------------------------------------------------
//
// FUNC P_PossessMonster
//
// Returns true if the player turns into the monster
//
//---------------------------------------------------------------------------

boolean P_PossessMonster(player_t *player, mobj_t *actor)
{
	mobj_t oldMonster;
	mobj_t *pmo; //player, and dummy
	mobj_t *beastMo;
    mobjtype_t moType;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	int health;
	angle_t angle;
	char cmd[20];
	
	if (actor == NULL)
		return false;

	// -JL- Don't try to possess a dead body
	if (actor->health <= 0)
		return false;

	if (actor->flags&MF_ICECORPSE)
		return false;

	moType = actor->type;
	switch(moType)
	{
		case MT_ETTIN:
			player->class = PCLASS_ETTIN;
			break;
		case MT_FIREDEMON: //Afrit
			player->class = PCLASS_AFRIT;
			break;
		case MT_CENTAUR:
			player->class = PCLASS_CENTAUR;
			break;
		case MT_CENTAURLEADER:
			player->class = PCLASS_CENTAURL;
			break;
		case MT_ICEGUY:
			player->class = PCLASS_ICEGUY;
			break;
		case MT_DEMON:
			player->class = PCLASS_SERPENT;
			break;
		case MT_DEMON2:
			player->class = PCLASS_BSERPENT;
			break;
		// -JL- Added buried wraiths
		case MT_WRAITHB:
			if (!actor->flags & MF_SHOOTABLE)
			{
				// Still buried
				return false;
			}
		case MT_WRAITH:
			player->class = PCLASS_REIVER;
			break;
		case MT_BISHOP:
			player->class = PCLASS_BISHOP;
			break;
		default:
			return(false);
	}
	
	oldMonster = *actor; //Monster being possessed
	x = oldMonster.x;
	y = oldMonster.y;
	z = oldMonster.z;

	pmo = player->plr->mo; //The old player actor    

	P_RemoveMobjFromTIDList(actor);
	P_SetMobjState(actor, S_FREETARGMOBJ);
	
	switch(player->class)
	{
		case PCLASS_ETTIN:
			beastMo = P_SpawnMobj(x, y, z, MT_POSS_ETTIN);
			break;
		case PCLASS_AFRIT:
			beastMo = P_SpawnMobj(x, y, z, MT_POSS_AFRIT);
			break;
		case PCLASS_CENTAUR:
			beastMo = P_SpawnMobj(x, y, z, MT_POSS_CENTAUR);
			break;
		case PCLASS_CENTAURL:
			beastMo = P_SpawnMobj(x, y, z, MT_POSS_CENTAURL);
			break;
		case PCLASS_ICEGUY:
			beastMo = P_SpawnMobj(x, y, z, MT_POSS_ICEGUY);
			break;
		case PCLASS_SERPENT:
			beastMo = P_SpawnMobj(x, y, z, MT_POSS_DEMON);
			break;
		case PCLASS_BSERPENT:
			beastMo = P_SpawnMobj(x, y, z, MT_POSS_DEMON2);
			break;
		case PCLASS_REIVER:
			beastMo = P_SpawnMobj(x, y, z, MT_POSS_WRAITH);
			break;
		case PCLASS_BISHOP:
			beastMo = P_SpawnMobj(x, y, z, MT_POSS_BISHOP);
			break;
		default: //Should never happen
			gi.Error("P_PossessMonster: Unknown possessed monster class type %d\n",player->class);
	}	

	health = pmo->health;
	x = pmo->x;
    y = pmo->y;
	z = pmo->z;
	angle = pmo->angle;
	P_SetMobjState(pmo, S_FREETARGMOBJ); //Gone with the old player
    switch(PlayerClass[P_GetPlayerNum(player)]) //Spawns the player dummy
	{
		case PCLASS_FIGHTER:
			pmo = P_SpawnMobj(x, y, z, MT_DUMMY_FIGHTER);
			break;
		case PCLASS_CLERIC:
			pmo = P_SpawnMobj(x, y, z, MT_DUMMY_CLERIC);
			break;
		case PCLASS_MAGE:
			pmo = P_SpawnMobj(x, y, z, MT_DUMMY_MAGE);
			break;
	}

	pmo->health = health;
	pmo->angle = angle;
	pmo->flags3 |= MF3_FRIENDLY;
   	
	beastMo->special2 = player->readyweapon; //Used for pendingweapon when becoming normal
	beastMo->angle = oldMonster.angle;
	beastMo->player = player;
	player->health = beastMo->health = oldMonster.health;
	player->plr->mo = beastMo;
	// -JL- Must also remember special and TID
	beastMo->special = oldMonster.special;
	beastMo->args[0] = oldMonster.args[0];
	beastMo->args[1] = oldMonster.args[1];
	beastMo->args[2] = oldMonster.args[2];
	beastMo->args[3] = oldMonster.args[3];
	beastMo->args[4] = oldMonster.args[4];
	beastMo->tid = oldMonster.tid;
	P_InsertMobjIntoTIDList(beastMo, beastMo->tid);
	
	pmo->special1 = (int)(beastMo); //pointer to master
	
	//memset(&player->armorpoints[0], 0, NUMARMOR*sizeof(int));
	P_ActivateMorphWeapon(player);
	player->damagecount += 50; //As graphical effect
	setsizeneeded = true; //Possessed monsters turn big screen
	sprintf(cmd, "r_FOV %d", 20);
	gi.Execute(cmd, true);
	player->possTics = 10; //FOV effect
	P_GiveArtifact(player, arti_summon, NULL); //Just so the possessed monster always has one item
	return(true);
}

//---------------------------------------------------------------------------
//
// FUNC P_UndoPossessMonster      
//
//---------------------------------------------------------------------------

static mobj_t *UnpossessedMobj;

void P_UndoPossessMonster(mobj_t *actor, player_t *player)
{
	mobj_t *olddummy;
	mobj_t *mo;
	mobjtype_t moType;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	int health;
	int playerNum;
	angle_t angle;
	weapontype_t weapon;
	int i;
	char cmd[20];
	// -JL- Remember special and TID
	byte special;
	byte args[5];
	int TID;

	olddummy = ActivePlayerDummy(player);

	x = olddummy->x;
	y = olddummy->y;
	z = olddummy->z;
	health = olddummy->health;
	angle = olddummy->angle;
	weapon = actor->special2;
	P_SetMobjState(olddummy, S_FREETARGMOBJ);		
	
	playerNum = P_GetPlayerNum(player);
	switch(PlayerClass[playerNum])
	{
		case PCLASS_FIGHTER:
			mo = P_SpawnMobj(x, y, z, MT_PLAYER_FIGHTER);
			break;
		case PCLASS_CLERIC:
			mo = P_SpawnMobj(x, y, z, MT_PLAYER_CLERIC);
			break;
		case PCLASS_MAGE:
			mo = P_SpawnMobj(x, y, z, MT_PLAYER_MAGE);
			break;
		default:
			gi.Error("P_UndoPossessMonster:  Unknown player class %d\n", 
				player->class);
	}

	mo->angle = angle;
	mo->player = player;
	actor->player = NULL; //Monster no player anymore
	mo->reactiontime = 18;
	player->health = mo->health = health;	
	player->plr->mo = mo;
	player->class = PlayerClass[playerNum];
	angle >>= ANGLETOFINESHIFT;
		
	if (actor->health >= 1) //If the player willingly departed from the monster, let him live again
	{
		x = actor->x;
		y = actor->y;
		z = actor->z;
		health = actor->health;
		angle = actor->angle;
		moType = actor->type;
		// -JL- Remember special and TID
		special = actor->special;
		memcpy(args, actor->args, 5);
		TID = actor->tid;
		P_RemoveMobjFromTIDList(actor);
		P_SetMobjState(actor, S_FREETARGMOBJ);
		switch(moType)
		{
		case MT_POSS_ETTIN:
			mo = P_SpawnMobj(x, y, z, MT_ETTIN);
			break;
		case MT_POSS_AFRIT:
			mo = P_SpawnMobj(x, y, z, MT_FIREDEMON);
			mo->flags2 &= ~MF2_INVULNERABLE;
			break;
		case MT_POSS_CENTAUR:
			mo = P_SpawnMobj(x, y, z, MT_CENTAUR);
			break;
		case MT_POSS_CENTAURL:
			mo = P_SpawnMobj(x, y, z, MT_CENTAURLEADER);
			break;
		case MT_POSS_ICEGUY:
			mo = P_SpawnMobj(x, y, z, MT_ICEGUY);
			break;
		case MT_POSS_DEMON:
			mo = P_SpawnMobj(x, y, z, MT_DEMON);
			break;
		case MT_POSS_DEMON2:
			mo = P_SpawnMobj(x, y, z, MT_DEMON2);
			break;
		case MT_POSS_WRAITH:
			mo = P_SpawnMobj(x, y, z, MT_WRAITH);
			break;
		case MT_POSS_BISHOP:
			mo = P_SpawnMobj(x, y, z, MT_BISHOP);
			break;
		default: //Should never happen
			gi.Error("P_UndoPossessMonster: Unknown possessed monster type %d\n",moType);
		}
		mo->angle = angle;
		mo->health = health;
		mo->reactiontime = 0;
		mo->target = player->plr->mo; //Payback time, target the player who possessed you
		P_SetMobjState(mo, mobjinfo[mo->type].painstate);
		// -JL- Restore special and TID
		mo->special = special;
		memcpy(mo->args, args, 5);
		mo->tid = TID;
		P_InsertMobjIntoTIDList(mo, TID);
		// -JL- Remember new mobj in a case it's immediately is morphed into a pig
		UnpossessedMobj = mo;
	}
	sprintf(cmd, "r_FOV %d", 160);
	gi.Execute(cmd, true);
	player->possTics = 10; //FOV effect
	setsizeneeded = true; //Possessed monsters turn from big screen
	player->damagecount += 50;
	for(i = 0; i < player->inventorySlotNum; i++) //The player ain't going to keep the summon item
	{
		if(player->inventory[i].type == arti_summon)
		{
			P_PlayerRemoveArtifact(player, i);
			break;
		}
	}
	P_PostMorphWeapon(player, weapon);	
}

//---------------------------------------------------------------------------
//
// FUNC P_MorphPlayer
//
// Returns true if the player gets turned into a pig
//
//---------------------------------------------------------------------------

boolean P_MorphPlayer(player_t *player)
{
	mobj_t *pmo;
	mobj_t *fog;
	mobj_t *beastMo;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	angle_t angle;
	int oldFlags2;

	if(player->powers[pw_invulnerability])
	{ // Immune when invulnerable
		return(false);
	}
	if(player->morphTics)
	{ // Player is already a beast
		return false;
	}
	
	if(player->berserkTics)
	{
		// -JL- Use P_UndoBerserk
		P_UndoBerserk(player);
	}
	
	pmo = player->plr->mo;
	x = pmo->x;
	y = pmo->y;
	z = pmo->z;
	angle = pmo->angle;
	oldFlags2 = pmo->flags2;
	P_SetMobjState(pmo, S_FREETARGMOBJ);
	fog = P_SpawnMobj(x, y, z+TELEFOGHEIGHT, MT_TFOG);
	S_StartSound(fog, SFX_TELEPORT);
	beastMo = P_SpawnMobj(x, y, z, MT_PIGPLAYER);
	beastMo->special1 = player->readyweapon;
	beastMo->angle = angle;
	beastMo->player = player;
	player->health = beastMo->health = MAXMORPHHEALTH;
	player->plr->mo = beastMo;
	memset(&player->armorpoints[0], 0, NUMARMOR*sizeof(int));
	player->class = PCLASS_PIG;
	if(oldFlags2&MF2_FLY)
	{
		beastMo->flags2 |= MF2_FLY;
	}
	player->morphTics = MORPHTICS;
	P_ActivateMorphWeapon(player);
	return(true);
}

//---------------------------------------------------------------------------
//
// FUNC P_MorphMonster
//
//---------------------------------------------------------------------------

boolean P_MorphMonster(mobj_t *actor)
{
	mobj_t *master, *monster, *fog;
	mobjtype_t moType;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	mobj_t oldMonster;

	if(actor->player) return(false);
	if(!(actor->flags&MF_COUNTKILL)) return false;
	if(actor->flags2&MF2_BOSS) return false;
	moType = actor->type;
	switch(moType)
	{
		case MT_PIG:
			return(false);
		case MT_FIGHTER_BOSS:
		case MT_CLERIC_BOSS:
		case MT_MAGE_BOSS:
			return(false);
		default:
			break;
	}

	oldMonster = *actor;
	x = oldMonster.x;
	y = oldMonster.y;
	z = oldMonster.z;
	P_RemoveMobjFromTIDList(actor);
	P_SetMobjState(actor, S_FREETARGMOBJ);
	fog = P_SpawnMobj(x, y, z+TELEFOGHEIGHT, MT_TFOG);
	S_StartSound(fog, SFX_TELEPORT);
	monster = P_SpawnMobj(x, y, z, MT_PIG);
	monster->special2 = moType;
	monster->special1 = MORPHTICS+P_Random();
	monster->flags |= (oldMonster.flags&MF_SHADOW);
	monster->target = oldMonster.target;
	monster->angle = oldMonster.angle;
	monster->tid = oldMonster.tid;
	monster->special = oldMonster.special;
	P_InsertMobjIntoTIDList(monster, oldMonster.tid);
	memcpy(monster->args, oldMonster.args, 5);

	// check for turning off minotaur power for active icon
	if (moType==MT_MINOTAUR)
	{
		master = (mobj_t *)oldMonster.special1;
		if(master->health > 0)
		{
			if (!ActiveMinotaur(master->player))
			{
				master->player->powers[pw_minotaur] = 0;
			}
		}		
	}
	return(true);
}

//---------------------------------------------------------------------------
//
// PROC P_AutoUseHealth
//
//---------------------------------------------------------------------------

void P_AutoUseHealth(player_t *player, int saveHealth)
{
	int i;
	int count;
	int normalCount;
	int normalSlot=0;
	int superCount;
	int superSlot=0;

	normalCount = superCount = 0;
	for(i = 0; i < player->inventorySlotNum; i++)
	{
		if(player->inventory[i].type == arti_health)
		{
			normalSlot = i;
			normalCount = player->inventory[i].count;
		}
		else if(player->inventory[i].type == arti_superhealth)
		{
			superSlot = i;
			superCount = player->inventory[i].count;
		}
	}
	if((gameskill == sk_baby) && (normalCount*25 >= saveHealth))
	{ // Use quartz flasks
		count = (saveHealth+24)/25;
		for(i = 0; i < count; i++)
		{
			player->health += 25;
			P_PlayerRemoveArtifact(player, normalSlot);
		}
	}
	else if(superCount*100 >= saveHealth)
	{ // Use mystic urns
		count = (saveHealth+99)/100;
		for(i = 0; i < count; i++)
		{
			player->health += 100;
			P_PlayerRemoveArtifact(player, superSlot);
		}
	}
	else if((gameskill == sk_baby)
		&& (superCount*100+normalCount*25 >= saveHealth))
	{ // Use mystic urns and quartz flasks
		count = (saveHealth+24)/25;
		saveHealth -= count*25;
		for(i = 0; i < count; i++)
		{
			player->health += 25;
			P_PlayerRemoveArtifact(player, normalSlot);
		}
		count = (saveHealth+99)/100;
		for(i = 0; i < count; i++)
		{
			player->health += 100;
			P_PlayerRemoveArtifact(player, normalSlot);
		}
	}
	player->plr->mo->health = player->health;
}

/*
=================
=
= P_DamageMobj
=
= Damages both enemies and players
= inflictor is the thing that caused the damage
= 		creature or missile, can be NULL (slime, etc)
= source is the thing to target after taking damage
=		creature or NULL
= Source and inflictor are the same for melee attacks
= source can be null for barrel explosions and other environmental stuff
==================
*/

void P_DamageMobj
(
	mobj_t *target,
	mobj_t *inflictor,
	mobj_t *source,
	int	damageP
)
{
	unsigned ang;
	int saved;
	fixed_t savedPercent;
	player_t *player;
	mobj_t *master;
	fixed_t thrust;
	int temp;
	int i;

    // the actual damage (== damageP * netMobDamageModifier for 
    // any non-player mob)
    int damage = damageP; 

	if(!(target->flags&MF_SHOOTABLE))
	{
		// Shouldn't happen
		return;
	}

    // use the cvar damage multiplier netMobDamageModifier
    // only if the inflictor is not a player 
    if(inflictor && !inflictor->player 
       && inflictor->type != MT_PLAYER_FIGHTER
       && inflictor->type != MT_PLAYER_MAGE
       && inflictor->type != MT_PLAYER_CLERIC
       && (!source 
           || (source && !source->player 
                && source->type != MT_PLAYER_FIGHTER
                && source->type != MT_PLAYER_MAGE
                && source->type != MT_PLAYER_CLERIC
              ) 
          ) 
       ) { 
        
        // means inflictor->type == MT_PLAYER(CLASS)
        //damage = (int) ((float) damage * netMobDamageModifier);
		damage *= netMobDamageModifier;
    }

	if(target->health <= 0)
	{
		if (inflictor && inflictor->flags2&MF2_ICEDAMAGE)
		{
			return;
		}
		else if (target->flags&MF_ICECORPSE) // frozen
		{
			target->tics = 1;
			target->momx = target->momy = 0;
		}
		return;
	}
	if ((target->flags2&MF2_INVULNERABLE) && damage < 10000)
	{ // mobj is invulnerable
		if(target->player) return;	// for player, no exceptions
		if(inflictor)
		{
			switch(inflictor->type)
			{
				// These inflictors aren't foiled by invulnerability
				case MT_HOLY_FX:
				case MT_POISONCLOUD:
				case MT_FIREBOMB:
					break;
				default:
					return;
			}
		}
		else
		{
			return;
		}
	}
	if(target->player)
	{
		if(damage < 1000 && ((target->player->cheats&CF_GODMODE)
			|| target->player->powers[pw_invulnerability]))
		{
			return;
		}
	}
	if(target->flags&MF_SKULLFLY)
	{
		target->momx = target->momy = target->momz = 0;
	}
	if(target->flags2&MF2_DORMANT)
	{
		// Invulnerable, and won't wake up
		return;
	}
	player = target->player;
	if(player && gameskill == sk_baby)
	{
		// Take half damage in trainer mode
		damage >>= 1;
	}
	// Special damage types
	if(inflictor)
	{
		switch(inflictor->type)
		{
			case MT_EGGFX:
				if (player && player->class >= PCLASS_ETTIN)
				{
					// -JL- Undo possession when monster gets morphed
					P_UndoPossessMonster(target, player);
					P_MorphMonster(UnpossessedMobj);
				}
				else if (target->type == MT_DUMMY_FIGHTER ||
					target->type == MT_DUMMY_CLERIC ||
					target->type == MT_DUMMY_MAGE)
				{
					// -JL- Player itself gets morphed while possessing, undo possession
					mobj_t *possessedMo;

					possessedMo = (mobj_t *)target->special1;
					player = possessedMo->player;
					P_UndoPossessMonster(possessedMo, player);
					P_MorphPlayer(player);
				}
				else if(player)
				{
					P_MorphPlayer(player);
				}
				else
				{
					P_MorphMonster(target);
				}
				return; // Always return
			case MT_POSS_PROJECTILE: //Possession of creatures
				P_PossessMonster(inflictor->target->player, target);
				return;
			case MT_TELOTHER_FX1:
			case MT_TELOTHER_FX2:
			case MT_TELOTHER_FX3:
			case MT_TELOTHER_FX4:
			case MT_TELOTHER_FX5:
				if ((target->flags&MF_COUNTKILL) &&
					(target->type != MT_SERPENT) &&
					(target->type != MT_MAGE_BOSS) &&
					(target->type != MT_FIGHTER_BOSS) &&
					(target->type != MT_CLERIC_BOSS) &&
					(target->type != MT_SERPENTLEADER) &&
					(!(target->flags2 & MF2_BOSS)))
				{
					if (inflictor->experience==666) P_DamageMobj(target,inflictor->target,inflictor->target, inflictor->special2*5000);
					else
					{
						mobj_t *fog;
						int x = target->x;
						int y = target->y;
						int z = target->z;
//						P_UnsetThingPosition (target);

/*						target->floorz = -16000*FRACUNIT;
						target->ceilingz = -16000*FRACUNIT;
						target->x = target->x;
						target->y = target->y;*/
						target->x=-16000*FRACUNIT;
						target->y=-16000*FRACUNIT;

//						P_SetThingPosition (target);
						fog = P_SpawnMobj(x, y, z+TELEFOGHEIGHT, MT_TFOG);
						S_StartSound(fog, SFX_TELEPORT);
						P_DamageMobj(target,inflictor->target,inflictor->target,target->health+1);
					}
				}
				if ((target->flags&MF_COUNTKILL) && (
					(target->type == MT_SERPENT) ||
					(target->type == MT_SERPENTLEADER) ||
					(target->type == MT_MAGE_BOSS) ||
					(target->type == MT_FIGHTER_BOSS) ||
					(target->type == MT_CLERIC_BOSS) ||
					(target->flags2 & MF2_BOSS)) && inflictor->experience==666) P_DamageMobj(target,inflictor->target,inflictor->target,5000);
				return;
			case MT_MINOTAUR:
				if(inflictor->flags&MF_SKULLFLY)
				{ // Slam only when in charge mode
					P_MinotaurSlam(inflictor, target);
					return;
				}
				break;
			case MT_BISH_FX:
				// Bishops are just too nasty
				damage >>= 1;
				break;
			case MT_SHARDFX1:
				switch(inflictor->special2)
				{
					case 3:
						damage <<= 3;
						break;
					case 2:
						damage <<= 2;
						break;
					case 1:
						damage <<= 1;
						break;
					default:
						break;
				}
				break;
			case MT_CSTAFF_MISSILE:
				// Cleric Serpent Staff does poison damage
				if(target->player)
				{
					P_PoisonPlayer(target->player, source, 20);
					damage >>= 1;
				}
				break;
			case MT_ICEGUY_FX2:
				damage >>= 1;
				break;
			case MT_POISONDART:
				if(target->player)
				{
					P_PoisonPlayer(target->player, source, 20);
					damage >>= 1;
				}
				break;
			case MT_POISONCLOUD:
				if(target->player)
				{
					if(target->player->poisoncount < 4)
					{
						P_PoisonDamage(target->player, source,
							15+(P_Random()&15), false); // Don't play painsound
						P_PoisonPlayer(target->player, source, 50);
						S_StartSound(target, SFX_PLAYER_POISONCOUGH);
					}	
					return;
				}
				else if(!(target->flags&MF_COUNTKILL))
				{ // only damage monsters/players with the poison cloud
					return;
				}
				break;
			case MT_FSWORD_MISSILE:
				if(target->player)
				{
					damage -= damage>>2;
				}
				break;
			default:
				break;
		}
	}
	// Push the target unless source is using the gauntlets
	if(inflictor && (!source || !source->player)
		&& !(inflictor->flags2&MF2_NODMGTHRUST))
	{
		ang = R_PointToAngle2(inflictor->x, inflictor->y,
			target->x, target->y);
		//thrust = damage*(FRACUNIT>>3)*100/target->info->mass;
		thrust = damage*(FRACUNIT>>3)*150/target->info->mass;
		// make fall forwards sometimes
		if((damage < 40) && (damage > target->health)
			&& (target->z-inflictor->z > 64*FRACUNIT) && (P_Random()&1))
		{
			ang += ANG180;
			thrust *= 4;
		}
		ang >>= ANGLETOFINESHIFT;
		target->momx += FixedMul(thrust, finecosine[ang]);
		target->momy += FixedMul(thrust, finesine[ang]);
	}

	//
	// player specific
	//
	if(player)
	{
		savedPercent = AutoArmorSave[player->class]
			+player->armorpoints[ARMOR_ARMOR]+player->armorpoints[ARMOR_SHIELD]
			+player->armorpoints[ARMOR_HELMET]
			+player->armorpoints[ARMOR_AMULET];
		if(savedPercent && (player->class == PCLASS_CLERIC 
			|| player->class == PCLASS_FIGHTER 
			|| player->class == PCLASS_MAGE)) //Remi
		{ // armor absorbed some damage
			if(savedPercent > 100*FRACUNIT)
			{
				savedPercent = 100*FRACUNIT;
			}
			for(i = 0; i < NUMARMOR; i++)
			{
				if(player->armorpoints[i])
				{
					player->armorpoints[i] -= 
						FixedDiv(FixedMul(damage<<FRACBITS,
						ArmorIncrement[player->class][i]), 300*FRACUNIT);
					if(player->armorpoints[i] < 2*FRACUNIT)
					{
						player->armorpoints[i] = 0;
					}
				}
			}
			saved = FixedDiv(FixedMul(damage<<FRACBITS, savedPercent),
				100*FRACUNIT);
			if(saved > savedPercent*2)
			{	
				saved = savedPercent*2;
			}
			damage -= saved>>FRACBITS;
		}
		if(damage >= player->health
			&& ((gameskill == sk_baby) || deathmatch)
			&& !player->morphTics && 
			(player->class == PCLASS_CLERIC 
			|| player->class == PCLASS_FIGHTER 
			|| player->class == PCLASS_MAGE)) //Remi
		{ // Try to use some inventory health
			P_AutoUseHealth(player, damage-player->health+1);
		}
		player->health -= damage; // mirror mobj health here for Dave
		if(player->health < 0)
		{
			player->health = 0;
		}
		player->attacker = source;
		player->damagecount += damage; // add damage after armor / invuln
		if(player->damagecount > 100)
		{
			player->damagecount = 100; // teleport stomp does 10k points...
		}
		temp = damage < 100 ? damage : 100;
		if(player == &players[consoleplayer])
		{
//			I_Tactile(40, 10, 40+temp*2);
			SB_PaletteFlash(false);
		}
	}

	//
	// do the damage
	//
	target->health -= damage;
	if(target->health <= 0)
	{ // Death
		if (target->type == MT_DUMMY_FIGHTER //If dummy dies, so does player
			|| target->type == MT_DUMMY_CLERIC
			|| target->type == MT_DUMMY_MAGE)
		{
			master = (mobj_t *)(target->special1);
			if (master->player)
			{
				master->player->health = master->health = 0;
				P_KillMobj(source, master);
			}
		}			
		if (target->player && player->class >= PCLASS_ETTIN) //Possession death
		{
			P_UndoPossessMonster(target, player);
		}
		if(inflictor)
		{ // check for special fire damage or ice damage deaths
			if(inflictor->flags2&MF2_FIREDAMAGE)
			{
				if(player && !player->morphTics)
				{ // Check for flame death
					if(target->health > -50 && damage > 25)
					{
						target->flags2 |= MF2_FIREDAMAGE;
					}
				}
				else
				{
					target->flags2 |= MF2_FIREDAMAGE;
				}
			}
			else if(inflictor->flags2&MF2_ICEDAMAGE)
			{
				target->flags2 |= MF2_ICEDAMAGE;
			}
		}
		if(source && (source->type == MT_MINOTAUR))
		{ // Minotaur's kills go to his master
			master = (mobj_t *)(source->special1);
			// Make sure still alive and not a pointer to fighter head
			if (master->player && (master->player->plr->mo == master))
			{
				source = master;
			}
		}
		if((source && (source->player) &&
			(source->player->readyweapon == WP_FIGHTER_SWORD
			|| source->player->readyweapon == WP_CLERIC_HOLY
			|| source->player->readyweapon == WP_MAGE_STAFF))
			|| (inflictor && (inflictor->type == MT_HOLY_FX)))
		{
			// Always extreme death from fourth weapon or spirits
			target->health = -5000;
		}
		P_KillMobj(source, target);
		return;
	}
	if((P_Random() < target->info->painchance)
		&& !(target->flags&MF_SKULLFLY))
	{
		if(inflictor && (inflictor->type >= MT_LIGHTNING_FLOOR
			&& inflictor->type <= MT_LIGHTNING_ZAP))
		{
			if(P_Random() < 96)
			{
				target->flags |= MF_JUSTHIT; // fight back!
				P_SetMobjState(target, target->info->painstate);
			}
			else
			{ // "electrocute" the target
				target->frame |= FF_FULLBRIGHT;
				if(target->flags&MF_COUNTKILL && P_Random() < 128
				&& !S_GetSoundPlayingInfo(target, SFX_PUPPYBEAT))
				{
					if ((target->type == MT_CENTAUR) ||
						(target->type == MT_CENTAURLEADER) ||
						(target->type == MT_ETTIN))
					{
						S_StartSound(target, SFX_PUPPYBEAT);
					}
				}
			}
		}
		else
		{
			target->flags |= MF_JUSTHIT; // fight back!
			P_SetMobjState(target, target->info->painstate);	
			if(inflictor && inflictor->type == MT_POISONCLOUD)
			{
				if(target->flags&MF_COUNTKILL && P_Random() < 128
				&& !S_GetSoundPlayingInfo(target, SFX_PUPPYBEAT))
				{
					if ((target->type == MT_CENTAUR) ||
						(target->type == MT_CENTAURLEADER) ||
						(target->type == MT_ETTIN))
					{
						S_StartSound(target, SFX_PUPPYBEAT);
					}
				}
			}
		}
	}
	target->reactiontime = 0; // we're awake now...
	if(!target->threshold && source && !(source->flags2&MF2_BOSS)
		&& !(target->type == MT_BISHOP) && !(target->type == MT_MINOTAUR))
	{
		// Target actor is not intent on another actor,
		// so make him chase after source
		if((target->type == MT_CENTAUR && source->type == MT_CENTAURLEADER)
			|| (target->type == MT_CENTAURLEADER 
				&& source->type == MT_CENTAUR))
		{
			return;
		}
		target->target = source;
		target->threshold = BASETHRESHOLD;
		if(target->state == &states[target->info->spawnstate]
			&& target->info->seestate != S_NULL)
		{
			P_SetMobjState(target, target->info->seestate);
		}
	}
}

//==========================================================================
//
// P_FallingDamage
//
//==========================================================================

void P_FallingDamage(player_t *player)
{
	int damage;
	int mom;
	int dist;

	mom = abs(player->plr->mo->momz);	
	dist = FixedMul(mom, 16*FRACUNIT/23);

	if(mom >= 63*FRACUNIT)
	{ // automatic death
		P_DamageMobj(player->plr->mo, NULL, NULL, 10000);
		return;
	}
	damage = ((FixedMul(dist, dist)/10)>>FRACBITS)-24;
	if(player->plr->mo->momz > -39*FRACUNIT && damage > player->plr->mo->health
		&& player->plr->mo->health != 1)
	{ // No-death threshold
		damage = player->plr->mo->health-1;
	}
	S_StartSound(player->plr->mo, SFX_PLAYER_LAND);
	P_DamageMobj(player->plr->mo, NULL, NULL, damage);
}

//==========================================================================
//
// P_PoisonPlayer - Sets up all data concerning poisoning
//
//==========================================================================

void P_PoisonPlayer(player_t *player, mobj_t *poisoner, int poison)
{
	if((player->cheats&CF_GODMODE) || player->powers[pw_invulnerability])
	{
		return;
	}
	player->poisoncount += poison;
	player->poisoner = poisoner;
	if(player->poisoncount > 100)
	{
		player->poisoncount = 100;
	}
}

//==========================================================================
//
// P_PoisonDamage - Similar to P_DamageMobj
//
//==========================================================================

void P_PoisonDamage(player_t *player, mobj_t *source, int damage,
	boolean playPainSound)
{
	mobj_t *target;
	mobj_t *inflictor;

	target = player->plr->mo;
	inflictor = source;
	if(target->health <= 0)
	{
		return;
	}
	if(target->flags2&MF2_INVULNERABLE && damage < 10000)
	{ // mobj is invulnerable
		return;
	}
	if(player && gameskill == sk_baby)
	{
		// Take half damage in trainer mode
		damage >>= 1;
	}
	if(damage < 1000 && ((player->cheats&CF_GODMODE)
		|| player->powers[pw_invulnerability]))
	{
		return;
	}
	if(damage >= player->health
		&& ((gameskill == sk_baby) || deathmatch)
		&& !player->morphTics)
	{ // Try to use some inventory health
		P_AutoUseHealth(player, damage-player->health+1);
	}
	player->health -= damage; // mirror mobj health here for Dave
	if(player->health < 0)
	{
		player->health = 0;
	}
	player->attacker = source;

	//
	// do the damage
	//
	target->health -= damage;
	if(target->health <= 0)
	{ // Death
		target->special1 = damage;
		if(player && inflictor && !player->morphTics)
		{ // Check for flame death
			if((inflictor->flags2&MF2_FIREDAMAGE)
				&& (target->health > -50) && (damage > 25))
			{
				target->flags2 |= MF2_FIREDAMAGE;
			}
			if(inflictor->flags2&MF2_ICEDAMAGE)
			{
				target->flags2 |= MF2_ICEDAMAGE;
			}
		}
		P_KillMobj(source, target);
		return;
	}
	if(!(leveltime&63) && playPainSound)
	{
		P_SetMobjState(target, target->info->painstate);
	}
/*
	if((P_Random() < target->info->painchance)
		&& !(target->flags&MF_SKULLFLY))
	{
		target->flags |= MF_JUSTHIT; // fight back!
		P_SetMobjState(target, target->info->painstate);
	}
*/
}
