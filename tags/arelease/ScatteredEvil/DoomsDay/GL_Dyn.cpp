
//**************************************************************************
//**
//** GL_DYN.C
//**
//** Dynamic lights.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dd_def.h"
#include "gl_def.h"
#include "gl_dyn.h"
#include "m_bams.h"
#include "gl_rl.h"
#include "i_win32.h"
#include "settings.h"

// MACROS ------------------------------------------------------------------

enum { CLIP_TOP, CLIP_BOTTOM, CLIP_LEFT, CLIP_RIGHT };

#define ZMOD_FLAT		0 //.05f	// How much difference?
#define ZMOD_WALL		.15f		
#define DISTFACTOR		1		// Really...

#define X_TO_DLBX(cx)			( ((cx) - dlBlockOrig.x) >> (FRACBITS+7) )
#define Y_TO_DLBY(cy)			( ((cy) - dlBlockOrig.y) >> (FRACBITS+7) )
#define DLB_ROOT_DLBXY(bx, by)	(dlBlockLinks + bx + by*dlBlockWidth)

// Wall Surface Present flags (for DL_ProcessWallSeg).
#define WSP_MIDDLE		0x1
#define WSP_TOP			0x2
#define WSP_BOTTOM		0x4

#define LUM_FACTOR(dist)	(1.5f - 1.5f*(dist)/lum->radius)

// TYPES -------------------------------------------------------------------

typedef struct
{
	boolean		lightFloor, lightCeiling;
	rendpoly_t	*poly;
	subsector_t	*subsector;
	float		fceil, ffloor;
} flatitervars_t;

typedef struct
{
	rendpoly_t	*poly;
	float		*v1, *v2;
} wallitervars_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

fvertex_t *edgeClipper(int *numpoints, fvertex_t *points, int numclippers, fdivline_t *clippers);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int			useDynLights;
extern subsector_t	*currentssec;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			useDynLights = true, dlBlend = 1;
float		dlFactor = 0.4f;

lumobj_t	*luminousList = 0;
int			numLuminous = 0, maxLuminous = 0;
int			dlMaxRad = 128; // Dynamic lights maximum radius.
float		dlRadFactor = 1;
int			maxDynLights = 0;
int			clipLights = 1;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

lumobj_t	**dlBlockLinks = 0;
vertex_t	dlBlockOrig;
int			dlBlockWidth, dlBlockHeight;	// In 128 blocks.

// CODE --------------------------------------------------------------------

// *VERY* similar to SegFacingDir(). Well, this is actually the same
// function, only a more general version.
/*__inline static int SegFacingPoint(float v1[2], float v2[2], float p[2])
{
	return((v1[VY]-v2[VY])*(v1[VX]-p[VX])+(v2[VX]-v1[VX])*(v1[VY]-p[VY]) > 0);
}*/

/*static void projectVector(float a[2], float b[2], float *a_on_b)
{
	float	factor = (a[0]*b[0] + a[1]*b[1]) / (b[0]*b[0] + b[1]*b[1]);
	a_on_b[0] = factor * b[0];
	a_on_b[1] = factor * b[1];
}*/

void thingDLRadius(lumobj_t *lum)
{
	int thingRad = lum->thing->radius >> FRACBITS, 
		thingHeight = spriteheight[lum->patch];
	int	rad = thingRad>thingHeight? thingRad : thingHeight;
	int bigRad;
	
	rad *= dlRadFactor;

	// No point to have a too small dynamic light. It would just be
	// a waste of time.
	bigRad = rad*5;
	rad *= 2;
	if(rad < 48) rad = 48;
	if(rad > dlMaxRad) rad = dlMaxRad;
	if(rad > bigRad) rad = bigRad;
	lum->radius = rad;

	rad = (7*thingHeight + thingRad)/8; // < thingRad? thingHeight : thingRad;
	//rad *= rad;
	//ST_Message( "%i\n", rad);
	lum->flaresize = rad;
}

