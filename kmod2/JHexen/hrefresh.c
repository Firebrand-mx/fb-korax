
//**************************************************************************
//**
//** HREFRESH.C
//**
//** Hexen-specific refresh stuff.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "h2def.h"
#include "p_local.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// This could hold much more detailed information...
typedef struct
{
	char	name[9];	// Name of the texture.
	int		type;		// Which type?
} textype_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void X_Drawer();
void P_SetDDFlags();
void H2_AdvanceDemo(void);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void PageDrawer(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern boolean automapactive;
extern boolean MenuActive;
extern boolean askforquit;
extern boolean advancedemo;
extern float lookOffset;

extern boolean dontrender;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int demosequence;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int pagetic;
static char *pagename;

static textype_t	*texTypes = 0;
static int			numTexTypes = 0;

// CODE --------------------------------------------------------------------

/*
==============
=
= R_SetViewSize
=
= Don't really change anything here, because i might be in the middle of
= a refresh.  The change will take effect next refresh.
=
==============
*/

boolean	setsizeneeded;
int		setblocks/*, setdetail*/;

void R_SetViewSize (int blocks, int detail)
{
	setsizeneeded = true;
	setblocks = blocks;
	gi.Update(DDUF_BORDER);
	//setdetail = detail;
}

void R_HandleSectorSpecials()
{
	sector_t	*sect = sectors;
	int			i, scrollOffset = leveltime>>1 & 63;

	for(i=0; i<numsectors; i++, sect++)
	{
		switch(sect->special)
		{ // Handle scrolling flats
		case 201: case 202: case 203: // Scroll_North_xxx
			sect->flatoffy = (63-scrollOffset) << (sect->special-201);
			break;
		case 204: case 205: case 206: // Scroll_East_xxx
			sect->flatoffx = (63-scrollOffset) << (sect->special-204);
			break;
		case 207: case 208: case 209: // Scroll_South_xxx
			sect->flatoffy = scrollOffset << (sect->special-207);
			break;
		case 210: case 211: case 212: // Scroll_West_xxx
			sect->flatoffx = scrollOffset << (sect->special-210);
			break;
		case 213: case 214: case 215: // Scroll_NorthWest_xxx
			sect->flatoffx = scrollOffset << (sect->special-213);
			sect->flatoffy = (63-scrollOffset) << (sect->special-213);
			break;
		case 216: case 217: case 218: // Scroll_NorthEast_xxx
			sect->flatoffx = (63-scrollOffset) << (sect->special-216);
			sect->flatoffy = (63-scrollOffset) << (sect->special-216);
			break;
		case 219: case 220: case 221: // Scroll_SouthEast_xxx
			sect->flatoffx = (63-scrollOffset) << (sect->special-219);
			sect->flatoffy = scrollOffset << (sect->special-219);
			break;
		case 222: case 223: case 224: // Scroll_SouthWest_xxx
			sect->flatoffx = scrollOffset << (sect->special-222);
			sect->flatoffy = scrollOffset << (sect->special-222);
			break;
		default:
			sect->flatoffx = sect->flatoffy = 0;
			break;
		}
	}
}

//==========================================================================
//
// G_Drawer
//
//==========================================================================

void G_Drawer(void)
{
	// Change the view size if needed.
	if(setsizeneeded)
	{
		setsizeneeded = false;
		if(setblocks > 10 
			|| players[consoleplayer].class >= PCLASS_ETTIN) //Remi: Possessed always big screen
		{
			// Full screen.
			gi.ViewWindow(0, 0, 320, 200);
		}
		else
		{
			int w = setblocks*32;
			int h = setblocks*(200-SBARHEIGHT*sbarscale/20)/10;
			gi.ViewWindow(160-(w>>1), (200-SBARHEIGHT*sbarscale/20-h)>>1, w, h);
		}
	}

	// Do buffered drawing
	switch(gamestate)
	{
		case GS_LEVEL:
			if(!gametic)
			{
				break;
			}
			if(automapactive)
			{
				AM_Drawer();
			}
			else
			{
				boolean special200 = false;
				R_HandleSectorSpecials();
				P_SetDDFlags();
				// The display player cannot be seen.
				players[displayplayer].plr->mo->ddflags |= DDMF_DONTDRAW;
				// Check for the sector special 200: use sky2.
				// I wonder where this is used?
				if(players[displayplayer].plr->mo->subsector->sector->special == 200)
				{
					special200 = true;
					gi.SkyParams(0, DD_DISABLE, 0);
					gi.SkyParams(1, DD_ENABLE, 0);
				}
				// How about a bit of quake?
				if(localQuakeHappening[displayplayer] && !paused)
				{
					int intensity = localQuakeHappening[displayplayer];
					gi.Set(DD_VIEWX_OFFSET, ((M_Random() % (intensity<<2))
							-(intensity<<1))<<FRACBITS);
					gi.Set(DD_VIEWY_OFFSET, ((M_Random() % (intensity<<2))
							-(intensity<<1))<<FRACBITS);
					players[consoleplayer].plr->mo->ddflags |= DDMF_DONTDRAW;
				}
				else
				{
					gi.Set(DD_VIEWX_OFFSET, 0);
					gi.Set(DD_VIEWY_OFFSET, 0);
				}
				// The view angle offset.
				gi.Set(DD_VIEWANGLE_OFFSET, ANGLE_MAX * -lookOffset);
				// Render the view.
				if(!dontrender)
					gi.RenderPlayerView(players[displayplayer].plr);
				if(special200)
				{
					gi.SkyParams(0, DD_ENABLE, 0);
					gi.SkyParams(1, DD_DISABLE, 0);
				}
				X_Drawer(); // Draw the crosshair.
			}
			gi.Update(DDUF_FULLVIEW);
			SB_Drawer();
			// We'll draw the chat text *after* the status bar to
			// be a bit clearer.
			CT_Drawer();
			break;
		case GS_INTERMISSION:
			IN_Drawer();
			break;
		case GS_FINALE:
			F_Drawer();
			break;
		case GS_DEMOSCREEN:
			PageDrawer();
			break;
	}

	if(paused && !MenuActive && !askforquit)
	{
		if(!netgame)
		{
			gi.GL_DrawPatch(160, gi.Get(DD_VIEWWINDOW_Y)+5, gi.W_GetNumForName("PAUSED"));
		}
		else
		{
			gi.GL_DrawPatch(160, 70, gi.W_GetNumForName("PAUSED"));
		}
	}
}

//==========================================================================
//
// PageDrawer
//
//==========================================================================

static void PageDrawer(void)
{
	gi.GL_DrawRawScreen(gi.W_GetNumForName(pagename));
	if(demosequence == 1)
	{
		gi.GL_DrawPatch(4, 160, gi.W_GetNumForName("ADVISOR"));
	}
	gi.Update(DDUF_FULLSCREEN);
}

#define FMAKERGBA(r,g,b,a) ( (byte)(0xff*r) + ((byte)(0xff*g)<<8) + ((byte)(0xff*b)<<16) + ((byte)(0xff*a)<<24) )

void H2_SetFilter(int filter)
{
	int rgba = 0;

	// We have to choose the right color and alpha.
	if(filter >= STARTREDPALS && filter < STARTREDPALS+NUMREDPALS) 
		// Red?
		rgba = FMAKERGBA(1, 0, 0, filter/9.0);	// Full red with filter 8.
	else if(filter >= STARTBONUSPALS && filter < STARTBONUSPALS+NUMBONUSPALS) 
		// Light Yellow?
		rgba = FMAKERGBA(1, 1, .5, (filter-STARTBONUSPALS+1)/16.0); 
	else if(filter >= STARTPOISONPALS && filter < STARTPOISONPALS+NUMPOISONPALS)
		// Green?
		rgba = FMAKERGBA(0, 1, 0, (filter-STARTPOISONPALS+1)/16.0);
	else if(filter >= STARTSCOURGEPAL)
		// Orange?
		rgba = FMAKERGBA(1, .5, 0, (STARTSCOURGEPAL+3-filter)/6.0);
	else if(filter >= STARTHOLYPAL)
		// White?
		rgba = FMAKERGBA(1, 1, 1, (STARTHOLYPAL+3-filter)/6.0);
	else if(filter == STARTICEPAL)
		// Light blue?
		rgba = FMAKERGBA(.5f, .5f, 1, .4f);
	else if(filter)
		gi.Error("H2_SetFilter: Real strange filter number: %d.\n", filter);

	gi.GL_SetFilter(rgba);		
}

void H2_EndFrame(void)
{
	S_UpdateSounds(players[displayplayer].plr->mo);
}

void H2_ConsoleBg(int *width, int *height)
{
	extern int consoleFlat;
	extern float consoleZoom;

	gi.GL_SetFlat(consoleFlat);
	*width = 64*consoleZoom;
	*height = 64*consoleZoom;
}


//==========================================================================
//
// H2_PageTicker
//
//==========================================================================

void H2_PageTicker(void)
{
	if(--pagetic < 0)
	{
		H2_AdvanceDemo();
	}
}

//==========================================================================
//
// H2_DoAdvanceDemo
//
//==========================================================================

void H2_DoAdvanceDemo(void)
{
	players[consoleplayer].playerstate = PST_LIVE; // don't reborn
	advancedemo = false;
	usergame = false; // can't save/end game here
	paused = false;
	gameaction = ga_nothing;
	demosequence = (demosequence+1)%7;
	switch(demosequence)
	{
		case 0:
			pagetic = 280;
			gamestate = GS_DEMOSCREEN;
			pagename = "TITLE";
			S_StartSongName("hexen", true);
			break;
		case 1:
			pagetic = 210;
			gamestate = GS_DEMOSCREEN;
			pagename = "TITLE";
			break;
		case 2:
			gi.Update(DDUF_BORDER | DDUF_FULLSCREEN);
			G_DeferedPlayDemo("demo1");
			break;
		case 3:
			pagetic = 200;
			gamestate = GS_DEMOSCREEN;
			pagename = "CREDIT";
			break;
		case 4:
			gi.Update(DDUF_BORDER | DDUF_FULLSCREEN);
			G_DeferedPlayDemo("demo2");
			break;
		case 5:
			pagetic = 200;
			gamestate = GS_DEMOSCREEN;
			pagename = "CREDIT";
			break;
		case 6:
			gi.Update(DDUF_BORDER | DDUF_FULLSCREEN);			
			G_DeferedPlayDemo("demo3");
			break;
	}
}


//==========================================================================
//
//
//
//==========================================================================

/*
========================
=
= R_DrawPSprite
=
========================
*/

player_t *viewplayer;

// Y-adjustment values for full screen (4 weapons)
/*int PSpriteSY[NUMCLASSES][NUMWEAPONS] =
{
	{ 0, -12*FRACUNIT, -10*FRACUNIT, 10*FRACUNIT }, // Fighter
	{ -8*FRACUNIT, 10*FRACUNIT, 10*FRACUNIT, 0 }, // Cleric
	{ 9*FRACUNIT, 20*FRACUNIT, 20*FRACUNIT, 20*FRACUNIT }, // Mage 
	{ 10*FRACUNIT, 10*FRACUNIT, 10*FRACUNIT, 10*FRACUNIT } // Pig
};*/

void R_DrawPSprite (pspdef_t *psp)
{
	fixed_t		tx;
	int			x1; 
	spriteinfo_t sprinfo;
	float		light, alpha;
	int			y;

	// Get the sprite info.
	gi.GetSpriteInfo(psp->state->sprite, psp->state->frame, &sprinfo);

	// Calculate edges of the shape.
	tx = psp->sx - (160 + lookOffset*1300) * FRACUNIT;

	tx -= sprinfo.offset;
	x1 = ((160<<FRACBITS)+tx ) >>FRACBITS;
	// Set the OpenGL color & alpha.
	light = 1;
	alpha = 1;
	if(viewplayer->powers[pw_invulnerability] && viewplayer->class
		== PCLASS_CLERIC)
	{
		if(viewplayer->powers[pw_invulnerability] > 4*32)
		{
			if(viewplayer->plr->mo->flags2 & MF2_DONTDRAW)
			{ // don't draw the psprite
				alpha = .333f;
			}
			else if(viewplayer->plr->mo->flags & MF_SHADOW)
			{
				alpha = .666f;
			}
		}
		else if(viewplayer->powers[pw_invulnerability]&8)
		{
			alpha = .333f;
		}
	}	
	else if(psp->state->frame & FF_FULLBRIGHT)
	{
		// Full bright
		light = 1;
	}
	else
	{
		// local light
		light = viewplayer->plr->mo->subsector->sector->lightlevel / 255.0;
	}

	// Do some OpenGL rendering, oh yeah.
	y = -(sprinfo.topOffset>>FRACBITS)+(psp->sy>>FRACBITS);
	if(gi.Get(DD_VIEWWINDOW_HEIGHT) == SCREENHEIGHT 
		|| players[consoleplayer].class >= PCLASS_ETTIN) //Possessed always big screen
	{
		//y += PSpriteSY[viewplayer->class][players[consoleplayer].readyweapon] >> FRACBITS;
		y += NewWeaponInfo[players[consoleplayer].readyweapon].PSpriteSY >> FRACBITS;
	}
	else 
	{
		y -= (39*sbarscale)/40;
		y += (NewWeaponInfo[players[consoleplayer].readyweapon].PSpriteSY >> FRACBITS) * (20-sbarscale)/20.0f;
	}
	light += .1f;	// Add some extra light.
	gi.GL_SetColorAndAlpha(light, light, light, alpha);
	gi.GL_DrawPSprite(x1, y, 1, sprinfo.flip, sprinfo.lump);
}

/*
========================
=
= R_DrawPlayerSprites
=
========================
*/

void R_DrawPlayerSprites(ddplayer_t *viewplr)
{
	int			i;
	pspdef_t	*psp;

	viewplayer = (player_t*) viewplr->extradata;
	for (i=0, psp=viewplayer->psprites ; i<NUMPSPRITES ; i++,psp++)
		if (psp->state)
			R_DrawPSprite (psp);
}

//==========================================================================
// Texture types

#if 0

static void readline(char *buffer, int len, FILE *file)
{
	int		p;

	fgets(buffer, len, file);
	p = strlen(buffer)-1;
	if(buffer[p] == '\n') buffer[p] = 0;
}

static char *firstchar(char *buffer)
{
	int		i = 0;

	while(isspace(buffer[i]) && buffer[i]) i++;
	return buffer + i;
}

void R_LoadTextureTypes()
{
	FILE	*file;
	char	buff[256], *ptr;
	int		curtype = TEXTYPE_UNKNOWN;
	textype_t *tt;

	if((file=fopen("textypes.txt", "rt")) == NULL)
		return;

	for(readline(buff, 255, file); !feof(file); readline(buff, 255, file))
	{
		ptr = firstchar(buff);
		if(*ptr == '#') continue; // A comment.
		if(*ptr == '*')
		{
			ptr = firstchar(ptr+1);
			if(!stricmp(ptr, "metal"))
				curtype = TEXTYPE_METAL;
			else if(!stricmp(ptr, "rock"))
				curtype = TEXTYPE_ROCK;
			else if(!stricmp(ptr, "wood"))
				curtype = TEXTYPE_WOOD;
			else if(!stricmp(ptr, "cloth"))
				curtype = TEXTYPE_CLOTH;
			continue;
		}
		// Allocate a new textype entry.
		texTypes = realloc(texTypes, sizeof(textype_t) * ++numTexTypes);
		// -JL- Paranoia
		if (!texTypes)
			gi.Error("R_LoadTextureTypes: realloc failed");
		tt = texTypes + numTexTypes-1;
		memset(tt, 0, sizeof(*tt));
		strncpy(tt->name, ptr, 8);
		tt->type = curtype;
	}
	gi.Message( "%d texture types loaded.\n", numTexTypes);
	fclose(file);
}

void R_FreeTextypeTypes()
{
	free(texTypes);
	texTypes = 0;
	numTexTypes = 0;
}

int R_TextureTypeForName(char *name)
{
	int		i;

	for(i=0; i<numTexTypes; i++)
		if(!strnicmp(name, texTypes[i].name, 8))
			return texTypes[i].type;
	return TEXTYPE_UNKNOWN;
}

#endif