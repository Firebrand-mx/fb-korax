
//**************************************************************************
//**
//** p_setup.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

#pragma optimize("g", off)

// HEADER FILES ------------------------------------------------------------

#include <math.h>
#include <stdlib.h>
#include "h2def.h"
#include "p_local.h"
#include "soundst.h"
#include "m_bams.h"
#include "gl_dyn.h"

// MACROS ------------------------------------------------------------------

#define MAPINFO_SCRIPT_NAME "MAPINFO"
#define MCMD_SKY1 1
#define MCMD_SKY2 2
#define MCMD_LIGHTNING 3
#define MCMD_FADETABLE 4
#define MCMD_DOUBLESKY 5
#define MCMD_CLUSTER 6
#define MCMD_WARPTRANS 7
#define MCMD_NEXT 8
#define MCMD_CDTRACK 9
#define MCMD_CD_STARTTRACK 10
#define MCMD_CD_END1TRACK 11
#define MCMD_CD_END2TRACK 12
#define MCMD_CD_END3TRACK 13
#define MCMD_CD_INTERTRACK 14
#define MCMD_CD_TITLETRACK 15

#define UNKNOWN_MAP_NAME "DEVELOPMENT MAP"
#define DEFAULT_SKY_NAME "SKY1"
#define DEFAULT_SONG_LUMP "DEFSONG"
#define DEFAULT_FADE_TABLE "COLORMAP"

// TYPES -------------------------------------------------------------------

typedef struct mapInfo_s mapInfo_t;
struct mapInfo_s
{
	short cluster;
	short warpTrans;
	short nextMap;
	short cdTrack;
	char name[32];
	short sky1Texture;
	short sky2Texture;
	fixed_t sky1ScrollDelta;
	fixed_t sky2ScrollDelta;
	boolean doubleSky;
	boolean lightning;
	int fadetable;
	char songLump[10];
};

typedef struct
{
	unsigned short	v1, v2;
	short			linedef, side;
	unsigned short	partner;
} glseg_t;

typedef struct
{
	int				x, y;
} glvert2_t;


// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

void P_SpawnMapThing(mapthing_t *mthing);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static int QualifyMap(int map);

float AccurateDistance(fixed_t dx,fixed_t dy);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int numvertexes;
vertex_t *vertexes;

int numsegs;
seg_t *segs;

int numsectors;
sector_t *sectors;

int numsubsectors;
subsector_t *subsectors;

int numnodes;
node_t *nodes;

int numlines;
line_t *lines;

int	numsides;
side_t *sides;

short *blockmaplump; // offsets in blockmap are from here
short *blockmap;
int bmapwidth, bmapheight; // in mapblocks
fixed_t bmaporgx, bmaporgy; // origin of block map
mobj_t **blocklinks; // for thing chains

byte *rejectmatrix; // for fast sight rejection

int MapCount;
mapthing_t deathmatchstarts[MAXDEATHMATCHSTARTS], *deathmatch_p;
mapthing_t playerstarts[MAX_PLAYER_STARTS][MAXPLAYERS];
int firstGLvertex = 0;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static mapInfo_t MapInfo[99];
static char *MapCmdNames[] =
{
	"SKY1",
	"SKY2",
	"DOUBLESKY",
	"LIGHTNING",
	"FADETABLE",
	"CLUSTER",
	"WARPTRANS",
	"NEXT",
	"CDTRACK",
	"CD_START_TRACK",
	"CD_END1_TRACK",
	"CD_END2_TRACK",
	"CD_END3_TRACK",
	"CD_INTERMISSION_TRACK",
	"CD_TITLE_TRACK",
	NULL
};
static int MapCmdIDs[] =
{
	MCMD_SKY1,
	MCMD_SKY2,
	MCMD_DOUBLESKY,
	MCMD_LIGHTNING,
	MCMD_FADETABLE,
	MCMD_CLUSTER,
	MCMD_WARPTRANS,
	MCMD_NEXT,
	MCMD_CDTRACK,
	MCMD_CD_STARTTRACK,
	MCMD_CD_END1TRACK,
	MCMD_CD_END2TRACK,
	MCMD_CD_END3TRACK,
	MCMD_CD_INTERTRACK,
	MCMD_CD_TITLETRACK
};

static int cd_NonLevelTracks[6]; // Non-level specific song cd track numbers 

// CODE --------------------------------------------------------------------

/*
=================
=
= P_LoadVertexes
=
=================
*/

