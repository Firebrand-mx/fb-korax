
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
#include "mn_def.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

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
static void DrawPossWeaponry(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern	sfxinfo_t	S_sfx[];

extern int ArmorIncrement[NUMCLASSES][NUMARMOR];
extern int AutoArmorSave[NUMCLASSES];

// PUBLIC DATA DECLARATIONS ------------------------------------------------

boolean inventory;
int curpos;
int inv_ptr;
int ArtifactFlash;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static byte CheatLookup[256];
static int HealthMarker;
static player_t *CPlayer;
static int SpinFlylump;
static int SpinMinotaurLump;
static int SpinSpeedLump;
static int SpinDefenseLump;

static int FontBNumBase;
static int PlayPalette;

static int PatchNumH2BAR;
static int PatchNumH2TOP;
static int PatchNumLFEDGE;
static int PatchNumRTEDGE;
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

	PatchNumH2BAR = gi.W_GetNumForName("H2BAR");
	PatchNumH2TOP = gi.W_GetNumForName("H2TOP");
	PatchNumINVBAR = gi.W_GetNumForName("INVBAR");
	PatchNumLFEDGE	= gi.W_GetNumForName("LFEDGE");
	PatchNumRTEDGE	= gi.W_GetNumForName("RTEDGE");
	PatchNumSTATBAR = gi.W_GetNumForName("STATBAR");
	PatchNumKEYBAR = gi.W_GetNumForName("KEYBAR");
	PatchNumSELECTBOX = gi.W_GetNumForName("SELECTBOX");
	PatchNumMANAVIAL1 = gi.W_GetNumForName("MANAVL1");
	PatchNumMANAVIAL2 = gi.W_GetNumForName("MANAVL2");
	PatchNumMANAVIALDIM1 = gi.W_GetNumForName("MANAVL1D");
	PatchNumMANAVIALDIM2 = gi.W_GetNumForName("MANAVL2D");
	PatchNumMANADIM1 = gi.W_GetNumForName("MANADIM1");
	PatchNumMANADIM2 = gi.W_GetNumForName("MANADIM2");
	PatchNumMANABRIGHT1 = gi.W_GetNumForName("MANABRT1");
	PatchNumMANABRIGHT2 = gi.W_GetNumForName("MANABRT2");
	PatchNumINVLFGEM1 = gi.W_GetNumForName("invgeml1");
	PatchNumINVLFGEM2 = gi.W_GetNumForName("invgeml2");
	PatchNumINVRTGEM1 = gi.W_GetNumForName("invgemr1");
	PatchNumINVRTGEM2 = gi.W_GetNumForName("invgemr2");

	startLump = gi.W_GetNumForName("IN0");
	for(i = 0; i < 10; i++)
	{
		PatchNumINumbers[i] = startLump+i;
	}
	PatchNumNEGATIVE = gi.W_GetNumForName("NEGNUM");
	FontBNumBase = gi.W_GetNumForName("FONTB16");
	startLump = gi.W_GetNumForName("SMALLIN0");
	for(i = 0; i < 10; i++)
	{
		PatchNumSmNumbers[i] = startLump+i;
	}
	PlayPalette = gi.W_GetNumForName("PLAYPAL");
	SpinFlylump = gi.W_GetNumForName("SPFLY0");
	SpinMinotaurLump = gi.W_GetNumForName("SPMINO0");
	SpinSpeedLump = gi.W_GetNumForName("SPBOOT0");
	SpinDefenseLump = gi.W_GetNumForName("SPSHLD0");

	PatchNumKILLS = gi.W_GetNumForName("KILLS");
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
	PatchNumWEAPONSLOT = gi.W_GetNumForName("wpslot0")+pclass;
	PatchNumWEAPONFULL = gi.W_GetNumForName("wpfull0")+pclass;
	PatchNumPIECE1 = gi.W_GetNumForName("wpiecef1")+pclass;
	PatchNumPIECE2 = gi.W_GetNumForName("wpiecef2")+pclass;
	PatchNumPIECE3 = gi.W_GetNumForName("wpiecef3")+pclass;
	PatchNumCHAIN =gi.W_GetNumForName("chain")+pclass;
	if(!netgame)
	{ // single player game uses red life gem (the second gem)
		PatchNumLIFEGEM = gi.W_GetNumForName("lifegem")+MAXPLAYERS*pclass+1;
	}
	else
	{
		PatchNumLIFEGEM = gi.W_GetNumForName("lifegem")+MAXPLAYERS*pclass+consoleplayer;
	}
	SB_state = -1;
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
			GCanvas->DrawPatch1(x+8, y, PatchNumINumbers[val/10]);
			GCanvas->DrawPatch1(x, y, PatchNumNEGATIVE);
		}
		else
		{
			GCanvas->DrawPatch1(x+8, y, PatchNumNEGATIVE);
		}
		val = val%10;
		GCanvas->DrawPatch1(x+16, y, PatchNumINumbers[val]);
		return;
	}
	if(val > 99)
	{
		GCanvas->DrawPatch1(x, y, PatchNumINumbers[val/100]);
	}
	val = val%100;
	if(val > 9 || oldval > 99)
	{
		GCanvas->DrawPatch1(x+8, y, PatchNumINumbers[val/10]);
	}
	val = val%10;
	GCanvas->DrawPatch1(x+16, y, PatchNumINumbers[val]);
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
	int oldval;
	int offset=0;

	oldval = val;
	if(val < 0)
	{
		val = 0;
	}
	if(val > 999)
	{
		GCanvas->DrawPatch1(x+offset, y, gi.W_GetNumForName("inred0")+val/1000);
		offset+=8;
	}
	if(val > 99)
	{
		GCanvas->DrawPatch1(x+offset, y, gi.W_GetNumForName("inred0")+(val%1000)/100);
	}
	val = val%100;
	if(val > 9 || oldval > 99)
	{
		GCanvas->DrawPatch1(x+8+offset, y, gi.W_GetNumForName("inred0")+val/10);
	}
	val = val%10;
	GCanvas->DrawPatch1(x+16+offset, y, gi.W_GetNumForName("inred0")+val);
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
		gi.GL_DrawShadowedPatch(xpos+6-patch->width/2, y, FontBNumBase+val/1000);
	}
	val = val%1000;
	xpos += 12;
	if(val > 99 || oldval > 999)
	{
		patch = (patch_t *)gi.W_CacheLumpNum(FontBNumBase+val/100, PU_CACHE);
		gi.GL_DrawShadowedPatch(xpos+6-patch->width/2, y, FontBNumBase+val/100);
	}
	val = val%100;
	xpos += 12;
	if(val > 9 || oldval > 99)
	{
		patch = (patch_t *)gi.W_CacheLumpNum(FontBNumBase+val/10, PU_CACHE);
		gi.GL_DrawShadowedPatch(xpos+6-patch->width/2, y, FontBNumBase+val/10);
	}
	val = val%10;
	xpos += 12;
	patch = (patch_t *)gi.W_CacheLumpNum(FontBNumBase+val, PU_CACHE);
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
		GCanvas->DrawPatch1(x, y, PatchNumSmNumbers[val/100]);
		GCanvas->DrawPatch1(x+4, y, PatchNumSmNumbers[(val%100)/10]);
	}
	else if(val > 9)
	{
		GCanvas->DrawPatch1(x+4, y, PatchNumSmNumbers[val/10]);
	}
	val %= 10;
	GCanvas->DrawPatch1(x+8, y, PatchNumSmNumbers[val]);
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
	gi.Update(DDUF_TOP | DDUF_BORDER);

	CPlayer = &players[consoleplayer];
	if (CPlayer->pclass >=PCLASS_ETTIN)
	{
		//Remi: Possessed monsters have forced fullscreen
		DrawPossWeaponry();
	}
	else if (gi.Get(DD_VIEWWINDOW_HEIGHT) == SCREENHEIGHT && !automapactive)
	{
		DrawFullScreenStuff();
	}
	else
	{
     	if (CPlayer->pclass<PCLASS_ETTIN) DrRedINumber(CPlayer->sp_power, 600 , 13);
		GCanvas->SetOrigin(160, 280);
		GCanvas->DrawPatch1(0, 134, PatchNumH2BAR);
		DrawCommonBar();
		if(!inventory)
		{
			// Main interface
			if(!automapactive)
			{
				GCanvas->DrawPatch1(38, 162, PatchNumSTATBAR);
			}
			else
			{
				GCanvas->DrawPatch1(38, 162, PatchNumKEYBAR);
			}
			if(!automapactive)
			{
				DrawMainBar();
			}
			else
			{
				DrawKeyBar();
			}
		}
		else
		{
			DrawInventoryBar();
		}
		GCanvas->SetOrigin(0, 0);
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
	if(screenblocks>10) leftoff = 42;

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
					GCanvas->DrawPatch1(20+leftoff, 19, SpinFlylump+15);
				}
				else
				{
					GCanvas->DrawPatch1(20+leftoff, 19, SpinFlylump+frame);
					hitCenterFrame = false;
				}
			}
			else
			{
				if(!hitCenterFrame && (frame != 15 && frame != 0))
				{
					GCanvas->DrawPatch1(20+leftoff, 19, SpinFlylump+frame);
					hitCenterFrame = false;
				}
				else
				{
					GCanvas->DrawPatch1(20+leftoff, 19, SpinFlylump+15);
					hitCenterFrame = true;
				}
			}
		}
		gi.Update(DDUF_TOP | DDUF_MESSAGES);
	}

	// Speed Boots
	if(CPlayer->powers[pw_speed])
	{
		if(CPlayer->powers[pw_speed] > BLINKTHRESHOLD
			|| !(CPlayer->powers[pw_speed]&16))
		{
			frame = (leveltime/3)&15;
			GCanvas->DrawPatch1(60+leftoff, 19, SpinSpeedLump+frame);
		}
		gi.Update(DDUF_TOP | DDUF_MESSAGES);
	}

	// Defensive power
	if(CPlayer->powers[pw_invulnerability])
	{
		if(CPlayer->powers[pw_invulnerability] > BLINKTHRESHOLD
			|| !(CPlayer->powers[pw_invulnerability]&16))
		{
			frame = (leveltime/3)&15;
			GCanvas->DrawPatch1(260, 19, SpinDefenseLump+frame);
		}
		gi.Update(DDUF_TOP | DDUF_MESSAGES);
	}

	// Minotaur Active
	if(CPlayer->powers[pw_minotaur])
	{
		if(CPlayer->powers[pw_minotaur] > BLINKTHRESHOLD
			|| !(CPlayer->powers[pw_minotaur]&16))
		{
			frame = (leveltime/3)&15;
			GCanvas->DrawPatch1(600, 19, SpinMinotaurLump+frame);
		}
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

	GCanvas->DrawPatch1(0, 134, PatchNumH2TOP);

	healthPos = HealthMarker*100/players[consoleplayer].maxhealth;
	if(healthPos < 0)
	{
		healthPos = 0;
	}
	if(healthPos > 100)
	{
		healthPos = 100;
	}
	GCanvas->DrawPatch1(28+(((healthPos*196)/100)%9), 193, PatchNumCHAIN);
	GCanvas->DrawPatch1(7+((healthPos*11)/5), 193, PatchNumLIFEGEM);
	GCanvas->DrawPatch1(0, 193, PatchNumLFEDGE);
	GCanvas->DrawPatch1(277, 193, PatchNumRTEDGE);
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
	int manaPatchNum1, manaPatchNum2, manaVialPatchNum1, manaVialPatchNum2;

	manaPatchNum1 = manaPatchNum2 = manaVialPatchNum1 = manaVialPatchNum2 = -1;

	// Ready artifact
	if(ArtifactFlash)
	{
		GCanvas->DrawPatch1(148, 164, gi.W_GetNumForName("useartia")+ArtifactFlash-1);
		ArtifactFlash--;
		oldarti = -1; // so that the correct artifact fills in after the flash
		gi.Update(DDUF_STATBAR);
	}
	else if(oldarti != CPlayer->readyArtifact
		|| oldartiCount != CPlayer->inventory[inv_ptr].count)
	{
		if(CPlayer->readyArtifact > 0)
		{
			GCanvas->DrawPatch1(143, 163, gi.W_GetNumForName(patcharti[CPlayer->readyArtifact]));
			if(CPlayer->inventory[inv_ptr].count > 1)
			{
				DrSmallNumber(CPlayer->inventory[inv_ptr].count, 162, 184);
			}
		}
	}

	// Frags
	if(deathmatch)
	{
		temp = 0;
		for(i = 0; i < MAXPLAYERS; i++)
		{
			temp += CPlayer->plr->frags[i];
		}
		if (PatchNumKILLS!=0) GCanvas->DrawPatch1(38, 162, PatchNumKILLS);
			DrINumber(temp, 40, 176);
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
		if(temp >= 25)
		{
			DrINumber(temp, 40, 176);
		}
		else
		{
			DrRedINumber(temp, 40, 176);
		}
	}
	// Mana
	temp = CPlayer->mana[0];
	DrSmallNumber(temp, 79, 181);
	if(temp == 0)
	{ // Draw Dim Mana icon
		manaPatchNum1 = PatchNumMANADIM1;
	}
	gi.Update(DDUF_STATBAR);
	temp = CPlayer->mana[1];
	DrSmallNumber(temp, 111, 181);
	if(temp == 0)
	{ // Draw Dim Mana icon
		manaPatchNum2 = PatchNumMANADIM2;
	}		
	gi.Update(DDUF_STATBAR);
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
	GCanvas->DrawPatch1(77, 164, manaPatchNum1);
	GCanvas->DrawPatch1(110, 164, manaPatchNum2);
	GCanvas->DrawPatch1(94, 164, manaVialPatchNum1);
	GCanvas->DrawPatch1(102, 164, manaVialPatchNum2);

	gi.GL_SetNoTexture();
	GCanvas->DrawRect(95, 165, 3, 22-(22*CPlayer->mana[0])/MAX_MANA, 0,0,0,1);
	GCanvas->DrawRect(103, 165, 3, 22-(22*CPlayer->mana[1])/MAX_MANA, 0,0,0,1);
		
	gi.Update(DDUF_STATBAR);
	// Armor
	temp = AutoArmorSave[CPlayer->pclass]
		+CPlayer->armorpoints[ARMOR_ARMOR]+CPlayer->armorpoints[ARMOR_SHIELD]
		+CPlayer->armorpoints[ARMOR_HELMET]+CPlayer->armorpoints[ARMOR_AMULET];
	DrINumber(FixedDiv(temp, 5*FRACUNIT)>>FRACBITS, 250, 176);
	// Weapon Pieces
	DrawWeaponPieces();
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
	GCanvas->DrawPatch1(38, 162, PatchNumINVBAR);
	for(i = 0; i < 7; i++)
	{
		if(CPlayer->inventorySlotNum > x+i
			&& CPlayer->inventory[x+i].type != arti_none)
		{
			GCanvas->DrawPatch1(50+i*31, 163, gi.W_GetNumForName(
				patcharti[CPlayer->inventory[x+i].type]));
			if(CPlayer->inventory[x+i].count > 1)
			{
				DrSmallNumber(CPlayer->inventory[x+i].count, 68+i*31, 185);
			}
		}
	}
	GCanvas->DrawPatch1(50+curpos*31, 163, PatchNumSELECTBOX);
	if(x != 0)
	{
		GCanvas->DrawPatch1(42, 163, !(leveltime&4) ? PatchNumINVLFGEM1 :
			PatchNumINVLFGEM2);
	}
	if(CPlayer->inventorySlotNum-x > 7)
	{
		GCanvas->DrawPatch1(269, 163, !(leveltime&4) ? PatchNumINVRTGEM1 :
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

	xPosition = 46;
	for(i = 0; i < NUMKEYS && xPosition <= 126; i++)
	{
		if(CPlayer->keys&(1<<i))
		{
			GCanvas->DrawPatch1(xPosition, 163, gi.W_GetNumForName("keyslot1")+i);
			xPosition += 20;
		}
	}
	temp = AutoArmorSave[CPlayer->pclass]
		+CPlayer->armorpoints[ARMOR_ARMOR]+CPlayer->armorpoints[ARMOR_SHIELD]
		+CPlayer->armorpoints[ARMOR_HELMET]+CPlayer->armorpoints[ARMOR_AMULET];
	for(i = 0; i < NUMARMOR; i++)
	{
		if(!CPlayer->armorpoints[i])
		{
			continue;
		}
		if(CPlayer->armorpoints[i] <= 
			(ArmorIncrement[CPlayer->pclass][i]>>2))
		{
			GCanvas->DrawFuzzPatch(150+31*i, 164, 
				gi.W_GetNumForName("armslot1")+i);
		}
		else if(CPlayer->armorpoints[i] <= 
			(ArmorIncrement[CPlayer->pclass][i]>>1))
		{
			gi.GL_DrawAltFuzzPatch(150+31*i, 164, 
				gi.W_GetNumForName("armslot1")+i);
		}
		else
		{
			GCanvas->DrawPatch1(150+31*i, 164, gi.W_GetNumForName("armslot1")+i);
		}
	}
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
		GCanvas->DrawPatch1(190, 162, PatchNumWEAPONFULL);
		return;
	}
	GCanvas->DrawPatch1(190, 162, PatchNumWEAPONSLOT);
	if(CPlayer->pieces&WPIECE1)
	{
		GCanvas->DrawPatch1(PieceX[PlayerClass[consoleplayer]][0], 162, PatchNumPIECE1);
	}
	if(CPlayer->pieces&WPIECE2)
	{
		GCanvas->DrawPatch1(PieceX[PlayerClass[consoleplayer]][1], 162, PatchNumPIECE2);
	}
	if(CPlayer->pieces&WPIECE3)
	{
		GCanvas->DrawPatch1(PieceX[PlayerClass[consoleplayer]][2], 162, PatchNumPIECE3);
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
	
	if(CPlayer->plr->mo->health > 0)
	{
		DrBNumber(CPlayer->plr->mo->health, 5, 440);
	}
	else
	{
		DrBNumber(0, 5, 440);
	}

	DrRedINumber(CPlayer->sp_power, 600, 13);

	if (CPlayer->pclass <PCLASS_CORVUS)
	{
		int dim[2] = { PatchNumMANADIM1, PatchNumMANADIM2 };
		int bright[2] = { PatchNumMANABRIGHT1, PatchNumMANABRIGHT2 };
		int patches[2] = { 0, 0 };
		int ypos = 2;//152;
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
			GCanvas->DrawPatch1(2, ypos + i*13, patches[i]);
			DrINumber(CPlayer->mana[i], 18, ypos + i*13);
		}
	}

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
		DrINumber(temp, 45, 445);
	}
	if (!inventory)
	{
		if (CPlayer->readyArtifact > 0)
		{
			GCanvas->DrawFuzzPatch(586, 430, gi.W_GetNumForName("ARTIBOX"));
			GCanvas->DrawPatch1(584, 429,
				gi.W_GetNumForName(patcharti[CPlayer->readyArtifact]));
			if(CPlayer->inventory[inv_ptr].count > 1)
			{
				DrSmallNumber(CPlayer->inventory[inv_ptr].count, 602, 452);
			}
		}
	}
	else
	{
		x = inv_ptr-curpos;
		for(i = 0; i < 7; i++)
		{
			GCanvas->DrawFuzzPatch(210+i*31, 428, gi.W_GetNumForName("ARTIBOX"));
			if(CPlayer->inventorySlotNum > x+i
				&& CPlayer->inventory[x+i].type != arti_none)
			{
				GCanvas->DrawPatch1(209+i*31, 427, gi.W_GetNumForName(
					patcharti[CPlayer->inventory[x+i].type]));

				if(CPlayer->inventory[x+i].count > 1)
				{
					DrSmallNumber(CPlayer->inventory[x+i].count, 226+i*31,
 						448);
				}
			}
		}
		GCanvas->DrawPatch1(210+curpos*31, 427, PatchNumSELECTBOX);
		if(x != 0)
		{
			GCanvas->DrawPatch1(200, 427, !(leveltime&4) ? PatchNumINVLFGEM1 :
				PatchNumINVLFGEM2);
		}
		if(CPlayer->inventorySlotNum-x > 7)
		{
			GCanvas->DrawPatch1(428, 427, !(leveltime&4) ? PatchNumINVRTGEM1 : PatchNumINVRTGEM2);
		}
	}
}

