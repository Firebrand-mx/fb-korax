//**************************************************************************
//**
//** GL_DRAW.C
//**
//** Version:		1.0
//** Last Build:	-?-
//** Author:		jk
//**
//** Rendering lists and other rendering.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "h2def.h"
#include "gl_def.h"
#include "gl_rl.h"
#include "gl_dyn.h"
#include "console.h"
#include "settings.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

void GL_ProjectionMatrix();

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int			useDynLights, translucentIceCorpse;
//extern fadeout_t	fadeOut[2];		// For both skies.
extern int			skyhemispheres, haloMode;
extern int			dlMaxRad;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int l_down=-110,l_up=110;
int time_mod=200;


boolean		whitefog = false;		// Is the white fog in use?
float		fieldOfView = 90.0f;
float		maxLightDist = 1024;

float		vx, vy, vz, vang, vpitch;

boolean		willRenderSprites = true, freezeRLs = false;
int			missileBlend = 1;
int			litSprites = 1;
int			r_ambient = 0;

float		nearClip=5, farClip=16500;

int viewpw, viewph;	// Viewport size, in pixels.
int viewpx, viewpy; // Viewpoint top left corner, in pixels.

float		maxSpriteAngle = 60;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static float yfov;
static float viewsidex, viewsidey;	// For the black fog.

static boolean firstsubsector;		// No range checking for the first one.

// CODE --------------------------------------------------------------------

// How far the point is from the viewside plane?
float PointDist2D(float c[2])
{
/*          (YA-YC)(XB-XA)-(XA-XC)(YB-YA)
        s = -----------------------------
                        L**2
	Luckily, L**2 is one. dist = s*L. Even more luckily, L is also one.
*/
	float dist = (vz-c[VY])*viewsidex - (vx-c[VX])*viewsidey;
	if(dist < 0) return -dist;	// Always return positive.
	return dist;
}

float SignedPointDist2D(float c[2])
{
/*          (YA-YC)(XB-XA)-(XA-XC)(YB-YA)
        s = -----------------------------
                        L**2
	Luckily, L**2 is one. dist = s*L. Even more luckily, L is also one.
*/
	return (vz-c[VY])*viewsidex - (vx-c[VX])*viewsidey;
}


// ---------------------------------------------------

void GL_InitData()
{
	GL_TexInit();		// OpenGL texture manager.
	bamsInit();			// Binary angle calculations.
	C_Init();			// Clipper.
	RL_Init();			// Rendering lists.
	R_SkyInit();		// The sky.
}

void GL_ResetData(void)	// Called before starting a new level.
{
	GL_TexReset();		// Textures are deleted (at least skies need this???).
	RL_DeleteLists();	// The rendering lists are destroyed.

	// We'll delete the sky textures. New ones will be generated for each level.
	//glDeleteTextures(2, skynames);
	//skynames[0] = skynames[1] = 0;
	
	// Ready for new fadeout colors.
//	fadeOut[0].set = fadeOut[1].set = 0;

	DL_Clear();
	H_Clear();
}

void GL_InitRenderer()	// Initializes the renderer to 2D state.
{
	//GLfloat fogcol[4] = { .7f, .7f, .7f, 1 };
	//GLfloat fogcol[4] = { .54f, .54f, .54f, 1 };
	byte fogcol[4] = { 138, 138, 138, 1 };

	// The variables.
/*	nearClip = 5;
	farClip = 16500;//8000;	*/

	// Here we configure the OpenGL state and set the projection matrix.
	gl.Disable(DGL_CULL_FACE);
	gl.Disable(DGL_DEPTH_TEST);
	gl.Enable(DGL_TEXTURING);

	// The projection matrix.
	gl.MatrixMode(DGL_PROJECTION);
	gl.LoadIdentity();
	gl.Ortho(0, 0, 640, 480, -1, 1);

	// Default state for the white fog is off.
	whitefog = false;
	gl.Disable(DGL_FOG);
	gl.Fog(DGL_FOG_MODE, DGL_LINEAR);
	gl.Fog(DGL_FOG_END, 2100);	// This should be tweaked a bit.
	gl.Fogv(DGL_FOG_COLOR, fogcol);

	/*glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);*/
}

void GL_UseWhiteFog(int yes)
{
	if(!whitefog && yes)
	{
		// White fog is turned on.
		whitefog = true;
		gl.Enable(DGL_FOG);
	}
	else if(whitefog && !yes)
	{
		// White fog must be turned off.
		whitefog = false;
		gl.Disable(DGL_FOG);
	}
	// Otherwise we won't do a thing.
}

