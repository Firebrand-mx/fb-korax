
//**************************************************************************
//**
//** g_game.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

#include <string.h>
#include <math.h>
#include "h2def.h"
#include "p_local.h"
#include "soundst.h"
#include "Settings.h"
#include "g_demo.h"
#include "h2_actn.h"
#include <assert.h>

//#define DEMOCAM			// Define this to make the democamera functional.

#define AM_STARTKEY	9
#define DEMO_VER 66

// External functions

extern void P_InitSky(int map);
extern void P_PlayerNextArtifact(player_t *player);
extern void P_UndoPossessMonster(mobj_t *actor, player_t *player); //Remi

// Functions

#ifdef DEMOCAM
void G_CameraControls(ticcmd_t *cmd);
#endif

//void D_CheckNetGame();

boolean G_CheckDemoStatus (void);
void G_ReadDemoTiccmd (ticcmd_t *cmd);
void G_WriteDemoTiccmd (ticcmd_t *cmd);
void G_InitNew (skill_t skill, int episode, int map);

void G_DoReborn (int playernum);

void G_DoLoadLevel(void);
void G_DoInitNew(void);
void G_DoNewGame(void);
void G_DoLoadGame(void);
void G_DoPlayDemo(void);
void G_DoTeleportNewMap(void);
void G_DoCompleted(void);
void G_DoVictory(void);
void G_DoWorldDone(void);
void G_DoSaveGame(void);
void G_DoSingleReborn(void);

void H2_PageTicker(void);
void H2_AdvanceDemo(void);

extern boolean mn_SuicideConsole;

byte demoDisabled = 0; // is demo playing disabled?

gameaction_t    gameaction;
gamestate_t     gamestate;
skill_t         gameskill;
//boolean         respawnmonsters;
int             gameepisode;
int             gamemap;
int				 prevmap;

boolean         paused;
boolean         sendpause;              // send a pause event next tic
boolean         sendsave;               // send a save event next tic
boolean         usergame;               // ok to save / end game

boolean         timingdemo;             // if true, exit with report on completion
int             starttime;              // for comparative timing purposes      

boolean         viewactive;

boolean         deathmatch;             // only if started as net death
//boolean         netgame;                // only true if packets are broadcast
//boolean         gi.PlayerInGame(MAXPLAYERS);
player_t		players[MAXPLAYERS];
pclass_t		PlayerClass[MAXPLAYERS];
byte			PlayerColor[MAXPLAYERS];
typedef struct
{
	int val[5];
} chr_val;
chr_val             PlayerValue[MAXPLAYERS];

chr_val Defaultroll(int Sclass);
chr_val Reroll(int Sclass);

extern chr_val MenuPValues;

democamdata_t	democam;

// Position indicator for cooperative net-play reborn
int RebornPosition;

//int             consoleplayer;          // player taking events and displaying
//int             displayplayer;          // view being displayed
//int             gametic;
int             levelstarttic;          // gametic at level start

char            demoname[32];
boolean         demorecording;
boolean         demoplayback;
byte            *demobuffer, *demo_p;
boolean         singledemo = false;		// quit after playing a demo from cmdline

boolean         precache = true;        // if true, load all graphics at start

short            consistancy[MAXPLAYERS][BACKUPTICS];

//FILE *ticdf = NULL;	// Ticcmd debug file

#ifdef ULTIMATE_DEBUG
FILE *udfile = NULL;	// The Ultimate Debug file
#endif

//
// controls (have defaults)
//


/*int controls[NUM_CONTROLS];
int mouseControls[NUM_MOUSECONTROLS];
int joyControls[NUM_JOYCONTROLS];*/

int chooseAndUse;

int usemlook;		// Mouse look (mouse Y => viewpitch)
int usejlook;		// Joy look (joy Y => viewpitch)
int alwaysRun;		// Always run.
int noAutoAim;		// No auto-aiming?
int mlookInverseY;	// Inverse mlook Y axis.
int jlookInverseY;	// Inverse jlook Y axis.
int showFullscreenMana=1;
int showFPS, sbarscale=20, lookSpring;
int translucentIceCorpse=0;
int mouseSensitivityX=8, mouseSensitivityY=8;
int joySensitivity=5;	// Joystick sensitivity (dead zone).
boolean povLookAround=false;
int dclickuse = true;

// Looking around.
float targetLookOffset=0;
float lookOffset=0;

//#define MOUSEX_SENSI_MUL	((mouseSensitivityX*2+5) / 6)
//#define MOUSEY_SENSI_MUL	((mouseSensitivityY*2+5) / 6)

int LeaveMap;
static int LeavePosition;

//#define MAXPLMOVE       0x32 // Old Heretic Max move

//Moving speed of players

fixed_t tenneg[10] = {9,8,7,6,5,4,3,2,1,0}; //Remi: Yeah, I suck in math

fixed_t health_table[NUMCLASSES][5] =
{
	{10,10,8,6,6},{7,7,5,3,3},{3,3,2,1,1},
	{7,7,5,3,3},//Corvus dummy
	{1,1,1,1}
};

fixed_t sppower_table[NUMCLASSES][5] =
{
	{3,3,2,1,1},{12,12,6,4,4},{18,18,12,8,8},
	{12,12,6,4,4},//Corvus dummy
	{0,0,0,0}
};

fixed_t skillp_table[5] =
{
	5,5,4,3,3
};

fixed_t MaxPlayerMove[NUMCLASSES] = { 0x3C, 0x32, 0x2D, 0x32, 0x31, 0x14, 0x15, 0x14, 0x14, 0x16, 
									  0x14, 0x14, 0x12, 0x10};

fixed_t forwardmove[NUMCLASSES][2] = 
{
	{ 0x1D, 0x3C }, //Fighter
	{ 0x19, 0x32 }, //Cleric
	{ 0x16, 0x2E }, //Mage
	{ 0x19, 0x32 }, //Corvus
	{ 0x18, 0x31 }, //Oink oink
	{ 0x0d, 0x14 }, //Possessed Ettin
	{ 0x0e, 0x15 }, //Possessed Afrit
	{ 0x0d, 0x14 }, //Possessed Centaur
	{ 0x0d, 0x14 }, //Possessed Centaur Leader
	{ 0x0f, 0x16 }, //Possessed Iceguy
	{ 0x0d, 0x14 }, //Possessed Serpent
	{ 0x0d, 0x14 }, //Possessed Brown Serpent
	{ 0x0b, 0x12 }, //Possessed Reiver
	{ 0x09, 0x10 }	//Possessed Bishop
};
	
fixed_t sidemove[NUMCLASSES][2] = 
{
	{ 0x1B, 0x3B },
	{ 0x18, 0x28 },
	{ 0x15, 0x25 },
	{ 0x18, 0x28 }, //Corvus
	{ 0x17, 0x27 }, //Oink oink
	{ 0x0c, 0x10 }, //Possessed Ettin
	{ 0x0e, 0x12 }, //Possessed Afrit
	{ 0x0c, 0x10 }, //Possessed Centaur
	{ 0x0c, 0x10 }, //Possessed Centaur Leader
	{ 0x0e, 0x13 }, //Possessed Iceguy
	{ 0x0c, 0x10 }, //Possessed Serpent
	{ 0x0c, 0x10 }, //Possessed Brown Serpent
	{ 0x0a, 0x0f }, //Possessed Reiver
	{ 0x08, 0x0d }  //Possessed Bishop
};

fixed_t heights[NUMCLASSES]=
{
	{52},{48},{44},
	{48}, //Corvus
	{48},
    {54}, //Possessed Ettin
	{35}, //Possessed Afrit
	{51}, //Possessed Centaur
	{51}, //Possessed Centaur Leader
	{65}, //Possessed Iceguy
	{70}, //Possessed Serpent
	{70}, //Possessed Brown Serpent
	{39}, //Possessed Reiver
	{68}  //Possessed Bishop
};

fixed_t angleturn[3] = {640, 1280, 320};     // + slow turn
#define SLOWTURNTICS    6

#define NUMKEYS 256
boolean         gamekeydown[NUMKEYS];
int             turnheld;                   // for accelerative turning
int				 lookheld;


boolean         mousearray[4];
boolean         *mousebuttons = &mousearray[1];
	// allow [-1]
int             mousex, mousey;             // mouse values are used once
int             dclicktime, dclickstate, dclicks;
int             dclicktime2, dclickstate2, dclicks2;

int             joyxmove, joyymove;         // joystick values are repeated
boolean         joyarray[33];				// 32 buttons (incl. 4 for POV) + the dummy
boolean         *joybuttons = &joyarray[1];     // allow [-1]

int				povangle = -1;				// -1 means centered (really 0 - 7).

int     savegameslot;
char    savedescription[32];

int inventoryTics;

#ifdef __WATCOMC__
extern externdata_t *i_ExternData;
#endif

static skill_t TempSkill;
static int TempEpisode;
static int TempMap;

static int demoBufferSize = 0;

//=============================================================================
/*
====================
=
= G_BuildTiccmd
=
= Builds a ticcmd from all of the available inputs or reads it from the
= demo buffer.
= If recording a demo, write it out
====================
*/

extern boolean inventory;
extern int curpos;
extern int inv_ptr;

extern  int             isCyberPresent;     // is CyberMan present?
boolean usearti = true;
void I_ReadCyberCmd (ticcmd_t *cmd);

