
//**************************************************************************
//**
//** r_main.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

#include <math.h>
#include "dd_def.h"
#include "gl_def.h"

int			viewangleoffset = 0;
int			borderUpd = 0;	// Update borders?

//int			newViewAngleOff;

int			validcount = 1;		// increment every time a check is made

int			centerx, centery;

int			framecount;		// just for profiling purposes

int			sscount, linecount, loopcount;

fixed_t		viewx, viewy, viewz;
fixed_t		viewxOffset=0, viewyOffset=0, viewzOffset=0;
angle_t		viewangle;
float		viewpitch;			// player->lookdir, global version
fixed_t		viewcos, viewsin;
player_t	*viewplayer;

int			detailshift;		// 0 = high, 1 = low

//
// precalculated math tables
//
fixed_t		*finecosine = &finesine[FINEANGLES/4];


int			extralight;			// bumped light from gun blasts


/*
===============================================================================
=
= R_PointOnSide
=
= Returns side 0 (front) or 1 (back)
===============================================================================
*/

int	R_PointOnSide (fixed_t x, fixed_t y, node_t *node)
{
	fixed_t	dx,dy;
	fixed_t	left, right;

	if (!node->dx)
	{
		if (x <= node->x)
			return node->dy > 0;
		return node->dy < 0;
	}
	if (!node->dy)
	{
		if (y <= node->y)
			return node->dx < 0;
		return node->dx > 0;
	}

	dx = (x - node->x);
	dy = (y - node->y);

// try to quickly decide by looking at sign bits
	if ( (node->dy ^ node->dx ^ dx ^ dy)&0x80000000 )
	{
		if  ( (node->dy ^ dx) & 0x80000000 )
			return 1;	// (left is negative)
		return 0;
	}

	left = FixedMul ( node->dy>>FRACBITS , dx );
	right = FixedMul ( dy , node->dx>>FRACBITS );

	if (right < left)
		return 0;		// front side
	return 1;			// back side
}


int	R_PointOnSegSide (fixed_t x, fixed_t y, seg_t *line)
{
	fixed_t	lx, ly;
	fixed_t	ldx, ldy;
	fixed_t	dx,dy;
	fixed_t	left, right;

	lx = line->v1->x;
	ly = line->v1->y;

	ldx = line->v2->x - lx;
	ldy = line->v2->y - ly;

	if (!ldx)
	{
		if (x <= lx)
			return ldy > 0;
		return ldy < 0;
	}
	if (!ldy)
	{
		if (y <= ly)
			return ldx < 0;
		return ldx > 0;
	}

	dx = (x - lx);
	dy = (y - ly);

// try to quickly decide by looking at sign bits
	if ( (ldy ^ ldx ^ dx ^ dy)&0x80000000 )
	{
		if  ( (ldy ^ dx) & 0x80000000 )
			return 1;	// (left is negative)
		return 0;
	}

	left = FixedMul ( ldy>>FRACBITS , dx );
	right = FixedMul ( dy , ldx>>FRACBITS );

	if (right < left)
		return 0;		// front side
	return 1;			// back side
}


/*
===============================================================================
=
= R_PointToAngle
=
===============================================================================
*/

// to get a global angle from cartesian coordinates, the coordinates are
// flipped until they are in the first octant of the coordinate system, then
// the y (<=x) is scaled and divided by x to get a tangent (slope) value
// which is looked up in the tantoangle[] table.  The +1 size is to handle
// the case when x==y without additional checking.
#define	SLOPERANGE	2048
#define	SLOPEBITS	11
#define	DBITS		(FRACBITS-SLOPEBITS)


extern	int	tantoangle[SLOPERANGE+1];		// get from tables.c

// int	tantoangle[SLOPERANGE+1];

