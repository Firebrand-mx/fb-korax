
//**************************************************************************
//**
//** DD_SETUP.C
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <math.h>
#include "dd_def.h"
#include "gl_dyn.h"
#include "i_win32.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int numvertexes;
byte *vertexes;

int numsegs;
byte *segs;

int numsectors;
byte *sectors;

int numsubsectors;
byte *subsectors;

int numnodes;
byte *nodes;

int numlines;
byte *lines;

int	numsides;
byte *sides;


// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// AccurateDistance
//
//==========================================================================

float AccurateDistance(fixed_t dx, fixed_t dy)
{
	float fx = FIX2FLT(dx), fy = FIX2FLT(dy);
	return (float)sqrt(fx*fx + fy*fy);
}

//==========================================================================
//
// detSideFloat
//
// Determines on which side of dline the point is. Returns true if the
// point is on the line or on the right side.
//
//==========================================================================

#pragma optimize("g", off)

__inline int detSideFloat(fvertex_t *pnt, fdivline_t *dline)
{
/*
            (AY-CY)(BX-AX)-(AX-CX)(BY-AY)
        s = -----------------------------
                        L**2

    If s<0      C is left of AB (you can just check the numerator)
    If s>0      C is right of AB
    If s=0      C is on AB
*/
	// We'll return false if the point c is on the left side.

	//float s = (dline->y-pnt->y)*dline->dx-(dline->x-pnt->x)*dline->dy;

	return ((dline->y-pnt->y)*dline->dx-(dline->x-pnt->x)*dline->dy >= 0);// return 0;
	//return 1;
}

//==========================================================================
//
// findIntersectionVertex
//
//==========================================================================

// Lines start-end and fdiv must intersect.
float findIntersectionVertex(fvertex_t *start, fvertex_t *end, fdivline_t *fdiv, 
							 fvertex_t *inter)
{
	float ax = start->x, ay = start->y, bx = end->x, by = end->y;
	float cx = fdiv->x, cy = fdiv->y, dx = cx+fdiv->dx, dy = cy+fdiv->dy;

	/*
	        (YA-YC)(XD-XC)-(XA-XC)(YD-YC)
        r = -----------------------------  (eqn 1)
            (XB-XA)(YD-YC)-(YB-YA)(XD-XC)
	*/

	float r = ((ay-cy)*(dx-cx)-(ax-cx)*(dy-cy)) / ((bx-ax)*(dy-cy)-(by-ay)*(dx-cx));
	/*
	    XI=XA+r(XB-XA)
        YI=YA+r(YB-YA)
	*/
	inter->x = ax + r*(bx-ax);
	inter->y = ay + r*(by-ay);
	return r;
}

#pragma optimize("", on)

//==========================================================================
//
// edgeClipper
//
//==========================================================================

// Returns a pointer to the list of points. It must be used.
fvertex_t *edgeClipper(int *numpoints, fvertex_t *points, int numclippers, fdivline_t *clippers)
{
	unsigned char	sidelist[MAX_POLY_SIDES];
	int				i, k, num = *numpoints;

	// We'll clip the polygon with each of the divlines. The left side of
	// each divline is discarded.
	for(i=0; i<numclippers; i++)
	{
		fdivline_t *curclip = clippers+i;

		// First we'll determine the side of each vertex. Points are allowed
		// to be on the line.
		for(k=0; k<num; k++)
		{
			sidelist[k] = detSideFloat(points+k, curclip);
		}
		
		for(k=0; k<num; k++)
		{
			int startIdx = k, endIdx = k+1;

			// Check the end index.
			if(endIdx == num) endIdx = 0;	// Wrap-around.

			// Clipping will happen when the ends are on different sides.
			if(sidelist[startIdx] != sidelist[endIdx])
			{
				fvertex_t newvert;
				// Find the intersection point of intersecting lines.
				findIntersectionVertex(points+startIdx, points+endIdx,
					curclip, &newvert);

				// Add the new vertex. Also modify the sidelist.
				points = (fvertex_t*)realloc(points,(++num)*sizeof(fvertex_t));
				// -JL- Paranoia
				if (!points)
					I_Error("edgeClipper: realloc failed");
				if(num >= MAX_POLY_SIDES) I_Error("Too many points in carver.\n");

				// Make room for the new vertex.
				memmove(points+endIdx+1, points+endIdx, 
					(num - endIdx-1)*sizeof(fvertex_t));
				memcpy(points+endIdx, &newvert, sizeof(newvert));

				memmove(sidelist+endIdx+1, sidelist+endIdx, num-endIdx-1);
				sidelist[endIdx] = 1;
				
				// Skip over the new vertex.
				k++;
			}
		}
		
		// Now we must discard the points that are on the wrong side.
		for(k=0; k<num; k++)
			if(!sidelist[k])
			{
				memmove(points+k, points+k+1, (num - k-1)*sizeof(fvertex_t));
				memmove(sidelist+k, sidelist+k+1, num - k-1);
				num--;
				k--;
			}
	}
	// Screen out consecutive identical points.
	for(i=0; i<num; i++)
	{
		int previdx = i-1;
		if(previdx < 0) previdx = num - 1;
		if(points[i].x == points[previdx].x 
			&& points[i].y == points[previdx].y)
		{
			// This point (i) must be removed.
			memmove(points+i, points+i+1, sizeof(fvertex_t)*(num-i-1));
			num--;
			i--;
		}
	}
	*numpoints = num;
	return points;
}