void GL_SwitchTo3DState()
{
	// Push the 2D state on the stack.
//	glPushAttrib(/*GL_VIEWPORT_BIT | */GL_ENABLE_BIT);
	gl.MatrixMode(DGL_PROJECTION);
	gl.PushMatrix();
	gl.MatrixMode(DGL_MODELVIEW);
	gl.PushMatrix();

	// Enable some... things.
	gl.Enable(DGL_CULL_FACE);
	gl.Enable(DGL_DEPTH_TEST);

	viewpx = viewwindowx * screenWidth/640,
	viewpy = viewwindowy * screenHeight/480;
	// Set the viewport.
	if (viewheight != SCREENHEIGHT)
	{
		viewpw = viewwidth * screenWidth/640;
		viewph = viewheight * screenHeight/480 + 1;
		//glViewport(viewpx, screenHeight-viewpy-viewph, viewpw, viewph);
		gl.Viewport(viewpx, viewpy, viewpw, viewph);
	}
	else
	{
		viewpw = screenWidth;
		viewph = screenHeight;
	}

	// The 3D projection matrix.
	GL_ProjectionMatrix();
}

void GL_Restore2DState(int step)
{
	if(step == 1)
	{
		gl.MatrixMode(DGL_PROJECTION);
		gl.LoadIdentity();
		gl.Ortho(0, 0, 320, (320*viewheight*200)/(viewwidth*240), -1, 1);
		gl.MatrixMode(DGL_MODELVIEW);
		gl.LoadIdentity();
	}
	if(step == 2)
	{
		gl.Viewport(0, 0, screenWidth, screenHeight);
	}
	if(step == 3)
	{
		gl.MatrixMode(DGL_PROJECTION);
		gl.PopMatrix();
		gl.MatrixMode(DGL_MODELVIEW);
		gl.PopMatrix();
				
		gl.Disable(DGL_CULL_FACE);
		gl.Disable(DGL_DEPTH_TEST);
	}
}

static void GL_ProjectionMatrix()
{
	// We're assuming pixels are squares... well, they are nowadays.
	float aspect = viewpw/(float)viewph;

	gl.MatrixMode(DGL_PROJECTION);
	gl.LoadIdentity();
	gl.Perspective(yfov=fieldOfView/aspect, aspect, nearClip, farClip);
	
	// We'd like to have a left-handed coordinate system.
	gl.Scalef(1,1,-1);
}

static void GL_ModelViewMatrix()
{
	vx = FIX2FLT(viewx);
	vy = FIX2FLT(viewz);
	vz = FIX2FLT(viewy);
	vang = viewangle / (float)ANGLE_MAX * 360 - 90;
	vpitch = viewpitch;

	gl.MatrixMode(DGL_MODELVIEW);
	gl.LoadIdentity();
	gl.Rotatef(vpitch, 1, 0, 0);
	gl.Rotatef(vang, 0, 1, 0);
	gl.Scalef(1, 1.2f, 1);	// This is the aspect correction.
	gl.Translatef(-vx,-vy,-vz);
}


static int SegFacingDir(float v1[2], float v2[2])
{
	float nx = v1[VY]-v2[VY], ny = v2[VX]-v1[VX];
	float vvx = v1[VX]-vx, vvy = v1[VY]-vz;

	// The dot product.
	if(nx*vvx+ny*vvy > 0) return 1;	// Facing front.
	return 0;	// Facing away.
}

