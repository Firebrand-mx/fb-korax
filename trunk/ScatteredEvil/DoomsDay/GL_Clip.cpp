
//**************************************************************************
//**
//** OGL_CLIP.C
//**
//** Version:		1.0
//** Last Build:	-?-
//** Author:		jk
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdlib.h>
#include "dd_def.h"
#include "i_win32.h"
#include "m_bams.h"
#include "gl_def.h"

// MACROS ------------------------------------------------------------------

#define MAXCLIPNODES	128		// We can have this many nodes at once.

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern float vx, vz;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

clipnode_t *clipnodes;	// The list of clipnodes.
clipnode_t *cliphead;	// The head node.

int maxnumnodes=0;		

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static binangle_t anglist[MAX_POLY_SIDES];

// CODE --------------------------------------------------------------------

static void C_CountNodes()
{
	int	i;
	clipnode_t *ci;
	for(i=0, ci=cliphead; ci; i++, ci=ci->next);
	if(i > maxnumnodes) maxnumnodes = i;
}

void C_Init()
{
	clipnodes = (clipnode_t *)Z_Malloc(sizeof(clipnode_t)*MAXCLIPNODES, PU_STATIC, 0);
}

void C_ClearRanges()
{
	memset(clipnodes, 0, sizeof(clipnode_t)*MAXCLIPNODES);
	cliphead = 0;
}

// Finds the first unused clip node.
static clipnode_t *C_NewRange(binangle_t stAng, binangle_t endAng)	
{
	int			i;
	clipnode_t	*cnode;

	for(i=0; i<MAXCLIPNODES; i++) if(!clipnodes[i].used) break;
	if(i==MAXCLIPNODES-1) I_Error("C_NewRange: Out of clipnodes (max %d).\n", MAXCLIPNODES);
	// Initialize the node.
	cnode = clipnodes+i;
	cnode->used = 1;
	cnode->start = stAng;
	cnode->end = endAng;
	return cnode;
}

static void C_RemoveRange(clipnode_t *crange)
{
	if(!crange->used) I_Error("Tried to remove an unused range.\n");

	// If this is the head, move it.
	if(cliphead == crange) cliphead = crange->next;

	crange->used = 0;
	if(crange->prev) crange->prev->next = crange->next;
	if(crange->next) crange->next->prev = crange->prev;
	crange->prev = crange->next = 0;
}

