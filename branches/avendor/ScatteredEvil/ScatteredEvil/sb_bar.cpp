
//**************************************************************************
//**
//** sb_bar.c : Heretic 2 : Raven Software, Corp.
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
#include "settings.h"

#ifdef DEMOCAM
#include "g_demo.h"
#endif

// MACROS ------------------------------------------------------------------

/*#define CHEAT_ENCRYPT(a) \
	((((a)&1)<<2)+ \
	(((a)&2)>>1)+ \
	(((a)&4)<<5)+ \
	(((a)&8)<<2)+ \
	(((a)&16)>>3)+ \
	(((a)&32)<<1)+ \
	(((a)&64)>>3)+ \
	(((a)&128)>>3))

// Cheat types for nofication.
typedef enum 
{
	CHT_GOD,
	CHT_NOCLIP,
	CHT_WEAPONS,
	CHT_HEALTH,
	CHT_KEYS,
	CHT_ARTIFACTS,
	CHT_PUZZLE
} cheattype_t;

// TYPES -------------------------------------------------------------------

typedef struct Cheat_s
{
	void (*func)(player_t *player, struct Cheat_s *cheat);
	byte *sequence;
	byte *pos;
	int args[2];
	int currentArg;
} Cheat_t;
*/
// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void SB_PaletteFlash(boolean forceChange);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void DrawSoundInfo(void);
static void DrINumber(signed int val, int x, int y);
static void DrRedINumber(signed int val, int x, int y);
static void DrBNumber(signed int val, int x, int y);
static void DrawCommonBar(void);
static void DrawMainBar(void);
static void DrawInventoryBar(void);
static void DrawKeyBar(void);
static void DrawWeaponPieces(void);
static void DrawFullScreenStuff(void);
static void DrawAnimatedIcons(void);
/*static boolean HandleCheats(byte key);
static boolean CheatAddKey(Cheat_t *cheat, byte key, boolean *eat);
static void CheatGodFunc(player_t *player, Cheat_t *cheat);
static void CheatNoClipFunc(player_t *player, Cheat_t *cheat);
static void CheatWeaponsFunc(player_t *player, Cheat_t *cheat);
static void CheatHealthFunc(player_t *player, Cheat_t *cheat);
static void CheatKeysFunc(player_t *player, Cheat_t *cheat);
static void CheatSoundFunc(player_t *player, Cheat_t *cheat);
static void CheatTickerFunc(player_t *player, Cheat_t *cheat);
static void CheatArtifactAllFunc(player_t *player, Cheat_t *cheat);
static void CheatPuzzleFunc(player_t *player, Cheat_t *cheat);
static void CheatWarpFunc(player_t *player, Cheat_t *cheat);
static void CheatPigFunc(player_t *player, Cheat_t *cheat);
static void CheatMassacreFunc(player_t *player, Cheat_t *cheat);
static void CheatIDKFAFunc(player_t *player, Cheat_t *cheat);
static void CheatQuickenFunc1(player_t *player, Cheat_t *cheat);
static void CheatQuickenFunc2(player_t *player, Cheat_t *cheat);
static void CheatQuickenFunc3(player_t *player, Cheat_t *cheat);
static void CheatClassFunc1(player_t *player, Cheat_t *cheat);
static void CheatClassFunc2(player_t *player, Cheat_t *cheat);
static void CheatInitFunc(player_t *player, Cheat_t *cheat);
static void CheatInitFunc(player_t *player, Cheat_t *cheat);
static void CheatVersionFunc(player_t *player, Cheat_t *cheat);
static void CheatDebugFunc(player_t *player, Cheat_t *cheat);
static void CheatScriptFunc1(player_t *player, Cheat_t *cheat);
static void CheatScriptFunc2(player_t *player, Cheat_t *cheat);
static void CheatScriptFunc3(player_t *player, Cheat_t *cheat);
static void CheatRevealFunc(player_t *player, Cheat_t *cheat);
static void CheatTrackFunc1(player_t *player, Cheat_t *cheat);
static void CheatTrackFunc2(player_t *player, Cheat_t *cheat);
*/
// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern	sfxinfo_t	S_sfx[];

extern byte *memscreen;
extern int ArmorIncrement[NUMCLASSES][NUMARMOR];
extern int AutoArmorSave[NUMCLASSES];

// PUBLIC DATA DECLARATIONS ------------------------------------------------

int DebugSound; // Debug flag for displaying sound info
boolean inventory;
int curpos;
int inv_ptr;
int ArtifactFlash;

#ifndef __WATCOMC__
boolean i_CDMusic; // in Watcom, defined in i_ibm
#endif

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static byte CheatLookup[256];
static int HealthMarker;
//static int ChainWiggle;
static player_t *CPlayer;
static int SpinFlylump;
static int SpinMinotaurLump;
static int SpinSpeedLump;
static int SpinDefenseLump;

static int FontBNumBase;
static int PlayPalette;

/*static patch_t *PatchH2BAR;
static patch_t *PatchH2TOP;
static patch_t *PatchLFEDGE;
static patch_t *PatchRTEDGE;
static patch_t *PatchARMCLEAR;
static patch_t *PatchARTICLEAR;
static patch_t *PatchMANACLEAR;
static patch_t *PatchKILLS;
static patch_t *PatchMANAVIAL1;
static patch_t *PatchMANAVIAL2;
static patch_t *PatchMANAVIALDIM1;
static patch_t *PatchMANAVIALDIM2;
static patch_t *PatchMANADIM1;
static patch_t *PatchMANADIM2;
static patch_t *PatchMANABRIGHT1;
static patch_t *PatchMANABRIGHT2;
static patch_t *PatchCHAIN;
static patch_t *PatchSTATBAR;
static patch_t *PatchKEYBAR;
static patch_t *PatchLIFEGEM;
static patch_t *PatchSELECTBOX;
static patch_t *PatchINumbers[10];
static patch_t *PatchNEGATIVE;
static patch_t *PatchSmNumbers[10];
static patch_t *PatchINVBAR;
static patch_t *PatchWEAPONSLOT;
static patch_t *PatchWEAPONFULL;
static patch_t *PatchPIECE1;
static patch_t *PatchPIECE2;
static patch_t *PatchPIECE3;
static patch_t *PatchINVLFGEM1;
static patch_t *PatchINVLFGEM2;
static patch_t *PatchINVRTGEM1;
static patch_t *PatchINVRTGEM2;*/

static int PatchNumH2BAR;
static int PatchNumH2TOP;
static int PatchNumLFEDGE;
static int PatchNumRTEDGE;
//static int PatchNumARMCLEAR;
//static int PatchNumARTICLEAR;
//static int PatchNumMANACLEAR;
static int PatchNumKILLS;
static int PatchNumMANAVIAL1;
static int PatchNumMANAVIAL2;
static int PatchNumMANAVIALDIM1;
static int PatchNumMANAVIALDIM2;
static int PatchNumMANADIM1;
static int PatchNumMANADIM2;
static int PatchNumMANABRIGHT1;
static int PatchNumMANABRIGHT2;
static int PatchNumCHAIN;
static int PatchNumSTATBAR;
static int PatchNumKEYBAR;
static int PatchNumLIFEGEM;
static int PatchNumSELECTBOX;
static int PatchNumINumbers[10];
static int PatchNumNEGATIVE;
static int PatchNumSmNumbers[10];
static int PatchNumINVBAR;
static int PatchNumWEAPONSLOT;
static int PatchNumWEAPONFULL;
static int PatchNumPIECE1;
static int PatchNumPIECE2;
static int PatchNumPIECE3;
static int PatchNumINVLFGEM1;
static int PatchNumINVLFGEM2;
static int PatchNumINVRTGEM1;
static int PatchNumINVRTGEM2;


/*// Toggle god mode
static byte CheatGodSeq[] =
{
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('n'),
	0xff
};

// Toggle no clipping mode
static byte CheatNoClipSeq[] =
{
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('p'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('r'),
	0xff
};

// Get all weapons and mana
static byte CheatWeaponsSeq[] =
{
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('a'),
	0xff
};

// Get full health
static byte CheatHealthSeq[] =
{
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('l'),
	CHEAT_ENCRYPT('u'),
	CHEAT_ENCRYPT('b'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('d'),
	0xff
};

// Get all keys
static byte CheatKeysSeq[] =
{
	CHEAT_ENCRYPT('l'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('h'),
	0xff, 0
};

// Toggle sound debug info
static byte CheatSoundSeq[] =
{
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('e'),
	0xff
};

// Toggle ticker
static byte CheatTickerSeq[] =
{
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('r'),
	0xff, 0
};

// Get all artifacts
static byte CheatArtifactAllSeq[] =
{
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('a'),
	0xff, 0
};

// Get all puzzle pieces
static byte CheatPuzzleSeq[] =
{
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('h'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('l'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('k'),
	0xff, 0
};

// Warp to new level
static byte CheatWarpSeq[] =
{
	CHEAT_ENCRYPT('v'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('t'),
	0, 0, 0xff, 0
};

// Become a pig
static byte CheatPigSeq[] =
{
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('l'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('v'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('e'),
	0xff, 0
};

// Kill all monsters
static byte CheatMassacreSeq[] =
{
	CHEAT_ENCRYPT('b'),
	CHEAT_ENCRYPT('u'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('h'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('r'),
	0xff, 0
};

static byte CheatIDKFASeq[] =
{
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('n'),
	0xff, 0
};

static byte CheatQuickenSeq1[] =
{
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('k'),
	0xff, 0
};

static byte CheatQuickenSeq2[] =
{
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('k'),
	0xff, 0
};

static byte CheatQuickenSeq3[] =
{
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('k'),
	0xff, 0
};

// New class
static byte CheatClass1Seq[] = 
{
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('h'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('w'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('r'),
	0xff, 0
};

static byte CheatClass2Seq[] = 
{
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('h'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('w'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('r'),
	0, 0xff, 0
};

static byte CheatInitSeq[] =
{
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('t'),
	0xff, 0
};

static byte CheatVersionSeq[] =
{
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('j'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('s'),
	0xff, 0
};

static byte CheatDebugSeq[] =
{
	CHEAT_ENCRYPT('w'),
	CHEAT_ENCRYPT('h'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('e'),
	0xff, 0
};

static byte CheatScriptSeq1[] =
{
	CHEAT_ENCRYPT('p'),
	CHEAT_ENCRYPT('u'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('e'),
	0xff, 0
};

static byte CheatScriptSeq2[] =
{
	CHEAT_ENCRYPT('p'),
	CHEAT_ENCRYPT('u'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('e'),
	0, 0xff, 0
};

static byte CheatScriptSeq3[] =
{
	CHEAT_ENCRYPT('p'),
	CHEAT_ENCRYPT('u'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('e'),
	0, 0, 0xff,
};

static byte CheatRevealSeq[] =
{
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('p'),
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('o'),
	0xff, 0
};

static byte CheatTrackSeq1[] = 
{
	//CHEAT_ENCRYPT('`'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('t'),
	0xff, 0
};

static byte CheatTrackSeq2[] = 
{
	//CHEAT_ENCRYPT('`'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('t'),
	0, 0, 0xff, 0
};

static Cheat_t Cheats[] =
{
	{ CheatTrackFunc1, CheatTrackSeq1, NULL, 0, 0, 0 },
	{ CheatTrackFunc2, CheatTrackSeq2, NULL, 0, 0, 0 },
	{ CheatGodFunc, CheatGodSeq, NULL, 0, 0, 0 },
	{ CheatNoClipFunc, CheatNoClipSeq, NULL, 0, 0, 0 },
	{ CheatWeaponsFunc, CheatWeaponsSeq, NULL, 0, 0, 0 },
	{ CheatHealthFunc, CheatHealthSeq, NULL, 0, 0, 0 },
	{ CheatKeysFunc, CheatKeysSeq, NULL, 0, 0, 0 },
	{ CheatSoundFunc, CheatSoundSeq, NULL, 0, 0, 0 },
	{ CheatTickerFunc, CheatTickerSeq, NULL, 0, 0, 0 },
	{ CheatArtifactAllFunc, CheatArtifactAllSeq, NULL, 0, 0, 0 },
	{ CheatPuzzleFunc, CheatPuzzleSeq, NULL, 0, 0, 0 },
	{ CheatWarpFunc, CheatWarpSeq, NULL, 0, 0, 0 },
	{ CheatPigFunc, CheatPigSeq, NULL, 0, 0, 0 },
	{ CheatMassacreFunc, CheatMassacreSeq, NULL, 0, 0, 0 },
	{ CheatIDKFAFunc, CheatIDKFASeq, NULL, 0, 0, 0 },
	{ CheatQuickenFunc1, CheatQuickenSeq1, NULL, 0, 0, 0 },
	{ CheatQuickenFunc2, CheatQuickenSeq2, NULL, 0, 0, 0 },
	{ CheatQuickenFunc3, CheatQuickenSeq3, NULL, 0, 0, 0 },
	{ CheatClassFunc1, CheatClass1Seq, NULL, 0, 0, 0 },
	{ CheatClassFunc2, CheatClass2Seq, NULL, 0, 0, 0 },
	{ CheatInitFunc, CheatInitSeq, NULL, 0, 0, 0 },
	{ CheatVersionFunc, CheatVersionSeq, NULL, 0, 0, 0 },
	{ CheatDebugFunc, CheatDebugSeq, NULL, 0, 0, 0 },
	{ CheatScriptFunc1, CheatScriptSeq1, NULL, 0, 0, 0 },
	{ CheatScriptFunc2, CheatScriptSeq2, NULL, 0, 0, 0 },
	{ CheatScriptFunc3, CheatScriptSeq3, NULL, 0, 0, 0 },
	{ CheatRevealFunc, CheatRevealSeq, NULL, 0, 0, 0 },
	{ NULL, NULL, NULL, 0, 0, 0 } // Terminator
};
*/
// CODE --------------------------------------------------------------------

