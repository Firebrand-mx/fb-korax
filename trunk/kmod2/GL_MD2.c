
//**************************************************************************
//**
//** GL_MD2.C
//**
//** Rendering MD2s.
//**
//**************************************************************************

//
// Notice! Light vectors and triangle normals are considered to be
// in a totally separate, right-handed coordinate system.
//

// HEADER FILES ------------------------------------------------------------

#include "dd_def.h"
#include "dd_md2.h"
#include "gl_def.h"
#include "gl_rl.h"
#include "gl_dyn.h"
#include "console.h"
#include "settings.h"

#include <math.h>

// MACROS ------------------------------------------------------------------

#define MAX_MODEL_LIGHTS	10

#define DOTPROD(a, b)		(a[0]*b[0] + a[1]*b[1] + a[2]*b[2])

// TYPES -------------------------------------------------------------------

typedef struct
{
	fixed_t		dist;		// Only an approximation.
	lumobj_t	*lum;
	float		vector[3];	// Light direction vector.
	float		color[3];	// How intense the light is (0..1, RGB).
} mlight_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int modelLight = 4;
int frameInter = true;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static float worldLight[3] = { .195180f, .097590f, .975900f };

static mlight_t lights[MAX_MODEL_LIGHTS] =
{
	// The first light is the world light.
	0, NULL, { 0, 0, 0 }, { 1, 1, 1 }
};

// Parameters for the modelLighter.
// Global variables as parameters. Urgh.
static vissprite_t *mlSpr;

// CODE --------------------------------------------------------------------

// First yaw, then pitch. Two consecutive 2D rotations.
// Probably could be done a bit more efficiently.
// Currently this is called once for each light per model.
static void ypRotateVector(float vec[3], float yaw, float pitch)
{
	float radYaw = yaw/180 * PI, radPitch = pitch/180 * PI;
	float Cos, Sin, res[3];

	// Yaw.
	if(radYaw != 0)
	{
		Cos = cos(radYaw); 
		Sin = sin(radYaw);
		res[VX] = vec[VX] * Cos + vec[VY] * Sin;
		res[VY] = vec[VX] * -Sin + vec[VY] * Cos;
		vec[VX] = res[VX];
		vec[VY] = res[VY];
	}
	// Pitch.
	if(radPitch != 0)
	{
		Cos = cos(radPitch);
		Sin = sin(radPitch);
/*		res[VY] = vec[VY] * Cos + vec[VZ] * Sin;
		res[VZ] = vec[VY] * -Sin + vec[VZ] * Cos;
		vec[VY] = res[VY];
		vec[VZ] = res[VZ];*/

		res[VZ] = vec[VZ] * Cos + vec[VX] * Sin;
		res[VX] = vec[VZ] * -Sin + vec[VX] * Cos;
		vec[VZ] = res[VZ];
		vec[VX] = res[VX];

	}
}

static boolean modelLighter(lumobj_t *lum, fixed_t xy_dist)
{
	fixed_t	z_dist = ((mlSpr->gz + mlSpr->gzt) >> 1) - (lum->thing->z + FRACUNIT*lum->center);
	fixed_t dist = M_AproxDistance(xy_dist, z_dist);
	int		i, maxIndex;
	fixed_t	maxDist = -1;
	mlight_t *light;

	// If the light is too far away, skip it.
	if(dist > (dlMaxRad << FRACBITS)) return true;

	// See if this lumobj is close enough to make it to the list.
	// (In most cases it should be the case.)
	for(i=1, light=lights+1; i<modelLight; i++, light++)
	{
		if(light->dist > maxDist)
		{
			maxDist = light->dist;
			maxIndex = i;
		}
	}
	// Now we know the farthest light on the current list.
	if(dist < maxDist)
	{
		// The new light is closer. Replace the old max.
		lights[maxIndex].lum = lum;
		lights[maxIndex].dist = dist;
	}
	return true;
}