int SlopeDiv (unsigned num, unsigned den)
{
	unsigned ans;
	if (den < 512)
		return SLOPERANGE;
	ans = (num<<3)/(den>>8);
	return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

angle_t R_PointToAngle (fixed_t x, fixed_t y)
{
	x -= viewx;
	y -= viewy;
	if ( (!x) && (!y) )
		return 0;
	if (x>= 0)
	{	// x >=0
		if (y>= 0)
		{	// y>= 0
			if (x>y)
				return tantoangle[ SlopeDiv(y,x)];     // octant 0
			else
				return ANG90-1-tantoangle[ SlopeDiv(x,y)];  // octant 1
		}
		else
		{	// y<0
			y = -y;
			if (x>y)
				return -tantoangle[SlopeDiv(y,x)];  // octant 8
			else
				return ANG270+tantoangle[ SlopeDiv(x,y)];  // octant 7
		}
	}
	else
	{	// x<0
		x = -x;
		if (y>= 0)
		{	// y>= 0
			if (x>y)
				return ANG180-1-tantoangle[ SlopeDiv(y,x)]; // octant 3
			else
				return ANG90+ tantoangle[ SlopeDiv(x,y)];  // octant 2
		}
		else
		{	// y<0
			y = -y;
			if (x>y)
				return ANG180+tantoangle[ SlopeDiv(y,x)];  // octant 4
			else
				return ANG270-1-tantoangle[ SlopeDiv(x,y)];  // octant 5
		}
	}

	return 0;
}


angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
	viewx = x1;
	viewy = y1;
	return R_PointToAngle (x2, y2);
}


fixed_t	R_PointToDist (fixed_t x, fixed_t y)
{
	int		angle;
	fixed_t	dx, dy, temp;
	fixed_t	dist;

	dx = abs(x - viewx);
	dy = abs(y - viewy);

	if (dy>dx)
	{
		temp = dx;
		dx = dy;
		dy = temp;
	}

	angle = (tantoangle[ FixedDiv(dy,dx)>>DBITS ]+ANG90) >> ANGLETOFINESHIFT;

	dist = FixedDiv (dx, finesine[angle] );	// use as cosine

	return dist;
}



/*
=================
=
= R_InitPointToAngle
=
=================
*/

void R_InitPointToAngle (void)
{
// now getting from tables.c
#if 0
	int	i;
	long	t;
	float	f;
//
// slope (tangent) to angle lookup
//
	for (i=0 ; i<=SLOPERANGE ; i++)
	{
		f = atan( (float)i/SLOPERANGE )/(3.141592657*2);
		t = 0xffffffff*f;
		tantoangle[i] = t;
	}
#endif
}

//=============================================================================

/*
================
=
= R_ScaleFromGlobalAngle
=
= Returns the texture mapping scale for the current line at the given angle
= rw_distance must be calculated first
================
*/

fixed_t R_ScaleFromGlobalAngle (angle_t visangle)
{
/*	fixed_t		scale;
	int			anglea, angleb;
	int			sinea, sineb;
	fixed_t		num,den;

#if 0
{
	fixed_t		dist,z;
	fixed_t		sinv, cosv;

	sinv = finesine[(visangle-rw_normalangle)>>ANGLETOFINESHIFT];
	dist = FixedDiv (rw_distance, sinv);
	cosv = finecosine[(viewangle-visangle)>>ANGLETOFINESHIFT];
	z = abs(FixedMul (dist, cosv));
	scale = FixedDiv(projection, z);
	return scale;
}
#endif

	anglea = ANG90 + (visangle-viewangle);
	angleb = ANG90 + (visangle-rw_normalangle);
// bothe sines are allways positive
	sinea = finesine[anglea>>ANGLETOFINESHIFT];
	sineb = finesine[angleb>>ANGLETOFINESHIFT];
	num = FixedMul(projection,sineb)<<detailshift;
	den = FixedMul(rw_distance,sinea);
	if (den > num>>16)
	{
		scale = FixedDiv (num, den);
		if (scale > 64*FRACUNIT)
			scale = 64*FRACUNIT;
		else if (scale < 256)
			scale = 256;
	}
	else
		scale = 64*FRACUNIT;

	return scale;*/
	return 0;
}



/*
=================
=
= R_InitTables
=
=================
*/