// The sector height should've been checked by now.
void R_RenderWallSeg(seg_t *seg, sector_t *frontsec, boolean accurate)
{
	sector_t		*backsec = seg->backsector;
	side_t			*sid = seg->sidedef;
	line_t			*ldef = seg->linedef;
	float			ffloor = FIX2FLT(frontsec->floorheight);
	float			fceil = FIX2FLT(frontsec->ceilingheight);
	float			bfloor, bceil, fsh = fceil-ffloor, bsh;
	float			openingTop, openingBottom, mfloor, mceil;
	float			tcyoff;
	rendpoly_t		quad;
	float			origv1[2], origv2[2];	// The original end points, for dynlights.
	float			*v1, *v2;
	int				sectorlight;

	memset(&quad, 0, sizeof(quad));		// Clear the quad.
	quad.type = rp_quad;
	quad.numvertices = 2;

	v1 = quad.vertices[0].pos;
	v2 = quad.vertices[1].pos;

	// Get the start and end points. Full floating point conversion is
	// actually only necessary for polyobjs.
	if(accurate)
	{
		v1[VX] = FIX2FLT(seg->v1->x);
		v1[VY] = FIX2FLT(seg->v1->y);
		v2[VX] = FIX2FLT(seg->v2->x);
		v2[VY] = FIX2FLT(seg->v2->y);
		// These are the original vertices, copy them.
		memcpy(origv1, v1, sizeof(origv1));
		memcpy(origv2, v2, sizeof(origv2));
	}
	else
	{
		//float dx, dy;
		// Not-so-accurate.
		v1[VX] = Q_FIX2FLT(seg->v1->x);
		v1[VY] = Q_FIX2FLT(seg->v1->y);
		v2[VX] = Q_FIX2FLT(seg->v2->x);
		v2[VY] = Q_FIX2FLT(seg->v2->y);
		// The original vertex. For dlights.
		memcpy(origv1, v1, sizeof(origv1));
		memcpy(origv2, v2, sizeof(origv2));
		// Make the very small gap-hiding adjustment.
		
		/*dx = v2[VX] - v1[VX];
		dy = v2[VY] - v1[VY];
		v2[VX] += dx/seg->length/4;
		v2[VY] += dy/seg->length/4;*/
	}

	// Let's first check which way this seg is facing.
	if(!SegFacingDir(v1, v2)) return;	// The wrong direction?

	// Calculate the distances.
	quad.vertices[0].dist = PointDist2D(v1);
	quad.vertices[1].dist = PointDist2D(v2);

	// Make the very small gap-hiding adjustment.
	if(!accurate)
	{
		float dx, dy;
		float str = (quad.vertices[0].dist + quad.vertices[1].dist)/2 / 300;
		if(str > 1.3f) str = 1.3f;
		if(str < .1f) str = .1f;
		dx = v2[VX] - v1[VX];
		dy = v2[VY] - v1[VY];
		v2[VX] += dx/seg->length * str;
		v2[VY] += dy/seg->length * str;
	}

	// Calculate the color at both vertices.
	sectorlight = LevelFullBright? 255 : frontsec->lightlevel;
	if(sectorlight < r_ambient) sectorlight = r_ambient;
	RL_VertexColors(&quad, sectorlight);

	// This line is now seen in the map.
	ldef->flags |= ML_MAPPED;

	// Some texture coordinates.
	quad.texoffx = (sid->textureoffset>>FRACBITS)+(seg->offset>>FRACBITS);
	quad.length = seg->length;
	tcyoff = Q_FIX2FLT(sid->rowoffset);

	// The middle texture, single sided.
	if(sid->midtexture && !backsec)
	{
		quad.tex = curtex = GL_PrepareTexture(sid->midtexture);		
		quad.texoffy = tcyoff;
		if(ldef->flags & ML_DONTPEGBOTTOM)
			quad.texoffy += texh-fsh;

		// Fill in the remaining quad data.
		quad.flags = seg->flags & DDSEGF_DLIGHT? RPF_DLIT : 0;
		// What about glow?
		if(R_TextureFlags(sid->midtexture) & TXF_GLOW)
			quad.flags |= RPF_GLOW;
		quad.top = fceil;
		quad.bottom = ffloor;
		quad.texw = texw;
		quad.texh = texh;
		RL_AddPoly(&quad);

		// This is guaranteed to be a solid segment.
		C_AddViewRelSeg(v1[VX], v1[VY], v2[VX], v2[VY]);
	}
	// The skyfix?
	if(frontsec->skyfix)
	{
		if(!backsec || (backsec && (backsec->ceilingheight+(backsec->skyfix<<FRACBITS) < 
			frontsec->ceilingheight+(frontsec->skyfix<<FRACBITS))))
		{
			quad.flags = RPF_SKY_MASK;
			quad.top = fceil + frontsec->skyfix;
			quad.bottom = fceil;
			quad.tex = curtex;
			RL_AddPoly(&quad);
		}
	}
	// If there is a back sector we may need upper and lower walls.
	if(backsec)	// A twosided seg?
	{
		bfloor = FIX2FLT(backsec->floorheight);
		bceil = FIX2FLT(backsec->ceilingheight);
		bsh = bceil - bfloor;
		if(bsh <= 0 || bceil <= ffloor || bfloor >= fceil)
		{
			//printf( "Solid segment in sector %p (backvol=0).\n", frontsec);
			// The backsector has no space. This is a solid segment.
			C_AddViewRelSeg(v1[VX], v1[VY], v2[VX], v2[VY]);
		}
		if(sid->midtexture)	// Quite probably a masked texture.
		{
			openingTop = (bceil<fceil)? bceil : fceil;
			openingBottom = (bfloor>ffloor)? bfloor : ffloor;
			mceil = openingTop;
			mfloor = openingBottom;
			if(openingTop - openingBottom > 0)
			{
				quad.tex = curtex = GL_PrepareTexture(sid->midtexture);
				// Calculate the texture coordinates. Also restrict
				// vertical tiling by adjusting mceil and mfloor.
				if(texmask)
				{
					quad.flags = RPF_MASKED;
					quad.texoffy = 0;
				}
				else
				{
					quad.flags = 0;
					quad.texoffy = tcyoff;
				}
				// We don't allow vertical tiling.
				if(ldef->flags & ML_DONTPEGBOTTOM)
				{
					mfloor += tcyoff;
					mceil = mfloor + texh;
					if(mceil > openingTop) // Don't go too high.
					{
						quad.texoffy += mceil-openingTop;
						mceil = openingTop;
					}
				}
				else
				{
					mceil += tcyoff;
					mfloor = mceil - texh;
				}
				// Clip it.
				if(mfloor < openingBottom) mfloor = openingBottom;

				// What about glow?
				if(R_TextureFlags(sid->midtexture) & TXF_GLOW)
					quad.flags |= RPF_GLOW;

				// Fill in the remainder of the data.
				quad.top = mceil;
				quad.bottom = mfloor;
				quad.texw = texw;
				quad.texh = texh;
				RL_AddPoly(&quad);
			}
		}
		// Upper wall.
		if(bceil < fceil && !(frontsec->ceilingpic == skyflatnum && backsec->ceilingpic == skyflatnum))
		{
			float topwh = fceil - bceil;
			if(sid->toptexture)	// A texture present?
			{
				quad.tex = curtex = GL_PrepareTexture(sid->toptexture);
				// Calculate texture coordinates.
				quad.texoffy = tcyoff;
				if(!(ldef->flags & ML_DONTPEGTOP))
				{
					// Normal alignment to bottom.
					quad.texoffy += texh-topwh;
				}								
				quad.flags = 0;
			}
			else
			{
				// No texture? Bad thing. You don't deserve texture 
				// coordinates. Take the ceiling texture.
				quad.tex = curtex = GL_PrepareFlat(frontsec->ceilingpic);				
				quad.flags = RPF_MISSING_WALL;
			}
			if(seg->flags & DDSEGF_DLIGHT) quad.flags |= RPF_DLIT;
			// What about glow?
			if(R_TextureFlags(sid->toptexture) & TXF_GLOW)
				quad.flags |= RPF_GLOW;
			quad.top = fceil;
			quad.bottom = bceil;
			quad.texw = texw;
			quad.texh = texh;
			RL_AddPoly(&quad);
		}
		// Lower wall.
		if(bfloor > ffloor && !(frontsec->floorpic == skyflatnum && backsec->floorpic == skyflatnum))
		{
			if(sid->bottomtexture)	// There is a texture?
			{
				quad.tex = curtex = GL_PrepareTexture(sid->bottomtexture);
				// Calculate texture coordinates.
				quad.texoffy = tcyoff;
				if(ldef->flags & ML_DONTPEGBOTTOM)
				{
					// Lower unpegged. Align with normal middle texture.
					//quad.texoffy += fsh-texh;
					quad.texoffy += fceil-bfloor;
				}
				quad.flags = 0;
			}
			else
			{
				// No texture? Again!
				quad.tex = curtex = GL_PrepareFlat(frontsec->floorpic);
				quad.flags = RPF_MISSING_WALL;
			}
			if(seg->flags & DDSEGF_DLIGHT) quad.flags |= RPF_DLIT;
			// What about glow?
			if(R_TextureFlags(sid->bottomtexture) & TXF_GLOW)
				quad.flags |= RPF_GLOW;
			quad.top = bfloor;
			quad.bottom = ffloor;
			quad.texw = texw;
			quad.texh = texh;
			RL_AddPoly(&quad);
		}
	}
}
	