void P_LoadVertexes (int lump, int gllump)
{
	byte            *data;
	int				i, orignum, ver;
	mapvertex_t     *ml;
	glvert2_t       *glv;
	vertex_t        *li;

	orignum = numvertexes = W_LumpLength(lump) / sizeof(mapvertex_t);
	// glBSP lump provided?
	if(gllump >= 0)
	{
		// There are additional vertices in gllump.
		numvertexes += W_LumpLength(gllump) / sizeof(mapvertex_t);
	}
	vertexes = (vertex_t *)Z_Malloc(numvertexes * sizeof(vertex_t), PU_LEVEL, 0);
	data = (byte *)W_CacheLumpNum (lump,PU_STATIC);
	ml = (mapvertex_t *)data;
	li = vertexes;
	for(i=0 ; i<orignum; i++, li++, ml++)
	{
		li->x = SHORT(ml->x)<<FRACBITS;
		li->y = SHORT(ml->y)<<FRACBITS;
	}
	Z_Free(data);

	firstGLvertex = orignum;
	// Also load the GL vertices.
	if(gllump >= 0)
	{
		data = (byte *)W_CacheLumpNum(gllump, PU_STATIC);
		ver = 1;
		if(data[0] == 'g' && data[1] == 'N' && data[2] == 'd' && data[3] == '2')
		{
			ST_Message( "GL_VERT v2.0\n");
			ver = 2;
			// -JL- Calculate new number of vertexes
			numvertexes = orignum + (W_LumpLength(gllump) - 4) / sizeof(glvert2_t);
		}
		ml = (mapvertex_t*) data;
		glv = (glvert2_t*) (data + 4);
		for(i=orignum; i<numvertexes; i++, li++, ml++, glv++)
		{
			if(ver == 1)
			{
				li->x = SHORT(ml->x)<<FRACBITS;
				li->y = SHORT(ml->y)<<FRACBITS;
			}
			else
			{
				li->x = glv->x;
				li->y = glv->y;
			}
		}
		Z_Free(data);
	}
}


/*
=================
=
= P_LoadSegs
=
=================
*/

void P_LoadSegs (int lump)
{
	void            *data;
	int             i;
	mapseg_t        *ml;
	seg_t           *li;
	line_t			*ldef;
	int             linedef, side;

	numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
	segs = (seg_t *)Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
	memset (segs, 0, numsegs*sizeof(seg_t));
	data = W_CacheLumpNum (lump,PU_STATIC);

	ml = (mapseg_t *)data;
	li = segs;
	for (i=0 ; i<numsegs ; i++, li++, ml++)
	{
		li->v1 = &vertexes[SHORT(ml->v1)];
		li->v2 = &vertexes[SHORT(ml->v2)];

		li->angle = (SHORT(ml->angle))<<16;
		li->offset = (SHORT(ml->offset))<<16;
		linedef = SHORT(ml->linedef);
		ldef = &lines[linedef];
		li->linedef = ldef;
		side = SHORT(ml->side);
		li->sidedef = &sides[ldef->sidenum[side]];
		li->frontsector = sides[ldef->sidenum[side]].sector;
		if (ldef-> flags & ML_TWOSIDED)
			li->backsector = sides[ldef->sidenum[side^1]].sector;
		else
			li->backsector = 0;
		
		// Calculate the length of the segment. We need this for
		// the texture coordinates. -jk
		li->length = AccurateDistance(li->v2->x - li->v1->x, li->v2->y - li->v1->y);								
	}

	Z_Free (data);
}

void P_LoadSegsGL(int lump)
{
	void		*data;
	int			i;
	glseg_t		*gls;
	seg_t		*li;
	line_t		*ldef;

	numsegs = W_LumpLength(lump) / sizeof(glseg_t);
	segs = (seg_t *)Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
	memset(segs, 0, numsegs * sizeof(seg_t));	
	data = W_CacheLumpNum(lump, PU_STATIC);

	gls = (glseg_t*) data;
	li = segs;
	for(i=0; i<numsegs; i++, li++, gls++)
	{
		li->v1 = &vertexes[gls->v1 & 0x8000? firstGLvertex + (gls->v1 & ~0x8000) : gls->v1];
		li->v2 = &vertexes[gls->v2 & 0x8000? firstGLvertex + (gls->v2 & ~0x8000) : gls->v2];
		if(gls->linedef != -1)
		{
			ldef = &lines[gls->linedef];
			li->linedef = ldef;
			li->sidedef = &sides[ldef->sidenum[gls->side]];
			li->frontsector = sides[ldef->sidenum[gls->side]].sector;
			if (ldef->flags & ML_TWOSIDED)
				li->backsector = sides[ldef->sidenum[gls->side^1]].sector;
			else
				li->backsector = 0;
			if(gls->side == 0)
				li->offset = FRACUNIT * AccurateDistance(li->v1->x - ldef->v1->x, li->v1->y - ldef->v1->y); 
			else 
				li->offset = FRACUNIT * AccurateDistance(li->v1->x - ldef->v2->x, li->v1->y - ldef->v2->y); 
			li->angle = bamsAtan2((li->v2->y - li->v1->y)>>FRACBITS, 
				(li->v2->x - li->v1->x)>>FRACBITS) << 16;
		}
		else
		{
			li->linedef = NULL;
			li->sidedef = NULL;
			li->frontsector = NULL;
			li->backsector = NULL;
		}
		
		// Calculate the length of the segment. We need this for
		// the texture coordinates. -jk
		li->length = AccurateDistance(li->v2->x - li->v1->x, li->v2->y - li->v1->y);								
	}
	
	Z_Free(data);
}

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



/*
=================
=
= P_LoadSubsectors
=
=================
*/