static int findWeapon(player_t *plr, boolean forward)
{
	int	i, c;

	for(i=plr->readyweapon + (forward? 1 : -1), c=0; c<NUMWEAPONS; c++, forward? i++ : i--)
	{
		if(i > NUMWEAPONS-1) i = 0;
		if(i < 0) i = NUMWEAPONS-1;
		if(plr->weaponowned[i]) return i;
	}
	return plr->readyweapon;
}

static boolean inventoryMove(player_t *plr, int dir)
{
	inventoryTics = 5*35;
	if(!inventory)
	{
		inventory = true;
		return(false);
	}

	if(dir == 0)
	{
		inv_ptr--;
		if(inv_ptr < 0)
		{
			inv_ptr = 0;
		}
		else
		{
			curpos--;
			if(curpos < 0)
			{
				curpos = 0;
			}
		}
	}
	else
	{
		inv_ptr++;
		if(inv_ptr >= plr->inventorySlotNum)
		{
			inv_ptr--;
			if(inv_ptr < 0)
				inv_ptr = 0;
		}
		else
		{
			curpos++;
			if(curpos > 6)
			{
				curpos = 6;
			}
		}
	}
	return(true);
}

int CCmdInventory(int argc, char **argv)
{
	inventoryMove(players+consoleplayer, !stricmp(argv[0], "invright"));
	return true;
}

boolean handleZ;