//==========================================================================
//
// SB_Init
//
//==========================================================================

void SB_Init(void)
{
	int i;
	int startLump;

	PatchNumH2BAR = gi.W_GetNumForName("H2BAR"); // PU_STATIC);
	PatchNumH2TOP = gi.W_GetNumForName("H2TOP"); // PU_STATIC);
	PatchNumINVBAR = gi.W_GetNumForName("INVBAR"); // PU_STATIC);
	PatchNumLFEDGE	= gi.W_GetNumForName("LFEDGE"); // PU_STATIC);
	PatchNumRTEDGE	= gi.W_GetNumForName("RTEDGE"); // PU_STATIC);
	PatchNumSTATBAR = gi.W_GetNumForName("STATBAR"); // PU_STATIC);
	PatchNumKEYBAR = gi.W_GetNumForName("KEYBAR"); // PU_STATIC);
	PatchNumSELECTBOX = gi.W_GetNumForName("SELECTBOX"); // PU_STATIC);
//	PatchNumARTICLEAR = gi.W_GetNumForName("ARTICLS"); // PU_STATIC);
//	PatchNumARMCLEAR = gi.W_GetNumForName("ARMCLS"); // PU_STATIC);
//	PatchNumMANACLEAR = gi.W_GetNumForName("MANACLS"); // PU_STATIC);
	PatchNumMANAVIAL1 = gi.W_GetNumForName("MANAVL1"); // PU_STATIC);
	PatchNumMANAVIAL2 = gi.W_GetNumForName("MANAVL2"); // PU_STATIC);
	PatchNumMANAVIALDIM1 = gi.W_GetNumForName("MANAVL1D"); // PU_STATIC);
	PatchNumMANAVIALDIM2 = gi.W_GetNumForName("MANAVL2D"); // PU_STATIC);
	PatchNumMANADIM1 = gi.W_GetNumForName("MANADIM1"); // PU_STATIC);
	PatchNumMANADIM2 = gi.W_GetNumForName("MANADIM2"); // PU_STATIC);
	PatchNumMANABRIGHT1 = gi.W_GetNumForName("MANABRT1"); // PU_STATIC);
	PatchNumMANABRIGHT2 = gi.W_GetNumForName("MANABRT2"); // PU_STATIC);
	PatchNumINVLFGEM1 = gi.W_GetNumForName("invgeml1"); // PU_STATIC);
	PatchNumINVLFGEM2 = gi.W_GetNumForName("invgeml2"); // PU_STATIC);
	PatchNumINVRTGEM1 = gi.W_GetNumForName("invgemr1"); // PU_STATIC);
	PatchNumINVRTGEM2 = gi.W_GetNumForName("invgemr2"); // PU_STATIC);

//	PatchCHAINBACK = W_CacheLumpName("CHAINBACK", PU_STATIC);
	startLump = gi.W_GetNumForName("IN0");
	for(i = 0; i < 10; i++)
	{
		PatchNumINumbers[i] = startLump+i;//, PU_STATIC);
	}
	PatchNumNEGATIVE = gi.W_GetNumForName("NEGNUM");//, PU_STATIC);
	FontBNumBase = gi.W_GetNumForName("FONTB16");
	startLump = gi.W_GetNumForName("SMALLIN0");
	for(i = 0; i < 10; i++)
	{
		PatchNumSmNumbers[i] = /*gi.W_CacheLumpNum(*/startLump+i/*, PU_STATIC)*/;
	}
	PlayPalette = gi.W_GetNumForName("PLAYPAL");
	SpinFlylump = gi.W_GetNumForName("SPFLY0");
	SpinMinotaurLump = gi.W_GetNumForName("SPMINO0");
	SpinSpeedLump = gi.W_GetNumForName("SPBOOT0");
	SpinDefenseLump = gi.W_GetNumForName("SPSHLD0");

/*	for(i = 0; i < 256; i++)
	{
		CheatLookup[i] = CHEAT_ENCRYPT(i);
	}*/

/*	if(deathmatch)
	{*/
		PatchNumKILLS = gi.W_GetNumForName("KILLS");//, PU_STATIC);
//	}
	SB_SetClassData();
}

//==========================================================================
//
// SB_SetClassData
//
//==========================================================================

void SB_SetClassData(void)
{
	int pclass;

	pclass = PlayerClass[consoleplayer]; // original player class (not pig)
	PatchNumWEAPONSLOT = gi.W_GetNumForName("wpslot0")+pclass;//, PU_STATIC);
	PatchNumWEAPONFULL = gi.W_GetNumForName("wpfull0")+pclass;//, PU_STATIC);
	PatchNumPIECE1 = gi.W_GetNumForName("wpiecef1")+pclass;//, PU_STATIC);
	PatchNumPIECE2 = gi.W_GetNumForName("wpiecef2")+pclass;//, PU_STATIC);
	PatchNumPIECE3 = gi.W_GetNumForName("wpiecef3")+pclass;//, PU_STATIC);
	PatchNumCHAIN =gi.W_GetNumForName("chain")+pclass;//, PU_STATIC);
	if(!netgame)
	{ // single player game uses red life gem (the second gem)
		PatchNumLIFEGEM = gi.W_GetNumForName("lifegem")+MAXPLAYERS*pclass+1;//, PU_STATIC);
	}
	else
	{
		PatchNumLIFEGEM = gi.W_GetNumForName("lifegem")+MAXPLAYERS*pclass+consoleplayer;//, PU_STATIC);
	}
	SB_state = -1;
	//UpdateState |= I_FULLSCRN;
	gi.Update(DDUF_FULLSCREEN);
}

//==========================================================================
//
// SB_Ticker
//
//==========================================================================

void SB_Ticker(void)
{
	int delta;
	int curHealth;

	curHealth = players[consoleplayer].plr->mo->health;
	if(curHealth < 0)
	{
		curHealth = 0;
	}
	if(curHealth < HealthMarker)
	{
		delta = (HealthMarker-curHealth)>>2;
		if(delta < 1)
		{
			delta = 1;
		}
		else if(delta > 6)
		{
			delta = 6;
		}
		HealthMarker -= delta;
	}
	else if(curHealth > HealthMarker)
	{
		delta = (curHealth-HealthMarker)>>2;
		if(delta < 1)
		{
			delta = 1;
		}
		else if(delta > 6)
		{
			delta = 6;
		}
		HealthMarker += delta;
	}
}

//==========================================================================
//
// DrINumber
//
// Draws a three digit number.
//
//==========================================================================

static void DrINumber(signed int val, int x, int y)
{
//	patch_t *patch;
	int oldval;

	oldval = val;
	if(val < 0)
	{
		val = -val;
		if(val > 99)
		{
			val = 99;
		}
		if(val > 9)
		{
			/*patch = PatchINumbers[val/10];
			V_DrawPatch(x+8, y, patch);
			V_DrawPatch(x, y, PatchNEGATIVE);*/
			gi.GL_DrawPatch(x+8, y, PatchNumINumbers[val/10]);
			gi.GL_DrawPatch(x, y, PatchNumNEGATIVE);
		}
		else
		{
			//V_DrawPatch(x+8, y, PatchNEGATIVE);
			gi.GL_DrawPatch(x+8, y, PatchNumNEGATIVE);
		}
		val = val%10;
		//patch = PatchINumbers[val];
		//V_DrawPatch(x+16, y, patch);
		gi.GL_DrawPatch(x+16, y, PatchNumINumbers[val]);
		return;
	}
	if(val > 99)
	{
		//patch = PatchINumbers[val/100];
		//V_DrawPatch(x, y, patch);
		gi.GL_DrawPatch(x, y, PatchNumINumbers[val/100]);
	}
	val = val%100;
	if(val > 9 || oldval > 99)
	{
		//patch = PatchINumbers[val/10];
		//V_DrawPatch(x+8, y, patch);
		gi.GL_DrawPatch(x+8, y, PatchNumINumbers[val/10]);
	}
	val = val%10;
	//patch = PatchINumbers[val];
	//V_DrawPatch(x+16, y, patch);
	gi.GL_DrawPatch(x+16, y, PatchNumINumbers[val]);
}

//==========================================================================
//
// DrRedINumber
//
// Draws a three digit number using the red font
//
//==========================================================================