void P_LoadSubsectors (int lump)
{
	void                    *data;
	int                     i;
	mapsubsector_t			*ms;
	subsector_t             *ss;

	numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
	subsectors = (subsector_t *)Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);
	memset(subsectors, 0, numsubsectors*sizeof(subsector_t));
	data = W_CacheLumpNum (lump,PU_STATIC);

	ms = (mapsubsector_t *)data;
	memset (subsectors,0, numsubsectors*sizeof(subsector_t));
	ss = subsectors;
	for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
	{
		ss->numlines = SHORT(ms->numSegs);
		ss->firstline = SHORT(ms->firstseg);
	}

	Z_Free (data);
}


/*
=================
=
= P_LoadSectors
=
=================
*/

void P_LoadSectors (int lump)
{
	void                    *data;
	int                             i;
	mapsector_t             *ms;
	sector_t                *ss;

	numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
	sectors = (sector_t *)Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);
	memset (sectors, 0, numsectors*sizeof(sector_t));
	data = W_CacheLumpNum (lump,PU_STATIC);

	ms = (mapsector_t *)data;
	ss = sectors;

	// Make sure primary lumps are used for flat searching
//	W_UsePrimary();

	for(i = 0; i < numsectors; i++, ss++, ms++)
	{
		ss->floorheight = SHORT(ms->floorheight)<<FRACBITS;
		ss->ceilingheight = SHORT(ms->ceilingheight)<<FRACBITS;
		ss->floorpic = R_FlatNumForName(ms->floorpic);
		ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
		ss->lightlevel = SHORT(ms->lightlevel);
		ss->special = SHORT(ms->special);
		ss->tag = SHORT(ms->tag);
		ss->thinglist = NULL;
		ss->seqType = SEQTYPE_STONE; // default seqType

//		ss->flatoffx = ss->flatoffy = 0;	// Flat scrolling. -jk
//		ss->skyfix = 0;		// Set if needed.
//		ss->reverbVolume = ss->reverbSpace = ss->reverbDecay = ss->reverbDamping = 0;
	}
/*	if(DevMaps)
	{
		W_UseAuxiliary();
	}*/
	Z_Free(data);
}


/*
=================
=
= P_LoadNodes
=
=================
*/

void P_LoadNodes (int lump)
{
	void            *data;
	int                     i,j,k;
	mapnode_t       *mn;
	node_t          *no;

	numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
	nodes = (node_t *)Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);
	data = W_CacheLumpNum (lump,PU_STATIC);

	mn = (mapnode_t *)data;
	no = nodes;
	for (i=0 ; i<numnodes ; i++, no++, mn++)
	{
		no->x = SHORT(mn->x)<<FRACBITS;
		no->y = SHORT(mn->y)<<FRACBITS;
		no->dx = SHORT(mn->dx)<<FRACBITS;
		no->dy = SHORT(mn->dy)<<FRACBITS;
		for (j=0 ; j<2 ; j++)
		{
			no->children[j] = SHORT(mn->children[j]);
			for (k=0 ; k<4 ; k++)
				no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
		}
	}
	Z_Free (data);
}

//==========================================================================
//
// P_LoadThings
//
//==========================================================================

void P_LoadThings(int lump)
{
	void *data;
	int i;
	mapthing_t *mt;
	int numthings;
	int playerCount;
	int deathSpotsCount;

	data = W_CacheLumpNum(lump, PU_STATIC);
	numthings = W_LumpLength(lump)/sizeof(mapthing_t);

	mt = (mapthing_t *)data;
	for(i = 0; i < numthings; i++, mt++)
	{
		mt->tid = SHORT(mt->tid);
		mt->x = SHORT(mt->x);
		mt->y = SHORT(mt->y);
		mt->height = SHORT(mt->height);
		mt->angle = SHORT(mt->angle);
		mt->type = SHORT(mt->type);
		mt->options = SHORT(mt->options);
		P_SpawnMapThing(mt);
	}
	P_CreateTIDList();
	P_InitCreatureCorpseQueue(false); // false = do NOT scan for corpses
	Z_Free(data);

	if(!deathmatch)
	{ // Don't need to check deathmatch spots
		return;
	}
	playerCount = 0;
	for(i = 0; i < MAXPLAYERS; i++)
	{
		playerCount += players[i].ingame;
	}
	deathSpotsCount = deathmatch_p-deathmatchstarts;
	if(deathSpotsCount < playerCount)
	{
		I_Error("P_LoadThings: Player count (%d) exceeds deathmatch "
			"spots (%d)", playerCount, deathSpotsCount);
	}
}

/*
=================
=
= P_LoadLineDefs
=
=================
*/