void G_BuildTiccmd (ticcmd_t *cmd)
{
	int             i;
	boolean         strafe, bstrafe;
#ifdef DEMOCAM
	boolean			externaldemo = demoplayback && democam.mode;
#endif
	int             speed, tspeed, lspeed;
	int             forward, side;
	int look, arti;
	int flyheight;
	int pClass;
	int pspeed;
	boolean human;

	extern boolean artiskip;

	
	pClass = players[consoleplayer].class;
	memset (cmd, 0, sizeof(*cmd));

//	cmd->consistancy =
//		consistancy[consoleplayer][(maketic*ticdup)%BACKUPTICS];

	cmd->consistancy =
		consistancy[consoleplayer][maketic%BACKUPTICS];
	
//printf ("cons: %i\n",cmd->consistancy);
	
	strafe = actions[H2A_STRAFE].on;
	speed = actions[H2A_SPEED].on | alwaysRun;
	forward = side = look = arti = flyheight = 0;
	
//
// use two stage accelerative turning on the keyboard and joystick
//
	if (joyxmove < 0 || joyxmove > 0 || actions[H2A_TURNRIGHT].on || actions[H2A_TURNLEFT].on)
		turnheld += ticdup;
	else
		turnheld = 0;
	if (turnheld < SLOWTURNTICS)
		tspeed = 2;             // slow turn
	else
		tspeed = speed;

	if(actions[H2A_LOOKDOWN].on || actions[H2A_LOOKUP].on)
	{
		lookheld += ticdup;
	}
	else
	{
		lookheld = 0;
	}
	if(lookheld < SLOWTURNTICS)
	{
		lspeed = 1; // 3;
	}
	else
	{
		lspeed = 2; // 5;
	}

//
// let movement keys cancel each other out
//
	if(pClass == PCLASS_FIGHTER || pClass == PCLASS_CLERIC || pClass == PCLASS_MAGE)
	   human = true;
	else human = false;

	pspeed = players[consoleplayer].speed;
	
	if(strafe)
	{
		if (actions[H2A_TURNRIGHT].on || joyxmove > 0)
		{
			side += sidemove[pClass][speed];
			if (human && pspeed != 10 && !(speed == 0 && pspeed >= 10))
			{
				if (pspeed >10) side += (sidemove[pClass][speed]*5/100)*(pspeed-10);
				else			side -= (sidemove[pClass][speed]*5/100)*(tenneg[pspeed]);
			}			
		}
		if (actions[H2A_TURNLEFT].on || joyxmove < 0)
		{
			side -= sidemove[pClass][speed];
			if (human && pspeed != 10 && !(speed == 0 && pspeed >= 10))
			{
				if (pspeed >10) side -= (sidemove[pClass][speed]*5/100)*(pspeed-10);
				else			side += (sidemove[pClass][speed]*5/100)*(tenneg[pspeed]);
			}
		}				
	}
	else
	{
		if (actions[H2A_TURNRIGHT].on || joyxmove > 0)
			cmd->angleturn -= angleturn[tspeed];
		if (actions[H2A_TURNLEFT].on || joyxmove < 0)
			cmd->angleturn += angleturn[tspeed];		
	}

	if (actions[H2A_FORWARD].on)
	{
		forward += (forwardmove[pClass][speed]);
		if (human && pspeed != 10 && !(speed == 0 && pspeed >= 10))			
		{
			if (pspeed >=10) forward += (forwardmove[pClass][speed]*5/100)*(pspeed-10);
			else			 forward -= (forwardmove[pClass][speed]*5/100)*(tenneg[pspeed]);
		}
	}
	if (actions[H2A_BACKWARD].on)
	{
		forward -= forwardmove[pClass][speed];
		if (human && pspeed != 10 && !(speed == 0 && pspeed >= 10))
		{
			if (pspeed >10) forward -= (forwardmove[pClass][speed]*5/100)*(pspeed-10);
			else			forward += (forwardmove[pClass][speed]*5/100)*(tenneg[pspeed]);
		}		
	}		

	if(!usejlook && !actions[H2A_JLOOK].on)
	{
		if (joyymove < 0)
		{
			forward += forwardmove[pClass][speed];
			if (human && pspeed != 10 && !(speed == 0 && pspeed >= 10))
			{
				if (pspeed >10) forward += (forwardmove[pClass][speed]*5/100)*(pspeed-10);
				else			forward -= (forwardmove[pClass][speed]*5/100)*(tenneg[pspeed]);
			}		
		}
		if (joyymove > 0)
		{
			forward -= forwardmove[pClass][speed];
			if (human && pspeed != 10 && !(speed == 0 && pspeed >= 10))
			{
				if (pspeed >10) forward -= (forwardmove[pClass][speed]*5/100)*(pspeed-10);
				else			forward += (forwardmove[pClass][speed]*5/100)*(tenneg[pspeed]);
			}		
		}
	}
	if (actions[H2A_STRAFERIGHT].on)
	{
		side += sidemove[pClass][speed];
		if (human && pspeed != 10 && !(speed == 0 && pspeed >= 10))
		{
			if (pspeed >10) side += (sidemove[pClass][speed]*5/100)*(pspeed-10);
			else			side -= (sidemove[pClass][speed]*5/100)*(tenneg[pspeed]);
		}
	}
	if (actions[H2A_STRAFELEFT].on)
	{
		side -= sidemove[pClass][speed];
		if (human && pspeed != 10 && !(speed == 0 && pspeed >= 10))
		{
			if (pspeed >10) side -= (sidemove[pClass][speed]*5/100)*(pspeed-10);
			else			side += (sidemove[pClass][speed]*5/100)*(tenneg[pspeed]);
		}
	}


	// Fly up/down/drop keys
	if(actions[H2A_FLYUP].on)
	{
		flyheight = 5; // note that the actual flyheight will be twice this
	}
	if(actions[H2A_FLYDOWN].on)
	{
		flyheight = -5;
	}
	if(actions[H2A_FLYCENTER].on)
	{
		flyheight = TOCENTER;
		look = TOCENTER;
	}
	// Use artifact key
	if(actions[H2A_USEARTIFACT].on)
	{
		if(speed && artiskip)
		{
			if(players[consoleplayer].inventory[inv_ptr].type != arti_none)
			{ // Skip an artifact
				actions[H2A_USEARTIFACT].on = false;
				P_PlayerNextArtifact(&players[consoleplayer]);			
			}
		}
		else
		{
			if(inventory)
			{
				players[consoleplayer].readyArtifact =
					players[consoleplayer].inventory[inv_ptr].type;
				inventory = false;
				if(chooseAndUse)
					cmd->arti |= players[consoleplayer].inventory[inv_ptr].type&AFLAG_MASK;
				else
					cmd->arti = 0;
				//usearti = false;
			}
			else if(usearti)
			{
				cmd->arti |= 
					players[consoleplayer].inventory[inv_ptr].type&AFLAG_MASK;
				//usearti = false;
			}
		}
		actions[H2A_USEARTIFACT].on = false;
	}


	//Sell actual Artifact
/*	if(actions[H2A_REPLY].on)
	{
		cmd->speccmd |= SPECCMD_REPLY;
		actions[H2A_REPLY].on = false;
	}*/

	if(actions[H2A_JUMP].on)
	{
		cmd->arti |= AFLAG_JUMP;
	}
	
	//cmd->speccmd=0;
	if (actions[H2A_SPELL1].on)
	{
		actions[H2A_SPELL1].on = false;
		cmd->speccmd |= SPECCMD_SPELL1;
	}
	if (actions[H2A_SPELL2].on)
	{
		actions[H2A_SPELL2].on = false;
		cmd->speccmd |= SPECCMD_SPELL2;
	}
	if (actions[H2A_SPELL3].on)
	{
		actions[H2A_SPELL3].on = false;
		cmd->speccmd |= SPECCMD_SPELL3;
	}
	if (actions[H2A_SPELL4].on)
	{
		actions[H2A_SPELL4].on = false;
		cmd->speccmd |= SPECCMD_SPELL4;
	}
	if (actions[H2A_SPELL5].on)
	{
		actions[H2A_SPELL5].on = false;
		cmd->speccmd |= SPECCMD_SPELL5;
	}
	if (actions[H2A_SPELL6].on)
	{
		actions[H2A_SPELL6].on = false;
		cmd->speccmd |= SPECCMD_SPELL6;
	}
	if (actions[H2A_SPELL7].on)
	{
		actions[H2A_SPELL7].on = false;
		cmd->speccmd |= SPECCMD_SPELL7;
	}
	if (actions[H2A_DUCK].on && !players[consoleplayer].morphTics)
	{
		cmd->speccmd |= SPECCMD_SPELL8;
	}

	if(mn_SuicideConsole)
	{
		cmd->arti |= AFLAG_SUICIDE;
		mn_SuicideConsole = false;
	}

	// Artifact hot keys
	if(actions[H2A_PANIC].on && !cmd->arti)
	{
		actions[H2A_PANIC].on = false; 	// Use one of each artifact
		cmd->arti = NUMARTIFACTS;
	}
	else if(players[consoleplayer].plr->mo && actions[H2A_HEALTH].on && !cmd->arti 
	&& (players[consoleplayer].plr->mo->health < players[consoleplayer].maxhealth))
	{
		actions[H2A_HEALTH].on = false;
		cmd->arti = arti_health;						
	}
	else if(actions[H2A_POISONBAG].on && !cmd->arti)
	{
		actions[H2A_POISONBAG].on = false;
		cmd->arti = arti_poisonbag;						
	}
	else if(actions[H2A_BLASTRADIUS].on && !cmd->arti)
	{
		actions[H2A_BLASTRADIUS].on = false;
		cmd->arti = arti_blastradius;					
	}
	else if(actions[H2A_TELEPORT].on && !cmd->arti)
	{
		actions[H2A_TELEPORT].on = false;
		cmd->arti = arti_teleport;						
	}
	else if(actions[H2A_TELEPORTOTHER].on && !cmd->arti)
	{
		actions[H2A_TELEPORTOTHER].on = false;
		cmd->arti = arti_teleportother;						
	}
	else if(actions[H2A_EGG].on && !cmd->arti)
	{
		actions[H2A_EGG].on = false;
		cmd->arti = arti_egg;						
	}
	else if(actions[H2A_INVULNERABILITY].on && !cmd->arti
		&& !players[consoleplayer].powers[pw_invulnerability])
	{
		actions[H2A_INVULNERABILITY].on = false;
		cmd->arti = arti_invulnerability;				
	}
	else if(actions[H2A_MYSTICURN].on && !cmd->arti)
	{
		actions[H2A_MYSTICURN].on = false;
		cmd->arti = arti_superhealth;
	}
	else if(actions[H2A_TORCH].on && !cmd->arti)
	{
		actions[H2A_TORCH].on = false;
		cmd->arti = arti_torch;
	}
	else if(actions[H2A_KRATER].on && !cmd->arti)
	{
		actions[H2A_KRATER].on = false;
		cmd->arti = arti_boostmana;
	}
	else if(actions[H2A_SPEEDBOOTS].on & !cmd->arti)
	{
		actions[H2A_SPEEDBOOTS].on = false;
		cmd->arti = arti_speed;
	}
	else if(actions[H2A_DARKSERVANT].on && !cmd->arti)
	{
		actions[H2A_DARKSERVANT].on = false;
		cmd->arti = arti_summon;
	}


//
// buttons
//
	//cmd->chatchar = CT_dequeueChatChar();

	if (actions[H2A_FIRE].on)
		cmd->buttons |= BT_ATTACK;

	if (actions[H2A_USE].on)
	{
		cmd->buttons |= BT_USE;
		dclicks = 0;                    // clear double clicks if hit use button
	}

	if(actions[H2A_PREVIOUSWEAPON].on)
	{
		cmd->buttons |= BT_CHANGE | (findWeapon(players+consoleplayer, false)<<BT_WEAPONSHIFT);
	}
	else if(actions[H2A_NEXTWEAPON].on)
	{
		cmd->buttons |= BT_CHANGE | (findWeapon(players+consoleplayer, true)<<BT_WEAPONSHIFT);
	}
	else for(i = 0; i < NUMWEAPONS; i++)
	{
		if(actions[H2A_WEAPON1+i].on)
		{
			cmd->buttons |= BT_CHANGE;
			cmd->buttons |= i<<BT_WEAPONSHIFT;
			break;
		}
	}

//
// forward double click
//
	if (dclickuse && actions[H2A_FORWARD].on != dclickstate && dclicktime > 1 )
	{
		dclickstate = actions[H2A_FORWARD].on;
		if (dclickstate)
			dclicks++;
		if (dclicks == 2)
		{
			cmd->buttons |= BT_USE;
			dclicks = 0;
		}
		else
			dclicktime = 0;
	}
	else
	{
		dclicktime += ticdup;
		if (dclicktime > 20)
		{
			dclicks = 0;
			dclickstate = 0;
		}
	}
	
//
// strafe double click
//
	bstrafe = actions[H2A_STRAFE].on;
	if(dclickuse && bstrafe != dclickstate2 && dclicktime2 > 1 )
	{
		dclickstate2 = bstrafe;
		if (dclickstate2)
			dclicks2++;
		if (dclicks2 == 2)
		{
			cmd->buttons |= BT_USE;
			dclicks2 = 0;
		}
		else
			dclicktime2 = 0;
	}
	else
	{
		dclicktime2 += ticdup;
		if (dclicktime2 > 20)
		{
			dclicks2 = 0;
			dclickstate2 = 0;
		}
	}

	if (strafe)
	{
		side += mousex*2;
	}
	else
	{
		cmd->angleturn -= mousex*0x8;
	}
	// If mouse look is active, mousey doesn't control forward.
	// During demo recording mouselook won't work.
#ifdef DEMOCAM
	if((!usemlook && !actions[H2A_MLOOK].on) || 
		demorecording || (demoplayback && !democam.plr->mode) || paused 
		|| (players[consoleplayer].playerstate == PST_DEAD && !externaldemo)) 
#else
	if((!usemlook && !actions[H2A_MLOOK].on) || 
		demorecording || demoplayback || paused || players[consoleplayer].playerstate == PST_DEAD) 
#endif
		forward += mousey;
	else
	{
		// We'll directly change the viewing pitch of the console player.
		// Isn't this an ugly bugger.
		float adj = (((mousey*0x8)<<16)/(float)ANGLE_180*180*110.0/85.0);
		boolean minus;
		
		if(mlookInverseY) adj = -adj;

		minus = adj < 0;
		adj = sqrt(fabs(adj)) * DELTAMUL;
		if(minus) adj = -adj;

		// It's only a char...
		if(adj > 127) adj = 127;
		if(adj < -128) adj = -128;
		cmd->lookdirdelta = (char) adj; 
	}
	mousex = mousey = 0;

	// Joylook.
	if((usejlook || actions[H2A_JLOOK].on) && !(demorecording || demoplayback || paused
		|| players[consoleplayer].playerstate == PST_DEAD))
	{
		// FIXME: Not like this...
		//players[consoleplayer].plr->lookdir = joyymove * 110 / 10000 * (jlookInverseY? -1 : 1);
	}

	// Lookspring.
	if(lookSpring && !actions[H2A_MLOOK].on)// && players[consoleplayer].plr->mo)
	{
		if(abs(forward) >= forwardmove[pClass][speed]/*players[consoleplayer].fspeed[speed];*/)
			//players[consoleplayer].plr->mo->player->centering = true;			
			look = TOCENTER;
	}

	// Look up/down/center keys
	if(!lookSpring || (lookSpring && !forward))
	{
		if(actions[H2A_LOOKUP].on)
		{
			look = lspeed;
		}
		if(actions[H2A_LOOKDOWN].on)
		{
			look = -lspeed;
		}
		if(actions[H2A_LOOKCENTER].on)
		{
			look = TOCENTER;
		}
	}

	if (forward > MaxPlayerMove[pClass])
	{
		forward = MaxPlayerMove[pClass];
	}
	else if (forward < -MaxPlayerMove[pClass])
	{
		forward = -MaxPlayerMove[pClass];
	}
	if (side > MaxPlayerMove[pClass])
	{
		side = MaxPlayerMove[pClass];
	}
	else if (side < -MaxPlayerMove[pClass])
	{
		side = -MaxPlayerMove[pClass];
	}

	if(players[consoleplayer].powers[pw_speed] && !players[consoleplayer].morphTics)
	{ // Adjust for a player with a speed artifact
		forward = (3*forward)>>1;
		side = (3*side)>>1;
	}
	cmd->forwardmove += forward;
	cmd->sidemove += side;
	if(players[consoleplayer].playerstate == PST_LIVE)
	{
		if(look < 0)
		{
			look += 16;
		}
		cmd->lookfly = look;
	}
	if(flyheight < 0)
	{
		flyheight += 16;
	}
	cmd->lookfly |= flyheight<<4;

//
// special buttons
//
	if (sendpause)
	{
		sendpause = false;
		cmd->buttons = BT_SPECIAL | BTS_PAUSE;
	}

	if (sendsave)
	{
		sendsave = false;
		cmd->buttons = BT_SPECIAL | BTS_SAVEGAME | (savegameslot<<BTS_SAVESHIFT);
	}
}