void R_RenderModel(vissprite_t *spr, int number)
{
	modelframe_t	*mf = spr->mf;
	submodelframe_t	*smf = &mf->sub[number];
	model_t			*mdl = modellist[smf->model];
	md2_frame_t		*frame = mdl->frames + smf->frame;
	md2_frame_t		*nextframe = NULL;
	byte			*ptr = (byte*) mdl->glCommands;
	int				i, c, count, sectorlight;
	md2_glCommandVertex_t *glc;
	md2_modelVertex_t *vtx, *nextvtx;
	boolean			additiveBlending = false;
	boolean			lightVertices = false;
	float			alpha, yawAngle, pitchAngle;
	rendpoly_t		tempquad;
	mlight_t		*light;
	float			ambientLight;	// The model gets at least this much.
	float			modelCenter[3];
	float			lightCenter[3], dist, intensity;
	lumobj_t		*lum;
	float			vtxLight[4], dot;
	int				mflags = smf->flags;

	// Check for possible interpolation.
	if(frameInter && spr->nextmf)
	{
		if(spr->nextmf->sub[number].model == smf->model)
		{
			nextframe = mdl->frames + spr->nextmf->sub[number].frame;
			nextvtx = nextframe->vertices;
		}
	}

	// Init. Set the texture.
	GL_BindTexture(GL_PrepareSkin(mdl, smf->skin));
	
	yawAngle = spr->yaw;
	pitchAngle = spr->pitch;

	// Light level.
	if(missileBlend && (spr->mobjflags & DDMF_BRIGHTSHADOW
		|| mflags & MFF_BRIGHTSHADOW))
	{
		alpha = .80f;	//204;	// 80 %.
		additiveBlending = true;
	}
	else if(spr->mobjflags & DDMF_SHADOW || mflags & MFF_SHADOW2)
		alpha = .333f;	//85;		// One third.
	else if(spr->mobjflags & DDMF_ALTSHADOW || mflags & MFF_SHADOW1)
		alpha = .666f;	//170;	// Two thirds.
	else
		alpha = 1;

	if(spr->lightlevel < 0 || mflags & MFF_FULLBRIGHT) 
	{
		gl.Color4f(1, 1, 1, alpha);
	}
	else
	{
		tempquad.vertices[0].dist = PointDist2D(spr->v1);
		tempquad.numvertices = 1;
		sectorlight = r_ambient > spr->lightlevel? r_ambient : spr->lightlevel;
		RL_VertexColors(&tempquad, sectorlight);
		// This way the distance darkening has an effect.
		ambientLight = tempquad.vertices[0].color.rgb[0] / 255.0f;		
		if(modelLight)
		{
			lightVertices = true;
			// The model should be lit with world light. 
			// Set the correct intensity.
			for(i=0; i<3; i++) 
			{
				lights[0].vector[i] = worldLight[i];
				lights[0].color[i] = ambientLight * 1.8f;
			}
			// Rotate the light direction to model space.
			ypRotateVector(lights[0].vector, -yawAngle, -pitchAngle);
			// Now we can diminish the actual ambient light that
			// hits the object. (Gotta have some contrast.)
			ambientLight /= 1.05f;
		}
		// Add extra light using dynamic lights.
		if(modelLight > 1)
		{
			// Find the nearest sources of light. They will be used to
			// light the vertices a bit later. First initialize the array.
			for(i=1; i<MAX_MODEL_LIGHTS; i++)
			{
				lights[i].dist = DDMAXINT;
				lights[i].lum = NULL;
			}
			mlSpr = spr;
			DL_RadiusIterator(spr->gx, spr->gy, dlMaxRad << FRACBITS, modelLighter);

			// Coordinates to the center of the model.
			modelCenter[VX] = Q_FIX2FLT(spr->gx);
			modelCenter[VY] = Q_FIX2FLT(spr->gy);
			modelCenter[VZ] = Q_FIX2FLT((spr->gz+spr->gzt) >> 1);
						
			// Calculate the directions and intensities of the lights.
			for(i=1, light=lights+1; i<modelLight; i++, light++)
			{
				if(!light->lum) continue;
				lum = light->lum;
			
				// This isn't entirely accurate, but who could notice?
				dist = FIX2FLT(light->dist);				

				// The intensity of the light.
				intensity = (1 - dist/dlMaxRad) * 1.5f;
				if(intensity < 0) intensity = 0;
				if(intensity > 1) intensity = 1;

				if(intensity == 0)
				{
					// No point in lighting with this!
					light->lum = NULL;
					continue;
				}

				// The center of the light source.
				lightCenter[VX] = Q_FIX2FLT(lum->thing->x);
				lightCenter[VY] = Q_FIX2FLT(lum->thing->y);
				lightCenter[VZ] = Q_FIX2FLT(lum->thing->z) + lum->center;

				// Calculate the normalized direction vector, 
				// pointing out of the model.
				for(c=0; c<3; c++) 
				{
					light->vector[c] = (lightCenter[c] - modelCenter[c]) / dist;
					// ...and the color of the light.
					light->color[c] = lum->rgb[c]/255.0f * intensity;
				}

				// We must transform the light vector to model space.
				ypRotateVector(light->vector, -yawAngle, -pitchAngle);				
			}
		}
		if(!modelLight)
		{
			// Just the ambient light.
			gl.Color4ub(tempquad.vertices[0].color.rgb[CR],
				tempquad.vertices[0].color.rgb[CG],
				tempquad.vertices[0].color.rgb[CB],
				alpha * 255);
			lightVertices = false;
		}
	}

	gl.MatrixMode(DGL_MODELVIEW);
	gl.PushMatrix();

	gl.Translatef(spr->v1[VX], FIX2FLT(spr->gz) + mf->offset, spr->v1[VY]);
	gl.Rotatef(yawAngle, 0, 1, 0);
	gl.Rotatef(pitchAngle, 0, 0, 1); 
	gl.Scalef(mf->scale, mf->scale, mf->scale);

/*	gl.Translatef(frame->translate[VX], 
		frame->translate[VZ], 
		frame->translate[VY]);
	gl.Scalef(frame->scale[VX], 
		frame->scale[VZ], //*.833333, 
		frame->scale[VY]);*/

	if(additiveBlending)
	{
		// Change the blending mode.
		gl.Func(DGL_BLENDING, DGL_SRC_ALPHA, DGL_ONE);
	}

	gl.Begin(DGL_SEQUENCE);
	
	// Draw the triangles using the GL commands.
	while(*ptr)
	{
		count = *(int*) ptr;
		ptr += 4;
		gl.Begin(count>0? DGL_TRIANGLE_STRIP : DGL_TRIANGLE_FAN);
		if(count < 0) count = -count;
		while(count--)
		{	
			glc = (md2_glCommandVertex_t*) ptr;
			ptr += sizeof(md2_glCommandVertex_t);
			// Give the data to DGL.
			gl.TexCoord2f(glc->s, glc->t);
			vtx = frame->vertices + glc->vertexIndex;
			if(nextframe)
				nextvtx = nextframe->vertices + glc->vertexIndex;

			// Also calculate the light level at the vertex?
			if(lightVertices) 
			{
				// Begin with total darkness.
				vtxLight[0] = vtxLight[1] = vtxLight[2] = 0;
				vtxLight[3] = alpha;

				// Add light from each source.
				for(i=0, light=lights; i<modelLight; i++, light++)
				{
					if(i && !light->lum) continue;
					dot = DOTPROD(light->vector, avertexnormals[vtx->lightNormalIndex]);
					if(dot <= 0) continue; // No light from the wrong side.
					for(c=0; c<3; c++)
						vtxLight[c] += dot * light->color[c];
				}

				// What about ambient light?
				for(i=0; i<3; i++)
					if(vtxLight[i] < ambientLight) 
						vtxLight[i] = ambientLight;

				// Set the color.
				gl.Color4fv(vtxLight);								
			}

			if(nextframe)
			{
				float inv = 1 - spr->inter;
				gl.Vertex3f(vtx->vertex[VX]*inv + nextvtx->vertex[VX]*spr->inter,
					vtx->vertex[VZ]*inv + nextvtx->vertex[VZ]*spr->inter,
					vtx->vertex[VY]*inv + nextvtx->vertex[VY]*spr->inter);
				//gl.Vertex3f(nextvtx->vertex[VX], nextvtx->vertex[VZ], nextvtx->vertex[VY]);
			}
			else
				gl.Vertex3f(vtx->vertex[VX], vtx->vertex[VZ], vtx->vertex[VY]);
		}
		gl.End();
	}	

	// End the sequence.
	gl.End();
	
	// Restore renderer state.
	gl.PopMatrix();

	if(additiveBlending)
	{
		// Change to normal blending.
		gl.Func(DGL_BLENDING, DGL_SRC_ALPHA, DGL_ONE_MINUS_SRC_ALPHA);
	}
}