
//**************************************************************************
//**
//** OGL_RL.C
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdlib.h>
#include "dd_def.h"
#include "i_win32.h"
#include "gl_def.h"
#include "gl_rl.h"

// MACROS ------------------------------------------------------------------

#define MAX_RLISTS			256
#define MAX_POLY_SIZE		735		// with 64 vertices!
#define REALLOC_ADDITION	(MAX_POLY_SIZE * 10) // bytes

#define INLINE __inline

#define GREATER(x,y) ((x)>(y) ? (x) : (y))

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

INLINE void RL_WriteByte(rendlist_t *li, byte b);
INLINE void RL_WriteShort(rendlist_t *li, short s);
INLINE void RL_WriteLong(rendlist_t *li, int i);
INLINE void RL_WriteFloat(rendlist_t *li, float f);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int			skyhemispheres;
extern int			useDynLights, dlBlend, simpleSky;
extern boolean		whitefog;
extern float		maxLightDist;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int act_int=0;
int					numrlists=0;			// Number of rendering lists.
boolean				renderTextures = true;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static rendlist_t rlists[MAX_RLISTS]; // The list of rendering lists.
static rendlist_t masked_rlist;	// Rendering list for masked textures.
static rendlist_t invsky_rlist;	// List for invisible sky triangles.
static rendlist_t invskywall_rlist; // List for invisible sky walls (w/skyfix).
static rendlist_t dlwall_rlist, dlflat_rlist; // Dynamic lighting for walls/flats.

// CODE --------------------------------------------------------------------

void RL_Init()
{
	numrlists = 0;
	memset(rlists, 0, sizeof(rlists));
	memset(&masked_rlist, 0, sizeof(masked_rlist));
	memset(&invsky_rlist, 0, sizeof(invsky_rlist));
	memset(&invskywall_rlist, 0, sizeof(invskywall_rlist));
	memset(&dlwall_rlist, 0, sizeof(dlwall_rlist));
	memset(&dlflat_rlist, 0, sizeof(dlflat_rlist));
}

void RL_DestroyList(rendlist_t *rl)
{
	// All the list data will be destroyed.
	if(rl->data) Z_Free(rl->data);
	memset(rl, 0, sizeof(rendlist_t));
}

void RL_DeleteLists()
{
	int		i;

	// Delete all lists.
	for(i=0; i<numrlists; i++) RL_DestroyList(&rlists[i]);
	RL_DestroyList(&masked_rlist);
	RL_DestroyList(&invsky_rlist);
	RL_DestroyList(&invskywall_rlist);
	RL_DestroyList(&dlwall_rlist);
	RL_DestroyList(&dlflat_rlist);

	memset(rlists, 0, sizeof(rlists));
	numrlists = 0;
}

void RL_RewindList(rendlist_t *rl)
{
	// Set the R/W cursor to the beginning.
	rl->cursor = rl->data;
	memset(rl->data, 0, rl->size);
}

// Reset the indices.
void RL_ClearLists()
{
	int		i;

	for(i=0; i<numrlists; i++) RL_RewindList(&rlists[i]);
	RL_RewindList(&masked_rlist);
	RL_RewindList(&invsky_rlist);
	RL_RewindList(&invskywall_rlist);
	RL_RewindList(&dlwall_rlist);
	RL_RewindList(&dlflat_rlist);

	skyhemispheres = 0;
}

// This is only called for floors/ceilings w/dynlights in use.
static void RL_DynLightPoly(rendpoly_t *poly, lumobj_t *lum)
{
	poly->texw = lum->radius*2;
	poly->texh = lum->radius*2;

	// The texture offset is what actually determines where the
	// dynamic light map will be rendered. For light polys the
	// texture offset is global.
	poly->texoffx = FIX2FLT(lum->thing->x) + lum->radius;
	poly->texoffy = FIX2FLT(lum->thing->y) + lum->radius;
}

static rendlist_t *RL_FindList(DGLuint tex)
{
	int	i;
	rendlist_t *dest;

	for(i=0; i<numrlists; i++)
		if(rlists[i].tex == tex)
			return rlists + i;

	// If all the lists are taken, use the last one.
	if(numrlists == MAX_RLISTS) 
		return &rlists[MAX_RLISTS-1];

	// Take the next available list.
	dest = rlists + numrlists++;
	memset(dest, 0, sizeof(rendlist_t));
	dest->tex = tex;
	return dest;
}