void thingDLColor(lumobj_t *lum, DGLubyte *outRGB, float light)
{
	int		i;

	if(light < 0) light = 0;
	if(light > 1) light = 1;
	light *= dlFactor;
	// Multiply the light color.
	for(i=0; i<3; i++)
	{
		outRGB[i] = (DGLubyte) (light * lum->rgb[i]);
		//outRGB[i] += (255 - outRGB[i])/4; // more brightness, less color...
	}
}

void DL_InitBlockMap()
{
	byte		*ptr;
	vertex_t	*vtx;
	vertex_t	min, max;
	int			i;

	memcpy(&min, vertexes, sizeof(min));
	memcpy(&max, vertexes, sizeof(max)); 
	for(i=1, ptr=vertexes+VTXSIZE; i<numvertexes; i++, ptr += VTXSIZE)
	{
		vtx = (vertex_t*) ptr;
		if(vtx->x < min.x) min.x = vtx->x;
		if(vtx->x > max.x) max.x = vtx->x;
		if(vtx->y < min.y) min.y = vtx->y;
		if(vtx->y > max.y) max.y = vtx->y;
	}
	memcpy(&dlBlockOrig, &min, sizeof(min));
	// -JL- Do it like this, because otherwise dimension larger than 32k ends 
	// up in negative number.
	dlBlockWidth = (((max.x >> FRACBITS) - (min.x >> FRACBITS)) >> 7) + 1;
	dlBlockHeight = (((max.y >> FRACBITS) - (min.y >> FRACBITS)) >> 7) + 1;
	// Blocklinks is a table of lumobj_t pointers.
	dlBlockLinks = (lumobj_t **)realloc(dlBlockLinks, sizeof(lumobj_t*) * dlBlockWidth * dlBlockHeight);
	// -JL- Make sure that realloc returned something
	if (!dlBlockLinks)
	{
		I_Error("DL_InitBlockMap: Failed to realloc %d bytes", 
			sizeof(lumobj_t*) * dlBlockWidth * dlBlockHeight);
	}
}

// Set dlq.top and .bottom before calling this.
static void DL_FinalizeSurface(rendpoly_t *dlq, lumobj_t *lum)
{
	dlq->texoffy = FIX2FLT(lum->thing->z) + lum->center + lum->radius - dlq->top;
	if(dlq->texoffy < -dlq->top+dlq->bottom) return;
	if(dlq->texoffy > dlq->texh) return;
	if(dlq->top+dlq->texoffy-dlq->texh >= dlq->bottom) // Fits completely?
		dlq->bottom = dlq->top+dlq->texoffy-dlq->texh;
	if(dlq->texoffy < 0)
	{
		dlq->top += dlq->texoffy;
		dlq->texoffy = 0;
	}
	// Give the new quad to the rendering engine.
	RL_AddPoly(dlq);
}

