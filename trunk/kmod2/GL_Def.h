#ifndef __DOOMSDAY_GRAPHICS__
#define __DOOMSDAY_GRAPHICS__

// Hope you don't mind the mess...

#include "dd_data.h"
#include "dd_md2.h"

enum {VX, VY, VZ};	// Vertex indices.
enum { CR, CG, CB, CA };	// Color indices.

// Lumobj Flags.
#define LUMF_USED		0x1
#define LUMF_RENDERED	0x2

typedef struct lumobj_s					// For dynamic lighting.
{
	struct lumobj_s *next;				// Next in the same DL block, or NULL.
	int		flags;
	mobj_t	*thing;
	float	center; 					// Offset to center from mobj Z.
	int		radius, patch, distance;	// Radius: lights are spheres.
	int		flaresize;					// Radius for this light source.
	byte	rgb[3];						// The color.
} lumobj_t;

// ScreenBits is currently unused.
extern int screenWidth, screenHeight, screenBits;
extern int viewph, viewpw, viewpx, viewpy;

void I_InitGraphics(void);
void I_ShutdownGraphics(void);

int I_ChangeResolution(int w, int h);

void GL_InitRenderer(void);
void GL_InitData(void);
void GL_ResetData(void);

// Returns a pointer to a copy of the screen. The pointer must be 
// deallocated by the caller.
unsigned char *GL_GrabScreen(void);

void GL_SwitchTo3DState();
void GL_Restore2DState(int step);	// Step 1: matrices, 2: attributes.
void GL_UseWhiteFog(int yes);

float PointDist2D(float c[2]);
float SignedPointDist2D(float c[2]);

void R_RenderSprite(vissprite_t *spr);

void GL_UsePatchOffset(boolean enable);

// 2D drawing routines.
void GL_DrawPatch_CS(int x, int y, int lumpnum);
void GL_DrawPatch(int x, int y, int lumpnum);
void GL_DrawPatchLitAlpha(int x, int y, float light, float alpha, int lumpnum);
void GL_DrawFuzzPatch(int x, int y, int lumpnum);
void GL_DrawAltFuzzPatch(int x, int y, int lumpnum);
void GL_DrawShadowedPatch(int x, int y, int lumpnum);
void GL_DrawRawScreen(int lump);	// Raw screens are 320 x 200.
void GL_DrawLine(float x1, float y1, float x2, float y2, 
				  float r, float g, float b, float a);
void GL_DrawRect(float x, float y, float w, float h, float r, float g, float b, float a);
void GL_DrawRectTiled(int x, int y, int w, int h, int tw, int th);
void GL_DrawCutRectTiled(int x, int y, int w, int h, int tw, int th, 
						  int cx, int cy, int cw, int ch);
void GL_SetColor(int palidx);
void GL_SetColorAndAlpha(float r, float g, float b, float a);
void GL_DrawPSprite(int x, int y, float scale, int flip, int lump);

// Filters.
void GL_SetFilter(int filterRGBA);
int GL_DrawFilter();


// ogl_tex.c
typedef struct
{
	unsigned short	w, h;
	short			offx, offy;
	unsigned short	w2;		// For split textures, width of the other part.
} texsize_t;

extern texsize_t *lumptexsizes;	// Sizes for all the lumps. 
//extern unsigned short *spriteheights;

extern float		texw, texh;
extern int			texmask;	
extern unsigned int	curtex;
extern int			pallump;

int GL_InitPalettedTexture();
void GL_TexInit();
void GL_TexReset(void);
void GL_ClearTextureMemory(void);
void GL_TexDestroy();
void GL_ResetLumpTexData();
void GL_UpdateGamma(void);

void PalToRGB(byte *palidx, byte *rgb);
void PalIdxToRGB(byte *pal, int idx, byte *rgb);
unsigned int GL_BindTexFlat(int lump);
void GL_SetFlat(int idx);
void GL_BindTexture(DGLuint texname);
void GL_TextureFilterMode(int target, int parm);

void GL_LowRes();

void TranslatePatch(patch_t *patch, byte *transTable);

// Returns the DGL texture name.
DGLuint GL_PrepareTexture(int idx);
DGLuint GL_PrepareFlat(int idx);// Returns the OpenGL name of the texture.
DGLuint GL_PrepareLightTexture();	// The dynamic light map.
DGLuint GL_PrepareFlareTexture(int flare);

void GL_SetTexture(int idx);
unsigned int GL_PrepareSky(int idx, boolean zeroMask);
void GL_GetSkyTopColor(int texidx, byte *rgb);

void GL_SetSprite(int pnum);
unsigned int GL_PrepareSprite(int pnum);
void GL_SetTranslatedSprite(int pnum, unsigned char *trans);
void GL_GetSpriteColor(int pnum, unsigned char *rgb);

void GL_NewSplitTex(int lump, DGLuint part2name);
DGLuint GL_GetOtherPart(int lump);

// Part is either 1 or 2. Part 0 means only the left side is loaded.
// No splittex is created in that case. Once a raw image is loaded
// as part 0 it must be deleted before the other part is loaded at the
// next loading.
void GL_SetRawImage(int lump, int part);
void GL_SetPatch(int lump);	// No mipmaps are generated.
void GL_SetNoTexture();

int GL_GetLumpTexWidth(int lump);
int GL_GetLumpTexHeight(int lump);
int GL_ValidTexHeight2(int width, int height);

void GL_UpdateTexParams(int mipmode);
void GL_UpdateRawScreenParams(int smoothing);

// returns the real DGL texture, if such exists
unsigned int GL_GetTextureName(int texidx); 

// Only for textures (not for flats, sprites, etc.)
void GL_DeleteTexture(int texidx); 

// Load the skin texture and prepare it for rendering.
unsigned int GL_PrepareSkin(model_t *mdl, int skin);


#include "m_bams.h"

// ogl_clip.c
typedef struct clipnode_s
{
	int					used;			// 1 if the node is in use.
	struct clipnode_s	*prev, *next;	// Previous and next nodes.
	binangle_t			start, end;		// The start and end angles (start < end).
} clipnode_t;

extern clipnode_t *clipnodes;	// The list of clipnodes.
extern clipnode_t *cliphead;	// The head node.

void C_Init();
void C_ClearRanges();
void C_Ranger();
void C_SafeAddRange(binangle_t startAngle, binangle_t endAngle);

// Add a segment relative to the current viewpoint.
void C_AddViewRelSeg(float x1, float y1, float x2, float y2);

// Check a segment relative to the current viewpoint.
int C_CheckViewRelSeg(float x1, float y1, float x2, float y2);

// Returns 1 if the specified angle is visible.
int C_IsAngleVisible(binangle_t bang);

clipnode_t *C_AngleClippedBy(binangle_t bang);

// Returns 1 if the subsector might be visible.
int C_CheckSubsector(subsector_t *ssec);


// ogl_sky.c

#include "gl_sky.h"

// ogl_halo.c

void H_Clear();

// Prepares the list of halos to render. Goes through the luminousList.
void H_InitForNewFrame();

// This must be called when the renderer is in player sprite rendering mode:
// 2D projection to the game view. 
void H_Render();

// gl_md2.c

void R_RenderModel(vissprite_t *spr, int number);


// r_model.c

void R_LoadSkinRGB(model_t *mdl, int skin, byte *outBuffer);


#endif