/*
==============
=
= G_DoLoadLevel
=
==============
*/

void G_DoLoadLevel (void)
{
    static firstFragReset = 1;
	int             i;
	action_t		*act;
	
	levelstarttic = gametic;        // for time calculation 
	gamestate = GS_LEVEL;
	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (players[i].plr->ingame && players[i].playerstate == PST_DEAD)
			players[i].playerstate = PST_REBORN;
        if(netgame == 0 || (netgame != 0 && deathmatch != 0) 
           || firstFragReset == 1) {
    		memset (players[i].plr->frags,0,sizeof(players[i].plr->frags));
            firstFragReset = 0;
        }
	}

	SN_StopAllSequences();	
	P_SetupLevel (gameepisode, gamemap, 0, gameskill);   
	gi.Set(DD_DISPLAYPLAYER, consoleplayer);
	//displayplayer = consoleplayer;      // view the guy you are playing   
	starttime = gi.GetTime ();
	gameaction = ga_nothing;
	gi.Z_CheckHeap ();

//
// clear cmd building stuff
// 
	memset (gamekeydown, 0, sizeof(gamekeydown));
	joyxmove = joyymove = 0;
	mousex = mousey = 0;
	sendpause = sendsave = paused = false;
	memset (mousebuttons, 0, sizeof(mousebuttons));
	memset (joybuttons, 0, sizeof(joybuttons));
	for(act=actions; act->name[0]; act++)
	{
		act->on = false;
	}
}


int CCmdCycleSpy(int argc, char **argv)
{
	if(gamestate == GS_LEVEL && !deathmatch)
	{ // Cycle the display player
		do
		{
			gi.Set(DD_DISPLAYPLAYER, displayplayer+1);
			if(displayplayer == MAXPLAYERS)
			{
				gi.Set(DD_DISPLAYPLAYER, 0);
			}
		} while(!players[displayplayer].plr->ingame
			&& displayplayer != consoleplayer);
	}
	return true;
}


/*
===============================================================================
=
= G_Responder 
=
= get info needed to make ticcmd_ts for the players
= return false if the event should be checked for bindings
=
===============================================================================
*/

boolean G_Responder(event_t *ev)
{
	player_t *plr;
	extern boolean MenuActive;
	int i;

	plr = &players[consoleplayer];
	if(!actions[H2A_USEARTIFACT].on)//ev->type == ev_keyup && ev->data1 == controls[HCK_USEARTIFACT])
	{ // flag to denote that it's okay to use an artifact
		if(!inventory)
		{
			plr->readyArtifact = plr->inventory[inv_ptr].type;
		}
		usearti = true;
	}

	if(CT_Responder(ev))
	{ // Chat ate the event
		return(true);
	}
	if(gamestate == GS_LEVEL)
	{
		if(SB_Responder(ev))
		{ // Status bar ate the event
			return(false);
		}
		if(AM_Responder(ev))
		{ // Automap ate the event
			return(true);
		}
	}

	switch(ev->type)
	{
		case ev_keydown:
			if(ev->data1 < NUMKEYS)
			{
				gamekeydown[ev->data1] = true;
			}
			// No break here.
		case ev_keyrepeat:
			return(false); // eat all key down events
		case ev_keyup:
			if(ev->data1 < NUMKEYS)
			{
				gamekeydown[ev->data1] = false;
			}
			return(false); // always let key up events filter down

		case ev_mouse:
			mousex = (ev->data1 * (mouseSensitivityX*2+5)) / 6;
			mousey = (ev->data2 * (mouseSensitivityY*2+5)) / 6;
			return(true); // eat events

		case ev_mousebdown:
			for(i=0; i<3; i++)
				if(ev->data1 & (1<<i))
					mousebuttons[i] = true;
			return(false);

		case ev_mousebup:
			for(i=0; i<3; i++)
				if(ev->data1 & (1<<i))
					mousebuttons[i] = false;
			return(false);

		case ev_joystick: // Joystick movement
			joyxmove = ev->data1;
			joyymove = ev->data2;
			return(true); // eat events

		case ev_joybdown: 
			for(i=0; i<32; i++) 
				if(ev->data1 & (1<<i))
				{
					joybuttons[i] = true;
				}
			return(false); // eat events

		case ev_joybup: 
			for(i=0; i<32; i++) if(ev->data1 & (1<<i)) joybuttons[i] = false;
			return(false); // eat events

		case ev_povup:
			povangle = -1;
			// If looking around with PoV, don't allow bindings.
			if(povLookAround) return true;
			break;

		case ev_povdown:
			povangle = ev->data1;
			if(povLookAround) return true;
			break;

		default:
			break;
	}
	return(false);
}


//==========================================================================
//
// G_Ticker
//
//==========================================================================

