// Data structures of the Doomsday. 
// The game DLLs' data must be based on these.

#ifndef __DOOMSDAY_DATA_H__
#define __DOOMSDAY_DATA_H__

#include "dd_share.h"
#include "dd_think.h"

enum
{ // bbox coordinates
	BOXTOP,
	BOXBOTTOM,
	BOXLEFT,
	BOXRIGHT
};

typedef int spritenum_t;


typedef struct mobj_s
{
	thinker_t		thinker;			// thinker node

	fixed_t			x,y,z;				// position
	struct mobj_s	*snext, *sprev;		// links in sector (if needed)
	angle_t			angle;
	spritenum_t		sprite;				// used to find patch_t and flip value
	int				frame;				// might be ord with FF_FULLBRIGHT
	fixed_t			radius;
	int				ddflags;			// Doomsday mobj flags (DDMF_*)
	struct subsector_s *subsector;
	fixed_t			floorclip;			// value to use for floor clipping
} 
mobj_t;

#define MAXWIDTH				1120
#define MAXHEIGHT				832

#define PI						3.141592657

#define CENTERY					(SCREENHEIGHT/2)

#define MINZ                    (FRACUNIT*4)

#define FIELDOFVIEW             2048    // fineangles in the SCREENWIDTH wide window

//
// lighting constants
//
#define LIGHTLEVELS				16
#define LIGHTSEGSHIFT           4
#define MAXLIGHTSCALE           48

/*
==============================================================================

					INTERNAL MAP TYPES

==============================================================================
*/

//================ used by play and refresh

typedef struct
{
	fixed_t         x,y;
} vertex_t;

typedef struct
{
	float x,y;
} fvertex_t;

struct line_s;

typedef struct sector_s
{
	fixed_t	floorheight, ceilingheight;
	short	floorpic, ceilingpic;
	short   lightlevel;
	int     validcount;             // if == validcount, already checked
	mobj_t  *thinglist;             // list of mobjs in sector
	int     linecount;
	struct line_s **lines;        // [linecount] size
	int		flatoffx, flatoffy;		// floor texture offset
	int		skyfix;					// Offset to ceiling height rendering w/sky.
	float	reverb[NUM_REVERB_DATA];
} sector_t;

typedef struct
{
	fixed_t         textureoffset;          // add this to the calculated texture col
	fixed_t         rowoffset;                      // add this to the calculated texture top
	short           toptexture, bottomtexture, midtexture;
	sector_t        *sector;
} side_t;


#define	ML_TWOSIDED		    0x0004
#define	ML_DONTPEGTOP		0x0008
#define	ML_DONTPEGBOTTOM	0x0010
#define	ML_MAPPED			0x0100	// set if already drawn in automap

typedef struct line_s
{
	vertex_t *v1;
	vertex_t *v2;
	short flags;
	sector_t *frontsector;
	sector_t *backsector;
/*	fixed_t	dx;
	fixed_t dy;
	slopetype_t slopetype;
	int	validcount;			*/
} line_t;

typedef struct
{
	vertex_t        *v1, *v2;
	float			length;			// Accurate length of the segment (v1 -> v2).
	fixed_t         offset;
	side_t          *sidedef;
	line_t          *linedef;
	sector_t        *frontsector;
	sector_t        *backsector;    // NULL for one sided lines
	byte			flags;
} seg_t;

// ===== Polyobj data =====
typedef struct
{
	int numsegs;
	seg_t **segs;
} polyobj_t;

#define MAX_POLY_SIDES	64		// A subsector has at most this many edges.

typedef struct subsector_s
{
	sector_t        *sector;
	short           numlines;
	short           firstline;
	polyobj_t		*poly;			// NULL if there is no polyobj
	// Sorted edge vertices for rendering floors and ceilings.
	char			numedgeverts;
	fvertex_t		*edgeverts;		// A list of edge vertices.
	fvertex_t		*origedgeverts;	// Unmodified, accurate edge vertices.
	fvertex_t		*diffverts;		// Unit modifiers.
	fvertex_t		bbox[2];		// Min and max points.
	fvertex_t		midpoint;		// Center of bounding box.
//	byte			flags;
} subsector_t;

#define	NF_SUBSECTOR	0x8000

