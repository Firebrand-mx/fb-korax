
//**************************************************************************
//**
//** r_things.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dd_def.h"
#include "i_win32.h"
#include "gl_def.h"
#include "m_bams.h"
#include "console.h"

/*

Sprite rotation 0 is facing the viewer, rotation 1 is one angle turn CLOCKWISE around the axis.
This is not the same as the angle, which increases counter clockwise
(protractor).  There was a lot of stuff grabbed wrong, so I changed it...

*/


int LevelFullBright = false;

/*
===============================================================================

						INITIALIZATION FUNCTIONS

===============================================================================
*/

int				alwaysAlign = 0;

// variables used to look up and range check thing_t sprites patches
spritedef_t		*sprites = 0;
int				numsprites;

spriteframe_t	sprtemp[30];
int				maxframe;
char			*spritename;
char			**spritenamelist = NULL;


void R_SetSpriteNameList(char **namelist)
{
	spritenamelist = namelist;
}



/*
=================
=
= R_InstallSpriteLump
=
= Local function for R_InitSprites
=================
*/

void R_InstallSpriteLump (int lump, unsigned frame, unsigned rotation, boolean flipped)
{
	int		r;

	if (frame >= 30 || rotation > 8)
		I_Error ("R_InstallSpriteLump: Bad frame characters in lump %i", lump);

	if ((int)frame > maxframe)
		maxframe = frame;

	if (rotation == 0)
	{
// the lump should be used for all rotations
		// -JL- Allow sprite replacements.
/*		if (sprtemp[frame].rotate == false)
			I_Error ("R_InitSprites: Sprite %s frame %c has multip rot=0 lump"
			, spritename, 'A'+frame);
		if (sprtemp[frame].rotate == true)
			I_Error ("R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump"
			, spritename, 'A'+frame);*/

		sprtemp[frame].rotate = false;
		for (r=0 ; r<8 ; r++)
		{
			sprtemp[frame].lump[r] = lump - firstspritelump;
			sprtemp[frame].flip[r] = (byte)flipped;
		}
		return;
	}

// the lump is only used for one rotation
	if (sprtemp[frame].rotate == false)
	{
		// -JL- A sprite with no rotations is replaced with rotations.
/*		I_Error ("R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump"
		, spritename, 'A'+frame);*/
		// -JL- Copy and paste from Vavoom.
		for (r = 0; r < 8; r++)
		{
			sprtemp[frame].lump[r] = -1;
			sprtemp[frame].flip[r] = false;
		}
	}

	sprtemp[frame].rotate = true;

	rotation--;		// make 0 based
	// -JL- Allow sprite replacements.
/*	if (sprtemp[frame].lump[rotation] != -1)
		I_Error ("R_InitSprites: Sprite %s : %c : %c has two lumps mapped to it"
		,spritename, 'A'+frame, '1'+rotation);*/

	sprtemp[frame].lump[rotation] = lump - firstspritelump;
	sprtemp[frame].flip[rotation] = (byte)flipped;
}

/*
=================
=
= R_InitSpriteDefs
=
= Pass a null terminated list of sprite names (4 chars exactly) to be used
= Builds the sprite rotation matrixes to account for horizontally flipped
= sprites.  Will report an error if the lumps are inconsistant
=
Only called at startup
=
= Sprite lump names are 4 characters for the actor, a letter for the frame,
= and a number for the rotation, A sprite that is flippable will have an
= additional letter/number appended.  The rotation character can be 0 to
= signify no rotations
=================
*/