// The front sector must be given because of polyobjs.
void DL_ProcessWallSeg(lumobj_t *lum, seg_t *seg, sector_t *frontsec)
{
	int			c, present = 0;
	sector_t	*backsec = seg->backsector;
	side_t		*sdef = seg->sidedef;
	float		dist, pntLight[2], uvecWall[2], fceil, ffloor, bceil, bfloor;
	rendpoly_t	dlq;

	// Let's begin with an analysis of the present surfaces.	
	if(sdef->midtexture)
	{
		present |= WSP_MIDDLE;
		if(backsec)
		{
			// Check the middle texture's mask status.
			GL_PrepareTexture(sdef->midtexture); 
			if(texmask)
			{
				// We can't light masked textures.
				// FIXME: Use vertex lighting.
				present &= ~WSP_MIDDLE;
			}
		}
	}
	// Is there a top wall segment?
	if(backsec)
	{
		if(frontsec->ceilingheight > backsec->ceilingheight)
		{
			present |= WSP_TOP;
		}
		// Is there a lower wall segment? 
		if(frontsec->floorheight < backsec->floorheight)
		{
			present |= WSP_BOTTOM;
		}
		// The top texture can't be present when front and back sectors
		// have both the sky ceiling.
		if(frontsec->ceilingpic == skyflatnum 
			&& backsec->ceilingpic == skyflatnum)
		{
			present &= ~WSP_TOP;
		}
	}
	// There are no surfaces to light!
	if(!present) return;

	dlq.type = rp_quad;
	dlq.numvertices = 2;
	dlq.vertices[0].pos[VX] = FIX2FLT(seg->v1->x);
	dlq.vertices[0].pos[VY] = FIX2FLT(seg->v1->y);
	dlq.vertices[1].pos[VX] = FIX2FLT(seg->v2->x);
	dlq.vertices[1].pos[VY] = FIX2FLT(seg->v2->y);

	pntLight[VX] = FIX2FLT(lum->thing->x);
	pntLight[VY] = FIX2FLT(lum->thing->y);
	
	dist = ((dlq.vertices[0].pos[VY] - pntLight[VY]) * 
		(dlq.vertices[1].pos[VX] - dlq.vertices[0].pos[VX]) - 
		(dlq.vertices[0].pos[VX] - pntLight[VX]) * 
		(dlq.vertices[1].pos[VY] - dlq.vertices[0].pos[VY])) / seg->length;

	// Is it close enough and on the right side?
	if(dist < 0 || dist > lum->radius) return; // Nope.

	// Initialize the poly used for all of the lights.
	dlq.flags = RPF_LIGHT; // This is a light polygon.
	dlq.texw = dlq.texh = lum->radius*2;
	dlq.length = seg->length;
	
	// Do a scalar projection for the offset.
	dlq.texoffx = ((dlq.vertices[0].pos[VY] - pntLight[VY]) * 
		(dlq.vertices[0].pos[VY] - dlq.vertices[1].pos[VY]) - 
		(dlq.vertices[0].pos[VX] - pntLight[VX]) * 
		(dlq.vertices[1].pos[VX] - dlq.vertices[0].pos[VX])) / seg->length - lum->radius;

	// There is no need to draw the *whole* wall always. Adjust the start
	// and end points so that only a relevant portion is included.
	if(dlq.texoffx > dlq.length) return; // Doesn't fit on the wall.
	if(dlq.texoffx < -dlq.texw) return; // Ditto, but in the other direction.

	// The wall vector.
	for(c=0; c<2; c++) 
		uvecWall[c] = (dlq.vertices[1].pos[c] - dlq.vertices[0].pos[c]) / dlq.length;

	if(dlq.texoffx > 0)
	{
		for(c=0; c<2; c++) dlq.vertices[0].pos[c] += dlq.texoffx * uvecWall[c];
		if(dlq.texoffx+dlq.texw <= dlq.length) // Fits completely?
		{
			for(c=0; c<2; c++) 
				dlq.vertices[1].pos[c] = dlq.vertices[0].pos[c] + dlq.texw*uvecWall[c];
			dlq.length = dlq.texw;
		}
		else 
		{
			// Doesn't fit, must truncate.
			dlq.length -= dlq.texoffx; 
		}
		dlq.texoffx = 0;
	}
	else // It goes off to the left.
	{
		if(dlq.texoffx+dlq.texw <= dlq.length) // Fits completely?
		{
			for(c=0; c<2; c++) 
			{
				dlq.vertices[1].pos[c] = dlq.vertices[0].pos[c] + 
					(dlq.texw + dlq.texoffx) * uvecWall[c];
			}
			dlq.length = dlq.texw + dlq.texoffx;
		}
	}

	// Calculate the color of the light.
	thingDLColor(lum, dlq.vertices[0].color.rgb, LUM_FACTOR(dist));

	fceil = FIX2FLT(frontsec->ceilingheight);// + frontsec->skyfix;
	ffloor = FIX2FLT(frontsec->floorheight);
	if(backsec)
	{
		bceil = FIX2FLT(backsec->ceilingheight);// + backsec->skyfix;
		bfloor = FIX2FLT(backsec->floorheight);
	}

	// Render the present surfaces.
	if(present & WSP_MIDDLE) 
	{
		if(backsec)
		{
			dlq.top = fceil < bceil? fceil : bceil;
			dlq.bottom = ffloor > bfloor? ffloor : bfloor;
		}
		else
		{
			dlq.top = fceil;
			dlq.bottom = ffloor;
		}
		DL_FinalizeSurface(&dlq, lum);
		// Mark the seg for dlight rendering.
		seg->flags |= DDSEGF_DLIGHT;
	}
	if(present & WSP_TOP)
	{
		dlq.top = fceil;
		dlq.bottom = bceil;
		DL_FinalizeSurface(&dlq, lum);
		// Mark the seg for dlight rendering.
		seg->flags |= DDSEGF_DLIGHT;
	}
	if(present & WSP_BOTTOM)
	{
		dlq.top = bfloor;
		dlq.bottom = ffloor;
		DL_FinalizeSurface(&dlq, lum);
		// Mark the seg for dlight rendering.
		seg->flags |= DDSEGF_DLIGHT;
	}
}