static void DrRedINumber(signed int val, int x, int y)
{
//	patch_t *patch;
	int oldval;
	int offset=0;

	oldval = val;
	if(val < 0)
	{
		val = 0;
	}
	if(val > 999)
	{
		//patch = gi.W_CacheLumpNum(gi.W_GetNumForName("inred0")+val/100, PU_CACHE);
		//V_DrawPatch(x, y, patch);
		gi.GL_DrawPatch(x+offset, y, gi.W_GetNumForName("inred0")+val/1000);
		offset+=8;
	}
	if(val > 99)
	{
		//patch = gi.W_CacheLumpNum(gi.W_GetNumForName("inred0")+val/100, PU_CACHE);
		//V_DrawPatch(x, y, patch);
		gi.GL_DrawPatch(x+offset, y, gi.W_GetNumForName("inred0")+(val%1000)/100);
	}
	val = val%100;
	if(val > 9 || oldval > 99)
	{
		//patch = gi.W_CacheLumpNum(gi.W_GetNumForName("inred0")+val/10, PU_CACHE);
		//V_DrawPatch(x+8, y, patch);
		gi.GL_DrawPatch(x+8+offset, y, gi.W_GetNumForName("inred0")+val/10);
	}
	val = val%10;
	//patch = gi.W_CacheLumpNum(gi.W_GetNumForName("inred0")+val, PU_CACHE);
	//V_DrawPatch(x+16, y, patch);
	gi.GL_DrawPatch(x+16+offset, y, gi.W_GetNumForName("inred0")+val);
}

//==========================================================================
//
// DrBNumber
//
// Draws a four digit number using FontB
//
//==========================================================================

static void DrBNumber(signed int val, int x, int y)
{
	patch_t *patch;
	int xpos;
	int oldval;

	oldval = val;
	xpos = x;
	if(val < 0)
	{
		val = 0;
	}
	if(val > 999)
	{
		patch = (patch_t *)gi.W_CacheLumpNum(FontBNumBase+val/1000, PU_CACHE);
		//V_DrawShadowedPatch(xpos+6-patch->width/2, y, patch);
		gi.GL_DrawShadowedPatch(xpos+6-patch->width/2, y, FontBNumBase+val/1000);
	}
	val = val%1000;
	xpos += 12;
	if(val > 99 || oldval > 999)
	{
		patch = (patch_t *)gi.W_CacheLumpNum(FontBNumBase+val/100, PU_CACHE);
		//V_DrawShadowedPatch(xpos+6-patch->width/2, y, patch);
		gi.GL_DrawShadowedPatch(xpos+6-patch->width/2, y, FontBNumBase+val/100);
	}
	val = val%100;
	xpos += 12;
	if(val > 9 || oldval > 99)
	{
		patch = (patch_t *)gi.W_CacheLumpNum(FontBNumBase+val/10, PU_CACHE);
		//V_DrawShadowedPatch(xpos+6-patch->width/2, y, patch);
		gi.GL_DrawShadowedPatch(xpos+6-patch->width/2, y, FontBNumBase+val/10);
	}
	val = val%10;
	xpos += 12;
	patch = (patch_t *)gi.W_CacheLumpNum(FontBNumBase+val, PU_CACHE);
	//V_DrawShadowedPatch(xpos+6-patch->width/2, y, patch);
	gi.GL_DrawShadowedPatch(xpos+6-patch->width/2, y, FontBNumBase+val);
}

//==========================================================================
//
// DrSmallNumber
//
// Draws a small two digit number.
//
//==========================================================================

static void DrSmallNumber(int val, int x, int y)
{
//	patch_t *patch;

	if(val <= 0)
	{
		return;
	}
	if(val > 999)
	{
		val %= 1000;
	}
	if(val > 99)
	{
		//patch = PatchSmNumbers[val/100];
		//V_DrawPatch(x, y, patch);
		gi.GL_DrawPatch(x, y, PatchNumSmNumbers[val/100]);
		//patch = PatchSmNumbers[(val%100)/10];
		//V_DrawPatch(x+4, y, patch);
		gi.GL_DrawPatch(x+4, y, PatchNumSmNumbers[(val%100)/10]);
	}
	else if(val > 9)
	{
		//patch = PatchSmNumbers[val/10];
		//V_DrawPatch(x+4, y, patch);
		gi.GL_DrawPatch(x+4, y, PatchNumSmNumbers[val/10]);
	}
	val %= 10;
	//patch = PatchSmNumbers[val];
	//V_DrawPatch(x+8, y, patch);
	gi.GL_DrawPatch(x+8, y, PatchNumSmNumbers[val]);
}

/*
//==========================================================================
//
// ShadeLine
//
//==========================================================================

static void ShadeLine(int x, int y, int height, int shade)
{
	byte *dest;
	byte *shades;

	shades = colormaps+9*256+shade*2*256;
	dest = memscreen+y*SCREENWIDTH+x;
	while(height--)
	{
		*(dest) = *(shades+*dest);
		dest += SCREENWIDTH;
	}
}

//==========================================================================
//
// ShadeChain
//
//==========================================================================

static void ShadeChain(void)
{
	int i;

	for(i = 0; i < 16; i++)
	{
		ShadeLine(277+i, 190, 10, i/2);
		ShadeLine(19+i, 190, 10, 7-(i/2));
	}
}
*/

//==========================================================================
//
// DrawSoundInfo
//
// Displays sound debugging information.
//
//==========================================================================

static void DrawSoundInfo(void)
{
	int i;
	SoundInfo_t s;
	ChanInfo_t *c;
	char text[32];
	int x;
	int y;
	int xPos[7] = {1, 75, 112, 156, 200, 230, 260};

	if(leveltime&16)
	{
		MN_DrTextA("*** SOUND DEBUG INFO ***", xPos[0], 20);
	}
	S_GetChannelInfo(&s);
	if(s.channelCount == 0)
	{
		return;
	}
	x = 0;
	MN_DrTextA("NAME", xPos[x++], 30);
	MN_DrTextA("MO.T", xPos[x++], 30);
	MN_DrTextA("MO.X", xPos[x++], 30);
	MN_DrTextA("MO.Y", xPos[x++], 30);
	MN_DrTextA("ID", xPos[x++], 30);
	MN_DrTextA("PRI", xPos[x++], 30);
	MN_DrTextA("DIST", xPos[x++], 30);
	for(i = 0; i < s.channelCount; i++)
	{
		c = &s.chan[i];
		x = 0;
		y = 40+i*10;
		if(c->mo == NULL)
		{ // Channel is unused
			MN_DrTextA("------", xPos[0], y);
			continue;
		}
		sprintf(text, "%s", c->name);
		//M_ForceUppercase(text);
		strupr(text);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->mo->type);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->mo->x>>FRACBITS);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->mo->y>>FRACBITS);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->id);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", S_sfx[c->id].usefulness);//c->mo->thinker.function);//c->priority);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->distance);
		//MN_DrTextA(text, xPos[x++], y);
	}
	/*UpdateState |= I_FULLSCRN;
	BorderNeedRefresh = true;*/
	gi.Update(DDUF_FULLSCREEN | DDUF_BORDER);
}

//==========================================================================
//
// SB_Drawer
//
//==========================================================================

char patcharti[][10] =
{
	{ "ARTIBOX" },    	// none
	{ "ARTIINVU" },   	// invulnerability
	{ "ARTIPTN2" },   	// health
	{ "ARTISPHL" },   	// superhealth
	{ "ARTIHRAD" },		// healing radius
	{ "ARTISUMN" },   	// summon maulator
	{ "ARTITRCH" },   	// torch
	{ "ARTIPORK" },   	// egg
	{ "ARTISOAR" },   	// fly
	{ "ARTIBLST" },		// blast radius
	{ "ARTIPSBG" },		// poison bag
	{ "ARTITELO" },		// teleport other
	{ "ARTISPED" },  	// speed
	{ "ARTIBMAN" },		// boost mana
	{ "ARTIBRAC" },		// boost armor
	{ "ARTIATLP" },   	// teleport
	{ "ARTISKLL" },		// arti_puzzskull
	{ "ARTIBGEM" },		// arti_puzzgembig
	{ "ARTIGEMR" },		// arti_puzzgemred
	{ "ARTIGEMG" },		// arti_puzzgemgreen1
	{ "ARTIGMG2" },		// arti_puzzgemgreen2
	{ "ARTIGEMB" },		// arti_puzzgemblue1
	{ "ARTIGMB2" },		// arti_puzzgemblue2
	{ "ARTIBOK1" },		// arti_puzzbook1
	{ "ARTIBOK2" },		// arti_puzzbook2
	{ "ARTISKL2" },		// arti_puzzskull2
	{ "ARTIFWEP" },		// arti_puzzfweapon
	{ "ARTICWEP" },		// arti_puzzcweapon
	{ "ARTIMWEP" },		// arti_puzzmweapon
	{ "ARTIGEAR" },		// arti_puzzgear1
	{ "ARTIGER2" },		// arti_puzzgear2
	{ "ARTIGER3" },		// arti_puzzgear3
	{ "ARTIGER4" },		// arti_puzzgear4
};

int SB_state = -1;
static int oldarti = 0;
static int oldartiCount = 0;
static int oldfrags = -9999;
static int oldmana1 = -1;
static int oldmana2 = -1;
static int oldarmor = -1;
static int oldhealth = -1;
static int oldlife = -1;
static int oldpieces = -1;
static int oldweapon = -1;
static int oldkeys = -1;

extern boolean automapactive;

void SB_Drawer(void)
{
	// Sound info debug stuff
	if(DebugSound == true)
	{
		DrawSoundInfo();
	}
	CPlayer = &players[consoleplayer];
	if((gi.Get(DD_VIEWWINDOW_HEIGHT) == SCREENHEIGHT
		|| CPlayer->pclass >=PCLASS_ETTIN) //Remi: Possessed monsters have forced fullscreen		
		&& !automapactive
		
#ifdef DEMOCAM 
		&& (demoplayback && democam.mode)
#endif
		)
	{
		DrawFullScreenStuff();
//		SB_state = -1;
	}
	else
	{
		float fscale = sbarscale/20.0f;
		// Setup special status bar matrix.
		if(sbarscale != 20)
		{
			gl.MatrixMode(DGL_MODELVIEW);
			gl.PushMatrix();
			gl.Translatef(160 - 320*fscale/2, 200*(1-fscale), 0);
			gl.Scalef(fscale, fscale, 1);
		}
		
/*		if(SB_state == -1)
		{*/
			//V_DrawPatch(0, 134, PatchH2BAR);
			gi.GL_DrawPatch(0, 134, PatchNumH2BAR);
/*			oldhealth = -1;
		}*/
		DrawCommonBar();
     	if (CPlayer->pclass<PCLASS_ETTIN) DrRedINumber(CPlayer->sp_power, 280 , 13);
		if(!inventory)
		{
/*			if(SB_state != 0)
			{*/
				// Main interface
				if(!automapactive)
				{
					//V_DrawPatch(38, 162, PatchSTATBAR);
					gi.GL_DrawPatch(38, 162, PatchNumSTATBAR);
				}
				else
				{
					//V_DrawPatch(38, 162, PatchKEYBAR);
					gi.GL_DrawPatch(38, 162, PatchNumKEYBAR);
				}
/*				oldarti = 0;
				oldmana1 = -1;
				oldmana2 = -1;
				oldarmor = -1;
				oldpieces = -1;
				oldfrags = -9999; //can't use -1, 'cuz of negative frags
				oldlife = -1;
				oldweapon = -1;
				oldkeys = -1;*/
			//}
			if(!automapactive)
			{
				DrawMainBar();
			}
			else
			{
				DrawKeyBar();
			}
			//SB_state = 0;
		}
		else
		{
			DrawInventoryBar();
			//SB_state = 1;
		}
		// Restore the old modelview matrix.
		if(sbarscale != 20)
		{
			gl.MatrixMode(DGL_MODELVIEW);
			gl.PopMatrix();
		}
	}
	SB_PaletteFlash(false);
	DrawAnimatedIcons();
}