void G_Ticker(void)
{
	int i, buf;
	ticcmd_t *cmd=NULL;

#ifdef ULTIMATE_DEBUG
	if(udfile == NULL) 
	{
		udfile = fopen("haps.txt", "wt");
		if(!udfile) gi.Error("Can't open the haps!\n");
	}
#endif

	/*if(ticdf == NULL)
	{
		if(gi.CheckParm("-ticdebug"))
			ticdf = fopen("ticdbg.txt", "wt");
	}*/

//
// do player reborns if needed
//
	for (i=0 ; i<MAXPLAYERS ; i++)
		if (players[i].plr->ingame && players[i].playerstate == PST_REBORN)
			G_DoReborn (i);

//
// do things to change the game state
//
	while (gameaction != ga_nothing)
	{
		switch (gameaction)
		{
			case ga_loadlevel:
				G_DoLoadLevel();
				break;
			case ga_initnew:
				G_DoInitNew();
				break;
			case ga_newgame:
				G_DoNewGame();
				break;
			case ga_loadgame:
				Draw_LoadIcon();
				G_DoLoadGame();
				break;
			case ga_savegame:
				Draw_SaveIcon();
				G_DoSaveGame();
				break;
			case ga_singlereborn:
				G_DoSingleReborn();
				break;
			case ga_playdemo:
                if(demoDisabled) {
                    gameaction = ga_nothing;
                }
                else {
    				G_DoPlayDemo();
                }
				break;
			case ga_screenshot:
				M_ScreenShot();
				gameaction = ga_nothing;
				break;
			case ga_leavemap:
				Draw_TeleportIcon();
				G_DoTeleportNewMap();
				break;
			case ga_completed:
				G_DoCompleted();
				break;
			case ga_worlddone:
				G_DoWorldDone();
				break;
			case ga_victory:
				F_StartFinale();
				break;
			default:
				break;
		}
	}

#ifdef ULTIMATE_DEBUG	
	if(netgame)
	{
		int count=0;
		thinker_t *iter;
		fprintf(udfile, "Tick %i: ", gametic);
		for(iter = gi.thinkercap->next; iter && iter != gi.thinkercap; iter = iter->next)
			count++;
		fprintf(udfile, "%i thinkers, prndidx = %i\n", count, prndindex);
	}
#endif

//
// get commands, check consistancy, and build new consistancy check
//
	//buf = gametic%BACKUPTICS;
	buf = (gametic/ticdup)%BACKUPTICS;

	//if(ticdf && netgame) fprintf(ticdf, "Tick %i", gametic);
	
	for (i=0 ; i<MAXPLAYERS ; i++)
		if (players[i].plr->ingame)
		{
			cmd = &players[i].cmd;

			//memcpy (cmd, &netcmds[i][buf], sizeof(ticcmd_t));
			gi.GetTicCmd(cmd, i);
	
			/*if(ticdf && netgame) fprintf(ticdf, " / %i [f%i s%i a%i %i c%i b%i l%i a%i]", i,
				cmd->forwardmove, cmd->sidemove, cmd->angleturn, cmd->consistancy,
				cmd->chatchar, cmd->buttons, cmd->lookfly, cmd->arti);*/

#ifdef DEMOCAM
			if(demoplayback && i==consoleplayer && democam.mode > 0)
				G_CameraControls(cmd);
#endif

			if (demoplayback)
				G_ReadDemoTiccmd (cmd);
			if (demorecording)
				G_WriteDemoTiccmd (cmd);

			if (netgame && !(gametic%ticdup) )
			{
				if (gametic > BACKUPTICS && consistancy[i][buf] != cmd->consistancy)
				{
					//gi.Error ("consistency failure (%i should be %i)",cmd->consistancy, consistancy[i][buf]);

					// Disconnect.
					gi.Execute(gi.Get(DD_SERVER)? "net server close" : "net disconnect", true);
					gi.Message("G_Ticker: CONSISTENCY FAILURE! Netgame aborted.\n");
				}
/*				if (players[i].plr->mo)
					consistancy[i][buf] = players[i].plr->mo->x;
				else*/
					// This used to have rndindex, but that can't be right...?
					consistancy[i][buf] = prndindex;
			}
		}


	//if(ticdf && netgame) fprintf(ticdf, "\n");

//
// check for special buttons
//
	for (i=0 ; i<MAXPLAYERS ; i++)
		if (players[i].plr->ingame)
		{
			if (players[i].cmd.buttons & BT_SPECIAL)
			{
				switch (players[i].cmd.buttons & BT_SPECIALMASK)
				{
				case BTS_PAUSE:
					paused ^= 1;
					if(paused)
					{
						S_PauseSound();
					}
					else
					{
						S_ResumeSound();
					}
					break;
					
				case BTS_SAVEGAME:
					if (!savedescription[0])
					{
						if(netgame)
						{
							strcpy (savedescription, "NET GAME");
						}
						else
						{
							strcpy(savedescription, "SAVE GAME");
						}
					}
					savegameslot = 
						(players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT;
					gameaction = ga_savegame;
					break;
				}
			}
		}
	
// turn inventory off after a certain amount of time
	if(inventory && !(--inventoryTics))
	{
		players[consoleplayer].readyArtifact =
			players[consoleplayer].inventory[inv_ptr].type;
		inventory = false;
		cmd->arti = 0;
	}

	// Look around.
	if(povangle != -1)
	{
		targetLookOffset = povangle/8.0f;
		if(targetLookOffset == .5f)
		{
			if(lookOffset < 0) targetLookOffset = -.5f;
		}
		else if(targetLookOffset > .5)
			targetLookOffset -= 1;
	}
	else targetLookOffset = 0;

	if(targetLookOffset != lookOffset && povLookAround)
	{
		float diff = (targetLookOffset - lookOffset)/2;
		if(diff > .075f) diff = .075f;
		if(diff < -.075f) diff = -.075f; 
		lookOffset += diff;
	}

//
// do main actions
//
	switch (gamestate)
	{
		case GS_LEVEL:
			P_Ticker ();
			SB_Ticker ();
			AM_Ticker ();
			CT_Ticker();
			break;
		case GS_INTERMISSION:
			IN_Ticker ();
			break;
		case GS_FINALE:
			F_Ticker();
			break;
		case GS_DEMOSCREEN:
			H2_PageTicker ();
			break;
	}       
	// Update 3D listener, if necessary.

}


/*
==============================================================================

						PLAYER STRUCTURE FUNCTIONS

also see P_SpawnPlayer in P_Things
==============================================================================
*/

//==========================================================================
//
// G_PlayerExitMap
//
// Called when the player leaves a map.
//
//==========================================================================

void G_PlayerExitMap(int playerNumber)
{
//	int i;
	player_t *player;
	int flightPower;

	player = &players[playerNumber];

//	if(deathmatch)
//	{
//		// Strip all but one of each type of artifact
//		for(i = 0; i < player->inventorySlotNum; i++)
//		{
//			player->inventory[i].count = 1;
//		}
//		player->artifactCount = player->inventorySlotNum;
//	}
//	else

	// Strip all current powers (retain flight)
	flightPower = player->powers[pw_flight];
	memset(player->powers, 0, sizeof(player->powers));
	player->powers[pw_flight] = flightPower;

	if(deathmatch)
	{
		player->powers[pw_flight] = 0;
	}
	else
	{
		if(P_GetMapCluster(gamemap) != P_GetMapCluster(LeaveMap))
		{ // Entering new cluster
			// Strip all keys
			player->keys = 0;

/* -JL- Allow carying of flight between hubs.
			// Strip flight artifact
			for(i = 0; i < 25; i++)
			{
				player->powers[pw_flight] = 0;
				P_PlayerUseArtifact(player, arti_fly);
			}
			player->powers[pw_flight] = 0; */
		}
	}

	//Remi: Berserker automatically deactivates at level change	
	if(player->berserkTics)
	{
		// -JL- Use P_UndoBerserk
		P_UndoBerserk(player);
	}

	if(player->morphTics)
	{
		player->readyweapon = player->plr->mo->special1; // Restore weapon
		player->morphTics = 0;
	}
	//Remi
	if (player->class >= PCLASS_ETTIN)
		P_UndoPossessMonster(player->plr->mo,player);
	player->messageTics = 0;
	player->messageTics2 = 0; //Remi
	player->plr->lookdir = 0;
	player->plr->mo->flags &= ~MF_SHADOW; // Remove invisibility
	player->plr->extralight = 0; // Remove weapon flashes
	player->plr->fixedcolormap = 0; // Remove torch
	player->damagecount = 0; // No palette changes
	player->bonuscount = 0;
	player->poisoncount = 0;
	if(player == &players[consoleplayer])
	{
		SB_state = -1; // refresh the status bar
		//viewangleoffset = 0;
	}
}

//==========================================================================
//
// ClearPlayer
//
// Safely clears the player data structures.
//
//==========================================================================

void ClearPlayer(player_t *p)
{
	ddplayer_t	*ddplayer = p->plr;
	int			playeringame = ddplayer->ingame;

	memset(p, 0, sizeof(*p));
	// Restore the pointer to ddplayer.
	p->plr = ddplayer;
	// Also clear ddplayer.
	memset(ddplayer, 0, sizeof(*ddplayer));
	// Restore the pointer to this player.
	ddplayer->extradata = p;
	// Restore the playeringame data.
	ddplayer->ingame = playeringame;
}

//==========================================================================
//
// G_PlayerReborn
//
// Called after a player dies.  Almost everything is cleared and
// initialized.
//
//==========================================================================

void G_PlayerReborn(int player)
{
	player_t *p;
	int frags[MAXPLAYERS];
	int killcount, itemcount, secretcount;
	uint worldTimer;
	int i; //Remi test

	memcpy(frags, players[player].plr->frags, sizeof(frags));
	killcount = players[player].killcount;
	itemcount = players[player].itemcount;
	secretcount = players[player].secretcount;
	worldTimer = players[player].worldTimer;

	p = &players[player];
//	memset(p, 0, sizeof(*p));
	ClearPlayer(p);

	memcpy(players[player].plr->frags, frags, sizeof(players[player].plr->frags));
	players[player].killcount = killcount;
	players[player].itemcount = itemcount;
	players[player].secretcount = secretcount;
	players[player].worldTimer = worldTimer;
	players[player].class = PlayerClass[player];
	players[player].colormap = PlayerColor[player];
	players[player].exp_level=1;
	players[player].next_level=2000;
	players[player].prev_level=1000;
	players[player].experience=1000;
	players[player].speed=PlayerValue[player].val[4];//init_speed[PlayerClass[player]];
	players[player].maxhealth_old=players[player].maxhealth=PlayerValue[player].val[0];//init_health[PlayerClass[player]];
	players[player].sp_power_old=players[player].sp_power=players[player].maxsp_power=PlayerValue[player].val[1];//80-players[player].maxhealth;
	players[player].strength=PlayerValue[player].val[2];//player_strength[players[player].class];
	players[player].agility=PlayerValue[player].val[3];//player_agility[players[player].class];
	players[player].misc_viewheight=heights[PlayerClass[player]]*FRACUNIT;
	players[player].money = 0; //Remi

	p->usedown = p->attackdown = true; // don't do anything immediately
	p->playerstate = PST_LIVE;
	p->health = players[player].maxhealth; //Inital Health

	switch (p->class)
	{
	case PCLASS_FIGHTER:
	case PCLASS_CLERIC:
		p->weaponowned[WP_FIRST] = true;
		P_NewPendingWeapon(p,WP_FIRST); //Remi
		p->readyweapon = p->pendingweapon;
		break;
	case PCLASS_MAGE:
		p->weaponowned[P_GetWeaponNum(p,WP_MAGE_WAND4)] = true;
		P_NewPendingWeapon(p,50);
		p->readyweapon = p->pendingweapon;
		break;
	case PCLASS_CORVUS:
		p->weaponowned[P_GetWeaponNum(p,WP_CORVUS_STAFF)] = true;
		p->weaponowned[P_GetWeaponNum(p,WP_CORVUS_GAUNTLETS)] = true;
		p->weaponowned[P_GetWeaponNum(p,WP_CORVUS_GWAND)] = true;
		p->weaponowned[P_GetWeaponNum(p,WP_CORVUS_CROSSBOW)] = true;
		p->weaponowned[P_GetWeaponNum(p,WP_CORVUS_MACE)] = true;
		P_NewPendingWeapon(p,50); //Remi
		for (i=0;i<NUMMANA;i++)
			p->mana[i]=50;
		p->readyweapon = p->pendingweapon;
		break;
	}
	
	boringmessage = 0; //Remi
	
	p->messageTics = 0;
	p->messageTics2 = 0;
	p->plr->lookdir = 0;
	localQuakeHappening[player] = false;
	if(p == &players[consoleplayer])
	{
		SB_state = -1; // refresh the status bar
		inv_ptr = 0; // reset the inventory pointer
		curpos = 0;
		//viewangleoffset = 0;
	}
}

/*
====================
=
= G_CheckSpot 
=
= Returns false if the player cannot be respawned at the given mapthing_t spot 
= because something is occupying it
====================
*/

void P_SpawnPlayer (mapthing_t *mthing);

boolean G_CheckSpot (int playernum, mapthing_t *mthing)
{
	fixed_t         x,y;
	subsector_t *ss;
	unsigned        an;
	mobj_t      *mo;
	
	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;

	players[playernum].plr->mo->flags2 &= ~MF2_PASSMOBJ;
	if (!P_CheckPosition (players[playernum].plr->mo, x, y) )
	{
		players[playernum].plr->mo->flags2 |= MF2_PASSMOBJ;
		return false;
	}
	players[playernum].plr->mo->flags2 |= MF2_PASSMOBJ;

// spawn a teleport fog
	ss = R_PointInSubsector (x,y);
	an = ( ANG45 * (mthing->angle/45) ) >> ANGLETOFINESHIFT;

	mo = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an],
		ss->sector->floorheight+TELEFOGHEIGHT, MT_TFOG);

	if (players[consoleplayer].plr->viewz != 1)
		S_StartSound (mo, SFX_TELEPORT);  // don't start sound on first frame

	return true;
}