void DL_Clear()
{
	if(luminousList) Z_Free(luminousList);
	luminousList = 0;
	maxLuminous = numLuminous = 0;

	free(dlBlockLinks);
	dlBlockLinks = 0;
	dlBlockOrig.x = dlBlockOrig.y = 0;
	dlBlockWidth = dlBlockHeight = 0;
}

void DL_AddLuminous(mobj_t *thing)
{
	if(thing->frame & FF_FULLBRIGHT && !(thing->ddflags&DDMF_DONTDRAW))
	{
		spritedef_t *sprdef;
		spriteframe_t *sprframe;
		int lump, i;
		float mul;
		lumobj_t *lum;

		// Only allocate memory when it's needed.
		if(++numLuminous > maxLuminous)
		{
			lumobj_t *newlist = (lumobj_t *)Z_Malloc(sizeof(lumobj_t) * (maxLuminous+=16), PU_STATIC, 0);
			// Copy the old data over to the new list.
			if(luminousList)
			{
				memcpy(newlist, luminousList, sizeof(lumobj_t) * (numLuminous-1));
				Z_Free(luminousList);
			}
			luminousList = newlist;
		}
		lum = luminousList + numLuminous-1;
		lum->thing = thing;
		// We need to know how tall the thing currently is.
		sprdef = &sprites[thing->sprite];
		sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK ];
		if(sprframe->rotate)
			lump = sprframe->lump[(R_PointToAngle(thing->x, thing->y) - thing->angle + (unsigned)(ANG45/2)*9) >> 29];
		else
			lump = sprframe->lump[0];
		
		// This'll ensure we have up-to-date information about the texture.
		// Notice that the texture is not bound now.
		GL_PrepareSprite(lump);
		lum->patch = lump;
		//lum->top = FIX2FLT(spritetopoffset[lump] - lum->thing->floorclip);
		//lum->height = spriteheights[lump];
		lum->center = FIX2FLT(spritetopoffset[lump] - lum->thing->floorclip) - spriteheight[lump]/2.0f;

		// Sets the dynlight and flare radii.
		thingDLRadius(lum);

		// Does the mobj use a light offset?
		if(thing->ddflags & DDMF_LIGHTOFFSET)
		{
			i = (thing->ddflags & DDMF_LIGHTOFFSET) >> DDMF_LIGHTOFFSETSHIFT;
			if(i & 0x8) i = 8-i; // Negative?
			lum->center += spriteheight[lump] * i/14.0f;
			// Reduce the size of the flare if it's moved off-center.
			mul = (12 - (i>0? i : -i)) / 12.0f;
			lum->radius *= mul;			
			lum->flaresize *= mul;
		}
		// Does the mobj use a light scale?
		if(thing->ddflags & DDMF_LIGHTSCALE)
		{
			// Also reduce the size of the light according to 
			// the scale flags. *Won't affect the flare.*
			mul = 1.0f - ((thing->ddflags & DDMF_LIGHTSCALE) >> DDMF_LIGHTSCALESHIFT)/4.0f;
			lum->radius *= mul;
		}
		lum->flags = 0;

		// The color, too.
		GL_GetSpriteColor(lum->patch, lum->rgb);

		// Approximate the distance, if necessary.
		if(maxDynLights || flareFadeMax) 
			lum->distance = M_AproxDistance(thing->x-viewx, thing->y-viewy);
	}
}

