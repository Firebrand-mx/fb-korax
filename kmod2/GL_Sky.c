
//**************************************************************************
//**
//** GL_SKY.C
//**
//** This version supports only two sky layers.
//** More would be a waste of resources.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <math.h>
#include "dd_def.h"
#include "gl_def.h"
#include "gl_rl.h"
#include "gl_sky.h"
#include "i_win32.h"

// MACROS ------------------------------------------------------------------

#define MAXSKYLAYERS		2		

#define SKYVTX_IDX(c, r)	( (r)*skyColumns + (c)%skyColumns )

// TYPES -------------------------------------------------------------------

typedef struct 
{
	float x,y,z;
} skyvertex_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern float		texw, texh;
extern float		vx, vy, vz;
extern byte			topLineRGB[3];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

skylayer_t	skyLayers[MAXSKYLAYERS];
int			firstLayer, activeLayers;
skyvertex_t	*skyVerts = NULL;	// Vertices for the upper hemisphere.
int			numSkyVerts = 0;
int			skyDetail=6, simpleSky;
int			skyColumns, skyRows=3;
int			skyhemispheres;
float		skyDist = 1600;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// The texture offset to be applied to the texture coordinates in SkyVertex().
static float		maxSideAngle = (float) PI/3;
static float		horizonOffset = 0;
static float		skyTexOff;
static boolean		yflip;
static fadeout_t	*currentFO;

// CODE --------------------------------------------------------------------

// Calculate the vertex and texture coordinates.
static void SkyVertex(int r, int c)
{
	// The direction must be clockwise.
	skyvertex_t *svtx = skyVerts + SKYVTX_IDX(c, r);

	// And the texture coordinates.
	if(!yflip)	// Flipped Y is for the lower hemisphere.
	{
		gl.TexCoord2f(4*c/ (float) skyColumns + skyTexOff/texw, r/ (float) skyRows);
	}
	else
	{
		gl.TexCoord2f(4*c/ (float) skyColumns + skyTexOff/texw, 
			(skyRows-r)/ (float) skyRows);
	}
	// Also the color.
	if(currentFO->use)
	{
		if(r==0) gl.Color4f(1,1,1,0); else gl.Color3f(1,1,1);
	}
	else
	{
		if(r==0) gl.Color3f(0,0,0); else gl.Color3f(1,1,1);
	}
	// And finally the vertex itself.
	//gl.Vertex3f(vx + svtx->x, vy + svtx->y*(yflip? -1 : 1), vz + svtx->z);
	gl.Vertex3f(svtx->x, svtx->y*(yflip? -1 : 1), svtx->z);
}

static void CapSideVertex(int r, int c)
{
	// Look up the precalculated vertex.
	skyvertex_t *svtx = skyVerts + SKYVTX_IDX(c, r);

	/*gl.Vertex3f(vx + svtx->x, vy + svtx->y*(yflip? -1 : 1),
		vz + svtx->z);*/
	gl.Vertex3f(svtx->x, svtx->y*(yflip? -1 : 1), svtx->z);
}

// Hemi is Upper or Lower. Zero is not acceptable.
// The current texture is used. SKYHEMI_NO_TOPCAP can be used.
void GL_SkyRenderer(int hemi)
{
	int		r, c;

	if(hemi & SKYHEMI_LOWER) yflip = true; else yflip = false;

	// The top row (row 0) is the one that's faded out.
	// There must be at least 4 columns. The preferable number
	// is 4n, where n is 1, 2, 3... There should be at least
	// two rows because the first one is always faded.
	if(hemi & SKYHEMI_JUST_CAP)
	{
		gl.Disable(DGL_TEXTURING);
		// Use the appropriate color.
		if(currentFO->use)
			gl.Color3fv(currentFO->rgb);
		else
			gl.Color3f(0,0,0);

		// Draw the cap.
		gl.Begin(DGL_TRIANGLE_FAN);
		for(c=0; c<skyColumns; c++) CapSideVertex(0, c);
		gl.End();

		// If we are doing a colored fadeout...
		if(hemi & SKYHEMI_FADEOUT_BG)
		{
			// We must fill the background for the top row since it'll
			// be partially translucent.
			gl.Begin(DGL_TRIANGLE_STRIP);
			CapSideVertex(0, 0);
			for(c=0; c<skyColumns; c++)
			{
				CapSideVertex(1, c);	// One step down.
				CapSideVertex(0, c+1);	// And one step right.
			}
			CapSideVertex(1, c);
			gl.End();
		}
		gl.Enable(DGL_TEXTURING);
		return;
	}

	// The total number of triangles per hemisphere can be calculated
	// as follows: rows * columns * 2 + 2 (for the top cap).
	for(r=0; r<skyRows; r++)
	{
		if(simpleSky)
		{
			gl.Begin(DGL_QUADS);
			for(c=0; c<skyColumns; c++)
			{	
				SkyVertex(r, c);
				SkyVertex(r+1, c);
				SkyVertex(r+1, c+1);
				SkyVertex(r, c+1);
			}
			gl.End();
		}
		else
		{
			gl.Begin(DGL_QUAD_STRIP);
			SkyVertex(r, 0);
			SkyVertex(r+1, 0);
			for(c=1; c<=skyColumns; c++)
			{
				SkyVertex(r, c);
				SkyVertex(r+1, c);
			}
			gl.End();
		}
	}
}