static void C_AddRange(binangle_t startAngle, binangle_t endAngle)
{
	clipnode_t	*ci, *crange;

	//printf( "=== C_AddRange(%x,%x):\n",startAngle,endAngle);
	if(startAngle == endAngle)
	{
		//printf( "  range has no length, skipping\n");
		return;
	}
	// There are previous ranges. Check that the new range isn't contained
	// by any of them.
	for(ci=cliphead; ci; ci=ci->next)
	{
		//printf( "      %p: %4x => %4x (%d)\n",ci,ci->start,ci->end,ci->used);
		if(startAngle >= ci->start && endAngle <= ci->end)
		{
			//printf( "  range already exists\n");
			return;	// The new range already exists.
		}
		//printf( "loop1\n");
		/*if(ci == ci->next)
			I_Error("%p linked to itself: %x => %x\n",ci,ci->start,ci->end);*/
	}

	// Now check if any of the old ranges are contained by the new one.
	for(ci=cliphead; ci;)
	{
		//printf( "loop2\n");
		if(ci->start >= startAngle && ci->end <= endAngle)
		{
			crange = ci;
			//printf( "  removing contained range %x => %x\n",crange->start,crange->end);
			// We must do this in order to keep the loop from breaking.
			ci = ci->next;	
			C_RemoveRange(crange);
			//if(!ci) ci = cliphead;
			//if(!ci) break;
			continue;
		}	
		ci = ci->next;
	}

	// If there is no head, this will be the first range.
	if(!cliphead)
	{
		cliphead = C_NewRange(startAngle, endAngle);
		//printf( "  new head node added, %x => %x\n", cliphead->start, cliphead->end);
		return;
	}

	// Now it is possible that the new range overlaps one or two old ranges.
	// If two are overlapped, they are consecutive. First we'll try to find
	// a range that overlaps the beginning.
	for(ci=cliphead; ci; ci=ci->next)
	{
		//printf( "loop3\n");
		if(ci->start >= startAngle && ci->start <= endAngle)
		{
			// New range's end and ci's beginning overlap. ci's end is outside.
			// Otherwise it would have been already removed.
			// It suffices to adjust ci.
			//printf( "  overlapping beginning with %x => %x, ",ci->start,ci->end);
			ci->start = startAngle;
			//printf( "adjusting ci to %x => %x\n",ci->start,ci->end);
			return;
		}
		// Check an overlapping end.
		if(ci->end >= startAngle && ci->end <= endAngle)
		{
			// Now it's possible that the ci->next's beginning overlaps the new
			// range's end. In that case there will be a merger.
			//printf( "  overlapping end with %x => %x:\n",ci->start,ci->end);
			crange = ci->next;
			if(!crange)
			{
				ci->end = endAngle;
				//printf( "    no next, adjusting end (now %x => %x)\n",ci->start,ci->end);
				return;
			}
			else
			{
				if(/*crange->start >= startAngle && */crange->start <= endAngle)
				{
					// Hello! A fusion will commence. Ci will eat the new
					// range AND crange.
					ci->end = crange->end;
					//printf( "    merging with the next (%x => %x)\n",crange->start,crange->end);
					C_RemoveRange(crange);
					return;
				}
				else
				{
					// No overlapping: the same, normal case.
					ci->end = endAngle;
					//printf( "    not merger w/next, ci is now %x => %x\n",ci->start,ci->end);
					return;
				}
			}
		}
	}
	
	// Still here? Now we know for sure that the range is disconnected from
	// the others. We still need to find a good place for it. Crange will mark
	// the spot. 
	
	// OPTIMIZE: Why not search this during the last loop?

	//printf( "  range doesn't overlap old ones:\n");
	crange = 0;
	for(ci=cliphead; ci; ci=ci->next)
	{
	//	//printf( "loop4\n");
		if(ci->start < endAngle) // Is this a suitable spot?
			crange = ci; // Add after this one.
		else break;	// We're done.
	}
	if(!crange)
	{
		//printf( "    no suitable new spot, adding to head\n");
		// We have a new head node.
		crange = cliphead;
		cliphead = C_NewRange(startAngle, endAngle);
		cliphead->next = crange;
		crange->prev = cliphead;
		return;
	}
	//printf("  spot found, adding after %x => %x\n",crange->start,crange->end);
	// Add the new range after crange.
	ci = C_NewRange(startAngle, endAngle);
	ci->next = crange->next;
	if(ci->next) ci->next->prev = ci;
	ci->prev = crange;
	crange->next = ci;
}

void C_Ranger()
{
	clipnode_t *ci;

	printf("Ranger:\n");
	for(ci=cliphead; ci; ci=ci->next)
	{
		if(ci==cliphead)
		{
			if(ci->prev != 0)
				I_Error("Cliphead->prev != 0.\n");
		}
		// Confirm that the links to prev and next are OK.
		if(ci->prev)
		{
			if(ci->prev->next != ci)
				I_Error("Prev->next != this\n");
		}
		else if(ci != cliphead) I_Error("prev == null, this isn't cliphead.\n");

		if(ci->next)
		{
			if(ci->next->prev != ci)
				I_Error("Next->prev != this\n");
		}

		printf( "  %p: %04x => %04x ", ci, ci->start, ci->end);
		if(ci->prev)
			printf( "(gap: %d)\n", ci->start-ci->prev->end);
		else
			printf( "\n");
	}
}

void C_SafeAddRange(binangle_t startAngle, binangle_t endAngle)
{
	binangle_t angLen = endAngle-startAngle;

	//printf( "Adding range %x => %x...\n", startAngle, endAngle);

	// Check if the range is valid.
	if(!angLen || angLen > BANG_180) return;

	// The range may still wrap around.
	if((int)startAngle+(int)angLen > BANG_MAX)
	{
	//	printf( "    adding in two parts\n");
		// The range has to added in two parts.
		C_AddRange(startAngle, BANG_MAX);
		//C_Ranger();
		C_AddRange(0, endAngle);
		//C_Ranger();
	}
	else
	{
	//	printf( "    adding normally\n");
		// Add the range as usual.
		C_AddRange(startAngle, endAngle);
		//C_Ranger();
	}
	//C_CountNodes();	// Just development info.
	//C_Ranger();
}