void DL_LinkLuminous()
{
	lumobj_t	**root, *lum = luminousList;
	int			i, bx, by;

	memset(dlBlockLinks, 0, sizeof(lumobj_t*) * dlBlockWidth * dlBlockHeight);
	for(i=0; i<numLuminous; i++, lum++)
	{
		// Link this lumobj to the dlBlockLinks, if it can be linked.
		lum->next = NULL;
		bx = X_TO_DLBX(lum->thing->x);
		by = Y_TO_DLBY(lum->thing->y);
		if(bx >= 0 && by >= 0 && bx < dlBlockWidth && by < dlBlockHeight)
		{
			root = DLB_ROOT_DLBXY(bx, by);
			lum->next = *root;
			*root = lum;
		}
	}
}

void intersectionVertex(fvertex_t *out, fvertex_t *a, fvertex_t *b,
						boolean horizontal, float boundary)
{
	float dx = b->x-a->x, dy = b->y-a->y;

	if(horizontal)
	{
		out->x = a->x + (boundary - a->y) / dy * dx;
		out->y = boundary;
	}
	else
	{
		out->x = boundary;
		out->y = a->y + (boundary - a->x) / dx * dy;
	}
}

// Clip the subsector to the light. Returns the number of vertices in 
// the out buffer.
int DL_SubsectorClipper(fvertex_t *out, subsector_t *sub, lumobj_t *lum)
{
	float		x = FIX2FLT(lum->thing->x), y = FIX2FLT(lum->thing->y);
	int			i, clip, aidx, bidx, num;
	boolean		clip_needed[4], sides[MAX_POLY_SIDES];
	fvertex_t	inverts[MAX_POLY_SIDES];
	fvertex_t	verts[MAX_POLY_SIDES]; // This many vertices, max!
	fvertex_t	*vptr = verts, *a, *b;
	fvertex_t	light[4];

	light[0].x = light[3].x = x - lum->radius;
	light[0].y = light[1].y = y - lum->radius;
	light[1].x = light[2].x = x + lum->radius;
	light[2].y = light[3].y = y + lum->radius;

	if(sub->bbox[0].x >= light[0].x && sub->bbox[0].y >= light[0].y 
		&& sub->bbox[1].x <= light[2].x && sub->bbox[1].y <= light[2].y)
	{
		// No clipping needed, the whole subsector is inside the light.
		memcpy(out, sub->origedgeverts, sizeof(fvertex_t) * sub->numedgeverts);
		return sub->numedgeverts;
	}

	// See which clips are needed.
	clip_needed[CLIP_LEFT] = light[0].x > sub->bbox[0].x;
	clip_needed[CLIP_TOP] = light[0].y > sub->bbox[0].y;
	clip_needed[CLIP_RIGHT] = light[2].x < sub->bbox[1].x;
	clip_needed[CLIP_BOTTOM] = light[2].y < sub->bbox[1].y;

	// Prepare the vertex array.
	memcpy(inverts, sub->origedgeverts, sizeof(fvertex_t) * sub->numedgeverts);
	num = sub->numedgeverts;

	// Horizontal clips first.
	for(clip=CLIP_TOP; clip<=CLIP_RIGHT; clip++)
	{
		if(!clip_needed[clip]) continue;

		// First determine on which side each vertex is.
		for(i=0; i<num; i++)
		{
			switch(clip)
			{
			case CLIP_TOP:
				sides[i] = inverts[i].y >= light[0].y;
				break;
			case CLIP_BOTTOM:
				sides[i] = inverts[i].y <= light[2].y;
				break;
			case CLIP_LEFT:
				sides[i] = inverts[i].x >= light[0].x;
				break;
			case CLIP_RIGHT:
				sides[i] = inverts[i].x <= light[2].x;
			}
		}
		for(i=0; i<num; i++)
		{
			a = inverts + (aidx = (i==0? num-1 : i-1));
			b = inverts + (bidx = i);
			if(sides[aidx] && sides[bidx])
			{
				// This edge is completely within the light, no need to clip it.
				memcpy(vptr++, a, sizeof(*a));
			}
			else if(sides[aidx])
			{	
				// Only the start point is inside.
				memcpy(vptr++, a, sizeof(*a));
				// Add an intersection vertex.
				intersectionVertex(vptr++, a, b, clip==CLIP_TOP || clip==CLIP_BOTTOM, 
					clip==CLIP_TOP? light[0].y
					: clip==CLIP_BOTTOM? light[2].y
					: clip==CLIP_LEFT? light[0].x
					: light[2].x);
			}
			else if(sides[bidx])
			{
				// First an intersection vertex.
				intersectionVertex(vptr++, a, b, clip==CLIP_TOP || clip==CLIP_BOTTOM, 
					clip==CLIP_TOP? light[0].y
					: clip==CLIP_BOTTOM? light[2].y
					: clip==CLIP_LEFT? light[0].x
					: light[2].x);
				// Then the end vertex, which is inside.
				memcpy(vptr++, b, sizeof(*b));
			}
		}
		// Prepare for another round.
		num = vptr - verts;
		vptr = verts;
		memcpy(inverts, verts, sizeof(fvertex_t) * num);
	}
	memcpy(out, inverts, sizeof(fvertex_t) * num);
	return num;
}