void R_InitTables (void)
{
// now getting from tables.c
#if 0
	int		i;
	float		a, fv;
	int			t;

//
// viewangle tangent table
//
	for (i=0 ; i<FINEANGLES/2 ; i++)
	{
		a = (i-FINEANGLES/4+0.5)*PI*2/FINEANGLES;
		fv = FRACUNIT*tan (a);
		t = fv;
		finetangent[i] = t;
	}

//
// finesine table
//
	for (i=0 ; i<5*FINEANGLES/4 ; i++)
	{
// OPTIMIZE: mirror...
		a = (i+0.5)*PI*2/FINEANGLES;
		t = FRACUNIT*sin (a);
		finesine[i] = t;
	}
#endif

}


/*
=================
=
= R_InitTextureMapping
=
=================
*/

void R_InitTextureMapping (void)
{
}

//=============================================================================

/*
====================
=
= R_InitLightTables
=
= Only inits the zlight table, because the scalelight table changes
= with view size
=
====================
*/

#define		DISTMAP	2

void R_InitLightTables (void)
{
}


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

void R_SetViewSize(int x, int y, int w, int h)
{
	viewwindowx = x;
	viewwindowy = y;
	viewwidth = w;
	viewheight = h;
}

/*
==============
=
= R_ExecuteSetViewSize
=
==============
*/

void R_ExecuteSetViewSize (void)
{
	//fixed_t	cosadj, dy;
//	int		i,j, level, startmap;

	setsizeneeded = false;

/*	if (setblocks == 11)
	{*/
//		scaledviewwidth = SCREENWIDTH;
		//viewheight = SCREENHEIGHT;
/*	}
	else
	{
		scaledviewwidth = setblocks*32;
		viewheight = (setblocks*(200-SBARHEIGHT*sbarscale/20)/10);
	}*/

//	detailshift = setdetail;
//	viewwidth = scaledviewwidth>>detailshift;

	centery = viewheight/2;
	centerx = viewwidth/2;
//	centerxfrac = centerx<<FRACBITS;
//	centeryfrac = centery<<FRACBITS;
//	projection = 0xffff<<FRACBITS;//centerxfrac;

	/*if (!detailshift)
	{
		colfunc = basecolfunc = R_DrawColumn;
		fuzzcolfunc = R_DrawFuzzColumn;
		transcolfunc = R_DrawTranslatedColumn;
		spanfunc = R_DrawSpan;
	}
	else
	{
		colfunc = basecolfunc = R_DrawColumnLow;
		fuzzcolfunc = R_DrawFuzzColumn;
		transcolfunc = R_DrawTranslatedColumn;
		spanfunc = R_DrawSpanLow;
	}*/

	//R_InitBuffer (scaledviewwidth, viewheight);

//	R_InitTextureMapping ();

//
// psprite scales
//
/*	pspritescale = FRACUNIT*viewwidth/SCREENWIDTH;
	pspriteiscale = FRACUNIT*SCREENWIDTH/viewwidth;*/

//
// thing clipping
//
/*	for (i=0 ; i<viewwidth ; i++)
		screenheightarray[i] = viewheight;*/

//
// planes
//
	/*for (i=0 ; i<viewheight ; i++)
	{
		dy = ((i-viewheight/2)<<FRACBITS)+FRACUNIT/2;
		dy = abs(dy);
		yslope[i] = FixedDiv ( (viewwidth<<detailshift)/2*FRACUNIT, dy);
	}

	for (i=0 ; i<viewwidth ; i++)
	{
		cosadj = abs(finecosine[xtoviewangle[i]>>ANGLETOFINESHIFT]);
		distscale[i] = FixedDiv (FRACUNIT,cosadj);
	}*/

//
// Calculate the light levels to use for each level / scale combination
//
	/*for (i=0 ; i< LIGHTLEVELS ; i++)
	{
		startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
		for (j=0 ; j<MAXLIGHTSCALE ; j++)
		{
			level = startmap - j*SCREENWIDTH/(viewwidth<<detailshift)/DISTMAP;
			if (level < 0)
				level = 0;
			if (level >= NUMCOLORMAPS)
				level = NUMCOLORMAPS-1;
			scalelight[i][j] = colormaps + level*256;
		}
	}*/

//
// draw the border
//
	R_DrawViewBorder ();    // erase old menu stuff
}