/*
====================
=
= G_DeathMatchSpawnPlayer
=
= Spawns a player at one of the random death match spots
= called at level load and each death
====================
*/

void G_DeathMatchSpawnPlayer (int playernum)
{
	int             i,j;
	int             selections;

#ifdef TIC_DEBUG
	FUNTAG("G_DeathMatchSpawnPlayer");
#endif

	selections = deathmatch_p - deathmatchstarts;

	// This check has been moved to p_setup.c:P_LoadThings()
	//if (selections < 8)
	//	gi.Error ("Only %i deathmatch spots, 8 required", selections);

	for (j=0 ; j<20 ; j++)
	{
		i = P_Random() % selections;

		//gi.Message("Checking dmstart %i (prndidx=%i)...\n", i, prndindex);

		if (G_CheckSpot (playernum, &deathmatchstarts[i]) )
		{
			//gi.Message("Spawning player %i at dmstart %i\n", playernum, i);
			deathmatchstarts[i].type = playernum+1;
			P_SpawnPlayer (&deathmatchstarts[i]);
			return;
		}
	}

// no good spot, so the player will probably get stuck
	P_SpawnPlayer (&playerstarts[0][playernum]);
}

//==========================================================================
//
// G_DoReborn
//
//==========================================================================

void G_DoReborn(int playernum)
{
	int i;
	boolean oldWeaponowned[NUMWEAPONS];
	int oldKeys;
	int oldPieces;
	boolean foundSpot;
	int bestWeapon;

	unsigned int experience;
	unsigned int strength;
	unsigned int agility;
	unsigned int speed;
	int maxhealth;
	int maxhealth_old;
	unsigned int exp_level;
	unsigned int prev_level;
	unsigned int next_level;
	unsigned int maxsp_power;
	unsigned int sp_power_old;
	unsigned int money;



	if(G_CheckDemoStatus())
	{
		return;
	}
	if(!netgame)
	{
		if(SV_RebornSlotAvailable())
		{ // Use the reborn code if the slot is available
			gameaction = ga_singlereborn;
		}
		else
		{ // Start a new game if there's no reborn info
			gameaction = ga_newgame;
		}
	}
	else
	{ // Net-game
		players[playernum].plr->mo->player = NULL; // Dissassociate the corpse

		if(deathmatch)
		{ // Spawn at random spot if in death match
			G_DeathMatchSpawnPlayer(playernum);
			return;
		}

		// Cooperative net-play, retain keys and weapons
		oldKeys = players[playernum].keys;
		oldPieces = players[playernum].pieces;
		for(i = 0; i < NUMWEAPONS; i++)
		{
			oldWeaponowned[i] = players[playernum].weaponowned[i];
		}
		experience = players[playernum].experience;
		strength = players[playernum].strength;
		agility = players[playernum].agility;
		speed = players[playernum].speed;
		exp_level = players[playernum].exp_level;
		next_level = players[playernum].next_level;
		prev_level = players[playernum].prev_level;
		maxhealth = players[playernum].maxhealth;
		maxhealth_old = players[playernum].maxhealth_old;
		maxsp_power = players[playernum].maxsp_power;
		sp_power_old = players[playernum].sp_power_old;
		money = players[playernum].money;

		foundSpot = false;
		if(G_CheckSpot(playernum,
			&playerstarts[RebornPosition][playernum]))
		{ // Appropriate player start spot is open
			P_SpawnPlayer(&playerstarts[RebornPosition][playernum]);
			foundSpot = true;
		}
		else
		{
			// Try to spawn at one of the other player start spots
			for(i = 0; i < MAXPLAYERS; i++)
			{
				if(G_CheckSpot(playernum, &playerstarts[RebornPosition][i]))
				{ // Found an open start spot

					// Fake as other player
					playerstarts[RebornPosition][i].type = playernum+1;
					P_SpawnPlayer(&playerstarts[RebornPosition][i]);

					// Restore proper player type
					playerstarts[RebornPosition][i].type = i+1;
	
					foundSpot = true;
					break;
				}
			}
		}

		if(foundSpot == false)
		{ // Player's going to be inside something
			P_SpawnPlayer(&playerstarts[RebornPosition][playernum]);
		}

		// Restore keys and weapons
		players[playernum].keys = oldKeys;
		players[playernum].pieces = oldPieces;
		for(bestWeapon = 0, i = 0; i < NUMWEAPONS; i++)
		{
			if(oldWeaponowned[i])
			{
				bestWeapon = i;
				players[playernum].weaponowned[i] = true;
			}
		}

		players[playernum].experience = experience;
		players[playernum].strength = strength;
		players[playernum].agility = agility;
		players[playernum].speed = speed;
		players[playernum].exp_level = exp_level;
		players[playernum].next_level = next_level;
		players[playernum].prev_level = prev_level;
		players[playernum].maxhealth = players[playernum].health = players[playernum].plr->mo->health = maxhealth;
		players[playernum].maxhealth_old = maxhealth_old;
		players[playernum].maxsp_power = players[playernum].sp_power = maxsp_power;
		players[playernum].sp_power_old = sp_power_old;
		players[playernum].money = money;
		
		players[playernum].mana[MANA_1] = 25;
		players[playernum].mana[MANA_2] = 25;
		if(bestWeapon)
		{ // Bring up the best weapon
			P_NewPendingWeapon(players[playernum].plr->mo->player,50); //Remi
			//players[playernum].pendingweapon = bestWeapon;
		}
	}
}

void G_ScreenShot (void)
{
	gameaction = ga_screenshot;
}

//==========================================================================
//
// G_StartNewInit
//
//==========================================================================

void G_StartNewInit(void)
{
	MenuPValues=Reroll(PlayerClass[consoleplayer]);
	SV_InitBaseSlot();
	SV_ClearRebornSlot();
	P_ACSInitNewGame();
	// Default the player start spot group to 0
	RebornPosition = 0;
}

//==========================================================================
//
// G_StartNewGame
//
//==========================================================================

void G_StartNewGame(skill_t skill)
{
	int realMap;

	SV_InitBaseSlot();
	SV_ClearRebornSlot();
	P_ACSInitNewGame();
	// Default the player start spot group to 0
	RebornPosition = 0;
//	G_StartNewInit();
	realMap = P_TranslateMap(1);
	if(realMap == -1)
	{
		realMap = 1;
	}
	G_InitNew(TempSkill, 1, realMap);
}

//==========================================================================
//
// G_TeleportNewMap
//
// Only called by the warp cheat code.  Works just like normal map to map
// teleporting, but doesn't do any interlude stuff.
//
//==========================================================================

void G_TeleportNewMap(int map, int position)
{
	gameaction = ga_leavemap;
	LeaveMap = map;
	LeavePosition = position;
}

//==========================================================================
//
// G_DoTeleportNewMap
//
//==========================================================================

void G_DoTeleportNewMap(void)
{
	SV_MapTeleport(LeaveMap, LeavePosition);
	gamestate = GS_LEVEL;
	gameaction = ga_nothing;
	RebornPosition = LeavePosition;
}

/*
boolean secretexit;
void G_ExitLevel (void)
{
	secretexit = false;
	gameaction = ga_completed;
}
void G_SecretExitLevel (void)
{
	secretexit = true;
	gameaction = ga_completed;
}
*/

//==========================================================================
//
// G_Completed
//
// Starts intermission routine, which is used only during hub exits,
// and DeathMatch games.
//==========================================================================

void G_Completed(int map, int position)
{
	gameaction = ga_completed;
	LeaveMap = map;
	LeavePosition = position;

	//Remi: Shops are forced in here
	if		(gamemap == 2 && map == 13)
		LeaveMap = 70;
	else if (gamemap == 12 && map == 27)
		LeaveMap = 71;
	else if (gamemap == 27 && map == 22)
		LeaveMap = 72;
	else if (gamemap == 23 && map == 35)
		LeaveMap = 73;
	else if (gamemap == 35 && map == 40)
		LeaveMap = 74;
	else if (gamemap == 42 && map == 48)  //DeathKings HUB 1
		LeaveMap = 75;
	else if (gamemap == 48 && map == 54)  //DeathKings HUB 2
		LeaveMap = 76;
	else if (gamemap == 54 && map == 60)  //DeathKings HUB 2 Final
		LeaveMap = 77;
}