void R_RenderSubsector(int ssecidx)
{
	subsector_t		*ssec = subsectors + ssecidx;
	int				i;
	seg_t			*seg;
	sector_t		*sect = ssec->sector;
	float			ffloor = FIX2FLT(sect->floorheight);
	float			fceil = FIX2FLT(sect->ceilingheight);
	rendpoly_t		poly;
	int				sectorlight;
	
	if(fceil-ffloor <= 0) 
	{
		return;	// Skip this, no volume.
	}

	if(!firstsubsector)
	{
		if(!C_CheckSubsector(ssec)) return;	// This isn't visible.
	}
	else
		firstsubsector = false;

//	printf( "*** Rendering subsector %d (sector %p)\n", ssecidx, sect);

	// Dynamic lights. Processes both the ceiling and the floor, and all
	// visible wall segments. First clear the necessary flags.
//	ssec->flags = 0;
	for(i=0, seg=segs+ssec->firstline; i<ssec->numlines; i++, seg++)
		seg->flags = 0;
	if(useDynLights) 
	{
		DL_ProcessSubsector(&poly, ssec);
	}
	else if(haloMode)
	{
		// Flares are on, but dynamic lights aren't processed. We must
		// mark the rendered luminous objects ourselves.
		for(i=0; i<numLuminous; i++)
		{
			if(luminousList[i].thing->subsector == ssec)
				luminousList[i].flags |= LUMF_RENDERED;
		}
	}

	// Sprites for this sector have to be drawn. This must be done before
	// the segments of this subsector are added to the clipper. Otherwise
	// the sprites would get clipped by them, and that wouldn't be right.
	R_AddSprites(sect);

	// Draw the walls.
	for(i=0, seg=segs+ssec->firstline; i<ssec->numlines; i++, seg++)
		if(seg->linedef)	// "minisegs" have no linedefs.
			R_RenderWallSeg(seg, sect, false);

	// Is there a polyobj on board?
	if(ssec->poly)
		for(i=0; i<ssec->poly->numsegs; i++)
			R_RenderWallSeg(ssec->poly->segs[i], sect, true);

	sectorlight = LevelFullBright? 255 : sect->lightlevel;
	if(sectorlight < r_ambient) sectorlight = r_ambient;
	// -JL- Clamp light
	if (sectorlight > 255) sectorlight = 255;

	// The floor.
	memset(&poly, 0, sizeof(poly));
	poly.type = rp_flat;
//	if(ssec->flags & DDSUBF_DLIGHT_FLOOR) poly.flags = RPF_DLIT;  //crap
//	if(R_FlatFlags(sect->floorpic) & TXF_GLOW) poly.flags |= RPF_GLOW;  //crap
	poly.vertices[0].color.rgb[CR] = sectorlight;
	if(viewz > sect->floorheight) // && vpitch < yfov)
	{
		// Check for sky... in the floor?
		if(sect->floorpic == skyflatnum) 
		{
			poly.flags |= RPF_SKY_MASK;
			skyhemispheres |= SKYHEMI_LOWER;
		}
		poly.tex = curtex = GL_PrepareFlat(sect->floorpic);
		poly.texw = texw;
		poly.texh = texh;
		poly.texoffx = sect->flatoffx;
		poly.texoffy = sect->flatoffy;
		poly.top = ffloor;
		// The first vertex is always the first in the whole list.
		RL_PrepareFlat(&poly, ssec->numedgeverts, 
			!dlBlend /*&& ssec->flags & DDSUBF_DLIGHT_FLOOR*/? ssec->origedgeverts : ssec->edgeverts, 
			0, ssec);
		RL_AddPoly(&poly);
	}
	// And the roof.
	poly.flags = 0;
//	if(ssec->flags & DDSUBF_DLIGHT_CEILING) poly.flags = RPF_DLIT;  //crap
//	if(R_FlatFlags(sect->ceilingpic) & TXF_GLOW) poly.flags |= RPF_GLOW; //crap
	poly.vertices[0].color.rgb[CR] = sectorlight;
	if(viewz < sect->ceilingheight) //&& vpitch > -yfov)
	{
		// Check for sky.
		if(sect->ceilingpic == skyflatnum) 
		{
			poly.flags |= RPF_SKY_MASK;
			skyhemispheres |= SKYHEMI_UPPER;
		}
		poly.tex = curtex = GL_PrepareFlat(sect->ceilingpic);
		poly.texw = texw;
		poly.texh = texh;
		poly.texoffx = 0;
		poly.texoffy = 0;
		poly.top = fceil + sect->skyfix;
		// The first vertex is always the last in the whole list.
		RL_PrepareFlat(&poly, ssec->numedgeverts, 
			!dlBlend/* && ssec->flags & DDSUBF_DLIGHT_CEILING*/? ssec->origedgeverts : ssec->edgeverts, 
			1, ssec);
		RL_AddPoly(&poly);
	}
}