int skyflatnum;
char skyflatname[9] = "F_SKY";

//==========================================================================
//
// R_InitSkyMap
//
// Called whenever the view size changes.
//
//==========================================================================

void R_InitSkyMap(void)
{
	skyflatnum = R_FlatNumForName(skyflatname);
}
/*
==============
=
= R_Init
=
==============
*/

int detailLevel;
//int screenblocks;

void R_Init(void)
{
	R_InitData();
	R_InitPointToAngle();
	R_InitTables();
	// viewwidth / viewheight / detailLevel are set by the defaults
//	R_SetViewSize(screenblocks, detailLevel);
	R_SetViewSize(0, 0, 320, 200);
	R_InitLightTables();
	R_InitSkyMap();
	R_InitTranslationTables();
	R_InitSprites();
	// Call the game DLL's refresh initialization, if necessary.
	if(gx.R_Init) gx.R_Init();
	GL_InitData();
	framecount = 0;
	R_InitViewBorder();
}

void R_Update(void)
{
	GL_TexDestroy();

	R_UpdateData();
	R_InitLightTables();
	R_InitSkyMap();
	R_UpdateTranslationTables();
	
	GL_TexInit();
}

/*
==============
=
= R_PointInSubsector
=
==============
*/

subsector_t *R_PointInSubsector (fixed_t x, fixed_t y)
{
	node_t	*node;
	int		side, nodenum;

	if (!numnodes)				// single subsector is a special case
		return (subsector_t*) subsectors;

	nodenum = numnodes-1;

	while (! (nodenum & NF_SUBSECTOR) )
	{
		node = NODE_PTR(nodenum);
		side = R_PointOnSide (x, y, node);
		nodenum = node->children[side];
	}

	//return &subsectors[nodenum & ~NF_SUBSECTOR];
	return SUBSECTOR_PTR(nodenum & ~NF_SUBSECTOR);

}


//----------------------------------------------------------------------------
//
// PROC R_SetupFrame
//
//----------------------------------------------------------------------------

void R_SetupFrame(player_t *player)
{
	int tableAngle;
	extern int BorderRefreshCount;

	viewplayer = player;
	viewangle = player->mo->angle + viewangleoffset;
	viewpitch = player->lookdir;
	viewx = player->mo->x + viewxOffset;
	viewy = player->mo->y + viewyOffset;
	viewz = player->viewz + viewzOffset;
	extralight = player->extralight;
	tableAngle = viewangle>>ANGLETOFINESHIFT;
	viewsin = finesine[tableAngle];
	viewcos = finecosine[tableAngle];
	sscount = 0;
	validcount++;
	if(BorderNeedRefresh || borderUpd)
	{
		R_DrawViewBorder();
		BorderNeedRefresh = false;
		BorderTopRefresh = false;
		UpdateState |= I_FULLSCRN;
	}
	if(BorderTopRefresh)
	{
		if(viewwindowx > 0)
		{
			R_DrawTopBorder();
		}
		BorderTopRefresh = false;
		UpdateState |= I_MESSAGES;
	}
}

/*
==============
=
= R_RenderView
=
==============
*/

void R_RenderPlayerView (player_t *player)
{
	map_rendered = true;

	R_SetupFrame (player);
	R_ClearSprites ();

	NetUpdate();				// check for new console commands
	if(!map_rendered) return;	// What? Quit now!
	
	GL_SwitchTo3DState();
	R_RenderMap();
	
	NetUpdate();				// check for new console commands
	if(!map_rendered)			// What? Quit now!
	{
		GL_Restore2DState(3); 
		return;	
	}

	R_DrawMasked ();
	
	NetUpdate();				// check for new console commands
	if(!map_rendered)			// What? Quit now!
	{
		GL_Restore2DState(3); 
		return;	
	}

	// Orthogonal projection to the view window.
	GL_Restore2DState(1); 
	gx.DrawPlayerSprites( (ddplayer_t*) viewplayer);

	// Fullscreen viewport.
	GL_Restore2DState(2); 
	if(haloMode) H_Render();

	// Original matrices and state.
	GL_Restore2DState(3); 
}