static void SetupFadeout(skylayer_t *slayer)
{
	int		i;
	byte	rgb[3];

	GL_GetSkyTopColor(slayer->texture, rgb);

	// topLineRGB is only valid right after a call to
	// GL_PrepareSky where a texture was actually uploaded!
	for(i=0; i<3; i++) slayer->fadeout.rgb[i] = rgb[i]/255.0f;
		
	// Determine if it should be used.
	for(slayer->fadeout.use=false, i=0; i<3; i++)
		if(slayer->fadeout.rgb[i] > slayer->fadeout.limit)
		{
			// Colored fadeout is needed.
			slayer->fadeout.use = true;
			break;
		}
}

void R_SkyRenderHemisphere(int whichHemi)
{
	int			i, resetup;
	skylayer_t	*slayer;

	// The current fadeout is the first layer's fadeout.
	currentFO = &skyLayers[firstLayer].fadeout;

	// First render the cap and the background for fadeouts, if needed.
	// The color for both is the current fadeout color.
	GL_SkyRenderer(whichHemi | SKYHEMI_JUST_CAP | (currentFO->use? SKYHEMI_FADEOUT_BG : 0));

	for(i=firstLayer, slayer=skyLayers+firstLayer; i<MAXSKYLAYERS; i++, slayer++)
		if(slayer->flags & SLF_ENABLED)
		{
			resetup = false;

			if(slayer->texture == -1) 
				I_Error("R_SkyRenderHemisphere: Sky layer without a texture!\n");

			// See if we have to re-setup the fadeout.
			// This happens if the texture is for some reason deleted
			// (forced texture reload, for example).
			if(!GL_GetTextureName(slayer->texture)) resetup = true;

			// The texture is actually loaded when an update is done.
			gl.Bind(GL_PrepareSky(slayer->texture,	slayer->flags&SLF_MASKED? true : false));

			if(resetup) SetupFadeout(slayer);

			skyTexOff = slayer->offset;
			GL_SkyRenderer(whichHemi);
		}
}

void R_RenderSky(int hemis)
{
	extern int whitefog;

	// IS there a sky to be rendered?
	if(!hemis || firstLayer == -1) return;

	// We don't want anything written in the depth buffer, not yet.
	gl.Disable(DGL_DEPTH_WRITE);
	// Disable culling, all triangles face the viewer.
	gl.Disable(DGL_CULL_FACE);

	// Setup a proper matrix.
	gl.MatrixMode(DGL_MODELVIEW);
	gl.PushMatrix();
	//gl.LoadIdentity();
	gl.Translatef(vx, vy, vz);
	gl.Scalef(skyDist, skyDist, skyDist);
	
	// Draw the possibly visible hemispheres.
	if(hemis & SKYHEMI_UPPER) R_SkyRenderHemisphere(SKYHEMI_UPPER);
	if(hemis & SKYHEMI_LOWER) R_SkyRenderHemisphere(SKYHEMI_LOWER);
	
	gl.MatrixMode(DGL_MODELVIEW);
	gl.PopMatrix();

	// Enable the disabled things.
	gl.Enable(DGL_CULL_FACE);
	gl.Enable(DGL_DEPTH_WRITE);
}

//------------------------------------------------------------------------
// SKY API
//

void R_SkyInit()
{
	int		i;

	firstLayer = 0;
	//maxSideAngle = (float) PI/3;

	R_SkyDetail(skyDetail, skyRows);

	// Initialize the layers.
	for(i=0; i<MAXSKYLAYERS; i++)
	{
		skyLayers[i].texture = -1; // No texture.
		skyLayers[i].fadeout.limit = .3f;
	}
}