void R_RenderNode(int bspnum)
{
	node_t          *bsp;
	int             side;

	// If the clipper is full we're pretty much done.
	if(cliphead)
		if(cliphead->start == 0 && cliphead->end == BANG_MAX)
			return;

	if (bspnum & NF_SUBSECTOR)
	{
		if (bspnum == -1)
			R_RenderSubsector(0);
		else
			R_RenderSubsector(bspnum&(~NF_SUBSECTOR));
		return;
	}

	bsp = &nodes[bspnum];

//
// decide which side the view point is on
//
	side = R_PointOnSide (viewx, viewy, bsp);
	
	R_RenderNode (bsp->children[side]); // recursively divide front space

/*	if(cliphead->start == 0 && cliphead->end == BANG_MAX)
		return;	// We can stop rendering.*/

	//if (R_CheckBBox (bsp->bbox[side^1]))    // possibly divide back space
	// We can't use that, unfortunately.
	R_RenderNode (bsp->children[side^1]);
	
}

void R_RenderMap()
{
	binangle_t	viewside;

	// This is all the clearing we'll do.
	gl.Clear(DGL_DEPTH_BUFFER_BIT);

	// Setup the modelview matrix.
	GL_ModelViewMatrix();

	if(!freezeRLs)
	{
		RL_ClearLists();	// Clear the lists for new quads.
		C_ClearRanges();	// Clear the clipper.

		if(useDynLights || haloMode || litSprites) DL_InitForNewFrame(); // Maintain luminous objects.

		// Add the backside clipping range, vpitch allowing.
		if(vpitch <= 90-yfov/2 && vpitch >= -90+yfov/2)
		{
			float a = fabs(vpitch) / (90-yfov/2);
			//binangle_t startAngle = (binangle_t) BANG_45*(1+a);
			binangle_t startAngle = (binangle_t) (BANG_45*fieldOfView/90)*(1+a);
			binangle_t angLen = BANG_180 - startAngle;																					
			viewside = (viewangle>>16) + startAngle;
			C_SafeAddRange(viewside, viewside+angLen);
			C_SafeAddRange(viewside+angLen, viewside+2*angLen);
		}
		// The viewside line for the depth cue.
		viewsidex = -FIX2FLT(viewsin);
		viewsidey = FIX2FLT(viewcos);

		// We don't want subsector clipchecking for the first subsector.
		firstsubsector = true;
		R_RenderNode(numnodes-1);
		
	}
	RL_RenderAllLists();

	// The halos.
	if(haloMode) H_InitForNewFrame();

	// Clipping fragmentation happens when there are holes in the walls.
	/*if(cliphead->next)
	{
		clipnode_t *ci;
		printf("\nTic: %d, Clipnodes are fragmented:\n", gametic);
		for(ci=cliphead; ci; ci=ci->next)
			printf( "range %p: %4x => %4x (%d)\n",ci,ci->start,ci->end,ci->used);
		I_Error("---Fragmented clipper---\n");
	}*/
}