rendlist_t *RL_GetListFor(rendpoly_t *poly)
{
	if(poly->flags & RPF_MASKED) 
	{
		// Masked quads go to the masked list.
		return &masked_rlist;
	}
	if(poly->flags & RPF_SKY_MASK)
	{
		return poly->type==rp_quad? &invskywall_rlist : &invsky_rlist;
	}
	if(poly->flags & RPF_LIGHT) // Dynamic lights?
	{
		return poly->type==rp_quad? &dlwall_rlist : &dlflat_rlist;
	}
	// Find a suitable list. 
	return RL_FindList(poly->tex);
}

void RL_WriteByte(rendlist_t *li, byte b)
{
	*li->cursor++ = b;
}

void RL_WriteShort(rendlist_t *li, short s)
{
	*(short*) li->cursor = s;
	li->cursor += 2;
}

void RL_WriteLong(rendlist_t *li, int i)
{
	*(int*) li->cursor = i;
	li->cursor += 4;
}

void RL_WriteFloat(rendlist_t *li, float f)
{
	*(float*) li->cursor = f;
	li->cursor += 4;
}

// Adds the given poly onto the correct list.
void RL_AddPoly(rendpoly_t *poly)
{
	rendlist_t *li = RL_GetListFor(poly);
	int i, num;
	rendpoly_vertex_t *vtx;

	// First check that the data buffer of the list is large enough.
	if(li->cursor - li->data > li->size - MAX_POLY_SIZE)
	{
		// Allocate more memory for the data buffer.
		int offset = li->cursor - li->data;
		int newsize = li->size + REALLOC_ADDITION;
		void *newdata = Z_Malloc(newsize, PU_STATIC, 0);
		// Copy the old data.
		if(li->data)
		{
			memcpy(newdata, li->data, li->cursor - li->data);
			// Free the old data.
			Z_Free(li->data);
		}
		li->data = (byte *)newdata;
		li->size = newsize;
		li->cursor = li->data + offset;
	}
	RL_WriteByte(li, poly->type);
	RL_WriteByte(li, poly->flags);
	RL_WriteShort(li, poly->texw);
	RL_WriteShort(li, poly->texh);
	RL_WriteFloat(li, poly->texoffx);
	RL_WriteFloat(li, poly->texoffy);
	if(poly->flags & RPF_MASKED) RL_WriteLong(li, poly->tex);
	RL_WriteFloat(li, poly->top);
	if(poly->type == rp_quad) 
	{
		RL_WriteFloat(li, poly->bottom);
		RL_WriteFloat(li, poly->length);
		num = 2;
	}
	else
	{
		RL_WriteByte(li, poly->numvertices);
		num = poly->numvertices;
	}
	for(i=0, vtx=poly->vertices; i<num; i++, vtx++)
	{
		RL_WriteFloat(li, vtx->pos[VX]);
		RL_WriteFloat(li, vtx->pos[VY]);
		RL_WriteByte(li, vtx->color.rgb[CR]);
		RL_WriteByte(li, vtx->color.rgb[CG]);
		RL_WriteByte(li, vtx->color.rgb[CB]);
	}
	// Add the end marker.
	RL_WriteByte(li, 0);

	// Next poly will naturally overwrite the end marker.
	li->cursor--;	
}

// Here we will add the poly onto the correct rendering list.
/*void RL_AddQuad(rendpoly_t *poly)
{
	poly->type = rp_quad;
	RL_AddPoly(poly);
}*/

/*	
	// Now we have a destination list. This is the only place where
	// quads are added. 
	if(++dest->numquads > dest->listsize)		// See if we have to allocate more memory.
	{
		dest->listsize = dest->numquads+10;		
		dest->quads = (rendquad_t*)realloc(dest->quads, 
			sizeof(rendquad_t) * dest->listsize);
	}
	dq = dest->quads + dest->numquads-1;
	memcpy(dq, quad, sizeof(rendquad_t));
	
	
}*/

/*void RL_AddFlat(rendpoly_t *poly)
{
	poly->type = rp_flat;
	RL_AddPoly(poly);
}*/

// Uh, this is a pretty ugly hack.
//subsector_t *currentssec;

//void RL_AddFlat(rendpoly_t *base, int numvrts, fvertex_t *origvrts, int dir)