boolean DL_LightIteratorFunc(lumobj_t *lum, void *ptr) 
{
	int				i;
	byte			*seg;
	flatitervars_t	*fi = (flatitervars_t *)ptr;
	float			x = FIX2FLT(lum->thing->x), y = FIX2FLT(lum->thing->y);
	float			z = FIX2FLT(lum->thing->z), cdiff, fdiff;	
	boolean			applyCeiling, applyFloor;	// Is the light on the right side?
	int				num_vertices;
	fvertex_t		vertices[MAX_POLY_SIDES];
	int				srcRadius = spriteheight[lum->patch] >> 1;

	if(haloMode)
	{
		if(lum->thing->subsector == fi->subsector)
			lum->flags |= LUMF_RENDERED;
	}

	// Center the Z.
	z += lum->center;

	// Determine on which side the light is for both the floor and ceiling.
	if(fi->lightCeiling && z - srcRadius < fi->fceil)
		applyCeiling = true;
	else
		applyCeiling = false;

	if(fi->lightFloor && z + srcRadius > fi->ffloor)
		applyFloor = true;
	else
		applyFloor = false;

	if(clipLights && (applyCeiling || applyFloor))
	{
		num_vertices = DL_SubsectorClipper(vertices, fi->subsector, lum);

		// Hmm? The light doesn't reach this subsector, we can move on.
		if(!num_vertices) return true;

		/*lightvtx[0].x = lightvtx[3].x = x - lum->radius;
		lightvtx[0].y = lightvtx[1].y = y - lum->radius;
		lightvtx[1].x = lightvtx[2].x = x + lum->radius;
		lightvtx[2].y = lightvtx[3].y = y + lum->radius;
		// See if all the vertices are within the same subsector.
		complete = true; 
		for(i=0; i<4; i++)
			if(R_PointInSubsector(FRACUNIT*lightvtx[i].x, FRACUNIT*lightvtx[i].y) != fi->subsector)
			{
				complete = false;
				break;
			}*/
	}

	fi->poly->tex = (int) lum;

	if(applyCeiling)
	{
		// Check that the height difference is tolerable.
		cdiff = fabs(fi->fceil - z);
		if(cdiff < lum->radius) 
		{
			fi->poly->top = fi->fceil - ZMOD_FLAT;
			thingDLColor(lum, fi->poly->vertices[0].color.rgb, LUM_FACTOR(cdiff));
			// We can add the light poly.
			if(clipLights)
				RL_PrepareFlat(fi->poly, num_vertices, vertices, 1, 0);
			else
				RL_PrepareFlat(fi->poly, fi->subsector->numedgeverts, 
					fi->subsector->origedgeverts, 1, 0);
			RL_AddPoly(fi->poly);
			// Mark the ceiling for dlighting.
//			fi->subsector->flags |= DDSUBF_DLIGHT_CEILING; //crap
		}
	}
	if(applyFloor)
	{
		fdiff = fabs(fi->ffloor - z);
		if(fdiff < lum->radius) 
		{
			fi->poly->top = fi->ffloor + ZMOD_FLAT;
			thingDLColor(lum, fi->poly->vertices[0].color.rgb, LUM_FACTOR(fdiff));
			// Add the light quads.
			if(clipLights)
				RL_PrepareFlat(fi->poly, num_vertices, vertices, 0, 0);
			else
				RL_PrepareFlat(fi->poly, fi->subsector->numedgeverts, 
					fi->subsector->origedgeverts, 0, 0);
			RL_AddPoly(fi->poly);
			// Mark the ceiling for dlighting.
//			fi->subsector->flags |= DDSUBF_DLIGHT_FLOOR; //crap
		}
	}

	// The wall segments.
	for(i=0, seg=segs+SEGIDX(fi->subsector->firstline); i<fi->subsector->numlines; i++, seg+=SEGSIZE)
		if(((seg_t*)seg)->linedef)	// "minisegs" have no linedefs.
			DL_ProcessWallSeg(lum, (seg_t*) seg, fi->subsector->sector);

	// Is there a polyobj on board? Light it, too.
	if(fi->subsector->poly)
		for(i=0; i<fi->subsector->poly->numsegs; i++)
			DL_ProcessWallSeg(lum, fi->subsector->poly->segs[i], fi->subsector->sector);
	
	return true;
}