//==========================================================================
//
// DD_ConvexCarver
//
//==========================================================================

void DD_ConvexCarver(subsector_t *ssec, int num, divline_t *list)
{
	int			numclippers = num+ssec->numlines;
	fdivline_t	*clippers = (fdivline_t*) Z_Malloc(numclippers*sizeof(fdivline_t), PU_STATIC, 0);
	int			i, numedgepoints;
	fvertex_t	*edgepoints;
	
	// Convert the divlines to float, in reverse order.
	for(i=0; i<numclippers; i++)
	{
		if(i<num)
		{
			clippers[i].x = FIX2FLT(list[num-i-1].x);
			clippers[i].y = FIX2FLT(list[num-i-1].y);
			clippers[i].dx = FIX2FLT(list[num-i-1].dx);
			clippers[i].dy = FIX2FLT(list[num-i-1].dy);
		}
		else
		{
			seg_t *seg = SEG_PTR(ssec->firstline+i-num);
			clippers[i].x = FIX2FLT(seg->v1->x);
			clippers[i].y = FIX2FLT(seg->v1->y);
			clippers[i].dx = FIX2FLT(seg->v2->x - seg->v1->x);
			clippers[i].dy = FIX2FLT(seg->v2->y - seg->v1->y);
		}
	}

	// Setup the 'worldwide' polygon.
	numedgepoints = 4;
	edgepoints = (fvertex_t*) malloc(numedgepoints*sizeof(fvertex_t));
	// -JL- Paranoia
	if (!edgepoints)
		I_Error("DD_ConvexCarver: malloc failed");
	
	edgepoints[0].x = -32768;
	edgepoints[0].y = 32768;
	
	edgepoints[1].x = 32768;
	edgepoints[1].y = 32768;
	
	edgepoints[2].x = 32768;
	edgepoints[2].y = -32768;
	
	edgepoints[3].x = -32768;
	edgepoints[3].y = -32768;

	// Do some clipping, <snip> <snip>
	edgepoints = edgeClipper(&numedgepoints, edgepoints, numclippers, clippers);
	
	if(!numedgepoints)
	{
		printf( "All carved away: subsector %d\n", ((byte*)ssec-subsectors)/SUBSIZE);
		ssec->numedgeverts = 0;
		ssec->edgeverts = 0;
		ssec->origedgeverts = 0;
		ssec->diffverts = 0;
	}
	else
	{
		// We need these with dynamic lights.
		ssec->origedgeverts = (fvertex_t*)Z_Malloc(sizeof(fvertex_t)*numedgepoints, PU_LEVEL, 0);
		memcpy(ssec->origedgeverts, edgepoints, sizeof(fvertex_t)*numedgepoints);
		
		// Find the center point. Do this by first finding the bounding box.
		ssec->bbox[0].x = ssec->bbox[1].x = edgepoints[0].x;
		ssec->bbox[0].y = ssec->bbox[1].y = edgepoints[0].y;
		for(i=1; i<numedgepoints; i++)
		{
			if(edgepoints[i].x < ssec->bbox[0].x) ssec->bbox[0].x = edgepoints[i].x;
			if(edgepoints[i].y < ssec->bbox[0].y) ssec->bbox[0].y = edgepoints[i].y;
			if(edgepoints[i].x > ssec->bbox[1].x) ssec->bbox[1].x = edgepoints[i].x;
			if(edgepoints[i].y > ssec->bbox[1].y) ssec->bbox[1].y = edgepoints[i].y;
		}
		ssec->midpoint.x = (ssec->bbox[1].x+ssec->bbox[0].x)/2;
		ssec->midpoint.y = (ssec->bbox[1].y+ssec->bbox[0].y)/2;

		// Allocate memory for the unit modifiers.
		ssec->diffverts = (fvertex_t*)Z_Malloc(sizeof(fvertex_t)*numedgepoints, PU_LEVEL, 0);

		// Make slight adjustments to patch up those ugly, small gaps.
		for(i=0; i<numedgepoints; i++)
		{
			fvertex_t *dv = ssec->diffverts + i;
			float dlen; // Delenn?
			dv->x = edgepoints[i].x - ssec->midpoint.x;
			dv->y = edgepoints[i].y - ssec->midpoint.y;
			dlen = (float) sqrt(dv->x*dv->x + dv->y*dv->y);
			if(dlen) // We don't want a divide-by-zero, now do we?
			{
				edgepoints[i].x += (dv->x /= dlen)/3;
				edgepoints[i].y += (dv->y /= dlen)/3;
			}
		}

		ssec->numedgeverts = numedgepoints;
		ssec->edgeverts = (fvertex_t*)Z_Malloc(sizeof(fvertex_t)*numedgepoints, PU_LEVEL, 0);
		memcpy(ssec->edgeverts, edgepoints, sizeof(fvertex_t)*numedgepoints);
	}

	// We're done, free the edgepoints memory.
	free(edgepoints);
	Z_Free(clippers);
}