//==========================================================================
//
// DrawAnimatedIcons
//
//==========================================================================

static void DrawAnimatedIcons(void)
{
	int leftoff = 0;
	int frame;
	static boolean hitCenterFrame;
	extern int screenblocks;

	// If the fullscreen mana is drawn, we need to move the icons on the left
	// a bit to the right.
	if(showFullscreenMana==1 && screenblocks>10) leftoff = 42;

	// Wings of wrath
	if(CPlayer->powers[pw_flight])
	{
		if(CPlayer->powers[pw_flight] > BLINKTHRESHOLD
			|| !(CPlayer->powers[pw_flight]&16))
		{
			frame = (leveltime/3)&15;
			if(CPlayer->plr->mo->flags2&MF2_FLY)
			{
				if(hitCenterFrame && (frame != 15 && frame != 0))
				{
					/*V_DrawPatch(20, 19, gi.W_CacheLumpNum(SpinFlylump+15, 
						PU_CACHE));*/
					gi.GL_DrawPatch(20+leftoff, 19, SpinFlylump+15);
				}
				else
				{
					/*V_DrawPatch(20, 19, gi.W_CacheLumpNum(SpinFlylump+frame, 
						PU_CACHE));*/
					gi.GL_DrawPatch(20+leftoff, 19, SpinFlylump+frame);
					hitCenterFrame = false;
				}
			}
			else
			{
				if(!hitCenterFrame && (frame != 15 && frame != 0))
				{
					/*V_DrawPatch(20, 19, gi.W_CacheLumpNum(SpinFlylump+frame, 
						PU_CACHE));*/
					gi.GL_DrawPatch(20+leftoff, 19, SpinFlylump+frame);
					hitCenterFrame = false;
				}
				else
				{
					/*V_DrawPatch(20, 19, gi.W_CacheLumpNum(SpinFlylump+15, 
						PU_CACHE));*/
					gi.GL_DrawPatch(20+leftoff, 19, SpinFlylump+15);
					hitCenterFrame = true;
				}
			}
		}
		/*BorderTopRefresh = true;
		UpdateState |= I_MESSAGES;*/
		gi.Update(DDUF_TOP | DDUF_MESSAGES);
	}

	// Speed Boots
	if(CPlayer->powers[pw_speed])
	{
		if(CPlayer->powers[pw_speed] > BLINKTHRESHOLD
			|| !(CPlayer->powers[pw_speed]&16))
		{
			frame = (leveltime/3)&15;
			/*V_DrawPatch(60, 19, gi.W_CacheLumpNum(SpinSpeedLump+frame, 
				PU_CACHE));*/
			gi.GL_DrawPatch(60+leftoff, 19, SpinSpeedLump+frame);
		}
		/*BorderTopRefresh = true;
		UpdateState |= I_MESSAGES;*/
		gi.Update(DDUF_TOP | DDUF_MESSAGES);
	}

	// Defensive power
	if(CPlayer->powers[pw_invulnerability])
	{
		if(CPlayer->powers[pw_invulnerability] > BLINKTHRESHOLD
			|| !(CPlayer->powers[pw_invulnerability]&16))
		{
			frame = (leveltime/3)&15;
			/*V_DrawPatch(260, 19, gi.W_CacheLumpNum(SpinDefenseLump+frame, 
				PU_CACHE));*/
			gi.GL_DrawPatch(260, 19, SpinDefenseLump+frame);
		}
		/*BorderTopRefresh = true;
		UpdateState |= I_MESSAGES;*/
		gi.Update(DDUF_TOP | DDUF_MESSAGES);
	}

	// Minotaur Active
	if(CPlayer->powers[pw_minotaur])
	{
		if(CPlayer->powers[pw_minotaur] > BLINKTHRESHOLD
			|| !(CPlayer->powers[pw_minotaur]&16))
		{
			frame = (leveltime/3)&15;
			/*V_DrawPatch(300, 19, gi.W_CacheLumpNum(SpinMinotaurLump+frame, 
				PU_CACHE));*/
			gi.GL_DrawPatch(300, 19, SpinMinotaurLump+frame);
		}
		/*BorderTopRefresh = true;
		UpdateState |= I_MESSAGES;*/
		gi.Update(DDUF_TOP | DDUF_MESSAGES);
	}
}

//==========================================================================
//
// SB_PaletteFlash
//
// Sets the new palette based upon the current values of
// consoleplayer->damagecount and consoleplayer->bonuscount.
//
//==========================================================================

void SB_PaletteFlash(boolean forceChange)
{
	static int sb_palette = 0;
	int palette;

	if(forceChange)
	{ 
		sb_palette = -1;
	}
	if(gamestate == GS_LEVEL)
	{
		CPlayer = &players[consoleplayer]; //Possessed Iceguy's view
		if(CPlayer->pclass == PCLASS_ICEGUY)
			palette = STARTICEPAL;
		else if (CPlayer->berserkTics)
			palette = STARTBONUSPALS;
		else if(CPlayer->poisoncount)
		{
			//palette = 0;
			palette = (CPlayer->poisoncount+7)>>3;
			if(palette >= NUMPOISONPALS)
			{
				palette = NUMPOISONPALS-1;
			}
			palette += STARTPOISONPALS;
		}
		else if(CPlayer->damagecount)
		{
			palette = (CPlayer->damagecount+7)>>3;
			if(palette >= NUMREDPALS)
			{
				palette = NUMREDPALS-1;
			}
			palette += STARTREDPALS;
		}
		else if(CPlayer->bonuscount)
		{
			palette = (CPlayer->bonuscount+7)>>3;
			if(palette >= NUMBONUSPALS)
			{
				palette = NUMBONUSPALS-1;
			}
			palette += STARTBONUSPALS;
		}
		else if(CPlayer->plr->mo->flags2&MF2_ICEDAMAGE)
		{ // Frozen player
			palette = STARTICEPAL;
		}
		else
		{
			palette = 0;
		}
	}
	else
	{
		palette = 0;
	}
	if(palette != sb_palette)
	{
		sb_palette = palette;
		/*pal = (byte *)gi.W_CacheLumpNum(PlayPalette, PU_CACHE)+palette*768;
		I_SetPalette(pal);*/
		H2_SetFilter(palette);
	}
}

//==========================================================================
//
// DrawCommonBar
//
//==========================================================================

void DrawCommonBar(void)
{
	int healthPos;

//	V_DrawPatch(0, 134, PatchH2TOP);
	gi.GL_DrawPatch(0, 134, PatchNumH2TOP);

/*	if(oldhealth != HealthMarker)
	{
		oldhealth = HealthMarker;*/
		healthPos = HealthMarker*100/players[consoleplayer].maxhealth;
		if(healthPos < 0)
		{
			healthPos = 0;
		}
		if(healthPos > 100)
		{
			healthPos = 100;
		}
		/*V_DrawPatch(28+(((healthPos*196)/100)%9), 193, PatchCHAIN);
		V_DrawPatch(7+((healthPos*11)/5), 193, PatchLIFEGEM);
		V_DrawPatch(0, 193, PatchLFEDGE);
		V_DrawPatch(277, 193, PatchRTEDGE);*/
		gi.GL_DrawPatch(28+(((healthPos*196)/100)%9), 193, PatchNumCHAIN);
		gi.GL_DrawPatch(7+((healthPos*11)/5), 193, PatchNumLIFEGEM);
		gi.GL_DrawPatch(0, 193, PatchNumLFEDGE);
		gi.GL_DrawPatch(277, 193, PatchNumRTEDGE);
//		ShadeChain();
/*		UpdateState |= I_STATBAR;
	}*/
}

//==========================================================================
//
// DrawMainBar
//
//==========================================================================