// Add a segment relative to the current viewpoint.
void C_AddViewRelSeg(float x1, float y1, float x2, float y2)
{
	float dx1 = x1-vx, dy1 = y1-vz, dx2 = x2-vx, dy2 = y2-vz;
	C_SafeAddRange(bamsAtan2((int)(dy2*10), (int)(dx2*10)), 
		bamsAtan2((int)(dy1*10), (int)(dx1*10)));
}

// The specified range must be safe!
static int C_IsRangeVisible(binangle_t startAngle, binangle_t endAngle)
{
	clipnode_t	*ci;
	
	for(ci=cliphead; ci; ci=ci->next)
		if(startAngle >= ci->start && endAngle <= ci->end)
			return 0;
	// No node fully contained the specified range.
	return 1;
}

// Returns 1 if the range is not entirely clipped.
static int C_SafeCheckRange(binangle_t startAngle, binangle_t endAngle)
{
	binangle_t angLen = endAngle - startAngle;

	// Check that the range is valid.
	if(!angLen || angLen >= BANG_180) return 0;
	if((int)startAngle+(int)angLen > BANG_MAX)
	{
		// The range wraps around.
		return (C_IsRangeVisible(startAngle, BANG_MAX) 
			|| C_IsRangeVisible(0, endAngle));
	}
	return C_IsRangeVisible(startAngle, endAngle);
}

int C_CheckViewRelSeg(float x1, float y1, float x2, float y2)
{
	float dx1 = x1-vx, dy1 = y1-vz, dx2 = x2-vx, dy2 = y2-vz;
	return C_SafeCheckRange(bamsAtan2((int)(dy2*10), (int)(dx2*10)), 
		bamsAtan2((int)(dy1*10), (int)(dx1*10)));
}

// Returns 1 if the specified angle is visible.
int C_IsAngleVisible(binangle_t bang)
{
	clipnode_t	*ci;

	for(ci=cliphead; ci; ci=ci->next)
		if(bang > ci->start && bang < ci->end) return 0;
	// No one clipped this angle.
	return 1;
}

clipnode_t *C_AngleClippedBy(binangle_t bang)
{
	clipnode_t	*ci;

	for(ci=cliphead; ci; ci=ci->next)
		if(bang > ci->start && bang < ci->end) return ci;
	// No one clipped this angle.
	return 0;
}

// Returns 1 if the subsector might be visible.
int C_CheckSubsector(subsector_t *ssec)
{
	int			i;
	clipnode_t	*cnode=0;

	for(i=0; i<ssec->numedgeverts; i++)	// Check all corners.
	{
		fvertex_t *vtx = ssec->edgeverts+i;
		// Shift for more accuracy.
		anglist[i] = bamsAtan2((int)(vtx->y - vz)<<7, (int)(vtx->x - vx)<<7);
	}
	// Check each of the ranges defined by the edges.
	for(i=0; i<ssec->numedgeverts-1; i++)
	{
		int end = i+1;
		binangle_t angLen;

		// The last edge won't be checked. This is because the edges
		// define a closed, convex polygon and the last edge's range is 
		// always already covered by the previous edges. (Right?)

		// If even one of the edges is not contained by a clipnode, 
		// the subsector is at least partially visible.
		angLen = anglist[end] - anglist[i];
		
		// The viewer is on an edge, the subsector should be visible.
		if(angLen == BANG_180) return 1;

		// Choose the start and end points so that length is < 180.
		if(angLen < BANG_180)
		{
			if(C_SafeCheckRange(anglist[i], anglist[end])) return 1;
		}
		else
		{
			if(C_SafeCheckRange(anglist[end], anglist[i])) return 1;
		}
	}
	// All clipped away, the subsector can not be seen.
	return 0;
}