void P_LoadLineDefs(int lump)
{
	void *data;
	int i;
	maplinedef_t *mld;
	line_t *ld;
	vertex_t *v1, *v2;

	numlines = W_LumpLength(lump)/sizeof(maplinedef_t);
	lines = (line_t *)Z_Malloc(numlines*sizeof(line_t), PU_LEVEL, 0);
	memset(lines, 0, numlines*sizeof(line_t));
	data = W_CacheLumpNum(lump, PU_STATIC);

	mld = (maplinedef_t *)data;
	ld = lines;
	for(i = 0; i < numlines; i++, mld++, ld++)
	{
		ld->flags = SHORT(mld->flags);

		// Old line special info ...
		//ld->special = SHORT(mld->special);
		//ld->tag = SHORT(mld->tag);

		// New line special info ...
		ld->special = mld->special;
		ld->arg1 = mld->arg1;
		ld->arg2 = mld->arg2;
		ld->arg3 = mld->arg3;
		ld->arg4 = mld->arg4;
		ld->arg5 = mld->arg5;

		v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
		v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
		ld->dx = v2->x - v1->x;
		ld->dy = v2->y - v1->y;
		if (!ld->dx)
			ld->slopetype = ST_VERTICAL;
		else if (!ld->dy)
			ld->slopetype = ST_HORIZONTAL;
		else
		{
			if (FixedDiv (ld->dy , ld->dx) > 0)
				ld->slopetype = ST_POSITIVE;
			else
				ld->slopetype = ST_NEGATIVE;
		}

		if (v1->x < v2->x)
		{
			ld->bbox[BOXLEFT] = v1->x;
			ld->bbox[BOXRIGHT] = v2->x;
		}
		else
		{
			ld->bbox[BOXLEFT] = v2->x;
			ld->bbox[BOXRIGHT] = v1->x;
		}
		if (v1->y < v2->y)
		{
			ld->bbox[BOXBOTTOM] = v1->y;
			ld->bbox[BOXTOP] = v2->y;
		}
		else
		{
			ld->bbox[BOXBOTTOM] = v2->y;
			ld->bbox[BOXTOP] = v1->y;
		}
		ld->sidenum[0] = SHORT(mld->sidenum[0]);
		ld->sidenum[1] = SHORT(mld->sidenum[1]);
		if (ld->sidenum[0] != -1)
			ld->frontsector = sides[ld->sidenum[0]].sector;
		else
			ld->frontsector = 0;
		if (ld->sidenum[1] != -1)
			ld->backsector = sides[ld->sidenum[1]].sector;
		else
			ld->backsector = 0;
	}

	Z_Free (data);
}


/*
=================
=
= P_LoadSideDefs
=
=================
*/

void P_LoadSideDefs (int lump)
{
	void                    *data;
	int                             i;
	mapsidedef_t    *msd;
	side_t                  *sd;

	numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
	sides = (side_t *)Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);
	memset (sides, 0, numsides*sizeof(side_t));
	data = W_CacheLumpNum (lump,PU_STATIC);

	msd = (mapsidedef_t *)data;
	sd = sides;

	// Make sure primary lumps are used for texture searching
//	W_UsePrimary();

	for(i = 0; i < numsides; i++, msd++, sd++)
	{
		sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
		sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;
		sd->toptexture = R_TextureNumForName(msd->toptexture);
		sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
		sd->midtexture = R_TextureNumForName(msd->midtexture);
		sd->sector = &sectors[SHORT(msd->sector)];
	}
/*	if(DevMaps)
	{
		W_UseAuxiliary();
	}*/
	Z_Free(data);
}

/*
=================
=
= P_LoadBlockMap
=
=================
*/

void P_LoadBlockMap (int lump)
{
	int             i, count;

	blockmaplump = (short *)W_CacheLumpNum (lump,PU_LEVEL);
	blockmap = blockmaplump+4;
	count = W_LumpLength (lump)/2;
	for (i=0 ; i<count ; i++)
		blockmaplump[i] = SHORT(blockmaplump[i]);

	bmaporgx = blockmaplump[0]<<FRACBITS;
	bmaporgy = blockmaplump[1]<<FRACBITS;
	bmapwidth = blockmaplump[2];
	bmapheight = blockmaplump[3];

// clear out mobj chains
	count = sizeof(*blocklinks)* bmapwidth*bmapheight;
	blocklinks = (mobj_t **)Z_Malloc (count,PU_LEVEL, 0);
	memset (blocklinks, 0, count);
}




/*
=================
=
= P_GroupLines
=
= Builds sector line lists and subsector sector numbers
= Finds block bounding boxes for sectors
=================
*/