void DrawMainBar(void)
{
	int i;
	int temp;
/*	patch_t *manaPatch1, *manaPatch2;
	patch_t *manaVialPatch1, *manaVialPatch2;*/
	int manaPatchNum1, manaPatchNum2, manaVialPatchNum1, manaVialPatchNum2;

/*	manaPatch1 = NULL;
	manaPatch2 = NULL;
	manaVialPatch1 = NULL;
	manaVialPatch2 = NULL;*/
	manaPatchNum1 = manaPatchNum2 = manaVialPatchNum1 = manaVialPatchNum2 = -1;

	// Ready artifact
	if(ArtifactFlash)
	{
		//V_DrawPatch(144, 160, PatchARTICLEAR);
//		gi.GL_DrawPatch(144, 160, PatchNumARTICLEAR);
		/*V_DrawPatch(148, 164, gi.W_CacheLumpNum(gi.W_GetNumForName("useartia")
			+ ArtifactFlash - 1, PU_CACHE));*/
		gi.GL_DrawPatch(148, 164, gi.W_GetNumForName("useartia")+ArtifactFlash-1);
		ArtifactFlash--;
		oldarti = -1; // so that the correct artifact fills in after the flash
		//UpdateState |= I_STATBAR;
		gi.Update(DDUF_STATBAR);
	}
	else if(oldarti != CPlayer->readyArtifact
		|| oldartiCount != CPlayer->inventory[inv_ptr].count)
	{
		//V_DrawPatch(144, 160, PatchARTICLEAR);
//		gi.GL_DrawPatch(144, 160, PatchNumARTICLEAR);
		if(CPlayer->readyArtifact > 0)
		{
			/*V_DrawPatch(143, 163, 
				W_CacheLumpName(patcharti[CPlayer->readyArtifact], PU_CACHE));*/
			gi.GL_DrawPatch(143, 163, gi.W_GetNumForName(patcharti[CPlayer->readyArtifact]));
			if(CPlayer->inventory[inv_ptr].count > 1)
			{
				DrSmallNumber(CPlayer->inventory[inv_ptr].count, 162, 184);
			}
		}
//		oldarti = CPlayer->readyArtifact;
		//oldartiCount = CPlayer->inventory[inv_ptr].count;
		//UpdateState |= I_STATBAR;
	}

	// Frags
	if(deathmatch)
	{
		temp = 0;
		for(i = 0; i < MAXPLAYERS; i++)
		{
			temp += CPlayer->plr->frags[i];
		}
		/*if(temp != oldfrags)
		{*/
			//V_DrawPatch(38, 162, PatchKILLS);
		if (PatchNumKILLS!=0) gi.GL_DrawPatch(38, 162, PatchNumKILLS);
			DrINumber(temp, 40, 176);
			//oldfrags = temp;
			//UpdateState |= I_STATBAR;
		//}
	}
	else
	{
		temp = HealthMarker;
		if(temp < 0)
		{
			temp = 0;
		}
		else if(temp > players[consoleplayer].maxhealth)
		{
			temp = players[consoleplayer].maxhealth;
		}
		/*if(oldlife != temp)
		{
			oldlife = temp;*/
			//V_DrawPatch(41, 178, PatchARMCLEAR);
			//gi.GL_DrawPatch(41, 178, PatchNumARMCLEAR);
			if(temp >= 25)
			{
				DrINumber(temp, 40, 176);
			}
			else
			{
				DrRedINumber(temp, 40, 176);
			}
			//UpdateState |= I_STATBAR;
		//}
	}
	// Mana
	temp = CPlayer->mana[0];
/*	if(oldmana1 != temp)
	{*/
		//V_DrawPatch(77, 178, PatchMANACLEAR);
		//gi.GL_DrawPatch(77, 178, PatchNumMANACLEAR);
		DrSmallNumber(temp, 79, 181);
		//manaVialPatch1 = (patch_t *)1; // force a vial update
		if(temp == 0)
		{ // Draw Dim Mana icon
			manaPatchNum1 = PatchNumMANADIM1;
		}
/*		else //if(oldmana1 == 0)
		{
			manaPatchNum1 = PatchNumMANABRIGHT1;
		}		*/
		//oldmana1 = temp;
		//UpdateState |= I_STATBAR;
		gi.Update(DDUF_STATBAR);
//	}
	temp = CPlayer->mana[1];
	//if(oldmana2 != temp)
	//{
		//V_DrawPatch(109, 178, PatchMANACLEAR);
		DrSmallNumber(temp, 111, 181);
		//manaVialPatch1 = (patch_t *)1; // force a vial update
		if(temp == 0)
		{ // Draw Dim Mana icon
			manaPatchNum2 = PatchNumMANADIM2;
		}		
		/*else //if(oldmana2 == 0)
		{
			manaPatchNum2 = PatchNumMANABRIGHT2;
		}*/		
		//oldmana2 = temp;
		//UpdateState |= I_STATBAR;
		gi.Update(DDUF_STATBAR);
//	}
/*	if(oldweapon != CPlayer->readyweapon || manaPatch1 || manaPatch2
		|| manaVialPatch1)
	{ // Update mana graphics based upon mana count/weapon type */
		if(NewWeaponInfo[CPlayer->readyweapon].mana == MANA_NONE)
		{
			manaPatchNum1 = PatchNumMANADIM1;
			manaPatchNum2 = PatchNumMANADIM2;
			manaVialPatchNum1 = PatchNumMANAVIALDIM1;
			manaVialPatchNum2 = PatchNumMANAVIALDIM2;
		}
		else if(NewWeaponInfo[CPlayer->readyweapon].mana == MANA_1)
		{
			// If there is mana for this weapon, make it bright!
			if(manaPatchNum1 == -1)
			{
				manaPatchNum1 = PatchNumMANABRIGHT1;
			}
			manaVialPatchNum1 = PatchNumMANAVIAL1;
			manaPatchNum2 = PatchNumMANADIM2;
			manaVialPatchNum2 = PatchNumMANAVIALDIM2;
		}
		else if(NewWeaponInfo[CPlayer->readyweapon].mana == MANA_2)
		{
			manaPatchNum1 = PatchNumMANADIM1;
			manaVialPatchNum1 = PatchNumMANAVIALDIM1;
			// If there is mana for this weapon, make it bright!
			if(manaPatchNum2 == -1)
			{
				manaPatchNum2 = PatchNumMANABRIGHT2;
			}
			manaVialPatchNum2 = PatchNumMANAVIAL2;
		}
		else
		{
			manaVialPatchNum1 = PatchNumMANAVIAL1;
			manaVialPatchNum2 = PatchNumMANAVIAL2;
			// If there is mana for this weapon, make it bright!
			if(manaPatchNum1 == -1)
			{
				manaPatchNum1 = PatchNumMANABRIGHT1;
			}
			if(manaPatchNum2 == -1)
			{
				manaPatchNum2 = PatchNumMANABRIGHT2;
			}
		}
/*		V_DrawPatch(77, 164, manaPatch1);
		V_DrawPatch(110, 164, manaPatch2);
		V_DrawPatch(94, 164, manaVialPatch1);*/
		gi.GL_DrawPatch(77, 164, manaPatchNum1);
		gi.GL_DrawPatch(110, 164, manaPatchNum2);
		gi.GL_DrawPatch(94, 164, manaVialPatchNum1);
		gi.GL_DrawPatch(102, 164, manaVialPatchNum2);

		/*for(i = 165; i < 187-(22*CPlayer->mana[0])/MAX_MANA; i++)
		{
			memscreen[i*SCREENWIDTH+95] = 0;
			memscreen[i*SCREENWIDTH+96] = 0;
			memscreen[i*SCREENWIDTH+97] = 0;
		}*/
		//V_DrawPatch(102, 164, manaVialPatch2);
		gi.GL_SetNoTexture();
		gi.GL_DrawRect(95, 165, 3, 22-(22*CPlayer->mana[0])/MAX_MANA, 0,0,0,1);
		gi.GL_DrawRect(103, 165, 3, 22-(22*CPlayer->mana[1])/MAX_MANA, 0,0,0,1);
		
		/*for(i = 165; i < 187-(22*CPlayer->mana[1])/MAX_MANA; i++)
		{
			memscreen[i*SCREENWIDTH+103] = 0;
			memscreen[i*SCREENWIDTH+104] = 0;
			memscreen[i*SCREENWIDTH+105] = 0;
		}*/
		//oldweapon = CPlayer->readyweapon;
		//UpdateState |= I_STATBAR;
		gi.Update(DDUF_STATBAR);
	//}
	// Armor
	temp = AutoArmorSave[CPlayer->pclass]
		+CPlayer->armorpoints[ARMOR_ARMOR]+CPlayer->armorpoints[ARMOR_SHIELD]
		+CPlayer->armorpoints[ARMOR_HELMET]+CPlayer->armorpoints[ARMOR_AMULET];
/*	if(oldarmor != temp)
	{
		oldarmor = temp;*/
		//V_DrawPatch(255, 178, PatchARMCLEAR);
		//gi.GL_DrawPatch(255, 178, PatchNumARMCLEAR);
		DrINumber(FixedDiv(temp, 5*FRACUNIT)>>FRACBITS, 250, 176);
		//UpdateState |= I_STATBAR;
	//}
	// Weapon Pieces
/*	if(oldpieces != CPlayer->pieces)
	{*/
		DrawWeaponPieces();
		//oldpieces = CPlayer->pieces;
		//UpdateState |= I_STATBAR;
	//}
}

//==========================================================================
//
// DrawInventoryBar
//
//==========================================================================

void DrawInventoryBar(void)
{
	int i;
	int x;

	x = inv_ptr-curpos;
//	UpdateState |= I_STATBAR;
	//V_DrawPatch(38, 162, PatchINVBAR);
	gi.GL_DrawPatch(38, 162, PatchNumINVBAR);
	for(i = 0; i < 7; i++)
	{
		//V_DrawPatch(50+i*31, 160, W_CacheLumpName("ARTIBOX", PU_CACHE));
		if(CPlayer->inventorySlotNum > x+i
			&& CPlayer->inventory[x+i].type != arti_none)
		{
			/*V_DrawPatch(50+i*31, 163, W_CacheLumpName(
				patcharti[CPlayer->inventory[x+i].type], PU_CACHE));*/
			gi.GL_DrawPatch(50+i*31, 163, gi.W_GetNumForName(
				patcharti[CPlayer->inventory[x+i].type]));
			if(CPlayer->inventory[x+i].count > 1)
			{
				DrSmallNumber(CPlayer->inventory[x+i].count, 68+i*31, 185);
			}
		}
	}
	//V_DrawPatch(50+curpos*31, 163, PatchSELECTBOX);
	gi.GL_DrawPatch(50+curpos*31, 163, PatchNumSELECTBOX);
	if(x != 0)
	{
	/*	V_DrawPatch(42, 163, !(leveltime&4) ? PatchINVLFGEM1 :
			PatchINVLFGEM2);*/
		gi.GL_DrawPatch(42, 163, !(leveltime&4) ? PatchNumINVLFGEM1 :
			PatchNumINVLFGEM2);
	}
	if(CPlayer->inventorySlotNum-x > 7)
	{
		/*V_DrawPatch(269, 163, !(leveltime&4) ? PatchINVRTGEM1 :
			PatchINVRTGEM2);*/
		gi.GL_DrawPatch(269, 163, !(leveltime&4) ? PatchNumINVRTGEM1 :
			PatchNumINVRTGEM2);
	}
}

//==========================================================================
//
// DrawKeyBar
//
//==========================================================================

void DrawKeyBar(void)
{
	int i;
	int xPosition;
	int temp;

/*	if(oldkeys != CPlayer->keys)
	{*/
		xPosition = 46;
		for(i = 0; i < NUMKEYS && xPosition <= 126; i++)
		{
			if(CPlayer->keys&(1<<i))
			{
				/*V_DrawPatch(xPosition, 164, 
					gi.W_CacheLumpNum(gi.W_GetNumForName("keyslot1")+i, PU_CACHE));*/
				gi.GL_DrawPatch(xPosition, 163, gi.W_GetNumForName("keyslot1")+i);
				xPosition += 20;
			}
		}
/*		oldkeys = CPlayer->keys;
		UpdateState |= I_STATBAR;
	}*/
	temp = AutoArmorSave[CPlayer->pclass]
		+CPlayer->armorpoints[ARMOR_ARMOR]+CPlayer->armorpoints[ARMOR_SHIELD]
		+CPlayer->armorpoints[ARMOR_HELMET]+CPlayer->armorpoints[ARMOR_AMULET];
/*	if(oldarmor != temp)
	{*/
		for(i = 0; i < NUMARMOR; i++)
		{
			if(!CPlayer->armorpoints[i])
			{
				continue;
			}
			if(CPlayer->armorpoints[i] <= 
				(ArmorIncrement[CPlayer->pclass][i]>>2))
			{
				/*V_DrawFuzzPatch(150+31*i, 164, 
					gi.W_CacheLumpNum(gi.W_GetNumForName("armslot1")+i, PU_CACHE));*/
				gi.GL_DrawFuzzPatch(150+31*i, 164, 
					gi.W_GetNumForName("armslot1")+i);
			}
			else if(CPlayer->armorpoints[i] <= 
				(ArmorIncrement[CPlayer->pclass][i]>>1))
			{
				/*V_DrawAltFuzzPatch(150+31*i, 164, 
					gi.W_CacheLumpNum(gi.W_GetNumForName("armslot1")+i, PU_CACHE));*/
				gi.GL_DrawAltFuzzPatch(150+31*i, 164, 
					gi.W_GetNumForName("armslot1")+i);
			}
			else
			{
			/*	V_DrawPatch(150+31*i, 164, 
					gi.W_CacheLumpNum(gi.W_GetNumForName("armslot1")+i, PU_CACHE));*/
				gi.GL_DrawPatch(150+31*i, 164, gi.W_GetNumForName("armslot1")+i);
			}
		}
		/*oldarmor = temp;
		UpdateState |= I_STATBAR;
	}*/
}

//==========================================================================
//
// DrawWeaponPieces
//
//==========================================================================

static int PieceX[NUMCLASSES][3] = 
{
	{ 190, 225, 234 },
	{ 190, 212, 225 },
	{ 190, 205, 224 },
	{ 0, 0, 0 },		// Corvus neither
	{ 0, 0, 0 }			// Pig is never used
};