void R_SkyShutdown()
{
	free(skyVerts);
	skyVerts = NULL;
	numSkyVerts = 0;
}

void R_SkyDetail(int quarterDivs, int rows)
{
	float			topAngle, sideAngle, realRadius, scale = 1/*32*/;
	int				c, r;
	skyvertex_t		*svtx;

	if(quarterDivs < 1) quarterDivs = 1;
	if(rows < 1) rows = 1;

	skyDetail = quarterDivs;
	skyColumns = 4*skyDetail;
	skyRows = rows;

	// Calculate the sky vertices.
	numSkyVerts = skyColumns*(skyRows+1);
	
	// Allocate memory for it.
	skyVerts = realloc(skyVerts, sizeof(*skyVerts) * numSkyVerts);
	// -JL- Paranoia
	if (!skyVerts)
		I_Error("R_SkyDetail: realloc failed");

	// Calculate the vertices.
	for(r=0; r<=skyRows; r++)
		for(c=0; c<skyColumns; c++)
		{
			svtx = skyVerts + SKYVTX_IDX(c, r);
			topAngle = c/(float)skyColumns * 2*PI;
			sideAngle = horizonOffset + maxSideAngle*(skyRows-r)/(float)skyRows;
			realRadius = scale * cos(sideAngle);
			svtx->x = realRadius * cos(topAngle);
			svtx->y = scale * sin(sideAngle);  // The height.
			svtx->z = realRadius * sin(topAngle);
		}
}

static void updateLayerStats()
{
	int		i=0;

	// -1 denotes 'no active layers'.
	firstLayer = -1;
	activeLayers = 0;
	for(i=0; i<MAXSKYLAYERS; i++)
	{
		if(skyLayers[i].flags & SLF_ENABLED)
		{
			activeLayers++;
			if(firstLayer == -1) firstLayer = i;
		}
	}
}

static void internalSkyParams(skylayer_t *slayer, int parm, float value)
{
	switch(parm)
	{
	case DD_ENABLE:
		slayer->flags |= SLF_ENABLED;
		updateLayerStats();
		break;

	case DD_DISABLE:
		slayer->flags &= ~SLF_ENABLED;
		updateLayerStats();
		break;

	case DD_MASK:
		if(value == DD_YES)
		{
			// Invalidate the loaded texture, if necessary.
			if(!(slayer->flags & SLF_MASKED)) GL_DeleteTexture(slayer->texture);
			slayer->flags |= SLF_MASKED;
		}
		else
		{
			// Invalidate the loaded texture, if necessary.
			if(slayer->flags & SLF_MASKED) GL_DeleteTexture(slayer->texture);
			slayer->flags &= ~SLF_MASKED;
		}
		break;

	case DD_TEXTURE:
		slayer->texture = (int) value;
		GL_PrepareSky(slayer->texture, slayer->flags&SLF_MASKED? true : false);
		SetupFadeout(slayer);
		break;

	case DD_OFFSET:
		slayer->offset = value;
		break;

	case DD_COLOR_LIMIT:
		slayer->fadeout.limit = value;
		break;

	default:
		I_Error("R_SkyParams: Bad parameter (%d).\n", parm);
	}
}

void R_SkyParams(int layer, int parm, float value)
{
	int		i;

//	if(isDedicated) return;

	if(layer == DD_SKY)	// The whole sky?
	{
		switch(parm)
		{
		case DD_COLUMNS:
			R_SkyDetail((int) value, skyRows);
			break;
		
		case DD_ROWS:
			R_SkyDetail(skyDetail, (int) value);			
			break;
		
		case DD_HEIGHT:
			maxSideAngle = PI/2 * value;
			// Recalculate the vertices.
			R_SkyDetail(skyDetail, skyRows);
			break;

		case DD_HORIZON: // horizon offset angle
			horizonOffset = PI/2 * value;
			// Recalculate the vertices.
			R_SkyDetail(skyDetail, skyRows);
			break;
		
		default:
			// Operate on all layers.
			for(i=0; i<MAXSKYLAYERS; i++)
				internalSkyParams(skyLayers+i, parm, value);
		}
	}
	// This is for a specific layer.
	else if(layer >= 0 && layer < MAXSKYLAYERS) 
		internalSkyParams(skyLayers+layer, parm, value);
}