void G_DoCompleted(void)
{
	int i;

	gameaction = ga_nothing;
	if(G_CheckDemoStatus())
	{
		return;
	}
	for(i = 0; i < MAXPLAYERS; i++)
	{
		if(players[i].plr->ingame)
		{
			G_PlayerExitMap(i);
		}
	}
	if(LeaveMap == -1 && LeavePosition == -1)
	{
		gameaction = ga_victory;
		return;
	}
	else
	{		
		gamestate = GS_INTERMISSION;
		IN_Start();
	}

/*
	int i;
	static int afterSecret[3] = { 7, 5, 5 };

	gameaction = ga_nothing;
	if(G_CheckDemoStatus())
	{
		return;
	}
	for(i = 0; i < MAXPLAYERS; i++)
	{
		if(gi.PlayerInGame(i))
		{
			G_PlayerFinishLevel(i);
		}
	}
	prevmap = gamemap;
	if(secretexit == true)
	{
		gamemap = 9;
	}
	else if(gamemap == 9)
	{ // Finished secret level
		gamemap = afterSecret[gameepisode-1];
	}
	else if(gamemap == 8)
	{
		gameaction = ga_victory;
		return;
	}
	else
	{
		gamemap++;
	}
	gamestate = GS_INTERMISSION;
	IN_Start();
*/
}

//============================================================================
//
// G_WorldDone
//
//============================================================================

void G_WorldDone(void)
{
	gameaction = ga_worlddone;
}

//============================================================================
//
// G_DoWorldDone
//
//============================================================================

void G_DoWorldDone(void)
{
	gamestate = GS_LEVEL;
	G_DoLoadLevel();
	gameaction = ga_nothing;
	viewactive = true;
}

//==========================================================================
//
// G_DoSingleReborn
//
// Called by G_Ticker based on gameaction.  Loads a game from the reborn
// save slot.
//
//==========================================================================

void G_DoSingleReborn(void)
{
	gameaction = ga_nothing;
	SV_LoadGame(SV_GetRebornSlot());
	SB_SetClassData();
}

//==========================================================================
//
// G_LoadGame
//
// Can be called by the startup code or the menu task.
//
//==========================================================================

static int GameLoadSlot;

void G_LoadGame(int slot)
{
	GameLoadSlot = slot;
	gameaction = ga_loadgame;
}

//==========================================================================
//
// G_DoLoadGame
//
// Called by G_Ticker based on gameaction.
//
//==========================================================================

void G_DoLoadGame(void)
{
	gameaction = ga_nothing;
	SV_LoadGame(GameLoadSlot);
	if(!netgame)
	{ // Copy the base slot to the reborn slot
		SV_UpdateRebornSlot();
	}
	SB_SetClassData();
}

//==========================================================================
//
// G_SaveGame
//
// Called by the menu task.  <description> is a 24 byte text string.
//
//==========================================================================

void G_SaveGame(int slot, char *description)
{
	savegameslot = slot;
	strcpy(savedescription, description);
	sendsave = true;
}

//==========================================================================
//
// G_DoSaveGame
//
// Called by G_Ticker based on gameaction.
//
//==========================================================================

void G_DoSaveGame(void)
{
	SV_SaveGame(savegameslot, savedescription);
	gameaction = ga_nothing;
	savedescription[0] = 0;
	P_SetMessage(&players[consoleplayer], TXT_GAMESAVED, true);
}

//==========================================================================
//
// G_DeferredNewGame
//
//==========================================================================

void G_DeferredNewGame(skill_t skill)
{
	TempSkill = skill;
	gameaction = ga_newgame;
}

//==========================================================================
//
// G_DoNewGame
//
//==========================================================================

void G_DoNewGame(void)
{
	G_StartNewGame(TempSkill);
	gameaction = ga_nothing;
}

/*
====================
=
= G_InitNew
=
= Can be called by the startup code or the menu task
= consoleplayer, displayplayer, playeringame[] should be set
====================
*/

void G_DeferedInitNew(skill_t skill, int episode, int map)
{
	TempSkill = skill;
	TempEpisode = episode;
	TempMap = map;
	gameaction = ga_initnew;
}

void G_DoInitNew(void)
{
	SV_InitBaseSlot();
	G_InitNew(TempSkill, TempEpisode, TempMap);
	gameaction = ga_nothing;
}

void G_InitNew(skill_t skill, int episode, int map)
{
	int i;

	if(paused)
	{
		paused = false;
		S_ResumeSound();
	}
	if(skill < sk_baby)
	{
		skill = sk_baby;
	}
	if(skill > sk_nightmare)
	{
		skill = sk_nightmare;
	}
	if(map < 1)
	{
		map = 1;
	}
	if(map > 99)
	{
		map = 99;
	}
	M_ClearRandom();
	// Force players to be initialized upon first level load
	for(i = 0; i < MAXPLAYERS; i++)
	{
		players[i].playerstate = PST_REBORN;
		players[i].worldTimer = 0;
		if (i!=consoleplayer)
		{
			PlayerValue[i]=Defaultroll(PlayerClass[i]);
			if (netgame) PlayerValue[i].val[4]=10;
		} else
		{
			PlayerValue[i]=(netgame?Defaultroll(PlayerClass[i]):MenuPValues);
			if (netgame) PlayerValue[i].val[4]=10;
		}
	}
	// Set up a bunch of globals
	usergame = true; // will be set false if a demo
	paused = false;
	demorecording = false;
	demoplayback = false;
	viewactive = true;
	gameepisode = episode;
	gamemap = map;
	gameskill = skill;
	//BorderNeedRefresh = true;
	gi.Update(DDUF_BORDER);

	// Initialize the sky
	P_InitSky(map);

	// Give one null ticcmd_t
	//gametic = 0;
	//maketic = 1;
	//for (i=0 ; i<MAXPLAYERS ; i++)
	//	nettics[i] = 1; // one null event for this gametic
	//memset (localcmds,0,sizeof(localcmds));
	//memset (netcmds,0,sizeof(netcmds));

	G_DoLoadLevel();
}

/*
===============================================================================

							DEMO RECORDING

===============================================================================
*/

#define DEMOMARKER      0x80

void G_ReadDemoTiccmd (ticcmd_t *cmd)
{
	if (*demo_p == DEMOMARKER || actions[H2A_STOPDEMO].on)
	{       // end of demo data stream
		G_CheckDemoStatus ();
		return;
	}
	cmd->forwardmove = ((signed char)*demo_p++);
	cmd->sidemove = ((signed char)*demo_p++);
	cmd->angleturn = ((unsigned char)*demo_p++)<<8;
	cmd->buttons = (unsigned char)*demo_p++;
	cmd->lookfly = (unsigned char)*demo_p++;
	cmd->arti = (unsigned char)*demo_p++;
	cmd->speccmd = (unsigned char)*demo_p++;
}

void G_WriteDemoTiccmd (ticcmd_t *cmd)
{
	int	offset = demo_p - demobuffer;
	
	// Check that we have enough memory in the buffer.
	if(offset+7 > demoBufferSize) // 6 bytes per ticcmd.
	{
		// We need to allocate a new, larger buffer! Allocate 32 kb more.
		byte *newbuffer = gi.Z_Malloc(demoBufferSize+=0x8000, PU_STATIC, NULL);
		// Copy the old buffer data.
		memcpy(newbuffer, demobuffer, offset);
		// Free the old buffer.
		gi.Z_Free(demobuffer);
		demobuffer = newbuffer;
		// Update the pointer.
		demo_p = demobuffer + offset;

		//gi.Message("demo buffer reallocated\n");
	}

	if (actions[H2A_STOPDEMO].on)     // press to end demo recording
		G_CheckDemoStatus ();
	*demo_p++ = cmd->forwardmove;
	*demo_p++ = cmd->sidemove;
	*demo_p++ = cmd->angleturn/256;
	*demo_p++ = cmd->buttons;
	*demo_p++ = cmd->lookfly;
	*demo_p++ = cmd->arti;
	*demo_p++ = cmd->speccmd;
	demo_p -= 7;
	G_ReadDemoTiccmd (cmd);         // make SURE it is exactly the same
}


/*
===================
=
= G_RecordDemo
=
===================
*/

void G_RecordDemo (skill_t skill, int numplayers, int episode, int map, char *name)
{
	int             i;
	MenuPValues = Defaultroll(PlayerClass[consoleplayer]);
	G_InitNew (skill, episode, map);
	usergame = false;
	strcpy (demoname, name);
	strcat (demoname, ".lmp");
	demobuffer = demo_p = gi.Z_Malloc(demoBufferSize=0x20000, PU_STATIC, NULL);
	*demo_p++ = DEMO_VER;
	*demo_p++ = skill;
	*demo_p++ = episode;
	*demo_p++ = map;
	
	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		*demo_p++ = players[i].plr->ingame;
		*demo_p++ = PlayerClass[i];
	}		
	demorecording = true;
}


/*
===================
=
= G_PlayDemo
=
===================
*/

char    *defdemoname;

void G_DeferedPlayDemo (char *name)
{
	defdemoname = name;
	gameaction = ga_playdemo;
}

void G_DoPlayDemo (void)
{
	skill_t skill;
	int             i, episode, map, demver;
	
	gameaction = ga_nothing;
	demobuffer = demo_p = gi.W_CacheLumpName (defdemoname, PU_STATIC);
	demver = *demo_p++;
	if (demver!=DEMO_VER) return;
	skill = *demo_p++;
	episode = *demo_p++;
	map = *demo_p++;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		//gi.SetPlayerInGame(i, *demo_p++);
		players[i].plr->ingame = *demo_p++;
		PlayerClass[i] = *demo_p++;
	}
	// Initialize world info, etc.
	G_StartNewInit();
	MenuPValues = Defaultroll(PlayerClass[consoleplayer]);

	precache = false;               // don't spend a lot of time in loadlevel
	G_InitNew (skill, episode, map);
	precache = true;
	usergame = false;
	demoplayback = true;

	memset(&democam, 0, sizeof(democam));