// Global variables... agh.
static byte *slRGB1, *slRGB2; // The colors to modify when sprites are lit.
static fvertex_t slViewVec;
static vissprite_t *slSpr;

static boolean spriteLighter(lumobj_t *lum, fixed_t dist)
{
	int			i, temp;
	float		fdist = FIX2FLT(dist);
	fvertex_t	lightVec = { FIX2FLT(slSpr->gx - lum->thing->x), FIX2FLT(slSpr->gy - lum->thing->y) };
	float		directness, side, inleft, inright, zfactor;
	
	if(!fdist) return true;
	if(slRGB1[0] == 0xff && slRGB1[1] == 0xff && slRGB1[2] == 0xff 
		&& slRGB2[0] == 0xff && slRGB2[1] == 0xff && slRGB2[2] == 0xff)
		return false; // No point in continuing, light is already white.

	zfactor = (FIX2FLT(slSpr->gz + slSpr->gzt)/2 - (FIX2FLT(lum->thing->z) + lum->center)) / dlMaxRad;
	if(zfactor < 0) zfactor = -zfactor;
	if(zfactor > 1) return true; // Too high or low.
	zfactor = 1-zfactor;
	// Enlarge the full-lit area.
	zfactor *= 2;
	if(zfactor > 1) zfactor = 1;
	
	lightVec.x /= fdist;
	lightVec.y /= fdist;

	// Also include the effect of the distance to zfactor.
	fdist /= dlMaxRad;
	fdist = 1-fdist;
	fdist *= 2;
	if(fdist > 1) fdist = 1;
	zfactor *= fdist;

	// Now the view vector and light vector are normalized.
	directness = slViewVec.x * lightVec.x + slViewVec.y * lightVec.y; // Dot product.
	side = -slViewVec.y * lightVec.x + slViewVec.x * lightVec.y;
	// If side > 0, the light comes from the right.
	if(directness > 0)
	{
		// The light comes from the front.
		if(side > 0)
		{
			inright = 1;
			inleft = directness;
		}
		else 
		{
			inleft = 1;
			inright = directness;
		}
	}
	else
	{
		// The light comes from the back.
		if(side > 0)
		{
			inright = side;
			inleft = 0;
		}
		else 
		{
			inleft = -side;
			inright = 0;
		}
	}
	inright *= zfactor;
	inleft *= zfactor;
	if(inleft > 0)
	{
		for(i=0; i<3; i++)
		{
			temp = slRGB1[i] + inleft*lum->rgb[i];
			if(temp > 0xff) temp = 0xff;
			slRGB1[i] = temp;
		}
	}
	if(inright > 0)
	{
		for(i=0; i<3; i++)
		{
			temp = slRGB2[i] + inright*lum->rgb[i];
			if(temp > 0xff) temp = 0xff;
			slRGB2[i] = temp;
		}
	}
	return true;
}