//==========================================================================
//
// DD_PolygonizeWithoutCarving
//
//==========================================================================

void DD_PolygonizeWithoutCarving()
{
	int			i, j;
	subsector_t *sub;
		
	for(i=0; i<numsubsectors; i++)
	{
		sub = SUBSECTOR_PTR(i);
		sub->origedgeverts = (fvertex_t*) Z_Malloc(sizeof(fvertex_t) * sub->numlines, PU_LEVEL, 0);
		sub->numedgeverts = sub->numlines;
		sub->edgeverts = (fvertex_t*) Z_Malloc(sizeof(fvertex_t) * sub->numlines, PU_LEVEL, 0);
		for(j=0; j<sub->numlines; j++)
		{
			sub->origedgeverts[j].x = SEG_PTR(sub->firstline + j)->v1->x >> FRACBITS;
			sub->origedgeverts[j].y = SEG_PTR(sub->firstline + j)->v1->y >> FRACBITS;
			sub->edgeverts[j].x = sub->origedgeverts[j].x;
			sub->edgeverts[j].y = sub->origedgeverts[j].y;
		}

		// Find the center point. First calculate the bounding box.
		sub->bbox[0].x = sub->bbox[1].x = sub->origedgeverts[0].x;
		sub->bbox[0].y = sub->bbox[1].y = sub->origedgeverts[0].y;
		for(j=1; j<sub->numlines; j++)
		{
			if(sub->origedgeverts[j].x < sub->bbox[0].x) sub->bbox[0].x = sub->origedgeverts[j].x;
			if(sub->origedgeverts[j].y < sub->bbox[0].y) sub->bbox[0].y = sub->origedgeverts[j].y;
			if(sub->origedgeverts[j].x > sub->bbox[1].x) sub->bbox[1].x = sub->origedgeverts[j].x;
			if(sub->origedgeverts[j].y > sub->bbox[1].y) sub->bbox[1].y = sub->origedgeverts[j].y;
		}
		sub->midpoint.x = (sub->bbox[1].x + sub->bbox[0].x) / 2;
		sub->midpoint.y = (sub->bbox[1].y + sub->bbox[0].y) / 2;

		// Allocate memory for the unit modifiers.
		sub->diffverts = (fvertex_t*) Z_Malloc(sizeof(fvertex_t) * sub->numlines, PU_LEVEL, 0);

		// Make slight adjustments to patch up those ugly, small gaps.
		for(j=0; j<sub->numlines; j++)
		{
			fvertex_t *dv = sub->diffverts + j;
			float dlen; 
			dv->x = sub->edgeverts[j].x - sub->midpoint.x;
			dv->y = sub->edgeverts[j].y - sub->midpoint.y;
			dlen = (float) sqrt(dv->x*dv->x + dv->y*dv->y);
			if(dlen) // We don't want a divide-by-zero, now do we?
			{
				sub->edgeverts[j].x += (dv->x /= dlen)/3;
				sub->edgeverts[j].y += (dv->y /= dlen)/3;
			}
		}
	}
}

//==========================================================================
//
// DD_CreateFloorsAndCeilings
//
// Recursively polygonizes all ceilings and floors.
//
//==========================================================================