void DL_ProcessSubsector(rendpoly_t *poly, subsector_t *ssec)
{
	sector_t		*sect = ssec->sector;
	flatitervars_t	fi;
	fixed_t			box[4];

	fi.poly = poly;
	fi.subsector = ssec;
	fi.fceil = FIX2FLT(sect->ceilingheight);// + sect->skyfix;
	fi.ffloor = FIX2FLT(sect->floorheight);

	// Check if lighting can be skipped.
	fi.lightFloor = (sect->floorpic != skyflatnum);
	fi.lightCeiling = (sect->ceilingpic != skyflatnum);

	// See if we can leave. This case is pretty rare, though (!).
	if(!fi.lightCeiling && !fi.lightFloor) return;

	poly->flags = RPF_LIGHT;
	
	// Prepare the bounding box.
	box[BLEFT] = (int) (ssec->bbox[0].x - dlMaxRad) << FRACBITS;
	box[BTOP] = (int) (ssec->bbox[0].y - dlMaxRad) << FRACBITS;
	box[BRIGHT] = (int) (ssec->bbox[1].x + dlMaxRad) << FRACBITS;
	box[BBOTTOM] = (int) (ssec->bbox[1].y + dlMaxRad) << FRACBITS;

	DL_BoxIterator(box, &fi, DL_LightIteratorFunc);
}

int lumobjSorter(const void *e1, const void *e2)
{
	lumobj_t *lum1 = (lumobj_t*) e1, *lum2 = (lumobj_t*) e2;

	if(lum1->distance > lum2->distance) return 1;
	if(lum1->distance < lum2->distance) return -1;
	return 0;
}

void DL_InitForNewFrame()
{
	sector_t	*seciter;//, *lastsec, *startsec;
	int			i, done = false;

	numLuminous = 0;	// Clear the luminous object list.
			
	// Add all the luminous objects to the list.
	// First add the lights in the player's sector.
	//i = (players[consoleplayer].mo->subsector->sector - sectors) / sizeof(sector_t);
	//lastsec = sectors + numsectors-1;
//	seciter = startsec = players[displayplayer].mo->subsector->sector;
	//while(!done)

	// Clearly there are smarter ways to do this, but...
	for(i=0; i<numsectors; i++)
	{
		mobj_t *iter;
		//seciter = sectors + i;
		seciter = SECTOR_PTR(i);
		for(iter=seciter->thinglist; iter; iter=iter->snext)
		{
			DL_AddLuminous(iter);
			/*if(maxDynLights && ++dlCounter >= maxDynLights) 
			{
				done = true;
				break;
			}*/
		}
		// Advance to the next sector.
		/*if(seciter == lastsec) seciter = sectors; else seciter++;
		// Are we done?
		if(seciter == startsec) break; */
	}
	if(maxDynLights)
	{
		// Sort the lumobjs based on their distance.
		qsort(luminousList, numLuminous, sizeof(lumobj_t), lumobjSorter);
		// Force the maximum.
		if(numLuminous > maxDynLights) numLuminous = maxDynLights;
	}
	// Link the luminous objects into the blockmap.
	DL_LinkLuminous();
}