static void DrawWeaponPieces(void)
{
	if(CPlayer->pieces == 7)
	{
		//V_DrawPatch(190, 162, PatchWEAPONFULL);
		gi.GL_DrawPatch(190, 162, PatchNumWEAPONFULL);
		return;
	}
	//V_DrawPatch(190, 162, PatchWEAPONSLOT);
	gi.GL_DrawPatch(190, 162, PatchNumWEAPONSLOT);
	if(CPlayer->pieces&WPIECE1)
	{
		//V_DrawPatch(PieceX[PlayerClass[consoleplayer]][0], 162, PatchPIECE1);
		gi.GL_DrawPatch(PieceX[PlayerClass[consoleplayer]][0], 162, PatchNumPIECE1);
	}
	if(CPlayer->pieces&WPIECE2)
	{
		//V_DrawPatch(PieceX[PlayerClass[consoleplayer]][1], 162, PatchPIECE2);
		gi.GL_DrawPatch(PieceX[PlayerClass[consoleplayer]][1], 162, PatchNumPIECE2);
	}
	if(CPlayer->pieces&WPIECE3)
	{
		//V_DrawPatch(PieceX[PlayerClass[consoleplayer]][2], 162, PatchPIECE3);
		gi.GL_DrawPatch(PieceX[PlayerClass[consoleplayer]][2], 162, PatchNumPIECE3);
	}
}

//==========================================================================
//
// DrawFullScreenStuff
//
//==========================================================================

void DrawFullScreenStuff(void)
{
	int i;
	int x;
	int temp;
	
#ifdef DEMOCAM
	if(demoplayback && democam.mode) return;
#endif

//	UpdateState |= I_FULLSCRN;
	if(CPlayer->plr->mo->health > 0)
	{
		DrBNumber(CPlayer->plr->mo->health, 5, 180);
	}
	else
	{
		DrBNumber(0, 5, 180);
	}

	if (CPlayer->pclass <PCLASS_ETTIN) 
		DrRedINumber(CPlayer->sp_power, 280 , 13);

	if(showFullscreenMana && CPlayer->pclass <PCLASS_CORVUS)
	{
		int dim[2] = { PatchNumMANADIM1, PatchNumMANADIM2 };
		int bright[2] = { PatchNumMANABRIGHT1, PatchNumMANABRIGHT2 };
		int patches[2] = { 0, 0 };
		int ypos = showFullscreenMana==2? 152 : 2;
		for(i=0; i<2; i++) if(CPlayer->mana[i] == 0) patches[i] = dim[i];		
		if(NewWeaponInfo[CPlayer->readyweapon].mana == MANA_NONE)
		{
			for(i=0; i<2; i++) patches[i] = dim[i];
		}
		if(NewWeaponInfo[CPlayer->readyweapon].mana == MANA_1)
		{
			if(!patches[0]) patches[0] = bright[0];
			patches[1] = dim[1];
		}
		if(NewWeaponInfo[CPlayer->readyweapon].mana == MANA_2)
		{
			patches[0] = dim[0];
			if(!patches[1]) patches[1] = bright[1];
		}
		if(NewWeaponInfo[CPlayer->readyweapon].mana == MANA_BOTH)
		{
			for(i=0; i<2; i++) if(!patches[i]) patches[i] = bright[i];
		}
		for(i=0; i<2; i++)
		{
			gi.GL_DrawPatch(2, ypos + i*13, patches[i]);
			DrINumber(CPlayer->mana[i], 18, ypos + i*13);
		}
	}

	//DrBNumber(viewpitch, 70, 180);
	if(deathmatch)
	{
		temp = 0;
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(players[i].plr->ingame)
			{
				temp += CPlayer->plr->frags[i];
			}
		}
		DrINumber(temp, 45, 185);
	}
	if(!inventory && CPlayer->pclass <PCLASS_ETTIN)
	{
		if(CPlayer->readyArtifact > 0)
		{
			/*V_DrawFuzzPatch(286, 170, W_CacheLumpName("ARTIBOX",
				PU_CACHE));*/
			gi.GL_DrawFuzzPatch(286, 170, gi.W_GetNumForName("ARTIBOX"));
			/*V_DrawPatch(284, 169,
				W_CacheLumpName(patcharti[CPlayer->readyArtifact], PU_CACHE));*/
			gi.GL_DrawPatch(284, 169,
				gi.W_GetNumForName(patcharti[CPlayer->readyArtifact]));
			if(CPlayer->inventory[inv_ptr].count > 1)
			{
				DrSmallNumber(CPlayer->inventory[inv_ptr].count, 302, 192);
			}
		}
	}
	else if(CPlayer->pclass <PCLASS_ETTIN)
	{
		x = inv_ptr-curpos;
		for(i = 0; i < 7; i++)
		{
			/*V_DrawFuzzPatch(50+i*31, 168, W_CacheLumpName("ARTIBOX",
				PU_CACHE));*/
			gi.GL_DrawFuzzPatch(50+i*31, 168, gi.W_GetNumForName("ARTIBOX"));
			if(CPlayer->inventorySlotNum > x+i
				&& CPlayer->inventory[x+i].type != arti_none)
			{
			//	V_DrawPatch(49+i*31, 167, W_CacheLumpName(
					//patcharti[CPlayer->inventory[x+i].type], PU_CACHE));
				gi.GL_DrawPatch(49+i*31, 167, gi.W_GetNumForName(
					patcharti[CPlayer->inventory[x+i].type]));//, PU_CACHE));

				if(CPlayer->inventory[x+i].count > 1)
				{
					DrSmallNumber(CPlayer->inventory[x+i].count, 66+i*31,
 						188);
				}
			}
		}
		//V_DrawPatch(50+curpos*31, 167, PatchSELECTBOX);
		gi.GL_DrawPatch(50+curpos*31, 167, PatchNumSELECTBOX);
		if(x != 0)
		{
			/*V_DrawPatch(40, 167, !(leveltime&4) ? PatchINVLFGEM1 :
				PatchINVLFGEM2);*/
			gi.GL_DrawPatch(40, 167, !(leveltime&4) ? PatchNumINVLFGEM1 :
				PatchNumINVLFGEM2);
		}
		if(CPlayer->inventorySlotNum-x > 7)
		{
			/*V_DrawPatch(268, 167, !(leveltime&4) ?
				PatchINVRTGEM1 : PatchINVRTGEM2);*/
			gi.GL_DrawPatch(268, 167, !(leveltime&4) ? PatchNumINVRTGEM1 : PatchNumINVRTGEM2);
		}
	}
}


//==========================================================================
//
// Draw_TeleportIcon
//
//==========================================================================
void Draw_TeleportIcon(void)
{
//	patch_t *patch;
	//patch = gi.W_CacheLumpNum(gi.W_GetNumForName("teleicon"), PU_CACHE);
	//V_DrawPatch(100, 68, patch);
	gi.GL_DrawPatch(100, 68, gi.W_GetNumForName("teleicon"));
	//UpdateState |= I_FULLSCRN;
	//I_Update();
	//UpdateState |= I_FULLSCRN;
	gi.Update(DDUF_FULLSCREEN | DDUF_UPDATE);
	gi.Update(DDUF_FULLSCREEN);
}

//==========================================================================
//
// Draw_SaveIcon
//
//==========================================================================
void Draw_SaveIcon(void)
{
//	patch_t *patch;
//	patch = gi.W_CacheLumpNum(gi.W_GetNumForName("saveicon"), PU_CACHE);
	//V_DrawPatch(100, 68, patch);
	gi.GL_DrawPatch(100, 68, gi.W_GetNumForName("saveicon"));
/*	UpdateState |= I_FULLSCRN;
	I_Update();
	UpdateState |= I_FULLSCRN;*/
	gi.Update(DDUF_FULLSCREEN | DDUF_UPDATE);
	gi.Update(DDUF_FULLSCREEN);
}

//==========================================================================
//
// Draw_LoadIcon
//
//==========================================================================
void Draw_LoadIcon(void)
{
//	patch_t *patch;
	//patch = gi.W_CacheLumpNum(gi.W_GetNumForName("loadicon"), PU_CACHE);
	//V_DrawPatch(100, 68, patch);
	gi.GL_DrawPatch(100, 68, gi.W_GetNumForName("loadicon"));
/*	UpdateState |= I_FULLSCRN;
	I_Update();
	UpdateState |= I_FULLSCRN;*/
	gi.Update(DDUF_FULLSCREEN | DDUF_UPDATE);
	gi.Update(DDUF_FULLSCREEN);
}


//==========================================================================
//
// SB_Responder
//
//==========================================================================

boolean SB_Responder(event_t *event)
{
	extern boolean willRenderSprites, freezeRLs;

	if(event->type == ev_keydown)
	{
/*		if(HandleCheats(event->data1))
		{ // Need to eat the key
			return(true);
		}*/
	}
	/*if(event->type == ev_keydown)
	{
		if(event->data1 == 'i')	// Turn sprites on/off.
		{
			willRenderSprites ^= 0x1;
			return true;
		}
		if(event->data1 == 'o') // Un/freeze RLs.
		{
			freezeRLs ^= 0x1;
			return true;
		}
	}*/
	return(false);
}

static boolean canCheat()
{
	return !(gameskill == sk_nightmare || (netgame && !netcheat) 
		|| players[consoleplayer].health <= 0);
}