typedef struct
{
	fixed_t         x,y,dx,dy;                      // partition line
	fixed_t         bbox[2][4];                     // bounding box for each child
	unsigned short  children[2];            // if NF_SUBSECTOR its a subsector
} node_t;

// If you modify this, remember to change ddplayer_t, too.
typedef struct player_s
{
	mobj_t		*mo;
	fixed_t		viewz;					// focal origin above r.z
	float		lookdir;				// It's now a float, for mlook. -jk
	int			fixedcolormap;			// can be set to REDCOLORMAP, etc
	int			extralight;				// so gun flashes light up areas
	signed int	frags[MAXPLAYERS];		// kills of other players
	int			ingame;					// is this player in game?
	void		*extradata;				// Pointer to any game-specific data.
} player_t;


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

typedef struct
{
	fixed_t x, y, dx, dy;
} divline_t;

typedef struct
{
	float x,y,dx,dy;
} fdivline_t;


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



//=============================================================================

extern int numvertexes;
extern byte *vertexes;

extern int numsegs;
extern byte *segs;

extern int numsectors;
extern byte *sectors;

extern int numsubsectors;
extern byte *subsectors;

extern int numnodes;
extern byte *nodes;

extern int numlines;
extern byte *lines;

extern int numsides;
extern byte *sides;

/*extern short *blockmaplump;			// offsets in blockmap are from here
extern short *blockmap;
extern int bmapwidth, bmapheight;	// in mapblocks
extern fixed_t bmaporgx, bmaporgy;	// origin of block map
extern mobj_t **blocklinks;			// for thing chains

extern byte *rejectmatrix;			// for fast sight rejection
*/

extern  fixed_t         viewx, viewy, viewz;
extern  fixed_t			viewxOffset, viewyOffset, viewzOffset;
extern  angle_t         viewangle;
extern	float			viewpitch;
extern  player_t        *viewplayer;


extern  angle_t         clipangle;

extern  fixed_t         finetangent[FINEANGLES/2];

extern  fixed_t         rw_distance;
extern  angle_t         rw_normalangle;

//
// R_main.c
//
extern  int                             viewwidth, viewheight, viewwindowx, viewwindowy;
extern  int                             centerx, centery;

extern  int                             validcount;

extern  int                             sscount, linecount, loopcount;

extern  int                             extralight;

extern  fixed_t                 viewcos, viewsin;

extern  int                             detailshift;            // 0 = high, 1 = low

int			R_PointOnSide (fixed_t x, fixed_t y, node_t *node);
int         R_PointOnSegSide (fixed_t x, fixed_t y, seg_t *line);
angle_t		R_PointToAngle (fixed_t x, fixed_t y);
angle_t		R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
fixed_t		R_PointToDist (fixed_t x, fixed_t y);
fixed_t		R_ScaleFromGlobalAngle (angle_t visangle);
subsector_t *R_PointInSubsector (fixed_t x, fixed_t y);


//
// R_data.c
//
typedef struct
{
	int		originx;	// block origin (allways UL), which has allready
	int		originy;	// accounted  for the patch's internal origin
	int		patch;
} texpatch_t;


#define TXF_MASKED		0x1
#define TXF_GLOW		0x2		// For lava etc, textures that glow.

// a maptexturedef_t describes a rectangular texture, which is composed of one
// or more mappatch_t structures that arrange graphic patches
typedef struct
{
	char		name[8];		// for switch changing, etc
	short		width;
	short		height;
	int			flags;			// TXF_* flags.
	short		patchcount;
	texpatch_t	patches[1];		// [patchcount] drawn back to front
								//  into the cached texture
	// Extra stuff. -jk
//	boolean		masked;			// from maptexture_t
} texture_t;

extern	int				*spriteheight;
extern  fixed_t         *textureheight;         // needed for texture pegging
extern  fixed_t         *spritewidth;           // needed for pre rendering (fracs)
extern  fixed_t         *spriteoffset;
extern  fixed_t         *spritetopoffset;
//extern  lighttable_t    *colormaps;
extern  int             viewwidth, /*scaledviewwidth,*/ viewheight;
extern  int             firstflat;
extern  int             numflats;

extern	int				numtextures;			// I need this. -jk
extern	texture_t		**textures;

//extern	int				*flatflags; //crap

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



#endif