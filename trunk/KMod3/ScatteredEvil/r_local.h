
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

#define MAXWIDTH				1120
#define MAXHEIGHT				832

#define MINZ                    (FRACUNIT*4)

#define FIELDOFVIEW             2048    // fineangles in the SCREENWIDTH wide window

//
// lighting constants
//
#define LIGHTLEVELS				16
#define LIGHTSEGSHIFT           4
#define MAXLIGHTSCALE           48

#define MAX_POLY_SIDES	64		// A subsector has at most this many edges.

/*
==============================================================================

					INTERNAL MAP TYPES

==============================================================================
*/

//================ used by play and refresh

struct vertex_t
{
	fixed_t         x,y;
};

struct line_t;

struct sector_t
{
	fixed_t	floorheight, ceilingheight;
	short	floorpic, ceilingpic;
	short   lightlevel;
	int     validcount;             // if == validcount, already checked
	mobj_t  *thinglist;             // list of mobjs in sector
	int     linecount;
	line_t	**lines;        // [linecount] size
	int		flatoffx, flatoffy;		// Scrolling flats.
	int		skyfix;					// Offset to ceiling height rendering w/sky.
	float	reverb[NUM_REVERB_DATA];
	short  	special, tag;
	int     soundtraversed;         // 0 = untraversed, 1,2 = sndlines -1
	mobj_t  *soundtarget;           // thing that made a sound (or null)
	seqtype_t seqType;				// stone, metal, heavy, etc...
	int     blockbox[4];            // mapblock bounding box for height changes
	degenmobj_t soundorg;           // for any sounds played by the sector
	void    *specialdata;           // thinker_t for reversable actions
};

struct side_t
{
	fixed_t         textureoffset;          // add this to the calculated texture col
	fixed_t         rowoffset;                      // add this to the calculated texture top
	short           toptexture, bottomtexture, midtexture;
	sector_t        *sector;
};

enum slopetype_t
{
	ST_HORIZONTAL,
	ST_VERTICAL,
	ST_POSITIVE,
	ST_NEGATIVE
};

struct line_t
{
	vertex_t *v1;
	vertex_t *v2;
	short flags;
	sector_t *frontsector;
	sector_t *backsector;
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
};

struct seg_t
{
	vertex_t        *v1, *v2;
	float			length;			// Length of the segment (v1 -> v2).
	fixed_t         offset;
	side_t          *sidedef;
	line_t          *linedef;
	sector_t        *frontsector;
	sector_t        *backsector;            // NULL for one sided lines
	int				flags;
	angle_t         angle;
};

// ===== Polyobj data =====
struct polyobj_t
{
	int numsegs;
	seg_t **segs;
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
};

struct polyblock_t
{
	polyobj_t *polyobj;
	polyblock_t *prev;
	polyblock_t *next;
};

struct fvertex_t
{
	float x,y;
};

struct subsector_t
{
	sector_t        *sector;
	short           numlines;
	short           firstline;
	polyobj_t		*poly;
	// Sorted edge vertices for rendering floors and ceilings.
	char			numedgeverts;
	fvertex_t		*edgeverts;		// A list of edge vertices.
	fvertex_t		*origedgeverts;	// Unmodified, accurate edge vertices.
	fvertex_t		*diffverts;		// Unit modifiers.
	fvertex_t		bbox[2];		// Min and max points.
	fvertex_t		midpoint;		// Center of bounding box.
};

struct node_t
{
	fixed_t         x,y,dx,dy;                      // partition line
	fixed_t         bbox[2][4];                     // bounding box for each child
	unsigned short  children[2];            // if NF_SUBSECTOR its a subsector
};

/*
==============================================================================

						OTHER TYPES

==============================================================================
*/

#define MAX_FRAME_MODELS 4

// Model frame flags.
#define MFF_FULLBRIGHT		0x01
#define MFF_SHADOW1			0x02
#define MFF_SHADOW2			0x04
#define MFF_BRIGHTSHADOW	0x08
#define MFF_MOVEMENT_PITCH	0x10		// Pitch aligned to movement.
#define MFF_SPIN			0x20		// Spin around (for bonus items).
#define MFF_AUTOSCALE		0x80		// Scale to match sprite height.

typedef struct 
{
	short	model;
	short	frame;
	short	skin;
	byte	flags;
} submodelframe_t;

typedef struct
{
	submodelframe_t sub[MAX_FRAME_MODELS];
	float autoscale;
	float offset;
	float size, scale;
} modelframe_t;


typedef byte    lighttable_t;           // this could be wider for >8 bit display

#define SIL_NONE        0
#define SIL_BOTTOM      1
#define SIL_TOP         2
#define SIL_BOTH        3

// A vissprite_t is a thing that will be drawn during a refresh
typedef struct vissprite_s
{
	struct vissprite_s *prev, *next;
	int             x1, x2;
	fixed_t         gx, gy;                 // for line side calculation
	fixed_t         gz, gzt;                // global bottom / top for silhouette clipping
	fixed_t         startfrac;              // horizontal position of x1
	fixed_t         scale;
	fixed_t         xiscale;                // negative if flipped
	fixed_t         texturemid;
	int             patch;
	int				lightlevel;
	float			v1[2], v2[2];		// The vertices (v1 is the left one).
	float			secfloor, secceil;
	int             mobjflags;        // for color translation and shadow draw
	boolean         psprite;                // true if psprite
	int				pclass;			// player class (used in translation)
	fixed_t         floorclip;               
	
	boolean			viewAligned;		// Align to view plane.
	modelframe_t	*mf, *nextmf;
	float			yaw, pitch;			// For models.
	float            inter;
} vissprite_t;

