// drD3D.dll
// Direct3D driver for the Doomsday Graphics Library
//
// list.c : List rendering

#include "drD3D.h"

#define INLINE				__inline
#define LIST_EMPTY(rl)		(!(rl)->data || !*(rl)->data)

rendlist_t *readlist;

// This is used by drawWall() and drawFlat() when !col.
int globalColor = 0;

// Undone with PopMatrix().
void setupZBias(int level)
{
	guard(setupZBias);
	float sc = .99f - level*.01f;

	MatrixMode(DGL_PROJECTION);
	PushMatrix();
	Scalef(sc, sc, sc);
	unguard;
}

static void drawWall(rendpoly_t *cq, byte tex, byte col)
{
	guard(drawWall);
	float		tcleft = 0, tcright = 1, tctop = 0, tcbottom = 1;
	vertex_t	vtx[4];
	
	// Calculate relative texture coordinates.
	if(tex)
	{
		tcright = (tcleft=cq->texoffx/(float)cq->texw) + cq->length/cq->texw;
		tcbottom = (tctop=cq->texoffy/cq->texh) + (cq->top - cq->bottom)/cq->texh;
	}
	
	// The vertices.
	vtx[0].rgba = col? MAKERGB_A(cq->vertices[0].color.rgb, 0xff) : globalColor;
	vtx[0].s = tcleft;
	vtx[0].t = tctop;
	vtx[0].x = cq->vertices[0].pos[VX];	
	vtx[0].y = cq->top;
	vtx[0].z = cq->vertices[0].pos[VY];

	vtx[1].rgba = col? MAKERGB_A(cq->vertices[1].color.rgb, 0xff) : globalColor;
	vtx[1].s = tcright;
	vtx[1].t = tctop;
	vtx[1].x = cq->vertices[1].pos[VX];	
	vtx[1].y = cq->top;
	vtx[1].z = cq->vertices[1].pos[VY];

	vtx[2].rgba = vtx[1].rgba;
	vtx[2].s = tcright;
	vtx[2].t = tcbottom;
	vtx[2].x = cq->vertices[1].pos[VX];	
	vtx[2].y = cq->bottom;
	vtx[2].z = cq->vertices[1].pos[VY];

	vtx[3].rgba = vtx[0].rgba;
	vtx[3].s = tcleft;
	vtx[3].t = tcbottom;
	vtx[3].x = cq->vertices[0].pos[VX];	
	vtx[3].y = cq->bottom;
	vtx[3].z = cq->vertices[0].pos[VY];

	IDirect3DDevice3_DrawPrimitive(d3dDevice,
		D3DPT_TRIANGLEFAN, VERTEX_FORMAT,
		vtx, 4, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
	unguard;
}

static void drawFlat(rendpoly_t *cq, byte tex, byte col)
{
	guard(drawFlat);
	vertex_t vtx[DGL_MAX_POLY_SIDES];
	rendpoly_vertex_t *rpVtx = cq->vertices;
	int i;

	if(cq->flags & RPF_MISSING_WALL)
	{
		drawWall(cq, tex, col);
		return;
	}
	
	for(i=0; i<cq->numvertices; i++, rpVtx++)
	{
		vtx[i].rgba = col? MAKERGB_A(rpVtx->color.rgb, 0xff) : globalColor;
		if(tex)
		{
			if(cq->flags & RPF_LIGHT)
			{
				vtx[i].s = (cq->texoffx - rpVtx->pos[VX]) / cq->texw;
				vtx[i].t = (cq->texoffy - rpVtx->pos[VY]) / cq->texh;
			}
			else
			{
				vtx[i].s = (rpVtx->pos[VX] + cq->texoffx) / cq->texw;
				vtx[i].t = (-rpVtx->pos[VY] - cq->texoffy) / cq->texh;
			}
		}
		vtx[i].x = rpVtx->pos[VX];
		vtx[i].y = cq->top;
		vtx[i].z = rpVtx->pos[VY];
	}
	
	IDirect3DDevice3_DrawPrimitive(d3dDevice,
		D3DPT_TRIANGLEFAN, VERTEX_FORMAT,
		vtx, cq->numvertices, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
	unguard;
}

void initForReading(rendlist_t *li)
{
	guard(initForReading);
	readlist = li;
	readlist->cursor = readlist->data;
	unguard;
}

INLINE byte readByte()
{
	return *readlist->cursor++;
}

INLINE short readShort()
{
	readlist->cursor += 2;
	return *(short*) (readlist->cursor-2);
}

INLINE int readLong()
{
	readlist->cursor += 4;
	return *(int*) (readlist->cursor-4);
}

INLINE float readFloat()
{
	readlist->cursor += 4;
	return *(float*) (readlist->cursor-4);
}

int readPoly(rendpoly_t *poly)
{
	guard(readPoly);
	int i, c, temp;
	rendpoly_vertex_t *vtx;
	
	poly->type = (rendpolytype_t)readByte();
	if(poly->type == rp_none) return DGL_FALSE;	// Time to stop.
	poly->flags = readByte();
	poly->texw = readShort();
	poly->texh = readShort();
	poly->texoffx = readFloat();
	poly->texoffy = readFloat();
	if(poly->flags & RPF_MASKED) poly->tex = readLong();
	poly->top = readFloat();
	if(poly->type == rp_quad) 
	{
		poly->bottom = readFloat();
		poly->length = readFloat();
		poly->numvertices = 2;
	}
	else
	{
		poly->numvertices = readByte();
	}
	for(i=0, vtx=poly->vertices; i<poly->numvertices; i++, vtx++)
	{
		vtx->pos[VX] = readFloat();
		vtx->pos[VY] = readFloat();
		for(c=0; c<3; c++)
		{
			temp = readByte();
			if(poly->flags & RPF_GLOW) temp = 255;
			vtx->color.rgb[c] = temp;
		}
	}
	return DGL_TRUE;
	unguard;
}

// This is only for solid, non-masked primitives.
void renderList(rendlist_t *rl)
{
	guard(renderList);
	rendpoly_t	cq;
	char		dlight = DGL_FALSE;
	char		flats;

	if(LIST_EMPTY(rl)) return; // The list is empty.

	initForReading(rl);

	// Disable alpha blending; some video cards think alpha zero is
	// is still translucent.
	Disable(DGL_BLENDING);	

	// Bind the right texture.
	Bind(rl->tex); 

	// Check what kind of primitives there are on the list.
	// There can only be one kind on each list.
	flats = (rl->data[0] == rp_flat) || (rl->data[1] & RPF_MISSING_WALL);

	while(readPoly(&cq))
	{
		if(!dlBlend && cq.flags & RPF_DLIT && !usefog)
		{
			dlight = DGL_TRUE;
			continue;
		}
		if(flats) drawFlat(&cq, 1, 1); else drawWall(&cq, 1, 1);
	}

	// Need to draw some dlit polys?
	if(dlight)
	{
		initForReading(rl);
		Disable(DGL_TEXTURING);
		while(readPoly(&cq))
		{
			if(!(cq.flags & RPF_DLIT)) continue;
			if(flats) drawFlat(&cq, 0, 1); else drawWall(&cq, 0, 1);
		}
		Enable(DGL_TEXTURING);
	}

	// Enable alpha blending once more.
	Enable(DGL_BLENDING);
	unguard;
}

// Masked lists only include quads.
void renderMaskedList(rendlist_t *mrl)
{
	guard(renderMaskedList);
	rendpoly_t	cq;
	DGLuint		currentTex;
	
	//if(!mrl->numquads) return;	// No quads to render, I'm afraid.

	if(LIST_EMPTY(mrl)) return;

	initForReading(mrl);

	// Curtex is used to keep track of the current texture.
	// Zero also denotes that no glBegin() has yet been called.
	currentTex = 0;

	// Render quads.
	BeginScene();

	while(readPoly(&cq))
	{
		//cq = mrl->quads+i;

		// Is there a need to change the texture?
		if(currentTex != cq.tex)	
		{
			Bind(currentTex = cq.tex);
		}

		drawWall(&cq, DGL_TRUE, DGL_TRUE);
	}
	EndScene();
	unguard;
}

void renderSkyMaskLists(rendlist_t *smrl, rendlist_t *skyw)
{
	guard(renderSkyMaskLists);
	rendpoly_t	cq;

	if(LIST_EMPTY(smrl) && LIST_EMPTY(skyw)) return; // Nothing to do here.

	// Nothing gets written to the color buffer.
	// Instead of setting texture 0 we'll temporarily disable texturing.
	Disable(DGL_TEXTURING);
	// This will effectively disable color buffer writes.
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	if(!LIST_EMPTY(smrl))
	{
		initForReading(smrl);
		BeginScene();
		while(readPoly(&cq)) drawFlat(&cq, DGL_FALSE, DGL_FALSE);
		EndScene();
	}
	
	// Then the walls.
	if(!LIST_EMPTY(skyw))
	{
		initForReading(skyw);
		BeginScene();
		while(readPoly(&cq)) drawWall(&cq, DGL_FALSE, DGL_FALSE);
		EndScene();
	}
	
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	
	Enable(DGL_TEXTURING);
	unguard;
}

void renderDynLightLists(rendlist_t *frl, rendlist_t *wrl)
{
	guard(renderDynLightLists);
	rendpoly_t	cq;

	if(LIST_EMPTY(frl) && LIST_EMPTY(wrl)) return; // Nothing to do.

	// Adjust the projection matrix for Z bias.
	setupZBias(0);

	// Setup the correct rendering state.	
	// Disable fog.
	if(usefog) SetRS(D3DRENDERSTATE_FOGENABLE, FALSE);

	// This'll allow multiple light quads to be rendered on top of each other.
	SetRS(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	SetRS(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);

	// Set up addition blending. The source is added to the destination.
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	// The light texture.
	Bind(lightTex);

	BeginScene();

	// The flats.
	if(!LIST_EMPTY(frl))
	{
		initForReading(frl);
		while(readPoly(&cq))
		{
			// Set the color.
			//glColor3ubv(cq.vertices[0].color.rgb);
			globalColor = MAKERGB_A(cq.vertices[0].color.rgb, 0xff);
			drawFlat(&cq, DGL_TRUE, DGL_FALSE);
		}
	}	

	// The walls.
	if(!LIST_EMPTY(wrl))
	{
		initForReading(wrl);
		while(readPoly(&cq))
		{
			// Set the color.
			globalColor = MAKERGB_A(cq.vertices[0].color.rgb, 0xff);

			// Calculate the texture coordinates.
			cq.texoffx = -cq.texoffx;

			drawWall(&cq, DGL_TRUE, DGL_FALSE);
		}
	}

	EndScene();

	//glLoadMatrixf(projmat);
	MatrixMode(DGL_PROJECTION);
	PopMatrix();

	// Restore the normal rendering state.
	if(usefog) SetRS(D3DRENDERSTATE_FOGENABLE, TRUE);
	
	SetRS(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	SetRS(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS);
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	unguard;
}

void renderDLitPass(rendlist_t *rl, int num)
{
	guard(renderDLitPass);
	int			k;//, i;
	rendpoly_t	cq;
	char		flats, inited;

	// Setup the correct rendering state.	
	if(usefog) return;

	// Adjust the projection matrix for Z bias.
	setupZBias(0);

	// This'll allow multiple light quads to be rendered on top of each other.
	SetRS(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	SetRS(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
	// Set up addition blending. The source is added to the destination.
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

	// All textures are rendered fullbright.
	globalColor = 0xffffffff;

	BeginScene();

	for(k=0; k<num; k++, rl++)
	{
		if(LIST_EMPTY(rl)) continue;
		initForReading(rl);
		flats = (*rl->data == rp_flat || rl->data[1] & RPF_MISSING_WALL);
		inited = DGL_FALSE;	// Only after the first dlit poly found.
		while(readPoly(&cq))
		{
			if(!(cq.flags & RPF_DLIT)) continue;
			// Has the state been initialized for this list?
			if(!inited) 
			{
				inited = DGL_TRUE;
				Bind(rl->tex);
			}
			if(flats) drawFlat(&cq, 1, 0); else drawWall(&cq, 1, 0);
		}
	}

	EndScene();

	// The old projection matrix.
	MatrixMode(DGL_PROJECTION);
	PopMatrix();

	// Restore the normal rendering state.
	SetRS(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	SetRS(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS);
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	unguard;
}

void RenderList(int format, int num, void *data)
{
	guard(RenderList);
	int	i;
	rendlist_t *rl = (rendlist_t *)data;
	rendlist_t **many = (rendlist_t **)data;

	switch(format)
	{
	case DGL_NORMAL_LIST:
		BeginScene();
		for(i=0; i<num; i++) renderList(rl+i);
		EndScene();
		break;

	case DGL_NORMAL_DLIT_LIST:
		renderDLitPass(rl, num);
		break;

	case DGL_MASKED_LIST:
		renderMaskedList(rl);
		break;

	case DGL_SKYMASK_LISTS:
		renderSkyMaskLists(many[0], many[1]);
		break;

	case DGL_LIGHT_LISTS:
		renderDynLightLists(many[0], many[1]);
		break;
	}		
	unguard;
}