void R_InitSpriteDefs (char **namelist)
{
	char		**check;
	int		i, l, intname, frame, rotation;
	int		start, end;

// count the number of sprite names
	check = namelist;
	while (*check != NULL)
		check++;
	numsprites = check-namelist;

	if (!numsprites)
		return;

	sprites = (spritedef_t *)Z_Malloc(numsprites * sizeof(*sprites), PU_SPRITE, NULL);

	start = firstspritelump-1;
	end = lastspritelump+1;

// scan all the lump names for each of the names, noting the highest
// frame letter
// Just compare 4 characters as ints
	for (i=0 ; i<numsprites ; i++)
	{
		spritename = namelist[i];
		memset (sprtemp,-1, sizeof(sprtemp));

		maxframe = -1;
		intname = *(int *)namelist[i];

		//
		// scan the lumps, filling in the frames for whatever is found
		//
		for (l=start+1 ; l<end ; l++)
			if (*(int *)lumpinfo[l].name == intname)
			{
				frame = lumpinfo[l].name[4] - 'A';
				rotation = lumpinfo[l].name[5] - '0';
				R_InstallSpriteLump (l, frame, rotation, false);
				if (lumpinfo[l].name[6])
				{
					frame = lumpinfo[l].name[6] - 'A';
					rotation = lumpinfo[l].name[7] - '0';
					R_InstallSpriteLump (l, frame, rotation, true);
				}
			}

		//
		// check the frames that were found for completeness
		//
		if (maxframe == -1)
		{
			//continue;
			sprites[i].numframes = 0;
			/*if (shareware)
				continue;
			I_Error ("R_InitSprites: No lumps found for sprite %s", namelist[i]);*/
		}

		maxframe++;
		for (frame = 0 ; frame < maxframe ; frame++)
		{
			switch ((int)sprtemp[frame].rotate)
			{
			case -1:	// no rotations were found for that frame at all
				I_Error ("R_InitSprites: No patches found for %s frame %c"
				, namelist[i], frame+'A');
			case 0:	// only the first rotation is needed
				break;

			case 1:	// must have all 8 frames
				for (rotation=0 ; rotation<8 ; rotation++)
					if (sprtemp[frame].lump[rotation] == -1)
						I_Error ("R_InitSprites: Sprite %s frame %c is missing rotations"
						, namelist[i], frame+'A');
			}
		}

		//
		// allocate space for the frames present and copy sprtemp to it
		//
		sprites[i].numframes = maxframe;
		sprites[i].spriteframes =
			(spriteframe_t *)Z_Malloc (maxframe * sizeof(spriteframe_t), PU_SPRITE, NULL);
		memcpy (sprites[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
		// The possible model frames are initialized elsewhere.
//		sprites[i].modelframes = NULL;
		sprites[i].modeldef=-1;
	}

}


void R_GetSpriteInfo(int sprite, int frame, spriteinfo_t *sprinfo)
{
	spritedef_t	*sprdef;
	spriteframe_t *sprframe;

#ifdef RANGECHECK
	if( (unsigned) sprite >= (unsigned) numsprites)
		I_Error( "R_GetSpriteInfo: invalid sprite number %i.\n", sprite);
#endif

	sprdef = &sprites[sprite];

#ifdef RANGECHECK
	if((frame & FF_FRAMEMASK) >= sprdef->numframes)
		I_Error( "R_ProjectSprite: invalid sprite frame %i : %i.\n", sprite, frame);
#endif

	sprframe = &sprdef->spriteframes[ frame & FF_FRAMEMASK ];

	sprinfo->lump = sprframe->lump[0];
	sprinfo->flip = sprframe->flip[0];
	sprinfo->offset = spriteoffset[sprframe->lump[0]];
	sprinfo->topOffset = spritetopoffset[sprframe->lump[0]];
}

/*
===============================================================================

							GAME FUNCTIONS

===============================================================================
*/

vissprite_t	vissprites[MAXVISSPRITES], *vissprite_p;
int			newvissprite;


/*
===================
=
= R_InitSprites
=
= Called at program start
===================
*/

void R_InitSprites (void)//char **namelist)
{
	// Free all previous sprite memory.
	Z_FreeTags(PU_SPRITE, PU_SPRITE);

	if(spritenamelist == NULL) 
		I_Error("R_InitSprites: No sprite name list.\n");

	R_InitSpriteDefs (spritenamelist);
	R_InitModels();
}	


/*
===================
=
= R_ClearSprites
=
= Called at frame start
===================
*/

void R_ClearSprites (void)
{
	vissprite_p = vissprites;
}


/*
===================
=
= R_NewVisSprite
=
===================
*/

vissprite_t		overflowsprite;

vissprite_t *R_NewVisSprite (void)
{
	if (vissprite_p == &vissprites[MAXVISSPRITES])
		return &overflowsprite;
	vissprite_p++;
	return vissprite_p-1;
}


/*
===================
=
= R_ProjectSprite
=
= Generates a vissprite for a thing if it might be visible
=
===================
*/

void R_ProjectSprite (mobj_t *thing)
{
	fixed_t		tr_x,tr_y;
	fixed_t		gxt,gyt;
	fixed_t		tx,tz;
	fixed_t		xscale;
	int			x1, x2;
	spritedef_t	*sprdef;
	spriteframe_t	*sprframe;
	int			lump;
	unsigned	rot;
	boolean		flip;
	vissprite_t	*vis;
	angle_t		ang;
	fixed_t		iscale;
	float		v1[2], v2[2];
	float		sinrv, cosrv, thangle;	// rv = real value
	boolean		align;
	modelframe_t *mf = NULL;
	
	if(thing->ddflags & DDMF_DONTDRAW)
	{ // Never make a vissprite when MF2_DONTDRAW is flagged.
		return;
	}

//
// transform the origin point
//
	tr_x = thing->x - viewx;
	tr_y = thing->y - viewy;

	gxt = FixedMul(tr_x,viewcos);
	gyt = -FixedMul(tr_y,viewsin);
	tz = gxt-gyt;

	if(tz < 0) tz = -tz;	// Make it positive. The clipper will handle backside.
	if (tz < FRACUNIT)//MINZ)
	//	return;		// thing is behind view plane
		tz = FRACUNIT;		
	
	xscale = FixedDiv(SCREENWIDTH<<FRACBITS, tz);

	gxt = -FixedMul(tr_x,viewsin);
	gyt = FixedMul(tr_y,viewcos);
	tx = -(gyt+gxt);
	
	/*if (abs(tx)>(tz<<2))
		return;		// too far off the side*/

//
// decide which patch to use for sprite reletive to player
//
	

#ifdef RANGECHECK
	if ((unsigned)thing->sprite >= (unsigned)numsprites)
		I_Error ("R_ProjectSprite: invalid sprite number %i ",thing->sprite);
#endif
	sprdef = &sprites[thing->sprite];
#ifdef RANGECHECK
	if ( (thing->frame&FF_FRAMEMASK) >= sprdef->numframes )
		I_Error ("R_ProjectSprite: invalid sprite frame %i : %i "
		,thing->sprite, thing->frame);
#endif
	sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK];

	// Check for a 3D model.
	if(useModels)
	{
		mf = R_CheckModelFor(thing->sprite, thing->frame & FF_FRAMEMASK);
	}

	if(sprframe->rotate && !mf)
	{	// choose a different rotation based on player view
		ang = R_PointToAngle (thing->x, thing->y);
		rot = (ang-thing->angle+(unsigned)(ANG45/2)*9)>>29;
		lump = sprframe->lump[rot];
		flip = (boolean)sprframe->flip[rot];
	}
	else
	{	// use single rotation for all views
		lump = sprframe->lump[0];
		flip = (boolean)sprframe->flip[0];
	}

	// Align to the view plane?
	if(thing->ddflags & DDMF_VIEWALIGN)
		align = true;
	else
		align = false;

	if(alwaysAlign == 1) align = true;
//
// calculate edges of the shape
//
	v1[VX] = FIX2FLT(thing->x);
	v1[VY] = FIX2FLT(thing->y);
	
	if(!mf)
	{
		if(align || alwaysAlign == 3)
		{
			// The sprite should be fully aligned to view plane.
			sinrv = -FIX2FLT(viewcos);
			cosrv = FIX2FLT(viewsin);
		}
		else
		{
			thangle = BANG2RAD(bamsAtan2(FIX2FLT(tr_y)*10, FIX2FLT(tr_x)*10)) - PI/2;
			sinrv = sin(thangle);
			cosrv = cos(thangle);
		}	
		
		//if(alwaysAlign == 2) align = true;
		
		v1[VX] -= cosrv*(spriteoffset[lump]>>FRACBITS);
		v1[VY] -= sinrv*(spriteoffset[lump]>>FRACBITS);
		v2[VX] = v1[VX] + cosrv*(spritewidth[lump]>>FRACBITS);
		v2[VY] = v1[VY] + sinrv*(spritewidth[lump]>>FRACBITS);
		
		if(!align && alwaysAlign != 2 && alwaysAlign != 3)
			// Check for visibility.
			if(!C_CheckViewRelSeg(v1[VX], v1[VY], v2[VX], v2[VY]))
				//if(!C_IsAngleVisible(RAD2BANG(thangle+PI/2)))
				return;	// Isn't visible.
	}
	x1 = x2 = 0;

//
// store information in a vissprite
//
	vis = R_NewVisSprite ();
	vis->mf = mf;
	vis->mobjflags = thing->ddflags;
	vis->psprite = false;
	vis->scale = xscale<<detailshift;
	vis->gx = thing->x;
	vis->gy = thing->y;
	vis->gz = thing->z;
	vis->gzt = thing->z + spritetopoffset[lump];

	vis->viewAligned = align;

	vis->secfloor = FIX2FLT(thing->subsector->sector->floorheight);
	vis->secceil = FIX2FLT(thing->subsector->sector->ceilingheight);

	if(thing->ddflags & DDMF_TRANSLATION)
	{
		vis->pclass = (thing->ddflags>>DDMF_CLASSTRSHIFT)&0x3;
	}
	// foot clipping
	vis->floorclip = thing->floorclip;
	vis->texturemid = vis->gzt-viewz-vis->floorclip;

	// The start and end vertices.
	vis->v1[VX] = v1[VX];
	vis->v1[VY] = v1[VY];
	if(!mf)
	{
		vis->v2[VX] = v2[VX];
		vis->v2[VY] = v2[VY];
	}
	else
	{
		// Determine the rotation angles (in degrees).
		vis->yaw = thing->angle / (float) ANGLE_MAX * -360;
		// FIXME: Pitch needs an 'align with movement' flag.
		vis->pitch = 0;
	}

	vis->x1 = x1 < 0 ? 0 : x1;
	vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;
	iscale = FixedDiv (FRACUNIT, xscale);
	if (flip)
	{
		vis->startfrac = spritewidth[lump]-1;
		vis->xiscale = -iscale;
	}
	else
	{
		vis->startfrac = 0;
		vis->xiscale = iscale;
	}
	if (vis->x1 > x1)
		vis->startfrac += vis->xiscale*(vis->x1-x1);
	vis->patch = lump;


//
// get light level
//
	if(LevelFullBright || thing->frame & FF_FULLBRIGHT)
	{
		vis->lightlevel = -1;
	}
	else
	{									// diminished light
		vis->lightlevel = thing->subsector->sector->lightlevel;
	}
}




/*
========================
=
= R_AddSprites
=
========================
*/

void R_AddSprites (sector_t *sec)
{
	mobj_t		*thing;
	//int			lightnum;

	if (sec->validcount == validcount)
		return;		// already added

	sec->validcount = validcount;

	for (thing = sec->thinglist ; thing ; thing = thing->snext)
		R_ProjectSprite (thing);
}

/*
========================
=
= R_SortVisSprites
=
========================
*/

vissprite_t	vsprsortedhead;

void R_SortVisSprites (void)
{
	int			i, count;
	vissprite_t	*ds, *best;
	vissprite_t	unsorted;
	fixed_t		bestscale;

	count = vissprite_p - vissprites;

	unsorted.next = unsorted.prev = &unsorted;
	if (!count)
		return;

	for (ds=vissprites ; ds<vissprite_p ; ds++)
	{
		ds->next = ds+1;
		ds->prev = ds-1;
	}
	vissprites[0].prev = &unsorted;
	unsorted.next = &vissprites[0];
	(vissprite_p-1)->next = &unsorted;
	unsorted.prev = vissprite_p-1;

//
// pull the vissprites out by scale
//
	best = 0;		// shut up the compiler warning
	vsprsortedhead.next = vsprsortedhead.prev = &vsprsortedhead;
	for (i=0 ; i<count ; i++)
	{
		bestscale = DDMAXINT;
		for (ds=unsorted.next ; ds!= &unsorted ; ds=ds->next)
		{
			if (ds->scale < bestscale)
			{
				bestscale = ds->scale;
				best = ds;
			}
		}
		best->next->prev = best->prev;
		best->prev->next = best->next;
		best->next = &vsprsortedhead;
		best->prev = vsprsortedhead.prev;
		vsprsortedhead.prev->next = best;
		vsprsortedhead.prev = best;
	}
}

/*
========================
=
= R_DrawMasked
=
========================
*/

void R_DrawMasked (void)
{
	extern boolean willRenderSprites;
	vissprite_t		*spr;
	int		i;

	if(willRenderSprites)
	{
		R_SortVisSprites ();

		if (vissprite_p > vissprites)
		{
			// draw all vissprites back to front
			if(useModels)
				for (spr = vsprsortedhead.next ; spr != &vsprsortedhead
					; spr=spr->next)
				{
					if(spr->mf)	
					{
						// Render all the models associated with 
						// the sprite/frame combination.
						for(i=0; i<MAX_FRAME_MODELS; i++)
							if(spr->mf->sub[i].model)
								R_RenderModel(spr, i);
					}
				}

			// Sprites look better with Z buffer writes turned off.
			if(!haloMode) gl.Disable(DGL_DEPTH_WRITE);
			for (spr = vsprsortedhead.next ; spr != &vsprsortedhead
				; spr=spr->next)
			{
				// There might be a model for this sprite, let's see.
				if(!spr->mf)
				{
					// Render an old fashioned sprite.
					// Ah, the nostalgia...
					R_RenderSprite(spr);
				}
			}
			if(!haloMode) gl.Enable(DGL_DEPTH_WRITE);
		}
	}
}