#ifdef DEMOCAM
	democam.x = democam.y = democam.z = 0;
	democam.lookdir = democam.viewangle = 0;
	democam.dist = 128;
	democam.mode = 2;
	// Creating the camera mobj makes random number generation go random...
	M_SaveRandom();
	democam.mo = P_SpawnMobj(players[consoleplayer].plr->mo->x,
		players[consoleplayer].plr->mo->y, players[consoleplayer].plr->mo->z, MT_CAMERA);
	M_RestoreRandom();
#endif
}


/*
===================
=
= G_TimeDemo
=
===================
*/

void G_TimeDemo (char *name)
{
	/*skill_t			skill;
	int             episode, map;
	
	demobuffer = demo_p = gi.W_CacheLumpName (name, PU_STATIC);
	skill = *demo_p++;
	episode = *demo_p++;
	map = *demo_p++;
	G_InitNew (skill, episode, map);*/

	skill_t skill;
	int             i, episode, map, demver;
	
	gameaction = ga_nothing;
	demobuffer = demo_p = gi.W_CacheLumpName (name, PU_STATIC);
	demver = *demo_p++;
	if (demver!=DEMO_VER) return;
	skill = *demo_p++;
	episode = *demo_p++;
	map = *demo_p++;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		players[i].plr->ingame = *demo_p++;
		PlayerClass[i] = *demo_p++;
	}

	// Initialize world info, etc.
	G_StartNewInit();

	precache = true;   
	G_InitNew (skill, episode, map);
	usergame = false;
	demoplayback = true;
	timingdemo = true;
	singletics = true;
}


/*
===================
=
= G_CheckDemoStatus
=
= Called after a death or level completion to allow demos to be cleaned up
= Returns true if a new demo loop action will take place
===================
*/

boolean G_CheckDemoStatus (void)
{
//	extern int		ticcount;
//	extern int		resendto[MAXNETNODES];
	int             endtime;

	actions[H2A_STOPDEMO].on = false;

	if (timingdemo)
	{
		float seconds;
		endtime = gi.GetTime ();
		gi.Message( "\nTimedemo results:\n");
		gi.Message( "* %d gametics in %d realtics.\n",gametic, endtime-starttime);
		seconds = (endtime-starttime)/35.0;
		gi.Message( "* Timedemo ran for %f seconds.\n", seconds);
		//printf( "* Average FPS is thus: %f\n",gametic/seconds);
		//gi.Error ("timed %i gametics in %i realtics",gametic, endtime-starttime);
		gi.Message("* Average FPS: %.3f\n--- Ending Timedemo ---\n", gametic/seconds);
		
		if(singledemo) gi.Quit();
		
		// Do some reseting.
		timingdemo = false;
		singletics = false;
		/*ticcount = 0;
		gametic = 0;
		maketic = 0;*/
//		D_CheckNetGame();
		/*for(i=0; i<MAXNETNODES; i++)
		{
		//	nodeingame[i] = 0;
			nettics[i] = H2MAXINT;
		//	resendto[i] = 0;                     
		}*/

		H2_AdvanceDemo();
		return true;
	}
	
	if (demoplayback)
	{
		if (singledemo) gi.Quit ();
			
		gi.Z_ChangeTag (demobuffer, PU_CACHE);
		demoplayback = false;
		H2_AdvanceDemo();
		return true;
	}

	if (demorecording)
	{
		int demosize;
		*demo_p++ = DEMOMARKER;
		demosize = demo_p - demobuffer;
		gi.WriteFile (demoname, demobuffer, demosize);
		gi.Z_Free (demobuffer);
		demorecording = false;
		
		gi.Message("Demo %s recorded (%d bytes).\n", demoname, demosize);

		if(singledemo) gi.Quit();

		demorecording = false;
		H2_AdvanceDemo();
		return true;
	}

	return false;
}

#ifdef DEMOCAM
// The camera is totally separate from the ticcmd system.
//void G_CameraControls(mobj_t *cam)
void G_CameraControls(ticcmd_t *cmd)
{
	mobj_t	*cam = democam.mo;
	int		fly;
/*	int		forward, side;
	int		speed, strafe = 0;

	strafe = gamekeydown[controls[HCK_STRAFE]] || mousebuttons[mouseControls[HCM_STRAFE]]
		|| joybuttons[joyControls[HCJ_STRAFE]];
	speed = gamekeydown[controls[HCK_SPEED]] || mousebuttons[mouseControls[HCM_SPEED]]
		|| joybuttons[joyControls[HCJ_SPEED]] || alwaysRun;

	if(gamekeydown[controls[HCK_LEFT]])
		cam->angle += ANGLE_1*2;
	if(gamekeydown[controls[HCK_RIGHT]])
		cam->angle -= ANGLE_1*2;
	cam->momz = 0;
	if(gamekeydown[controls[HCK_FLYUP]])
		cam->momz += FRACUNIT;
	if(gamekeydown[controls[HCK_FLYDOWN]])
		cam->momz -= FRACUNIT;
	forward = 0;
	if(gamekeydown[controls[HCK_UP]])
		forward += FRACUNIT*3;
	if(gamekeydown[controls[HCK_DOWN]])
		forward -= FRACUNIT*3;
	side = 0;
	if(gamekeydown[controls[HCK_STRAFELEFT]])
		side -= FRACUNIT*3;
	if(gamekeydown[controls[HCK_STRAFERIGHT]])
		side += FRACUNIT*3;

	if(speed) forward *= 2;

	// Looking with the mouse.
	if(strafe)
	{
		side += mousex*2;
	}
	else
	{
		cam->angle -= mousex*0x8 << 16;
	}
	mousex = 0;

	if(!usemlook && !gamekeydown[controls[HCK_MLOOK]] && !mousebuttons[mouseControls[HCM_LOOK]]) 
		forward += mousey;
	else
	{
		// We'll directly change the viewing pitch of the demo camera.
		float adj = (((mousey*0x8)<<16)/(float)ANGLE_180*180*110.0/85.0), newlookdir;
		if(mlookInverseY) adj = -adj;
		newlookdir = democam.lookdir + adj;
		if(newlookdir > 110) newlookdir = 110;
		if(newlookdir < -110) newlookdir = -110;
		democam.lookdir = newlookdir;
	}
	mousey = 0;*/

/*typedef struct
{
	char		forwardmove;		// *2048 for move
	char		sidemove;			// *2048 for move
	short		angleturn;			// <<16 for angle delta
	short		consistancy;		// checks for net game
	byte		chatchar;
	byte		buttons;
	byte		lookfly;			// look/fly up/down/centering
	byte		arti;				// artitype_t to use
} ticcmd_t;
*/
	cam->angle += cmd->angleturn << 16;

	// Move.
	//cam->momx = cam->momy = 0;
	if(cmd->forwardmove)
	{
		cam->momx += FixedMul(cmd->forwardmove*2048, finecosine[cam->angle>>ANGLETOFINESHIFT]);
		cam->momy += FixedMul(cmd->forwardmove*2048, finesine[cam->angle>>ANGLETOFINESHIFT]);
	}
	if(cmd->sidemove)
	{
		cam->momx += FixedMul(cmd->sidemove*2048, finesine[cam->angle>>ANGLETOFINESHIFT]);
		cam->momy -= FixedMul(cmd->sidemove*2048, finecosine[cam->angle>>ANGLETOFINESHIFT]);
	}
	cam->momz = 0;
	fly = cmd->lookfly>>4;
	if(fly > 7)
	{
		fly -= 16;
	}
	if(fly != TOCENTER)
	{
		cam->momz = (fly*2)<<FRACBITS;
	}

	
/*	cam->x = players[consoleplayer].mo->x;
	cam->y = players[consoleplayer].mo->y;
	cam->z = players[consoleplayer].mo->z + players[consoleplayer].mo->height/2;
	cam->momx = -FixedMul(FRACUNIT*democam.dist, finecosine[cam->angle>>ANGLETOFINESHIFT]);
	cam->momy = -FixedMul(FRACUNIT*democam.dist, finesine[cam->angle>>ANGLETOFINESHIFT]);
	cam->momz = FRACUNIT*(democam.dist*sin(-democam.lookdir/160*PI));*/
}
#endif

int CCmdPause(int argc, char **argv)
{
	extern boolean MenuActive;

	if(!MenuActive) sendpause = true;
	return true;
}

int CCmdSetDemoMode(int argc, char **argv)
{
	if(argc != 2)
	{
		gi.conprintf( "Usage: demomode (0-3)\n");
		gi.conprintf( "0=normal, 1=fixed, 2=free, 3=movie.\n");
		gi.conprintf( "Current mode is %d.\n", democam.mode);
		return true;
	}
	democam.mode = atoi(argv[1]);
	if(democam.mode < 0) democam.mode = 0;
	if(democam.mode > 3) democam.mode = 3;
	gi.conprintf( "Demo mode %d set.\n", democam.mode);
	return true;
}
