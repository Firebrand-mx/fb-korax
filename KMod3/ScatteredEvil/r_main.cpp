
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
#include "h2def.h"
#include "gl_def.h"

int			viewangleoffset = 0;
int			borderUpd = 0;	// Update borders?

//int			newViewAngleOff;

int			validcount = 1;		// increment every time a check is made

int			framecount;		// just for profiling purposes

int			sscount, linecount, loopcount;

fixed_t		viewx, viewy, viewz;
fixed_t		viewxOffset=0, viewyOffset=0, viewzOffset=0;
angle_t		viewangle;
float		viewpitch;			// player->lookdir, global version
fixed_t		viewcos, viewsin;
player_t	*viewplayer;

int			detailshift;		// 0 = high, 1 = low

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



//=============================================================================

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

void R_Init(void)
{
	R_InitData();
	// viewwidth / viewheight are set by the defaults
	R_SetViewSize(0, 0, 640, 480);
	R_InitSkyMap();
	R_InitTranslationTables();
	R_InitSprites();
	GL_InitData();
	framecount = 0;
	R_InitViewBorder();
}

void R_Update(void)
{
	GL_TexDestroy();

	R_UpdateData();
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
		return (subsector_t*)subsectors;

	nodenum = numnodes-1;

	while (! (nodenum & NF_SUBSECTOR) )
	{
		node = &nodes[nodenum];
		side = R_PointOnSide(x, y, node);
		nodenum = node->children[side];
	}

	return &subsectors[nodenum & ~NF_SUBSECTOR];
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
	if (player->ThirdPersonView)
	{
		viewangle = player->ThirdPersonAngle;
		viewpitch = player->ThirdPersonPitch;
		viewx = player->ThirdPersonOrigin[0];
		viewy = player->ThirdPersonOrigin[1];
		viewz = player->ThirdPersonOrigin[2];
	}
	else
	{
		viewangle = player->mo->angle + viewangleoffset;
		viewpitch = player->lookdir * 85.0 / 110.0;
		viewx = player->mo->x + viewxOffset;
		viewy = player->mo->y + viewyOffset;
		viewz = player->viewz + viewzOffset;
	}
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

	if (!player->ThirdPersonView)
	{
		// Orthogonal projection to the view window.
		GL_Restore2DState(1); 
		R_DrawPlayerSprites();
	}

	// Fullscreen viewport.
	GL_Restore2DState(2); 
	if(haloMode) H_Render();

	// Original matrices and state.
	GL_Restore2DState(3); 
}
