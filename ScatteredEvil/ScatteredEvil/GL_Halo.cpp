
//**************************************************************************
//**
//** GL_HALO.C
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "h2def.h"
#include "i_win32.h"
#include "gl_def.h"
#include "gl_rl.h"
#include "gl_dyn.h"
#include "console.h"
#include "settings.h"

// MACROS ------------------------------------------------------------------

#define Z_TEST_BIAS		.00015
#define NUM_FLARES		5

//enum { CR, CG, CB, CA };

// TYPES -------------------------------------------------------------------

typedef struct
{
	float offset;
	float size;
	float alpha;
	int texture;		// -1=dlight, 0=flare, 1=brflare, 2=bigflare
} flare_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

flare_t flares[NUM_FLARES] =
{
	{ 0,	1.1f,	.6f,	2 }, 		// Primary flare.
	{ 1,	.35f,	.3f,	0 },		// Main secondary flare.
	{ 1.5f,	.25f,	.2f,	1 },
	{ -.6f,	.2f,	.2f,	0 },
	{ .4f,	.25f,	.15f,	0 }
};

/*	{ 0, 1 }, 		// Primary flare.
	{ 1, 2 },		// Main secondary flare.
	{ 1.5f, 4 },
	{ -.6f, 4 },
	{ .4f, 4 }
*/

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern float	nearClip, farClip;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int				haloMode = 5, flareBoldness = 50, flareSize = 3;
gl_fc3vertex_t	*haloData = 0;
int				dataSize = 0, usedData;
float			flareFadeMax = 0, flareFadeMin = 0, minFlareSize = 200;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

// Free the halo data. Done at shutdown.
void H_Clear()
{
	if(haloData) Z_Free(haloData);
	haloData = 0;
	dataSize = 0;
	usedData = 0;
}

// Prepares the list of halos to render. Goes through the luminousList.
void H_InitForNewFrame()
{
	extern float vx, vy, vz;
	float		vpos[3] = { vx, vy, vz };
	int			i, c, neededSize = numLuminous;
	lumobj_t	*lum;
	gl_fc3vertex_t *vtx;
	float		factor;

	// Allocate a large enough buffer for the halo data.
	if(neededSize > dataSize)
	{
		dataSize = neededSize;
		if(haloData) Z_Free(haloData);
		haloData = (gl_fc3vertex_t *)Z_Malloc(sizeof(*haloData) * dataSize, PU_STATIC, 0);
	}

	// Is there something to do?
	if(!dataSize || !haloData || !neededSize)
	{
		usedData = 0;
		return;
	}

	for(i=0, vtx=haloData, lum=luminousList; i<numLuminous; i++, lum++)
	{
		if(!(lum->flags & LUMF_RENDERED)) continue;
		// Insert the data into the halodata buffer.
		vtx->pos[VX] = FIX2FLT(lum->thing->x);
		vtx->pos[VY] = FIX2FLT(lum->thing->z) + lum->center;
		vtx->pos[VZ] = FIX2FLT(lum->thing->y);

		// Do some offseting for a viewaligned sprites.
		if(lum->thing->ddflags & DDMF_VIEWALIGN || alwaysAlign)
		{
			for(c=0; c<3; c++) 
			{
				vtx->pos[c] -= vpos[c];
				vtx->pos[c] *= .98f;
				vtx->pos[c] += vpos[c];
			}
		}

		if(flareFadeMax && FIX2FLT(lum->distance) < flareFadeMax)
		{
			if(FIX2FLT(lum->distance) < flareFadeMin) continue;
			factor = (FIX2FLT(lum->distance) - flareFadeMin) / (flareFadeMax - flareFadeMin);
		}
		else
		{
			factor = 1;
		}
		for(c=0; c<3; c++) vtx->color[c] = lum->rgb[c] / 255.0f * factor;
		vtx->color[CA] = lum->flaresize / 255.0f;
		vtx++;
	}
	// Project the vertices.
	usedData = gl.Project(vtx - haloData, haloData, haloData);
}