//==========================================================================
//
// HandleCheats
//
// Returns true if the caller should eat the key.
//
//==========================================================================
/*
static boolean HandleCheats(byte key)
{
	int i;
	boolean eat;

	if(gameskill == sk_nightmare)
	{ // Can't cheat in nightmare mode
		return(false);
	}
	else if(netgame)
	{ // change CD track is the only cheat available in deathmatch
		eat = false;
		if(i_CDMusic)
		{
			if(CheatAddKey(&Cheats[0], key, &eat))
			{
				Cheats[0].func(&players[consoleplayer], &Cheats[0]);
				S_StartSound(NULL, SFX_PLATFORM_STOP);
			}
			if(CheatAddKey(&Cheats[1], key, &eat))
			{
				Cheats[1].func(&players[consoleplayer], &Cheats[1]);
				S_StartSound(NULL, SFX_PLATFORM_STOP);
			}
		}
		return eat;
	}
	if(players[consoleplayer].health <= 0)
	{ // Dead players can't cheat
		return(false);
	}
	eat = false;
	for(i = 0; Cheats[i].func != NULL; i++)
	{
		if(CheatAddKey(&Cheats[i], key, &eat))
		{
			Cheats[i].func(&players[consoleplayer], &Cheats[i]);
			S_StartSound(NULL, SFX_PLATFORM_STOP);
		}
	}
	//printf( "end of handlecheat: %d (%d)\n",key,eat);
	return(eat);
}
*/
//==========================================================================
//
// CheatAddkey
//
// Returns true if the added key completed the cheat, false otherwise.
//
//==========================================================================
/*
static boolean CheatAddKey(Cheat_t *cheat, byte key, boolean *eat)
{
	if(!cheat->pos)
	{
		cheat->pos = cheat->sequence;
		cheat->currentArg = 0;
	}
	if(*cheat->pos == 0)
	{
		*eat = true;
		cheat->args[cheat->currentArg++] = key;
		cheat->pos++;
	}
	else if(CheatLookup[key] == *cheat->pos)
	{
		cheat->pos++;
	}
	else
	{
		cheat->pos = cheat->sequence;
		cheat->currentArg = 0;
	}
	if(*cheat->pos == 0xff)
	{
		cheat->pos = cheat->sequence;
		cheat->currentArg = 0;
		return(true);
	}
	return(false);
}

//==========================================================================
//
// CHEAT FUNCTIONS
//
//==========================================================================

static void SendCheatNotification(cheattype_t type, player_t *player)
{
	if(netcheat && netgame && player == players+consoleplayer)
	{
		gi.SendPacket(DDPM_ALL_PLAYERS, &type, sizeof(type));
	}
}

void SB_HandleCheatNotification(int fromplayer, void *data, int length)
{
	cheattype_t type = *(cheattype_t*) data;
	void (*cheatfuncs[])(player_t*, Cheat_t*) =
	{
		CheatGodFunc,
		CheatNoClipFunc,
		CheatWeaponsFunc,
		CheatHealthFunc,
		CheatKeysFunc,
		CheatArtifactAllFunc,
		CheatPuzzleFunc
	};

	if(length != sizeof(type)) 
		gi.Error("HandleCheatNotification: wrong packet length\n");

	cheatfuncs[type](players+fromplayer, NULL);
}

static void CheatGodFunc(player_t *player, Cheat_t *cheat)
{
	player->cheats ^= CF_GODMODE;
	if(player->cheats&CF_GODMODE)
	{
		P_SetMessage(player, TXT_CHEATGODON, true);
	}
	else
	{
		P_SetMessage(player, TXT_CHEATGODOFF, true);
	}
	SB_state = -1;

	// This cheat can be used with -netcheat.
	SendCheatNotification(CHT_GOD, player);
}

static void CheatNoClipFunc(player_t *player, Cheat_t *cheat)
{
	player->cheats ^= CF_NOCLIP;
	if(player->cheats&CF_NOCLIP)
	{
		P_SetMessage(player, TXT_CHEATNOCLIPON, true);
	}
	else
	{
		P_SetMessage(player, TXT_CHEATNOCLIPOFF, true);
	}

	// This cheat can be used with -netcheat.
	SendCheatNotification(CHT_NOCLIP, player);
}

static void CheatWeaponsFunc(player_t *player, Cheat_t *cheat)
{
	int i;
	//extern boolean *WeaponInShareware;

	for(i = 0; i < NUMARMOR; i++)
	{
		player->armorpoints[i] = ArmorIncrement[player->pclass][i];
	}
	for(i = 0; i < NUMWEAPONS; i++)
	{
		player->weaponowned[i] = true;
	}
	for(i = 0; i < NUMMANA; i++)
	{
		player->mana[i] = MAX_MANA;
	}
	P_SetMessage(player, TXT_CHEATWEAPONS, true);

	// This cheat can be used with -netcheat.
	SendCheatNotification(CHT_WEAPONS, player);
}

static void CheatHealthFunc(player_t *player, Cheat_t *cheat)
{
	if(player->morphTics)
	{
		player->health = player->plr->mo->health = MAXMORPHHEALTH;
	}
	else
	{
		player->health = player->plr->mo->health = player->maxhealth;
	}
	P_SetMessage(player, TXT_CHEATHEALTH, true);

	// This cheat can be used with -netcheat.
	SendCheatNotification(CHT_HEALTH, player);
}

static void CheatKeysFunc(player_t *player, Cheat_t *cheat)
{
	player->keys = 2047;
	P_SetMessage(player, TXT_CHEATKEYS, true);

	// This cheat can be used with -netcheat.
	SendCheatNotification(CHT_KEYS, player);
}

static void CheatSoundFunc(player_t *player, Cheat_t *cheat)
{
	DebugSound = !DebugSound;
	if(DebugSound)
	{
		P_SetMessage(player, TXT_CHEATSOUNDON, true);
	}
	else
	{
		P_SetMessage(player, TXT_CHEATSOUNDOFF, true);
	}
}

static void CheatTickerFunc(player_t *player, Cheat_t *cheat)
{
/*	extern int DisplayTicker;

	DisplayTicker = !DisplayTicker;
	if(DisplayTicker)
	{
		P_SetMessage(player, TXT_CHEATTICKERON, true);
	}
	else
	{
		P_SetMessage(player, TXT_CHEATTICKEROFF, true);
	}
}

static void CheatArtifactAllFunc(player_t *player, Cheat_t *cheat)
{
	int i;
	int j;

	for(i = arti_none+1; i < arti_firstpuzzitem; i++)
	{
		for(j = 0; j < 25; j++)
		{
			P_GiveArtifact(player, i, NULL);
		}
	}
	P_SetMessage(player, TXT_CHEATARTIFACTS3, true);

	// This cheat can be used with -netcheat.
	SendCheatNotification(CHT_ARTIFACTS, player);
}

static void CheatPuzzleFunc(player_t *player, Cheat_t *cheat)
{
	int i;

	for(i = arti_firstpuzzitem; i < NUMARTIFACTS; i++)
	{
		P_GiveArtifact(player, i, NULL);
	}
	P_SetMessage(player, TXT_CHEATARTIFACTS3, true);

	// This cheat can be used with -netcheat.
	SendCheatNotification(CHT_PUZZLE, player);
}

static void CheatInitFunc(player_t *player, Cheat_t *cheat)
{
	G_DeferedInitNew(gameskill, gameepisode, gamemap);
	P_SetMessage(player, TXT_CHEATWARP, true);
}

static void CheatWarpFunc(player_t *player, Cheat_t *cheat)
{
	int tens;
	int ones;
	int map;
	char mapName[9];
	char auxName[128];
	FILE *fp;

	tens = cheat->args[0]-'0';
	ones = cheat->args[1]-'0';
	if(tens < 0 || tens > 9 || ones < 0 || ones > 9)
	{ // Bad map
		P_SetMessage(player, TXT_CHEATBADINPUT, true);
		return;
	}
	//map = P_TranslateMap((cheat->args[0]-'0')*10+cheat->args[1]-'0');
	map = P_TranslateMap(tens*10+ones);
	if(map == -1)
	{ // Not found
		P_SetMessage(player, TXT_CHEATNOMAP, true);
		return;
	}
	if(map == gamemap)
	{ // Don't try to teleport to current map
		P_SetMessage(player, TXT_CHEATBADINPUT, true);
		return;
	}
	if(DevMaps)
	{ // Search map development directory
		sprintf(auxName, "%sMAP%02d.WAD", DevMapsDir, map);
		fp = fopen(auxName, "rb");
		if(fp)
		{
			fclose(fp);
		}
		else
		{ // Can't find
			P_SetMessage(player, TXT_CHEATNOMAP, true);
			return;
		}
	}
	else
	{ // Search primary lumps
		sprintf(mapName, "MAP%02d", map);
		if(gi.W_CheckNumForName(mapName) == -1)
		{ // Can't find
			P_SetMessage(player, TXT_CHEATNOMAP, true);
			return;
		}
	}
	P_SetMessage(player, TXT_CHEATWARP, true);
	G_TeleportNewMap(map, 0);
	//G_Completed(-1, -1);//map, 0);
}

static void CheatPigFunc(player_t *player, Cheat_t *cheat)
{
	extern boolean P_UndoPlayerMorph(player_t *player);

	if(player->morphTics)
	{
		P_UndoPlayerMorph(player);
	}
	else
	{
	 	P_MorphPlayer(player);
	}
	P_SetMessage(player, "SQUEAL!!", true);
}

static void CheatMassacreFunc(player_t *player, Cheat_t *cheat)
{
	int count;
	char buffer[80];

	count = P_Massacre();
	sprintf(buffer, "%d MONSTERS KILLED\n", count);
	P_SetMessage(player, buffer, true);
}

static void CheatIDKFAFunc(player_t *player, Cheat_t *cheat)
{
	int i;
	if(player->morphTics)
	{
		return;
	}
	for(i = 1; i < 8; i++)
	{
		player->weaponowned[i] = false;
	}
	player->pendingweapon = WP_FIRST;
	P_SetMessage(player, TXT_CHEATIDKFA, true);
}

static void CheatQuickenFunc1(player_t *player, Cheat_t *cheat)
{
	P_SetMessage(player, "TRYING TO CHEAT?  THAT'S ONE....", true);
}

static void CheatQuickenFunc2(player_t *player, Cheat_t *cheat)
{
	P_SetMessage(player, "THAT'S TWO....", true);
}

static void CheatQuickenFunc3(player_t *player, Cheat_t *cheat)
{
	P_DamageMobj(player->plr->mo, NULL, player->plr->mo, 10000);
	P_SetMessage(player, "THAT'S THREE!  TIME TO DIE.", true);
}

static void CheatClassFunc1(player_t *player, Cheat_t *cheat)
{	
	P_SetMessage(player, "ENTER NEW PLAYER CLASS (0 - 2)", true);
}

static void CheatClassFunc2(player_t *player, Cheat_t *cheat)
{
	int i;
	int pclass;

	if(player->morphTics)
	{ // don't change class if the player is morphed
		return;
	}
	pclass = cheat->args[0]-'0';
	if(pclass > 2 || pclass < 0)
	{
		P_SetMessage(player, "INVALID PLAYER CLASS", true);
		return;
	}
	player->pclass = pclass;
	for(i = 0; i < NUMARMOR; i++)
	{
		player->armorpoints[i] = 0;
	}
	PlayerClass[consoleplayer] = pclass;
	P_PostMorphWeapon(player, WP_FIRST);
	SB_SetClassData();
	SB_state = -1;
	//UpdateState |= I_FULLSCRN;
	gi.Update(DDUF_FULLSCREEN);
}

static void CheatVersionFunc(player_t *player, Cheat_t *cheat)
{
	P_SetMessage(player, VERSIONTEXT, true);
}

static void CheatDebugFunc(player_t *player, Cheat_t *cheat)
{
	char textBuffer[50];
	sprintf(textBuffer, "MAP %d (%d)  X:%5d  Y:%5d  Z:%5d",
				P_GetMapWarpTrans(gamemap),
				gamemap,
				player->plr->mo->x >> FRACBITS,
				player->plr->mo->y >> FRACBITS,
				player->plr->mo->z >> FRACBITS);
	P_SetMessage(player, textBuffer, true);
}

static void CheatScriptFunc1(player_t *player, Cheat_t *cheat)
{
	P_SetMessage(player, "RUN WHICH SCRIPT(01-99)?", true);
}

static void CheatScriptFunc2(player_t *player, Cheat_t *cheat)
{
	P_SetMessage(player, "RUN WHICH SCRIPT(01-99)?", true);
}

static void CheatScriptFunc3(player_t *player, Cheat_t *cheat)
{
	int script;
	byte args[3];
	int tens, ones;
	char textBuffer[40];

	tens = cheat->args[0]-'0';
	ones = cheat->args[1]-'0';
	script = tens*10 + ones;
	if (script < 1) return;
	if (script > 99) return;
	args[0]=args[1]=args[2]=0;

	if(P_StartACS(script, 0, args, player->plr->mo, NULL, 0))
	{
		sprintf(textBuffer, "RUNNING SCRIPT %.2d", script);
		P_SetMessage(player, textBuffer, true);
	}
}

extern int cheating;

static void CheatRevealFunc(player_t *player, Cheat_t *cheat)
{
	cheating = (cheating+1) % 3;
}

//===========================================================================
//
// CheatTrackFunc1
//
//===========================================================================

static void CheatTrackFunc1(player_t *player, Cheat_t *cheat)
{
//#ifdef __WATCOMC__
	char buffer[80];

	if(!i_CDMusic)
	{
		return;
	}
	if(gi.CD(DD_INIT, 0) == -1)
	{
		P_SetMessage(player, "ERROR INITIALIZING CD", true);
	}
	sprintf(buffer, "ENTER DESIRED CD TRACK (%.2d - %.2d):\n",
		gi.CD(DD_GET_FIRST_TRACK, 0), gi.CD(DD_GET_LAST_TRACK, 0));	
	P_SetMessage(player, buffer, true);
//#endif
}

//===========================================================================
//
// CheatTrackFunc2
//
//===========================================================================

static void CheatTrackFunc2(player_t *player, Cheat_t *cheat)
{
	char buffer[80];
	int track;

	if(!i_CDMusic)
	{
		return;
	}
	track = (cheat->args[0]-'0')*10+(cheat->args[1]-'0');
	if(track < gi.CD(DD_GET_FIRST_TRACK, 0) || track > gi.CD(DD_GET_LAST_TRACK, 0))
	{
		P_SetMessage(player, "INVALID TRACK NUMBER\n", true);
		return;
	} 
	if(track == gi.CD(DD_GET_CURRENT_TRACK,0))
	{
		return;
	}
	if(gi.CD(DD_PLAY_LOOP, track))//I_CDMusPlay(track, true))
	{
		sprintf(buffer, "ERROR WHILE TRYING TO PLAY CD TRACK: %.2d\n", track);
		P_SetMessage(player, buffer, true);
	}
	else
	{ // No error encountered while attempting to play the track
		sprintf(buffer, "PLAYING TRACK: %.2d\n", track);
		P_SetMessage(player, buffer, true);	
		//i_CDMusicLength = 35*gi.CD(DD_GET_TRACK_LENGTH, track);//I_CDMusTrackLength(track);
		//oldTic = gametic;
		//i_CDTrack = track;
		//i_CDCurrentTrack = track;
	}
}

*/
//===========================================================================
//
// Console Commands
//
//===========================================================================