void R_RenderSprite(vissprite_t *spr)
{
	float		bot,top;
	float		off = FIX2FLT(spriteoffset[spr->patch]);
	float		w = FIX2FLT(spritewidth[spr->patch]);
	int			sprh;
	float		v1[2];
	DGLubyte	alpha;
	boolean		additiveBlending = false, flip, restoreMatrix = false;
	rendpoly_t	tempquad;

	// Do we need to translate any of the colors?
	if(spr->mobjflags & DDMF_TRANSLATION)
	{
		dc_translation = translationtables-256 + spr->pclass*((/*MAXPLAYERS*/8-1)*256) +
			((spr->mobjflags & DDMF_TRANSLATION) >> (DDMF_TRANSSHIFT-8));
		// We need to prepare a translated version of the sprite.
		GL_SetTranslatedSprite(spr->patch, dc_translation);
	}
	else
	{
		// Set the texture. No translation required.
		GL_SetSprite(spr->patch);
	}
	sprh = spriteheight[spr->patch];

	// Set the lighting and alpha.
	if(missileBlend && spr->mobjflags & DDMF_BRIGHTSHADOW)
	{
		alpha = 204;	// 80 %.
		additiveBlending = true;
	}
	else if(spr->mobjflags & DDMF_SHADOW)
		alpha = 85;		// One third.
	else if(spr->mobjflags & DDMF_ALTSHADOW)
		alpha = 170;	// Two thirds.
	else
		alpha = 255;

	if(spr->lightlevel < 0)
		gl.Color4ub(255, 255, 255, alpha);
	else
	{
		v1[VX] = Q_FIX2FLT(spr->gx);
		v1[VY] = Q_FIX2FLT(spr->gy);
		tempquad.vertices[0].dist = PointDist2D(v1);
		tempquad.numvertices = 1;
		RL_VertexColors(&tempquad, 
			r_ambient > spr->lightlevel? r_ambient : spr->lightlevel);

		// Add extra light using dynamic lights.
		if(litSprites)
		{
			float len;
			memcpy(&tempquad.vertices[1].color, &tempquad.vertices[0].color, 3);
			// Global variables as parameters... ugly.
			slSpr = spr;
			slRGB1 = tempquad.vertices[0].color.rgb;
			slRGB2 = tempquad.vertices[1].color.rgb;
			slViewVec.x = FIX2FLT(spr->gx - viewx);
			slViewVec.y = FIX2FLT(spr->gy - viewy);
			len = sqrt(slViewVec.x*slViewVec.x + slViewVec.y*slViewVec.y);
			if(len)
			{
				slViewVec.x /= len;
				slViewVec.y /= len;
				DL_RadiusIterator(spr->gx, spr->gy, dlMaxRad << FRACBITS, spriteLighter);
			}
		}
		gl.Color4ub(tempquad.vertices[0].color.rgb[CR],
			tempquad.vertices[0].color.rgb[CG],
			tempquad.vertices[0].color.rgb[CB],
			alpha);
	}

	// We must find the correct positioning using the sector floor and ceiling
	// heights as an aid.
	top = FIX2FLT(spr->gzt);
	if(sprh < spr->secceil-spr->secfloor)	// Sprite fits in, adjustment possible?
	{
		// Check top.
		if(spr->mobjflags & DDMF_FITTOP && top > spr->secceil) 
			top = spr->secceil;
		// Check bottom.
		if(!(spr->mobjflags & DDMF_NOFITBOTTOM) && top-sprh < spr->secfloor)
			top = spr->secfloor+sprh;
	}
	// Adjust by the floor clip.
	top -= FIX2FLT(spr->floorclip);
	bot = top - sprh;

	// Should the texture be flipped?
	flip = spr->xiscale < 0;

	// Do we need to do some aligning?
	if(spr->viewAligned || alwaysAlign >= 2)
	{
		float centerx = FIX2FLT(spr->gx), centery = FIX2FLT(spr->gy);
		float centerz = (top+bot)/2;
		// We must set up a modelview transformation matrix.
		restoreMatrix = true;
		gl.MatrixMode(DGL_MODELVIEW);
		gl.PushMatrix();
		// Rotate around the center of the sprite.
		gl.Translatef(centerx, centerz, centery);
		if(!spr->viewAligned)
		{
			float s_dx = spr->v1[VX] - spr->v2[VX];
			float s_dy = spr->v1[VY] - spr->v2[VY];
			if(alwaysAlign == 2) // restricted camera alignment
			{
				float dx = centerx - vx, dy = centery - vz;	
				float spriteAngle = BANG2DEG(bamsAtan2(centerz-vy, sqrt(dx*dx + dy*dy)));
				if(spriteAngle > 180) spriteAngle -= 360;
				if(fabs(spriteAngle) > maxSpriteAngle)
				{
					float turnAngle = spriteAngle>0? spriteAngle-maxSpriteAngle 
						: spriteAngle+maxSpriteAngle;
					// Rotate along the sprite edge.
					gl.Rotatef(turnAngle, s_dx, 0, s_dy);			
				}
			}
			else // restricted view plane alignment
			{
				// This'll do, for now... Really it should notice both the
				// sprite angle and vpitch.
				gl.Rotatef(vpitch/2, s_dx, 0, s_dy);
			}
		}
		else
		{
			// Normal rotation perpendicular to the view plane.
			gl.Rotatef(vpitch, viewsidex, 0, viewsidey);
		}
		gl.Translatef(-centerx, -centerz, -centery);
	}

	if(additiveBlending)
	{
		// Change the blending mode.
		gl.Func(DGL_BLENDING, DGL_SRC_ALPHA, DGL_ONE);
	}

	// Render the sprite.
	gl.Begin(DGL_QUADS);
	gl.TexCoord2f(flip, 1);
	gl.Vertex3f(spr->v1[VX], bot, spr->v1[VY]);
	gl.TexCoord2f(flip, 0);
	gl.Vertex3f(spr->v1[VX], top, spr->v1[VY]);

	if(litSprites && spr->lightlevel >= 0)
	{
		gl.Color4ub(tempquad.vertices[1].color.rgb[CR],
			tempquad.vertices[1].color.rgb[CG],
			tempquad.vertices[1].color.rgb[CB],
			alpha);
	}
	gl.TexCoord2f(!flip, 0);
	gl.Vertex3f(spr->v2[VX], top, spr->v2[VY]);
	gl.TexCoord2f(!flip, 1);
	gl.Vertex3f(spr->v2[VX], bot, spr->v2[VY]);
	gl.End();

	/*{
		gl.Disable(DGL_DEPTH_TEST);

		// Draw a "test mirroring".
		gl.Begin(DGL_QUADS);
		gl.Color4ub(tempquad.color[0].rgb[0],
			tempquad.color[0].rgb[1],
			tempquad.color[0].rgb[2],
			alpha/3);
		gl.TexCoord2f(flip, 0);
		gl.Vertex3f(spr->v1[VX], 2*spr->secfloor - top, spr->v1[VY]);
		gl.TexCoord2f(flip, 1);
		gl.Vertex3f(spr->v1[VX], 2*spr->secfloor - bot, spr->v1[VY]);
		gl.TexCoord2f(!flip, 1);
		gl.Vertex3f(spr->v2[VX], 2*spr->secfloor - bot, spr->v2[VY]);
		gl.TexCoord2f(!flip, 0);
		gl.Vertex3f(spr->v2[VX], 2*spr->secfloor - top, spr->v2[VY]);
		gl.End();

		gl.Enable(DGL_DEPTH_TEST);
	}*/

	if(restoreMatrix)
	{
		// Restore the original modelview matrix.
		gl.PopMatrix();
	}

	if(additiveBlending)
	{
		// Change to normal blending.
		gl.Func(DGL_BLENDING, DGL_SRC_ALPHA, DGL_ONE_MINUS_SRC_ALPHA);
	}
}