// Calls func for all luminous objects within the specified range from (x,y).
boolean DL_RadiusIterator(fixed_t x, fixed_t y, fixed_t radius, 
						  boolean (*func)(lumobj_t*,fixed_t))
{
	int		s, t, bx1, by1, bx2, by2;
	fixed_t	dist;
	lumobj_t *iter;
	
	bx1 = X_TO_DLBX(x-radius);
	bx2 = X_TO_DLBX(x+radius);
	by1 = Y_TO_DLBY(y-radius);
	by2 = Y_TO_DLBY(y+radius);
	// Walk through the blocks.
	for(t=by1; t<=by2; t++)
		for(s=bx1; s<=bx2; s++)
		{
			// We can't go outside the blockmap.
			if(s < 0 || t < 0 || s >= dlBlockWidth || t >= dlBlockHeight) continue;
			for(iter = *DLB_ROOT_DLBXY(s, t); iter; iter = iter->next)
			{
				dist = M_AproxDistance(iter->thing->x - x, iter->thing->y - y);
				if(dist <= radius)
				{
					if(!func(iter, dist)) return false;						
				}
			}
		}
	return true;
}

// Box contains the coordinates of the top left and bottom right corners.
boolean DL_BoxIterator(fixed_t box[4], void *ptr, boolean (*func)(lumobj_t*,void*))
{
	register fixed_t x, y;
	int			s, t, bbc[4];	// box block coordinates
	lumobj_t	*iter;
	boolean		accurate;
	
	bbc[BLEFT] = X_TO_DLBX(box[BLEFT]);
	bbc[BRIGHT] = X_TO_DLBX(box[BRIGHT]);
	bbc[BTOP] = Y_TO_DLBY(box[BTOP]);
	bbc[BBOTTOM] = Y_TO_DLBY(box[BBOTTOM]);

	// Can we skip the whole test?
	if(bbc[BRIGHT] < 0 || bbc[BBOTTOM] < 0 || bbc[BLEFT] >= dlBlockWidth 
		|| bbc[BTOP] >= dlBlockHeight)
	{
		// No lights outside the map.
		return true;		
	}

	// Clip to box to fit inside the blockmap.
	if(bbc[BLEFT] < 0) bbc[BLEFT] = 0;
	if(bbc[BTOP] < 0) bbc[BTOP] = 0;
	if(bbc[BRIGHT] >= dlBlockWidth) bbc[BRIGHT] = dlBlockWidth-1;
	if(bbc[BBOTTOM] >= dlBlockHeight) bbc[BBOTTOM] = dlBlockHeight-1;
	
	// Now the box lies in valid blocks.

	// The most typical box spans 3x3 blocks (dlMaxRad is 128 => diameter 256).
	for(t = bbc[BTOP]; t <= bbc[BBOTTOM]; t++)
		for(s = bbc[BLEFT]; s <= bbc[BRIGHT]; s++)
		{
			// If the edge of the box is in this block, we must do an accurate check.
			accurate = (t==bbc[BTOP] || t==bbc[BBOTTOM] || s==bbc[BLEFT] || s==bbc[BRIGHT]);
			// Iterate the lights in this block.
			for(iter = *DLB_ROOT_DLBXY(s, t); iter; iter = iter->next)
			{
				if(accurate)
				{
					x = iter->thing->x;
					y = iter->thing->y;
					if(x >= box[BLEFT] && y >= box[BTOP] && x <= box[BRIGHT] && y <= box[BBOTTOM])
					{
						if(!func(iter, ptr)) return false;
					}
				}
				else if(!func(iter, ptr)) return false;
			}
		}
	return true;
}