// Sprites are patches with a special naming convention so they can be
// recognized by R_InitSprites.  The sprite and frame specified by a
// thing_t is range checked at run time.
// a sprite is a patch_t that is assumed to represent a three dimensional
// object and may have multiple rotations pre drawn.  Horizontal flipping
// is used to save space. Some sprites will only have one picture used
// for all views.

typedef struct
{
	boolean         rotate;         // if false use 0 for any position
	short           lump[8];        // lump to use for view angles 0-7
	byte            flip[8];        // flip (1 = flip) to use for view angles 0-7
} spriteframe_t;

typedef struct
{
	int             numframes;
	spriteframe_t   *spriteframes;
        short           modeldef;
//	modelframe_t	*modelframes;	// Might be null.
} spritedef_t;


extern  spritedef_t		*sprites;
extern  int             numsprites;


// Modeldefs define model objects. A model object consists of a number
// of model frames. Each model frame has individual flags and possibly 
// an MD2 model of its own. Model objects can be used instead of spritedefs
// when rendering.

#define MAX_MODELDEF_NAME	64

typedef struct
{
	char			name[MAX_MODELDEF_NAME];
	int				numframes;
	modelframe_t	*modelframes;
} modeldef_t;

extern	modeldef_t		*models;
extern	int				nummodels;



// ==============================================================================

extern int numvertexes;
extern vertex_t *vertexes;

extern int numsegs;
extern seg_t *segs;

extern int numsectors;
extern sector_t *sectors;

extern int numsubsectors;
extern subsector_t *subsectors;

extern int numnodes;
extern node_t *nodes;

extern int numlines;
extern line_t *lines;

extern int numsides;
extern side_t *sides;

extern short *blockmaplump;			// offsets in blockmap are from here
extern short *blockmap;
extern int bmapwidth, bmapheight;	// in mapblocks
extern fixed_t bmaporgx, bmaporgy;	// origin of block map
extern mobj_t **blocklinks;			// for thing chains

extern byte *rejectmatrix;			// for fast sight rejection

extern  fixed_t         viewx, viewy, viewz;
extern  fixed_t			viewxOffset, viewyOffset, viewzOffset;
extern  angle_t         viewangle;
extern	float			viewpitch;
extern  player_t		*viewplayer;


extern  angle_t         clipangle;

extern  fixed_t         finetangent[FINEANGLES/2];

extern  fixed_t         rw_distance;
extern  angle_t         rw_normalangle;

//
// R_main.c
//
extern  int				viewwidth, viewheight, viewwindowx, viewwindowy;

extern  int				validcount;

extern  int				sscount, linecount, loopcount;

extern  int				extralight;

extern  fixed_t			viewcos, viewsin;

extern  int				detailshift;            // 0 = high, 1 = low

int			R_PointOnSide (fixed_t x, fixed_t y, node_t *node);
int         R_PointOnSegSide (fixed_t x, fixed_t y, seg_t *line);
angle_t		R_PointToAngle (fixed_t x, fixed_t y);
angle_t		R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
fixed_t		R_PointToDist (fixed_t x, fixed_t y);
subsector_t *R_PointInSubsector (fixed_t x, fixed_t y);


//
// R_data.c
//
struct texpatch_t
{
	int		originx;	// block origin (allways UL), which has allready
	int		originy;	// accounted  for the patch's internal origin
	int		patch;
};


#define TXF_MASKED		0x1
#define TXF_GLOW		0x2		// For lava etc, textures that glow.

// a maptexturedef_t describes a rectangular texture, which is composed of one
// or more mappatch_t structures that arrange graphic patches
struct texture_t
{
	char		name[8];		// for switch changing, etc
	short		width;
	short		height;
	int			flags;			// TXF_* flags.
	short		patchcount;
	texpatch_t	patches[1];		// [patchcount] drawn back to front
								//  into the cached texture
};

extern	int				*spriteheight;
extern  fixed_t         *textureheight;         // needed for texture pegging
extern  fixed_t         *spritewidth;           // needed for pre rendering (fracs)
extern  fixed_t         *spriteoffset;
extern  fixed_t         *spritetopoffset;
extern  int             viewwidth, viewheight;
extern  int             firstflat;
extern  int             numflats;

extern	int				numtextures;			// I need this. -jk
extern	texture_t		**textures;

extern  int             *flattranslation;               // for global animation
extern  int             *texturetranslation;    // for global animation

extern  int             firstspritelump, lastspritelump, numspritelumps;
extern	int				LevelFullBright;

extern	int				r_texglow;

byte    *R_GetColumn (int tex, int col);
void    R_InitData (void);
void	R_UpdateData (void);
void	R_PrecacheLevel (void);
int		R_TextureFlags(int texture);
//int		R_FlatFlags(int flat); //crap


//
// R_things.c
//
#define MAXVISSPRITES   1024 //192

extern  vissprite_t     vissprites[MAXVISSPRITES], *vissprite_p;
extern  vissprite_t     vsprsortedhead;


void    R_SortVisSprites (void);

void    R_AddSprites (sector_t *sec);
void    R_AddPSprites (void);
void    R_DrawSprites (void);
void    R_InitSprites (void);
void    R_ClearSprites (void);
void    R_DrawMasked (void);
void    R_ClipVisSprite (vissprite_t *vis, int xl, int xh);

//=============================================================================
//
// R_draw.c
//
//=============================================================================

extern  byte    *translationtables;
extern  byte    *dc_translation;

void    R_InitBuffer (int width, int height);
void    R_InitTranslationTables (void);
void	R_UpdateTranslationTables (void);

// OpenGL stuff.
void	R_RenderMap (void);

modelframe_t *R_CheckModelFor(int sprite, int frame);


extern  int		skyflatnum;
extern int		viewangleoffset;

#endif          // __R_LOCAL__

