
//**************************************************************************
//**
//** r_local.h : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

#ifndef __R_LOCAL__
#define __R_LOCAL__

#define PI                                      3.141592657

/*
==============================================================================

					INTERNAL MAP TYPES

==============================================================================
*/

//================ used by play and refresh

typedef struct
{
	fixed_t         x,y;

	// --- You can freely make changes after this.	

} vertex_t;

struct line_s;

typedef struct
{
	fixed_t	floorheight, ceilingheight;
	short	floorpic, ceilingpic;
	short   lightlevel;
	int     validcount;             // if == validcount, already checked
	mobj_t  *thinglist;             // list of mobjs in sector
	int     linecount;
	struct line_s **Lines;        // [linecount] size
	int		flatoffx, flatoffy;		// Scrolling flats.
	int		skyfix;					// Offset to ceiling height rendering w/sky.
	float	reverb[NUM_REVERB_DATA];

	// --- You can freely make changes after this.

	short  	special, tag;
	int     soundtraversed;         // 0 = untraversed, 1,2 = sndlines -1
	mobj_t  *soundtarget;           // thing that made a sound (or null)
	seqtype_t seqType;				// stone, metal, heavy, etc...
	int     blockbox[4];            // mapblock bounding box for height changes
	degenmobj_t soundorg;           // for any sounds played by the sector
	void    *specialdata;           // thinker_t for reversable actions
} sector_t;

typedef struct
{
	fixed_t         textureoffset;          // add this to the calculated texture col
	fixed_t         rowoffset;                      // add this to the calculated texture top
	short           toptexture, bottomtexture, midtexture;
	sector_t        *sector;

	// --- You can freely make changes after this.	

} side_t;

typedef enum
{
	ST_HORIZONTAL,
	ST_VERTICAL,
	ST_POSITIVE,
	ST_NEGATIVE
} slopetype_t;

typedef struct line_s
{
	vertex_t *v1;
	vertex_t *v2;
	short flags;
	sector_t *frontsector;
	sector_t *backsector;

	// --- You can freely make changes after this.	

	fixed_t dx;
	fixed_t dy;
	byte special;
	byte arg1;
	byte arg2;
	byte arg3;
	byte arg4;
	byte arg5;
	short sidenum[2];
	fixed_t bbox[4];
	slopetype_t slopetype;
	int validcount;
	void *specialdata;
} line_t;

typedef struct
{
	vertex_t        *v1, *v2;
	float			length;			// Length of the segment (v1 -> v2).
	fixed_t         offset;
	side_t          *sidedef;
	line_t          *linedef;
	sector_t        *frontsector;
	sector_t        *backsector;            // NULL for one sided lines

	// --- You can freely make changes after this.	

	angle_t         angle;
} seg_t;

// ===== Polyobj data =====
typedef struct
{
	int numSegs;
	seg_t **Segs;

	// --- You can freely make changes after this.	

	degenmobj_t startSpot;
	vertex_t *originalPts; 	// used as the base for the rotations
	vertex_t *prevPts; 		// use to restore the old point values
	angle_t angle;
	int tag;						// reference tag assigned in HereticEd
	int bbox[4];
	int validcount;
	boolean crush; 			// should the polyobj attempt to crush mobjs?
	int seqType;
	fixed_t size; // polyobj size (area of POLY_AREAUNIT == size of FRACUNIT)
	void *specialdata; // pointer a thinker, if the poly is moving
} polyobj_t;

typedef struct polyblock_s
{
	polyobj_t *polyobj;
	struct polyblock_s *prev;
	struct polyblock_s *next;
} polyblock_t;

typedef struct
{
	float x,y;
} fvertex_t;

typedef struct subsector_s
{
	sector_t        *sector;
	short           numLines;
	short           firstline;
	polyobj_t		*poly;
	// Sorted edge vertices for rendering floors and ceilings.
	char			numedgeverts;
	fvertex_t		*edgeverts;		// A list of edge vertices.
	fvertex_t		*origedgeverts;	// Unmodified, accurate edge vertices.
	fvertex_t		*diffverts;		// Unit modifiers.
	fvertex_t		bbox[2];		// Min and max points.
	fvertex_t		midpoint;		// Center of bounding box.

	// --- You can freely make changes after this.	

} subsector_t;

typedef struct
{
	fixed_t         x,y,dx,dy;                      // partition line
	fixed_t         bbox[2][4];                     // bounding box for each child
	unsigned short  children[2];            // if NF_SUBSECTOR its a subsector

	// --- You can freely make changes after this.	

} node_t;

// ==============================================================================

// Map data is in the main engine, so these are helpers...
#define numvertexes		(*gi.numvertexes)
#define numsegs			(*gi.numsegs)
#define	numsectors		(*gi.numsectors)
#define numsubsectors	(*gi.numsubsectors)
#define numnodes		(*gi.numnodes)
#define numlines		(*gi.numlines)
#define numsides		(*gi.numsides)

#define vertexes		((vertex_t*)(*gi.vertexes))
#define segs			((seg_t*)(*gi.segs))
#define	sectors			((sector_t*)(*gi.sectors))
#define subsectors		((subsector_t*)(*gi.subsectors))
#define nodes			((node_t*)(*gi.nodes))
#define lines			((line_t*)(*gi.lines))
#define sides			((side_t*)(*gi.sides))

#define Validcount		(*gi.validcount)

#define skyflatnum		gi.Get(DD_SKYFLATNUM)

#define firstflat		gi.Get(DD_FIRSTFLAT)
#define numflats		gi.Get(DD_NUMFLATS)

#define R_PointToAngle2		gi.R_PointToAngle2
#define R_PointInSubsector	(subsector_t*)gi.R_PointInSubsector

#endif          // __R_LOCAL__