static void H_DrawRect(float x, float y, float size)
{
	gl.TexCoord2f(0, 0);
	gl.Vertex2f(x-size, y-size);

	gl.TexCoord2f(1, 0);
	gl.Vertex2f(x+size, y-size);

	gl.TexCoord2f(1, 1);
	gl.Vertex2f(x+size, y+size);

	gl.TexCoord2f(0, 1);
	gl.Vertex2f(x-size, y+size);
}

// This must be called when the renderer is in player sprite rendering mode:
// 2D projection to the game view. What will happen is that each point in 
// the haloData buffer gets a halo if it passes the Z buffer test.
//
// Can only be called once per frame!
//
void H_Render()
{
	int			k, i, num = usedData, tex, numRenderedFlares;
	float		centerX = viewpx + viewpw/2.0f;
	float		centerY = viewpy + viewph/2.0f;
	gl_fc3vertex_t *vtx;
	float		radius, size, clipRange = farClip-nearClip, secDiff[2];
	float		factor = flareBoldness/100.0f;
	float		scaler = 2 - flareSize/10.0f;
	float		viewscaler = viewwidth/320.0f;
	float		flareSizeFactor, secondaryAlpha;
	boolean		needScissor = false;
	int			*readData, *iter;
	float		*zValues;
    int         oldFog;

	if(!num || !haloData || !dataSize) return;

	gl.MatrixMode(DGL_MODELVIEW);
	gl.LoadIdentity();
	gl.MatrixMode(DGL_PROJECTION);
	gl.LoadIdentity();
	gl.Ortho(0, 0, screenWidth, screenHeight, -1, 1);

	// Setup the appropriate state.
	gl.Disable(DGL_DEPTH_TEST);
	gl.Disable(DGL_CULL_FACE);
	gl.GetIntegerv(DGL_FOG, &oldFog);
	if(oldFog) gl.Disable(DGL_FOG);

	// Do we need to scissor the flares to the view window?
	if(viewph != screenHeight)
	{
		needScissor = true;
		gl.Enable(DGL_SCISSOR_TEST);
		gl.Scissor(viewpx, viewpy, viewpw, viewph);
	}

	// First determine which of the halos are worth rendering.
	// Fill the Single Pixels inquiry.
	readData = (int *)Z_Malloc(sizeof(int)*(2 + num*2), PU_STATIC, 0);
	zValues = (float *)Z_Malloc(sizeof(float) * num, PU_STATIC, 0);
	readData[0] = DGL_SINGLE_PIXELS;
	readData[1] = num;
	for(i=0, vtx=haloData, iter=readData+2; i<num; i++, vtx++, iter+=2)
	{
		iter[0] = (int) vtx->pos[VX];
		iter[1] = (int) vtx->pos[VY];
	}
/*		ptr = haloData + i*8;
		// Read the depth value.
		glReadPixels(ptr[HDO_X], ptr[HDO_Y], 1, 1, GL_DEPTH_COMPONENT,
			GL_FLOAT, &depth);

		if(depth < ptr[HDO_Z]-.0001) ptr[HDO_TOKEN] = 0;

		// Obviously, this test is not the best way to do things.
		// First of all, the sprite itself may be in front of the
		// flare when the sprite is viewed from up or down. Also,
		// the Z buffer is not the most accurate thing in the world,
		// so errors may occur.
	}*/
	gl.ReadPixels(readData, DGL_DEPTH_COMPONENT, zValues);
	for(i=0, vtx=haloData; i<num; i++, vtx++)
	{
		//printf( "%i: %f\n", i, zValues[i]);
		if(zValues[i] < vtx->pos[VZ] - Z_TEST_BIAS) 
			vtx->pos[VX] = -1;	// This marks the halo off-screen.
	}
	Z_Free(readData);
	Z_Free(zValues);

	// Additive blending.
	gl.Func(DGL_BLENDING, DGL_SRC_ALPHA, DGL_ONE);

	gl.MatrixMode(DGL_TEXTURE);
	gl.PushMatrix();
	for(i=0, vtx=haloData; i<num; i++, vtx++)
	{
		if(vtx->pos[VX] == -1) continue; // Blocked flare?

		radius = vtx->color[CA] * 10;
		if(radius > 1.5) radius = 1.5;

		// Determine the size of the halo.
		flareSizeFactor = (farClip - clipRange*vtx->pos[VZ]) * radius;
		size = flareSizeFactor * screenHeight/(1100*scaler) * viewscaler;
	
		// Determine the offset to the main secondary flare.
		secDiff[0] = 2 * (centerX - vtx->pos[VX]);
		secDiff[1] = 2 * (centerY - vtx->pos[VY]);

		// The rotation (around the center of the flare texture).
		gl.LoadIdentity();
		gl.Translatef(.5, .5, 0);
		gl.Rotatef(BANG2DEG(bamsAtan2(2*secDiff[1], -2*secDiff[0])), 0, 0, 1);
		gl.Translatef(-.5, -.5, 0);

		// If the source object is small enough, don't render
		// secondary flares.
		if(flareSizeFactor < minFlareSize*.667f) // Two thirds, completely vanish.
		{
			numRenderedFlares = 1;
			// No secondary flares rendered.
		}
		else if(flareSizeFactor < minFlareSize)
		{
			numRenderedFlares = haloMode;
			secondaryAlpha = (3*flareSizeFactor - 2*minFlareSize) / minFlareSize;
		}
		else
		{
			numRenderedFlares = haloMode;
			secondaryAlpha = 1;
		}
		//if(radius < .3 && flare->size < 1) continue;
		for(k=0; k<numRenderedFlares; k++)
		{
			flare_t *flare = flares + k;

			// The color and the alpha.
			vtx->color[CA] = factor * flare->alpha;
			if(k) vtx->color[CA] *= secondaryAlpha;
			
			tex = flare->texture;
			if(!k && tex == 2)
			{
				if(radius < .9 && radius >= .5) tex = 1;
			}
			GL_BindTexture(tex<0? GL_PrepareLightTexture() : GL_PrepareFlareTexture(tex));

			gl.Begin(DGL_QUADS);
			gl.Color4fv(vtx->color);
			H_DrawRect(vtx->pos[VX] + flare->offset*secDiff[0], 
				vtx->pos[VY] + flare->offset*secDiff[1], 
				size * flare->size);
			gl.End();
		}
	}
	// Pop back the original texture matrix.
	gl.PopMatrix();

	// Restore normal rendering state.
	gl.Func(DGL_BLENDING, DGL_SRC_ALPHA, DGL_ONE_MINUS_SRC_ALPHA);
	gl.Enable(DGL_DEPTH_TEST);
	gl.Enable(DGL_CULL_FACE);
	if(needScissor)	gl.Disable(DGL_SCISSOR_TEST);
	if(oldFog) gl.Enable(DGL_FOG);
}