void P_GroupLines (void)
{
	line_t          **linebuffer;
	int             i, j, total;
	line_t          *li;
	sector_t        *sector;
	subsector_t     *ss;
	seg_t           *seg;
	fixed_t         bbox[4];
	int             block;

// look up sector number for each subsector
	ss = subsectors;
	for(i=0; i<numsubsectors; i++, ss++)
	{
		seg = &segs[ss->firstline];
		ss->sector = NULL;
		for(j=0; j<ss->numlines; j++, seg++)
			if(seg->sidedef)
			{
				ss->sector = seg->sidedef->sector;
				break;
			}
		if(ss->sector == NULL)
			I_Error("P_GroupLines: Subsector a part of no sector.\n");
	}

// count number of lines in each sector
	li = lines;
	total = 0;
	for (i=0 ; i<numlines ; i++, li++)
	{
		total++;
		li->frontsector->linecount++;
		if (li->backsector && li->backsector != li->frontsector)
		{
			li->backsector->linecount++;
			total++;
		}
	}

// build line tables for each sector
	linebuffer = (line_t **)Z_Malloc (total*4, PU_LEVEL, 0);
	sector = sectors;
	for (i=0 ; i<numsectors ; i++, sector++)
	{
		M_ClearBox(bbox);
		sector->lines = linebuffer;
		li = lines;
		for (j=0 ; j<numlines ; j++, li++)
		{
			if (li->frontsector == sector || li->backsector == sector)
			{
				*linebuffer++ = li;
				M_AddToBox(bbox, li->v1->x, li->v1->y);
				M_AddToBox(bbox, li->v2->x, li->v2->y);
			}
		}
		if (linebuffer - sector->lines != sector->linecount)
			I_Error ("P_GroupLines: miscounted");

		// set the degenmobj_t to the middle of the bounding box
		sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
		sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;

		// adjust bounding box to map blocks
		block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
		block = block >= bmapheight ? bmapheight-1 : block;
		sector->blockbox[BOXTOP]=block;

		block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
		block = block < 0 ? 0 : block;
		sector->blockbox[BOXBOTTOM]=block;

		block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
		block = block >= bmapwidth ? bmapwidth-1 : block;
		sector->blockbox[BOXRIGHT]=block;

		block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
		block = block < 0 ? 0 : block;
		sector->blockbox[BOXLEFT]=block;
	}

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

inline int detSideFloat(fvertex_t *pnt, fdivline_t *dline)
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
static float findIntersectionVertex(fvertex_t *start, fvertex_t *end, fdivline_t *fdiv, 
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
static fvertex_t *edgeClipper(int *numpoints, fvertex_t *points, int numclippers, fdivline_t *clippers)
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

static void DD_ConvexCarver(subsector_t *ssec, int num, divline_t *list)
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
			seg_t *seg = segs + (ssec->firstline+i-num);
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
		printf( "All carved away: subsector %d\n", ssec - subsectors);
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

static void DD_PolygonizeWithoutCarving()
{
	int			i, j;
	subsector_t *sub;
		
	for(i=0; i<numsubsectors; i++)
	{
		sub = subsectors + i;
		sub->origedgeverts = (fvertex_t*) Z_Malloc(sizeof(fvertex_t) * sub->numlines, PU_LEVEL, 0);
		sub->numedgeverts = sub->numlines;
		sub->edgeverts = (fvertex_t*) Z_Malloc(sizeof(fvertex_t) * sub->numlines, PU_LEVEL, 0);
		for(j=0; j<sub->numlines; j++)
		{
			sub->origedgeverts[j].x = segs[sub->firstline + j].v1->x >> FRACBITS;
			sub->origedgeverts[j].y = segs[sub->firstline + j].v1->y >> FRACBITS;
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

static void DD_CreateFloorsAndCeilings(int bspnode, int numdivlines, divline_t *divlines)
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
		DD_ConvexCarver(&subsectors[ssidx], numdivlines, divlines);

		// Now we check all the previous subsectors and try to find any
		// mergeable edgepoints. This may help us reduce cracks and holes.
		/*for(i=0; i<ssidx; i++)
			P_MergeSubsectorEdges(subsectors+ssidx, subsectors+i);*/

		//printf( "subsector %d: %d edgeverts\n", ssidx, subsectors[ssidx].numedgeverts);
		// This leaf is done.
		return;
	}

	// Get a pointer to the node.
	nod = nodes + bspnode;

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

static void DD_SkyFix()
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
			line_t *line = lines + i;
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

/*
=================
=
= P_SetupLevel
=
=================
*/

void P_SetupLevel(int episode, int map, int playermask, skill_t skill)
{
	int i;
	int parm;
	char lumpname[9];
//	char auxName[128];
	int lumpnum, gllumpnum;
	mobj_t *mobj;

	// No excessss keypressesss.
	I_ClearKeyRepeaters();
	
	for(i = 0; i < MAXPLAYERS; i++)
	{
		players[i].killcount = players[i].secretcount
			= players[i].itemcount = 0;
	}
	players[consoleplayer].viewz = 1; // will be set by player think

	Z_FreeTags(PU_LEVEL, PU_PURGELEVEL-1);
	GL_ResetData();
	ConSys_DestroyConversations();

	P_InitThinkers();
	leveltime = 0;

/*	if(DevMaps)
	{
		sprintf(auxName, "%sMAP%02d.WAD", DevMapsDir, map);
		W_OpenAuxiliary(auxName);
	}*/
	sprintf(lumpname, "MAP%02d", map);
	lumpnum = W_GetNumForName(lumpname);

	ConSys_LoadConversations(lumpname);

//	gi.LoadBlockMap(lumpnum+ML_BLOCKMAP);
	P_LoadBlockMap(lumpnum+ML_BLOCKMAP);

	// Check for GL lumps.
	sprintf(lumpname, "GL_MAP%02d", map);
	gllumpnum = W_CheckNumForName(lumpname);
	if(gllumpnum > lumpnum)
	{
		// We have GL nodes! Let's load them in.
		P_LoadVertexes(lumpnum+ML_VERTEXES, gllumpnum+1);				
		P_LoadSectors(lumpnum+ML_SECTORS);
		P_LoadSideDefs(lumpnum+ML_SIDEDEFS);
		P_LoadLineDefs(lumpnum+ML_LINEDEFS);
		P_LoadSubsectors(gllumpnum+3);
		P_LoadNodes(gllumpnum+4);
		P_LoadSegsGL(gllumpnum+2);
		// Polygonize right now; evidently must be done before PO init. 
		// I guess it mixes up the segs/subsectors somehow (?).
		DD_PolygonizeWithoutCarving();
	}
	else
	{
		//
		// Begin processing map lumps
		// Note: most of this ordering is important
		//
		P_LoadVertexes(lumpnum+ML_VERTEXES, -1);
		P_LoadSectors(lumpnum+ML_SECTORS);
		P_LoadSideDefs(lumpnum+ML_SIDEDEFS);
		P_LoadLineDefs(lumpnum+ML_LINEDEFS);
		P_LoadSubsectors(lumpnum+ML_SSECTORS);
		P_LoadNodes(lumpnum+ML_NODES);
		P_LoadSegs(lumpnum+ML_SEGS);
		DD_CreateFloorsAndCeilings(numnodes-1, 0, NULL);
	}

	//gi.LoadReject(lumpnum+ML_REJECT);
	rejectmatrix = (byte *)W_CacheLumpNum(lumpnum+ML_REJECT, PU_LEVEL);
	P_GroupLines();
	bodyqueslot = 0;
	po_NumPolyobjs = 0;
	deathmatch_p = deathmatchstarts;
	P_LoadThings(lumpnum+ML_THINGS);
	PO_Init(lumpnum+ML_THINGS); // Initialize the polyobjs
	P_LoadACScripts(lumpnum+ML_BEHAVIOR); // ACS object code

	//
	// End of map lump processing
	//
/*	if(DevMaps)
	{
		// Close the auxiliary file, but don't free its loaded lumps.
		// The next call to W_OpenAuxiliary() will do a full shutdown
		// of the current auxiliary WAD (free lumps and info lists).
		W_CloseAuxiliaryFile();
		W_UsePrimary();
	}*/

	// If deathmatch, randomly spawn the active players
	TimerGame = 0;
	if(deathmatch)
	{
		for (i=0 ; i<MAXPLAYERS ; i++)
		{
			if (players[i].ingame)
			{   // must give a player spot before deathmatchspawn
				mobj = P_SpawnMobj (playerstarts[0][i].x<<16,
					playerstarts[0][i].y<<16,0, MT_PLAYER_FIGHTER);
				players[i].mo = mobj;
				G_DeathMatchSpawnPlayer (i);
				P_RemoveMobj (mobj);
			}
		}
		parm = M_CheckParm("-timer");
		if(parm && parm < Argc()-1)
		{
			TimerGame = atoi(Argv(parm+1))*35*60;
		}
	}

// set up world state
	P_SpawnSpecials ();

// build subsector connect matrix
//      P_ConnectSubsectors ();

// Load colormap and set the fullbright flag
	i = P_GetMapFadeTable(gamemap);
	if(i == W_GetNumForName("COLORMAP"))
	{
//		LevelUseFullBright = true;
		// We don't want fog in this case.
		GL_UseWhiteFog(false);
	}
	else
	{ // Probably fog ... don't use fullbright sprites
//		LevelUseFullBright = false;
		
		if(i == W_GetNumForName("FOGMAP"))
		{
			// Tell the renderer to turn on the fog.
			GL_UseWhiteFog(true);
		}
	}

	DD_SkyFix();
	S_CalcSectorReverbs();
	// Create a big enough dlBlockLinks.
	DL_InitBlockMap();	
	map_rendered = false;

	// preload graphics
	if(precache) R_PrecacheLevel();

	// Check if the level is a lightning level
	P_InitLightning();

	S_StopAllSound();
	SN_StopAllSequences();
	S_StartSong(gamemap, true);

	// Print a message in the console about this level.
	ST_Message( "Map %d (%d): %s\n", P_GetMapWarpTrans(map), map, P_GetMapName(map));
}

//==========================================================================
//
//	DD_ValidateLevel
//
//	Make sure all texture references in the level data are good.
//
//==========================================================================

void DD_ValidateLevel(void)
{
	int		i;
	//mobj_t	*iter;

	for(i=0; i<numsectors; i++)
	{
		sector_t *sec = sectors + i;
		if(sec->ceilingpic > numflats-1) sec->ceilingpic = 0;
		if(sec->floorpic > numflats-1) sec->floorpic = 0;
		// Update mobj floorpics.
	/*	for(iter=sec->thinglist; iter; iter=iter->snext)
			iter->floorpic = sec->floorpic;*/
	}
	for(i=0; i<numsides; i++)
	{
		side_t *side = sides + i;
		if(side->toptexture > numtextures-1) side->toptexture = 0;
		if(side->midtexture > numtextures-1) side->midtexture = 0;
		if(side->bottomtexture > numtextures-1) side->bottomtexture = 0;
	}
}
//==========================================================================
//
// InitMapInfo
//
//==========================================================================

static void InitMapInfo(void)
{
	int map;
	int mapMax;
	int mcmdValue;
	mapInfo_t *info;
	char songMulch[10];

	mapMax = 1;

	// Put defaults into MapInfo[0]
	info = MapInfo;
	info->cluster = 0;
	info->warpTrans = 0;
	info->nextMap = 1; // Always go to map 1 if not specified
	info->cdTrack = 1;
	info->sky1Texture = R_TextureNumForName(DEFAULT_SKY_NAME);
	info->sky2Texture = info->sky1Texture;
	info->sky1ScrollDelta = 0;
	info->sky2ScrollDelta = 0;
	info->doubleSky = false;
	info->lightning = false;
	info->fadetable = W_GetNumForName(DEFAULT_FADE_TABLE);
	strcpy(info->name, UNKNOWN_MAP_NAME);

	for(map=0; map<99; map++) MapInfo[map].warpTrans = 0;

//	strcpy(info->songLump, DEFAULT_SONG_LUMP);
	SC_Open(MAPINFO_SCRIPT_NAME);
	while(SC_GetString())
	{
		if(SC_Compare("MAP") == false)
		{
			SC_ScriptError(NULL);
		}
		SC_MustGetNumber();
		if(sc_Number < 1 || sc_Number > 99)
		{ // 
			SC_ScriptError(NULL);
		}
		map = sc_Number;

		info = &MapInfo[map];

		// Save song lump name
		strcpy(songMulch, info->songLump);

		// Copy defaults to current map definition
		memcpy(info, &MapInfo[0], sizeof(*info));

		// Restore song lump name
		strcpy(info->songLump, songMulch);

		// The warp translation defaults to the map number
		info->warpTrans = map;

		// Map name must follow the number
		SC_MustGetString();
		strcpy(info->name, sc_String);

		// Process optional tokens
		while(SC_GetString())
		{
			if(SC_Compare("MAP"))
			{ // Start next map definition
				SC_UnGet();
				break;
			}
			mcmdValue = MapCmdIDs[SC_MustMatchString(MapCmdNames)];
			switch(mcmdValue)
			{
				case MCMD_CLUSTER:
					SC_MustGetNumber();
					info->cluster = sc_Number;
					break;
				case MCMD_WARPTRANS:
					SC_MustGetNumber();
					info->warpTrans = sc_Number;
					break;
				case MCMD_NEXT:
					SC_MustGetNumber();
					info->nextMap = sc_Number;
					break;
				case MCMD_CDTRACK:
					SC_MustGetNumber();
					info->cdTrack = sc_Number;
					break;
				case MCMD_SKY1:
					SC_MustGetString();
					info->sky1Texture = R_TextureNumForName(sc_String);
					SC_MustGetNumber();
					info->sky1ScrollDelta = sc_Number<<8;
					break;
				case MCMD_SKY2:
					SC_MustGetString();
					info->sky2Texture = R_TextureNumForName(sc_String);
					SC_MustGetNumber();
					info->sky2ScrollDelta = sc_Number<<8;
					break;
				case MCMD_DOUBLESKY:
					info->doubleSky = true;
					break;
				case MCMD_LIGHTNING:
					info->lightning = true;
					break;
				case MCMD_FADETABLE:
					SC_MustGetString();
					info->fadetable = W_GetNumForName(sc_String);
					break;
				case MCMD_CD_STARTTRACK:
				case MCMD_CD_END1TRACK:
				case MCMD_CD_END2TRACK:
				case MCMD_CD_END3TRACK:
				case MCMD_CD_INTERTRACK:
				case MCMD_CD_TITLETRACK:
					SC_MustGetNumber();
					cd_NonLevelTracks[mcmdValue-MCMD_CD_STARTTRACK] = 
						sc_Number;
					break;
			}
		}
		mapMax = map > mapMax ? map : mapMax;
	}
	SC_Close();
	MapCount = mapMax;
}

//==========================================================================
//
// P_GetMapCluster
//
//==========================================================================

int P_GetMapCluster(int map)
{
	return MapInfo[QualifyMap(map)].cluster;
}

//==========================================================================
//
// P_GetMapCDTrack
//
//==========================================================================

int P_GetMapCDTrack(int map)
{
	return MapInfo[QualifyMap(map)].cdTrack;
}

//==========================================================================
//
// P_GetMapWarpTrans
//
//==========================================================================

int P_GetMapWarpTrans(int map)
{
	return MapInfo[QualifyMap(map)].warpTrans;
}

//==========================================================================
//
// P_GetMapNextMap
//
//==========================================================================

int P_GetMapNextMap(int map)
{
	return MapInfo[QualifyMap(map)].nextMap;
}

//==========================================================================
//
// P_TranslateMap
//
// Returns the actual map number given a warp map number.
//
//==========================================================================

int P_TranslateMap(int map)
{
	int i;

/*	ST_Message("P_TranslateMap(%d):\n", map);
	for(i = 1; i < 99; i++) // Make this a macro
		ST_Message("- %d: warp to %d\n", i, MapInfo[i].warpTrans);*/

	for(i = 1; i < 99; i++) // Make this a macro
	{
		if(MapInfo[i].warpTrans == map)
		{
			return i;
		}
	}
	// Not found
	return -1;
}

//==========================================================================
//
// P_GetMapSky1Texture
//
//==========================================================================

int P_GetMapSky1Texture(int map)
{
	return MapInfo[QualifyMap(map)].sky1Texture;
}

//==========================================================================
//
// P_GetMapSky2Texture
//
//==========================================================================

int P_GetMapSky2Texture(int map)
{
	return MapInfo[QualifyMap(map)].sky2Texture;
}

//==========================================================================
//
// P_GetMapName
//
//==========================================================================

char *P_GetMapName(int map)
{
	return MapInfo[QualifyMap(map)].name;
}

//==========================================================================
//
// P_GetMapSky1ScrollDelta
//
//==========================================================================

fixed_t P_GetMapSky1ScrollDelta(int map)
{
	return MapInfo[QualifyMap(map)].sky1ScrollDelta;
}

//==========================================================================
//
// P_GetMapSky2ScrollDelta
//
//==========================================================================

fixed_t P_GetMapSky2ScrollDelta(int map)
{
	return MapInfo[QualifyMap(map)].sky2ScrollDelta;
}

//==========================================================================
//
// P_GetMapDoubleSky
//
//==========================================================================

boolean P_GetMapDoubleSky(int map)
{
	return MapInfo[QualifyMap(map)].doubleSky;
}

//==========================================================================
//
// P_GetMapLightning
//
//==========================================================================

boolean P_GetMapLightning(int map)
{
	return MapInfo[QualifyMap(map)].lightning;
}

//==========================================================================
//
// P_GetMapFadeTable
//
//==========================================================================

boolean P_GetMapFadeTable(int map)
{
	return MapInfo[QualifyMap(map)].fadetable;
}

//==========================================================================
//
// P_GetMapSongLump
//
//==========================================================================

char *P_GetMapSongLump(int map)
{
	if(!strcasecmp(MapInfo[QualifyMap(map)].songLump, DEFAULT_SONG_LUMP))
	{
		return NULL;
	}
	else
	{
		return MapInfo[QualifyMap(map)].songLump;
	}
}

//==========================================================================
//
// P_PutMapSongLump
//
//==========================================================================

void P_PutMapSongLump(int map, char *lumpName)
{
	if(map < 1 || map > MapCount)
	{
		return;
	}
	strcpy(MapInfo[map].songLump, lumpName);
}

//==========================================================================
//
// P_GetCDStartTrack
//
//==========================================================================

int P_GetCDStartTrack(void)
{
	return cd_NonLevelTracks[MCMD_CD_STARTTRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd1Track
//
//==========================================================================

int P_GetCDEnd1Track(void)
{
	return cd_NonLevelTracks[MCMD_CD_END1TRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd2Track
//
//==========================================================================

int P_GetCDEnd2Track(void)
{
	return cd_NonLevelTracks[MCMD_CD_END2TRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDEnd3Track
//
//==========================================================================

int P_GetCDEnd3Track(void)
{
	return cd_NonLevelTracks[MCMD_CD_END3TRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDIntermissionTrack
//
//==========================================================================

int P_GetCDIntermissionTrack(void)
{
	return cd_NonLevelTracks[MCMD_CD_INTERTRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// P_GetCDTitleTrack
//
//==========================================================================

int P_GetCDTitleTrack(void)
{
	return cd_NonLevelTracks[MCMD_CD_TITLETRACK-MCMD_CD_STARTTRACK];
}

//==========================================================================
//
// QualifyMap
//
//==========================================================================

static int QualifyMap(int map)
{
	return (map < 1 || map > MapCount) ? 0 : map;
}

//==========================================================================
//
// P_Init
//
//==========================================================================

void P_Init(void)
{
	bamsInit();
	InitMapInfo();
	P_InitSwitchList();
	P_InitFTAnims(); // Init flat and texture animations
	P_InitTerrainTypes();
	P_InitLava();
//	gi.R_InitSprites(sprnames);
}


// Special early initializer needed to start sound before R_Init()
void InitMapMusicInfo(void)
{
	int i;

	for (i=0; i<99; i++)
	{
		strcpy(MapInfo[i].songLump, DEFAULT_SONG_LUMP);
	}
	MapCount = 98;
}

/*
void My_Debug(void)
{
	int i;

	printf("My debug stuff ----------------------\n");
	printf("gamemap=%d\n",gamemap);
	for (i=0; i<10; i++)
	{
		printf("i=%d  songlump=%s\n",i,MapInfo[i].songLump);
	}
}
*/

// Is this really necessary?
#pragma optimize("", on)