void RL_VertexColors(rendpoly_t *poly, int lightlevel)
{
	int		i;
	float	light=lightlevel, real, minimum;
	rendpoly_vertex_t *vtx;

/*	if(poly->numvertices == 2) // A quad?
	{
		// Do a lighting adjustment based on orientation.
		lightlevel += (poly->vertices[1].pos[VY] - poly->vertices[0].pos[VY]) 
			/ poly->length * 16;	
		if(lightlevel < 0) lightlevel = 0;
		if(lightlevel > 255) lightlevel = 255;
	}*/

	light = light+(act_int/6);

	light = light/255.0f;

	for(i=0, vtx=poly->vertices; i<poly->numvertices; i++, vtx++)
	{
		real = light - (vtx->dist - 32)/maxLightDist * (1-light);
		minimum = light*light + (light - .63f) / 2;
		if(real < minimum) real = minimum; // Clamp it.

		// Add extra light.
		real += extralight/16.0f;

		// Check for torch.
		if(viewplayer->fixedcolormap)
		{
			// Colormap 1 is the brightest. I'm guessing 16 would be the darkest.
			int ll = 16 - viewplayer->fixedcolormap;
			float d = (1024 - vtx->dist) / 512.0f;
			float newmin = d*ll / 15.0f;
			if(real < newmin) real = newmin;
		}

		// Clamp the final light.
		if(real < 0) real = 0;
		if(real > 1) real = 1;

		vtx->color.rgb[CR] = vtx->color.rgb[CG] = vtx->color.rgb[CB] = 
			(DGLubyte) (0xff * real);
	}
}

void RL_PrepareFlat(rendpoly_t *poly, int numvrts, fvertex_t *origvrts, 
					  int dir, subsector_t *subsector)
{
	float distances[MAX_POLY_SIDES], middist;
	fvertex_t *vtx, modvrts[MAX_POLY_SIDES], *vrts = origvrts;
	int	i, lightlevel = poly->vertices[0].color.rgb[CR];
	rendpoly_vertex_t *rpv;

	poly->type = rp_flat;	

	// Calculate the distance to each vertex.
	if(!(poly->flags & RPF_LIGHT)) 
	{
		for(i=0; i<numvrts; i++) 
			distances[i] = PointDist2D(&origvrts[i].x);

		// Make a distance modification. This is a bit of a hack...
		// But a useful one, it hides many of the tiny gaps between polygons.
		memcpy(vrts = modvrts, origvrts, sizeof(fvertex_t)*numvrts);	
		if(dlBlend)
		{
			middist = PointDist2D(&subsector->midpoint.x);
#define MINDIST 256
			if(middist > MINDIST)
				for(i=0; i<numvrts; i++)
				{
					float mod = (middist-MINDIST)/128; //128;
					if(mod > 1) mod = 1;
					vrts[i].x += mod * subsector->diffverts[i].x;
					vrts[i].y += mod * subsector->diffverts[i].y;
				}
		}
	}
	else
	{
		// This is a light polygon.
		RL_DynLightPoly(poly, (lumobj_t*) poly->tex);
	}

	// Copy the vertices to the poly.
	poly->numvertices = numvrts;
	for(vtx = (!dir? vrts : vrts+numvrts-1), rpv = poly->vertices; numvrts > 0; 
		numvrts--, !dir? vtx++ : vtx--, rpv++)
	{
		rpv->pos[VX] = vtx->x;
		rpv->pos[VY] = vtx->y;
		rpv->dist = distances[vtx - vrts];
	}
	
	// Calculate the color for each vertex.
	if(!(poly->flags & RPF_LIGHT)) 
	{
		RL_VertexColors(poly, lightlevel);
	}
}

void RL_RenderAllLists()
{
	rendlist_t	*lists[2];

	if(!renderTextures) gl.Disable(DGL_TEXTURING);

	// The sky might be visible. Render the needed hemispheres.
	R_RenderSky(skyhemispheres);				

	lists[0] = &invsky_rlist;
	lists[1] = &invskywall_rlist;
	gl.RenderList(DGL_SKYMASK_LISTS, 0, lists);

	if(!renderTextures) gl.Disable(DGL_TEXTURING);

	// Update the light blending mode.
	gl.SetInteger(DGL_DL_BLEND_MODE, dlBlend);

	gl.RenderList(DGL_NORMAL_LIST, numrlists, rlists);

	if(dlBlend != 3)
	{
		gl.SetInteger(DGL_DL_BLEND_MODE, dlBlend);

		lists[0] = &dlflat_rlist;
		lists[1] = &dlwall_rlist;
		gl.RenderList(DGL_LIGHT_LISTS, 0, lists);	

		// The dlit pass of the normal lists.
		if(!dlBlend) gl.RenderList(DGL_NORMAL_DLIT_LIST, numrlists, rlists);
	}

	gl.RenderList(DGL_MASKED_LIST, 0, &masked_rlist);

	if(!renderTextures) gl.Enable(DGL_TEXTURING);
}