// FlareConfig list
// FlareConfig (num) pos/size/alpha/tex (val)
int CCmdFlareConfig(int argc, char **argv)
{	
	int		i;
	float	val;

	if(argc == 2)
	{
		if(!stricmp(argv[1], "list"))
		{
			for(i=0; i<NUM_FLARES; i++)
			{
				ST_Message( "%i: pos:%f s:%.2f a:%.2f tex:%i\n", 
					i, flares[i].offset, flares[i].size, flares[i].alpha, flares[i].texture);
			}
		}
	}
	else if(argc == 4)
	{
		i = atoi(argv[1]);
		val = strtod(argv[3], NULL);
		if(i < 0 || i >= NUM_FLARES) return false;
		if(!stricmp(argv[2], "pos"))
		{
			flares[i].offset = val;
		}
		else if(!stricmp(argv[2], "size"))
		{
			flares[i].size = val;
		}
		else if(!stricmp(argv[2], "alpha"))
		{
			flares[i].alpha = val;
		}
		else if(!stricmp(argv[2], "tex"))
		{
			flares[i].texture = (int) val;
		}
	}	
	else
	{
		CON_Printf( "Usage:\n");
		CON_Printf( "  %s list\n", argv[0]);
		CON_Printf( "  %s (num) pos/size/alpha/tex (val)\n", argv[0]);
	}
	return true;
}