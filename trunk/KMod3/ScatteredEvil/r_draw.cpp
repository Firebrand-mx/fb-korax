
//**************************************************************************
//**
//** r_draw.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

#include "h2def.h"
#include "gl_def.h"

/*

All drawing to the view buffer is accomplished in this file.  The other refresh
files only know about ccordinates, not the architecture of the frame buffer.

*/

byte *viewimage;

// The view window.
int viewwidth, viewheight, viewwindowx, viewwindowy;

// View border width.
int bwidth;


enum // A logical ordering (twice around).
{
	BG_BACKGROUND,
	BG_TOP,
	BG_RIGHT,
	BG_BOTTOM,
	BG_LEFT,
	BG_TOPLEFT,
	BG_TOPRIGHT,
	BG_BOTTOMRIGHT,
	BG_BOTTOMLEFT
};

// The view border graphics.
char borderGfx[9][9];

byte *dc_translation;
byte *translationtables;


void R_SetBorderGfx(char *gfx[9])
{
	int		i;
	for(i=0; i<9; i++)
		if(gfx[i]) 
			strcpy(borderGfx[i], gfx[i]);
		else
			strcpy(borderGfx[i], "-");

	R_InitViewBorder();
}					

//--------------------------------------------------------------------------
//
// PROC R_InitTranslationTables
//
//--------------------------------------------------------------------------

void R_InitTranslationTables (void)
{
	int i;
	byte *transLump;

	// Load tint table (I don't need this -jk).
	//tinttable = W_CacheLumpName("TINTTAB", PU_STATIC);

	// Allocate translation tables
	translationtables = (byte *)Z_Malloc(256*3*(MAXPLAYERS-1)+255, 
		PU_REFRESHTRANS, 0);
	translationtables = (byte *)(((int)translationtables+255)&~255);

	for(i = 0; i < 3*(MAXPLAYERS-1); i++)
	{
		// If this can't be found, it's reasonable to expect that the game dll
		// will initialize the translation tables as it wishes.
		if(W_CheckNumForName("trantbl0") < 0) break;

		transLump = (byte *)W_CacheLumpNum(W_GetNumForName("trantbl0")+i, PU_STATIC);
		memcpy(translationtables+i*256, transLump, 256);
		Z_Free(transLump);
	}
}

void R_UpdateTranslationTables (void)
{
	Z_FreeTags(PU_REFRESHTRANS, PU_REFRESHTRANS);
	R_InitTranslationTables();
}


/*
================
=
= R_InitBuffer
=
=================
*/

void R_InitBuffer (int width, int height)
{
}


void R_InitViewBorder()
{
	patch_t *patch;

	// Detemine the view border width.
	if(W_CheckNumForName(borderGfx[BG_TOP]) == -1) return;
	patch = (patch_t *)W_CacheLumpName(borderGfx[BG_TOP], PU_CACHE);
	bwidth = patch->height;
}

/*
==================
=
= R_DrawViewBorder
=
= Draws the border around the view for different size windows
==================
*/

boolean BorderNeedRefresh;

void R_DrawViewBorder (void)
{
	int		lump;

	if(viewwidth == 640 && viewheight == 480) return;

	int x = viewwindowx;
	int y = viewwindowy;
	int w = viewwidth;
	int h = viewheight;

	// View background.
	GL_SetColorAndAlpha(1,1,1,1);
	GL_SetFlat(R_FlatNumForName(borderGfx[BG_BACKGROUND]));
	GL_DrawCutRectTiled(0, 0, 640, 480, 64, 64,
		x-bwidth, y-bwidth, w+2*bwidth, h+2*bwidth);

	// The border top.
	GL_SetPatch(lump=W_GetNumForName(borderGfx[BG_TOP]));
	GL_DrawRectTiled(x, y-bwidth, w, 
		lumptexsizes[lump].h, 16, lumptexsizes[lump].h);
	// Border bottom.
	GL_SetPatch(lump=W_GetNumForName(borderGfx[BG_BOTTOM]));
	GL_DrawRectTiled(x, y+h, w, 
		lumptexsizes[lump].h, 16, lumptexsizes[lump].h);

	// Left view border.
	GL_SetPatch(lump=W_GetNumForName(borderGfx[BG_LEFT]));
	GL_DrawRectTiled(x-bwidth, y, lumptexsizes[lump].w, h, 
		lumptexsizes[lump].w, 16);
	// Right view border.
	GL_SetPatch(lump=W_GetNumForName(borderGfx[BG_RIGHT]));
	GL_DrawRectTiled(x+w, y, lumptexsizes[lump].w, h, 
		lumptexsizes[lump].w, 16);

	GL_UsePatchOffset(false);
	GL_DrawPatch(x-bwidth, y-bwidth, 
		W_GetNumForName(borderGfx[BG_TOPLEFT]));
	GL_DrawPatch(x+w, y-bwidth, 
		W_GetNumForName(borderGfx[BG_TOPRIGHT]));
	GL_DrawPatch(x+w, y+h, 
		W_GetNumForName(borderGfx[BG_BOTTOMRIGHT]));
	GL_DrawPatch(x-bwidth, y+h, 
		W_GetNumForName(borderGfx[BG_BOTTOMLEFT]));
	GL_UsePatchOffset(true);
}

/*
==================
=
= R_DrawTopBorder
=
= Draws the top border around the view for different size windows
==================
*/

boolean BorderTopRefresh;

void R_DrawTopBorder (void)
{
	if (viewwidth == 640 && viewheight == 480)
		return;

	GL_SetColorAndAlpha(1,1,1,1);
	GL_SetFlat(R_FlatNumForName(borderGfx[BG_BACKGROUND]));
	
	int x = viewwindowx;
	int y = viewwindowy;
	int w = viewwidth;
	int h = viewheight;

	GL_DrawRectTiled(0, 0, 640, 64, 64, 64);
	if(viewwindowy < 65)
	{
		int lump;
		GL_SetPatch(lump=W_GetNumForName(borderGfx[BG_TOP]));
		GL_DrawRectTiled(x, y-bwidth, w, 
			lumptexsizes[lump].h, 16, lumptexsizes[lump].h);

		GL_UsePatchOffset(false);
		GL_DrawPatch(x-bwidth, y, W_GetNumForName(borderGfx[BG_LEFT]));
		GL_DrawPatch(x+viewwidth, y, W_GetNumForName(borderGfx[BG_RIGHT]));
		GL_DrawPatch(x-bwidth, y+16, W_GetNumForName(borderGfx[BG_LEFT]));
		GL_DrawPatch(x+viewwidth, y+16, W_GetNumForName(borderGfx[BG_RIGHT]));

		GL_DrawPatch(x-bwidth, y-bwidth, W_GetNumForName(borderGfx[BG_TOPLEFT]));
		GL_DrawPatch(x+viewwidth, y-bwidth, W_GetNumForName(borderGfx[BG_TOPRIGHT]));
		GL_UsePatchOffset(true);
	}
}