// This is the multipurpose cheat ccmd.
/*int CCmdCheat(int argc, char **argv)
{
	unsigned int		i;

	if(argc != 2)
	{
		// Usage information.
		gi.conprintf( "Usage: cheat (cheat)\nFor example, 'cheat visit21'.\n");
		return true;
	}
	// Give each of the characters in argument two to the SB event handler.
	for(i=0; i<strlen(argv[1]); i++)
	{
		event_t ev;
		ev.type = ev_keydown;
		ev.data1 = argv[1][i];
		ev.data2 = ev.data3 = 0;
		SB_Responder(&ev);
	}
	return true;
}*/

int CCmdCheatGod(int argc, char **argv)
{
	if(!canCheat()) return false; // Can't cheat!
//	CheatGodFunc(players+consoleplayer, NULL);
	players[consoleplayer].cheats ^= CF_GODMODE;
	if(players[consoleplayer].cheats&CF_GODMODE)
	{
		P_SetMessage(players+consoleplayer, TXT_CHEATGODON, true);
	}
	else
	{
		P_SetMessage(players+consoleplayer, TXT_CHEATGODOFF, true);
	}
	SB_state = -1;
	return true;
}

int CCmdCheatClip(int argc, char **argv)
{
	if(!canCheat()) return false; // Can't cheat!
//	CheatNoClipFunc(players+consoleplayer, NULL);
	players[consoleplayer].cheats ^= CF_NOCLIP;
	if(players[consoleplayer].cheats&CF_NOCLIP)
	{
		P_SetMessage(players+consoleplayer, TXT_CHEATNOCLIPON, true);
	}
	else
	{
		P_SetMessage(players+consoleplayer, TXT_CHEATNOCLIPOFF, true);
	}
	return true;
}

void Give_Experience(player_t *player,int experience);
void Give_Level(player_t *player,unsigned int level);

int CCmdCheatGive(int argc, char **argv)
{
//	int tellUsage = false;

	if(!canCheat()) return false; // Can't cheat!
	// Check the arguments.
	if(argc != 3) return false;
//		tellUsage = true;
	if (!strnicmp(argv[1], "money", 5))
		players[consoleplayer].money+=atoi(argv[2]);
	else if (!strnicmp(argv[1], "experience", 10))
		Give_Experience(players+consoleplayer,atoi(argv[2]));
	else if (!strnicmp(argv[1], "level", 5))
		Give_Level(players+consoleplayer,atoi(argv[2]));
	else if (!strnicmp(argv[1], "magic", 5))
		players[consoleplayer].sp_power += atoi(argv[2]);

/*	else if(!strnicmp(argv[1], "weapons", 1))
		CheatWeaponsFunc(players+consoleplayer, NULL);
	else if(!strnicmp(argv[1], "health", 1))
		CheatHealthFunc(players+consoleplayer, NULL);
	else if(!strnicmp(argv[1], "keys", 1))
		CheatKeysFunc(players+consoleplayer, NULL);
	else if(!strnicmp(argv[1], "artifacts", 1))
		CheatArtifactAllFunc(players+consoleplayer, NULL);
	else if(!strnicmp(argv[1], "puzzle", 1))
		CheatPuzzleFunc(players+consoleplayer, NULL);
	else 
		tellUsage = true;

	if(tellUsage)
	{
		gi.conprintf( "Usage: give weapons/health/keys/artifacts/puzzle\n");
		gi.conprintf( "The first letter is enough, e.g. 'give h'.\n");
	}*/
	return true;
}

int CCmdCheatMassacre(int argc, char **argv)
{
	int count;
	char buffer[80];

	if(!canCheat()) return false; // Can't cheat!
	if (argc!=2 || strnicmp(argv[1], "butcher", 5)) return false;

	count = P_Massacre();
	sprintf(buffer, "%d MONSTERS BUTCHERED IN COLD BLOOD\n", count);
	P_SetMessage(players+consoleplayer, buffer, true);
	return true;
}

int CCmdCheatWarp(int argc, char **argv)
{
	int		num;
	int tens;
	int ones;
	int map;
	char mapName[9];
	char auxName[128];
	FILE *fp;

	if(!canCheat()) return false; // Can't cheat!
	if(argc != 2)
	{
		gi.conprintf( "Usage: warp (num)\n");
		return true;
	}
	num = atoi(argv[1]);
	tens = num/10 + '0';
	ones = num%10 + '0';
	// We don't want that keys are repeated while we wait.
	gi.ClearKeyRepeaters();
//	CheatWarpFunc(players+consoleplayer, &cheat);

	tens = tens-'0';
	ones = ones-'0';
	if(tens < 0 || tens > 9 || ones < 0 || ones > 9)
	{ // Bad map
		P_SetMessage(players+consoleplayer, TXT_CHEATBADINPUT, true);
		return false;
	}
	//map = P_TranslateMap((cheat->args[0]-'0')*10+cheat->args[1]-'0');
	map = P_TranslateMap(tens*10+ones);
	if(map == -1)
	{ // Not found
		P_SetMessage(players+consoleplayer, TXT_CHEATNOMAP, true);
		return false;
	}
	if(map == gamemap)
	{ // Don't try to teleport to current map
		P_SetMessage(players+consoleplayer, TXT_CHEATBADINPUT, true);
		return false;
	}
	if(DevMaps)
	{ // Search map development directory
		sprintf(auxName, "%sMAP%02d.WAD", DevMapsDir, map);
		fp = fopen(auxName, "rb");
		if(fp)
		{
			fclose(fp);
		}
		else
		{ // Can't find
			P_SetMessage(players+consoleplayer, TXT_CHEATNOMAP, true);
			return false;
		}
	}
	else
	{ // Search primary lumps
		sprintf(mapName, "MAP%02d", map);
		if(gi.W_CheckNumForName(mapName) == -1)
		{ // Can't find
			P_SetMessage(players+consoleplayer, TXT_CHEATNOMAP, true);
			return false;
		}
	}
	P_SetMessage(players+consoleplayer, TXT_CHEATWARP, true);
	G_TeleportNewMap(map, 0);
	//G_Completed(-1, -1);//map, 0);
	return true;
}

/*int CCmdCheatPig(int argc, char **argv)
{
/*	if(!canCheat()) return false; // Can't cheat!
	CheatPigFunc(players+consoleplayer, NULL);*/
/*	return true;
}

int CCmdCheatShadowcaster(int argc, char **argv)
{
/*	Cheat_t	cheat;

	if(!canCheat()) return false; // Can't cheat!
	if(argc != 2)
	{
		gi.conprintf( "Usage: class (0-2)\n");
		gi.conprintf( "0=Fighter, 1=Cleric, 2=Mage.\n");
		return true;
	}
	cheat.args[0] = atoi(argv[1]) + '0';
	CheatClassFunc2(players+consoleplayer, &cheat);*/
/*	return true;
}

int CCmdCheatWhere(int argc, char **argv)
{
/*	if(!canCheat()) return false; // Can't cheat!
	CheatDebugFunc(players+consoleplayer, NULL);*/
/*	return true;
}

int CCmdCheatRunScript(int argc, char **argv)
{
/*	Cheat_t cheat;
	int		num;

	if(!canCheat()) return false; // Can't cheat!
	if(argc != 2)
	{
		gi.conprintf( "Usage: runscript (1-99)\n");
		return true;
	}
	num = atoi(argv[1]);
	cheat.args[0] = num/10 + '0';
	cheat.args[1] = num%10 + '0';
	CheatScriptFunc3(players+consoleplayer, &cheat);*/
/*	return true;
}

int CCmdCheatReveal(int argc, char **argv)
{
/*	int	option;

	if(!canCheat()) return false; // Can't cheat!
	if(argc != 2)
	{
		gi.conprintf( "Usage: reveal (0-3)\n");
		gi.conprintf( "0=nothing, 1=show unseen, 2=full map, 3=map+things\n");
		return true;
	}
	// Reset them (for 'nothing'). :-)
	cheating = 0;
	players[consoleplayer].powers[pw_allmap] = false;
	option = atoi(argv[1]);
	if(option < 0 || option > 3) return false;
	if(option == 1)
		players[consoleplayer].powers[pw_allmap] = true;
	else if(option == 2)
		cheating = 1;
	else if(option == 3)
		cheating = 2;*/
/*	return true;
}*/

int CCmdCheatWeapons(int argc, char **argv) //Remi: Cheats submitted by Camper
{
	int i;

	if(!canCheat()) return false; // Can't cheat!
	if (argc!=3 || strnicmp(argv[1], "of", 2) || strnicmp(argv[2], "weapons", 7)) return false;

	for(i = 0; i < NUMARMOR; i++)
	{
		players[consoleplayer].armorpoints[i] = ArmorIncrement[players[consoleplayer].pclass][i];
	}
	for(i = 0; i < NUMWEAPONS; i++)
	{
		players[consoleplayer].weaponowned[i] = true;
	}
	for(i = 0; i < NUMMANA; i++)
	{	
		players[consoleplayer].mana[i] = MAX_MANA;
	}
	P_SetMessage(players+consoleplayer, TXT_CHEATWEAPONS, true);

	return true;
}

int CCmdCheatArtifactAll(int argc, char **argv)
{
	int i;
	int j;

	if(!canCheat()) return false; // Can't cheat!
	if (argc!=2 || strnicmp(argv[1], "center", 6)) return false;

	for(i = arti_none+1; i < arti_firstpuzzitem; i++)
	{
		for(j = 0; j < 25; j++)
		{
			P_GiveArtifact(players+consoleplayer, (artitype_t)i, NULL);
		}
	}
	P_SetMessage(players+consoleplayer, TXT_CHEATARTIFACTS3, true);
	return true;
}