void DD_CreateFloorsAndCeilings(int bspnode, int numdivlines, divline_t *divlines)
{
	node_t		*nod;
	divline_t	*childlist, *dl;
	int			childlistsize = numdivlines+1;
	
	// If this is a subsector we are dealing with, begin carving with the
	// given list.
	if(bspnode & NF_SUBSECTOR)
	{
		/*if(bspnum == -1)
			R_Subsector (0);
		else
			R_Subsector (bspnum&(~NF_SUBSECTOR));*/
		
		// We have arrived at a subsector. The divline list contains all
		// the partition lines that carve out the subsector.

		//printf( "subsector %d: %d divlines\n",bspnode&(~NF_SUBSECTOR),numdivlines);

		int ssidx = bspnode & (~NF_SUBSECTOR);
		//if(ssidx < 10)
		DD_ConvexCarver(SUBSECTOR_PTR(ssidx), numdivlines, divlines);

		// Now we check all the previous subsectors and try to find any
		// mergeable edgepoints. This may help us reduce cracks and holes.
		/*for(i=0; i<ssidx; i++)
			P_MergeSubsectorEdges(subsectors+ssidx, subsectors+i);*/

		//printf( "subsector %d: %d edgeverts\n", ssidx, subsectors[ssidx].numedgeverts);
		// This leaf is done.
		return;
	}

	// Get a pointer to the node.
//	nod = nodes + bspnode;
	nod = NODE_PTR(bspnode);

	// Allocate a new list for each child.
	childlist = (divline_t*)malloc(childlistsize*sizeof(divline_t));
	// -JL- Paranoia
	if (!childlist)
		I_Error("DD_CreateFloorsAndCeilings: malloc failed");

	// Copy the previous lines, from the parent nodes.
	if(divlines) memcpy(childlist,divlines,numdivlines*sizeof(divline_t));

	dl = childlist + numdivlines;
	dl->x = nod->x;
	dl->y = nod->y;
	// The right child gets the original line (LEFT side clipped).
	dl->dx = nod->dx;
	dl->dy = nod->dy;
	DD_CreateFloorsAndCeilings(nod->children[0],childlistsize,childlist);

	// The left side. We must reverse the line, otherwise the wrong
	// side would get clipped.
	dl->dx = -nod->dx;
	dl->dy = -nod->dy;
	DD_CreateFloorsAndCeilings(nod->children[1],childlistsize,childlist);

	// We are finishing with this node, free the allocated list.
	free(childlist);
}

//==========================================================================
//
// DD_SkyFix
//
// Fixing the sky means that for adjacent sky sectors the lower sky is
// lifted to match the upper sky. The raising only affects rendering,
// obviously.
//
//==========================================================================

void DD_SkyFix()
{
	boolean		adjusted;
	int			i;

	// We'll do this as long as we must to be sure all the sectors are fixed.
	do 
	{
		adjusted = false;

		// We need to check all the linedefs.
		for(i=0; i<numlines; i++)
		{
			line_t *line = LINE_PTR(i);
			sector_t *front = line->frontsector, *back = line->backsector;
			int fix = 0;

			// The conditions!
			if(!front || !back) continue;
			
			// Both the front and back sectors must have the sky ceiling.
			if(front->ceilingpic != skyflatnum || back->ceilingpic != skyflatnum) 
				continue;
			
			// Operate on the lower sector.
			if(front->ceilingheight < back->ceilingheight)
			{
				fix = (back->ceilingheight-front->ceilingheight) >> FRACBITS;
				if(fix > front->skyfix) 
				{
					front->skyfix = fix;
					adjusted = true;
				}
			}
			else if(front->ceilingheight > back->ceilingheight)
			{
				fix = (front->ceilingheight-back->ceilingheight) >> FRACBITS;
				if(fix > back->skyfix) 
				{
					back->skyfix = fix;
					adjusted = true;
				}
			}
		}
	}
	while(adjusted);
}

//==========================================================================
//
// DD_PolygonizeLevel
//
// This routine is called from the game dll to polygonize the current level.
// Creates floors and ceilings and fixes the adjacent sky sector heights.
// Creates a big enough dlBlockLinks.
//
//==========================================================================

void DD_SetupLevel(int flags)
{
	if(flags & DDSLF_POLYGONIZE)
	{
		if(flags & DDSLF_DONT_CLIP)
			DD_PolygonizeWithoutCarving();
		else
			DD_CreateFloorsAndCeilings(numnodes-1, 0, NULL);
	}
	if(flags & DDSLF_FIX_SKY) DD_SkyFix();
	if(flags & DDSLF_REVERB) S_CalcSectorReverbs();
	// Create a big enough dlBlockLinks.
	DL_InitBlockMap();	
	map_rendered = false;
}

// Make sure all texture references in the level data are good.
void DD_ValidateLevel(void)
{
	int		i;
	//mobj_t	*iter;

	for(i=0; i<numsectors; i++)
	{
		sector_t *sec = SECTOR_PTR(i);
		if(sec->ceilingpic > numflats-1) sec->ceilingpic = 0;
		if(sec->floorpic > numflats-1) sec->floorpic = 0;
		// Update mobj floorpics.
	/*	for(iter=sec->thinglist; iter; iter=iter->snext)
			iter->floorpic = sec->floorpic;*/
	}
	for(i=0; i<numsides; i++)
	{
		side_t *side = SIDE_PTR(i);
		if(side->toptexture > numtextures-1) side->toptexture = 0;
		if(side->midtexture > numtextures-1) side->midtexture = 0;
		if(side->bottomtexture > numtextures-1) side->bottomtexture = 0;
	}
}