void GL_DrawPSprite(int x, int y, float scale, int flip, int lump)
{
	int		w, h;

	GL_SetSprite(lump);
	w = spritewidth[lump] >> FRACBITS;
	h = spriteheight[lump];

	if(flip) flip = 1; // Ensure it's zero or one.

	gl.Begin(DGL_QUADS);
	gl.TexCoord2f(flip, 0);
	gl.Vertex2f(x, y);
	gl.TexCoord2f(!flip, 0);
	gl.Vertex2f(x + w*scale, y);
	gl.TexCoord2f(!flip, 1);
	gl.Vertex2f(x + w*scale, y + h*scale);
	gl.TexCoord2f(flip, 1);
	gl.Vertex2f(x, y+h*scale);
	gl.End();
}



// Console commands.
int CCmdFog(int argc, char **argv)
{
	int		i;

	if(argc == 1)
	{
		CON_Printf( "Usage: %s (cmd) (args)\n", argv[0]);
		CON_Printf( "Commands: on, off, mode, color, start, end, density.\n");
		CON_Printf( "Modes: linear, exp, exp2.\n");
		//CON_Printf( "Hints: fastest, nicest, dontcare.\n");
		CON_Printf( "Color is given as RGB (0-255).\n");
		CON_Printf( "Start and end are for linear fog, density for exponential.\n");
		return true;		
	}
	if(!stricmp(argv[1], "on"))
	{
		GL_UseWhiteFog(true);
		CON_Printf( "Fog is now active.\n");
	}
	else if(!stricmp(argv[1], "off"))
	{
		GL_UseWhiteFog(false);
		CON_Printf( "Fog is now disabled.\n");
	}
	else if(!stricmp(argv[1], "mode") && argc == 3)
	{
		if(!stricmp(argv[2], "linear"))	
		{
			gl.Fog(DGL_FOG_MODE, DGL_LINEAR);
			CON_Printf( "Fog mode set to linear.\n");
		}
		else if(!stricmp(argv[2], "exp")) 
		{
			gl.Fog(DGL_FOG_MODE, DGL_EXP);
			CON_Printf( "Fog mode set to exp.\n");
		}
		else if(!stricmp(argv[2], "exp2")) 
		{
			gl.Fog(DGL_FOG_MODE, DGL_EXP2);
			CON_Printf( "Fog mode set to exp2.\n");
		}
		else return false;
	}
/*	else if(!stricmp(argv[1], "hint") && argc == 3)
	{
		if(!stricmp(argv[2], "fastest")) 
		{
			glHint(GL_FOG_HINT, GL_FASTEST);
			CON_Printf( "Fog quality set to fastest.\n");
		}
		else if(!stricmp(argv[2], "nicest")) 
		{
			glHint(GL_FOG_HINT, GL_NICEST);
			CON_Printf( "Fog quality set to nicest.\n");
		}
		else if(!stricmp(argv[2], "dontcare")) 
		{
			glHint(GL_FOG_HINT, GL_DONT_CARE);
			CON_Printf( "Fog quality set to \"don't care\".\n");
		}
		else return false;
	}*/
	else if(!stricmp(argv[1], "color") && argc == 5)
	{
		byte col[4];
		for(i=0; i<3; i++)
			col[i] = strtol(argv[2+i], NULL, 0)/*/255.0f*/;
		col[3] = 1;
		gl.Fogv(DGL_FOG_COLOR, col);
		CON_Printf( "Fog color set.\n");
	}	
	else if(!stricmp(argv[1], "start") && argc == 3)
	{
		gl.Fog(DGL_FOG_START, strtod(argv[2], NULL));
		CON_Printf( "Fog start distance set.\n");
	}
	else if(!stricmp(argv[1], "end") && argc == 3)
	{
		gl.Fog(DGL_FOG_END, strtod(argv[2], NULL));
		CON_Printf( "Fog end distance set.\n");
	}
	else if(!stricmp(argv[1], "density") && argc == 3)
	{
		gl.Fog(DGL_FOG_DENSITY, strtod(argv[2], NULL));
		CON_Printf( "Fog density set.\n");
	}
	else return false;
	// Exit with a success.
	return true;
}