//==========================================================================
//
// DrawPossWeaponry
//
//==========================================================================

static void DrawPossWeaponry(void)
{
	int i;
	int j;
	char name[16];

	j = 460;

	for(i = NUMACTUALWEAPONS; i > 0; i--)
	{
		if (NewWeaponInfo[i].classtype != CPlayer->pclass) continue;				
		sprintf(name, "%d: %s",NewWeaponInfo[i].bindkey+1,NewWeaponInfo[i].name);
		if (i == CPlayer->readyweapon && CPlayer->pendingweapon == WP_NOCHANGE)
			MN_DrTextAYellow(name,550,j);
		else MN_DrTextA(name,550,j);
		j -= 10;
	}

	if(CPlayer->plr->mo->health > 0)
	{
		DrBNumber(CPlayer->plr->mo->health, 5, 440);
	}
	else
	{
		DrBNumber(0, 5, 440);
	}

	if(deathmatch)
	{
		int temp = 0;
		for(int i=0; i<MAXPLAYERS; i++)
		{
			if(players[i].plr->ingame)
			{
				temp += CPlayer->plr->frags[i];
			}
		}
		DrINumber(temp, 45, 445);
	}
}


//==========================================================================
//
// Draw_TeleportIcon
//
//==========================================================================

void Draw_TeleportIcon(void)
{
	GCanvas->DrawPatch1(100, 68, gi.W_GetNumForName("teleicon"));
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
	GCanvas->DrawPatch1(100, 68, gi.W_GetNumForName("saveicon"));
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
	GCanvas->DrawPatch1(100, 68, gi.W_GetNumForName("loadicon"));
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
	return false;
}

//===========================================================================
//
// Cheats
//
//===========================================================================

static boolean canCheat()
{
	return !(gameskill == sk_nightmare || (netgame && !netcheat) 
		|| players[consoleplayer].health <= 0);
}

int CCmdCheatGod(int argc, char **argv)
{
	if(!canCheat()) return false; // Can't cheat!
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
	if(!canCheat()) return false; // Can't cheat!
	// Check the arguments.
	if(argc != 3) return false;
	if (!strnicmp(argv[1], "money", 5))
		players[consoleplayer].money+=atoi(argv[2]);
	else if (!strnicmp(argv[1], "experience", 10))
		Give_Experience(players+consoleplayer,atoi(argv[2]));
	else if (!strnicmp(argv[1], "level", 5))
		Give_Level(players+consoleplayer,atoi(argv[2]));
	else if (!strnicmp(argv[1], "magic", 5))
		players[consoleplayer].sp_power += atoi(argv[2]);
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

	tens = tens-'0';
	ones = ones-'0';
	if(tens < 0 || tens > 9 || ones < 0 || ones > 9)
	{ // Bad map
		P_SetMessage(players+consoleplayer, TXT_CHEATBADINPUT, true);
		return false;
	}
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
	return true;
}

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

