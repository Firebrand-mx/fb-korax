// drD3D.dll
// Direct3D driver for the Doomsday Graphics Library
//
// list.c : List rendering

#include "drD3D.h"

/*
// This is only for solid, non-masked primitives.
void renderList(rendlist_t *rl)
{
	int			i;
	float		tcleft, tcright, tctop, tcbottom;
	rendquad_t	*cq;
	vertex_t	vtx[4];

	if(!rl->numquads) return;	// The list is empty.

	// Bind the right texture.
	Bind(rl->tex);

	BeginScene();

	// Check what kind of primitives there are on the list.
	// There can only be one kind on each list.
	if(rl->quads->flags & RQF_FLAT)	// Check the first primitive.
	{
		// There's only triangles here, I see.				
		for(i=0; i<rl->numquads; i++)
		{
			cq = rl->quads+i;
			if(cq->flags & RQF_MISSING_WALL)
			{
				// This triangle is REALLY a quad that originally had no 
				// texture. We have to render it as two triangles.
				tcright = (tcleft=0) + cq->u.quad.len/cq->texw;
				tcbottom = (tctop=0) + (cq->top - cq->u.quad.bottom)/cq->texh;
				
				vtx[0].rgba = MAKERGB_A(cq->color[0].rgb, 0xff);
				vtx[0].s = tcleft;
				vtx[0].t = tctop;
				vtx[0].x = cq->v1[VX];
				vtx[0].y = cq->top;
				vtx[0].z = cq->v1[VY];

				vtx[1].rgba = MAKERGB_A(cq->color[1].rgb, 0xff);
				vtx[1].s = tcright;
				vtx[1].t = tctop;
				vtx[1].x = cq->v2[VX];
				vtx[1].y = cq->top;
				vtx[1].z = cq->v2[VY];

				vtx[2].rgba = vtx[1].rgba;
				vtx[2].s = tcright;
				vtx[2].t = tcbottom;
				vtx[2].x = cq->v2[VX];
				vtx[2].y = cq->u.quad.bottom;
				vtx[2].z = cq->v2[VY];

				vtx[3].rgba = vtx[0].rgba;
				vtx[3].s = tcleft;
				vtx[3].t = tcbottom;
				vtx[3].x = cq->v1[VX];
				vtx[3].y = cq->u.quad.bottom;
				vtx[3].z = cq->v1[VY];

				IDirect3DDevice3_DrawPrimitive(d3dDevice,
					D3DPT_TRIANGLEFAN, VERTEX_FORMAT,
					vtx, 4, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);

				continue;
			}

			// The vertices.
			vtx[0].rgba = MAKERGB_A(cq->color[0].rgb, 0xff);
			vtx[0].s = (cq->v1[VX]+cq->texoffx) / cq->texw;
			vtx[0].t = (-cq->v1[VY]-cq->texoffy) / cq->texh;
			vtx[0].x = cq->v1[VX];
			vtx[0].y = cq->top;
			vtx[0].z = cq->v1[VY];

			vtx[1].rgba = MAKERGB_A(cq->color[1].rgb, 0xff);
			vtx[1].s = (cq->v2[VX]+cq->texoffx) / cq->texw;
			vtx[1].t = (-cq->v2[VY]-cq->texoffy) / cq->texh;
			vtx[1].x = cq->v2[VX];
			vtx[1].y = cq->top;
			vtx[1].z = cq->v2[VY];

			vtx[2].rgba = MAKERGB_A(cq->color[2].rgb, 0xff);
			vtx[2].s = (cq->u.flat.v3[VX]+cq->texoffx) / cq->texw;
			vtx[2].t = (-cq->u.flat.v3[VY]-cq->texoffy) / cq->texh;
			vtx[2].x = cq->u.flat.v3[VX];
			vtx[2].y = cq->top;
			vtx[2].z = cq->u.flat.v3[VY];

			IDirect3DDevice3_DrawPrimitive(d3dDevice,
				D3DPT_TRIANGLELIST, VERTEX_FORMAT,
				vtx, 3, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
		}
	}
	else
	{
		// Render quads.
		for(i=0; i<rl->numquads; i++)
		{
			cq = rl->quads+i;

			// Calculate relative texture coordinates.
			tcright = (tcleft=cq->texoffx/(float)cq->texw) + cq->u.quad.len/cq->texw;
			tcbottom = (tctop=cq->texoffy/cq->texh) + (cq->top - cq->u.quad.bottom)/cq->texh;

			// The vertices.
			vtx[0].rgba = MAKERGB_A(cq->color[0].rgb, 0xff);
			vtx[0].s = tcleft;
			vtx[0].t = tctop;
			vtx[0].x = cq->v1[VX];
			vtx[0].y = cq->top;
			vtx[0].z = cq->v1[VY];

			vtx[1].rgba = MAKERGB_A(cq->color[1].rgb, 0xff);
			vtx[1].s = tcright;
			vtx[1].t = tctop;
			vtx[1].x = cq->v2[VX];
			vtx[1].y = cq->top;
			vtx[1].z = cq->v2[VY];

			vtx[2].rgba = vtx[1].rgba; 
			vtx[2].s = tcright;
			vtx[2].t = tcbottom;
			vtx[2].x = cq->v2[VX];
			vtx[2].y = cq->u.quad.bottom;
			vtx[2].z = cq->v2[VY];

			vtx[3].rgba = vtx[0].rgba; 
			vtx[3].s = tcleft;
			vtx[3].t = tcbottom;
			vtx[3].x = cq->v1[VX];
			vtx[3].y = cq->u.quad.bottom;
			vtx[3].z = cq->v1[VY];

			IDirect3DDevice3_DrawPrimitive(d3dDevice,
				D3DPT_TRIANGLEFAN, VERTEX_FORMAT,
				vtx, 4, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
		}
	}
	EndScene();
}

// Masked lists only include quads.
void renderMaskedList(rendlist_t *mrl)
{
	int			i;
	float		tcleft, tcright, tctop, tcbottom;
	rendquad_t	*cq;
	vertex_t	vtx[4];
	
	if(!mrl->numquads) return;	// No quads to render, I'm afraid.

	// Curtex is used to keep track of the current texture.
	// Zero also denotes that no glBegin() has yet been called.
	currentTexName = 0;

	BeginScene();

	// Render quads.
	for(i=mrl->numquads-1; i>=0; i--)	// Render back to front.
	{
		cq = mrl->quads+i;

		// Calculate relative texture coordinates.
		tcright = (tcleft=cq->texoffx/cq->texw) + cq->u.quad.len/cq->texw;
		tcbottom = (tctop=cq->texoffy/cq->texh) + (cq->top - cq->u.quad.bottom)/cq->texh;

		// Is there a need to change the texture?
		if(currentTexName != cq->masktex)	
		{
			Bind(cq->masktex);
		}
		
		// The vertices.
		vtx[0].rgba = MAKERGB_A(cq->color[0].rgb, 0xff);
		vtx[0].s = tcleft;
		vtx[0].t = tctop;
		vtx[0].x = cq->v1[VX];
		vtx[0].y = cq->top;
		vtx[0].z = cq->v1[VY];

		vtx[1].rgba = MAKERGB_A(cq->color[1].rgb, 0xff);
		vtx[1].s = tcright;
		vtx[1].t = tctop;
		vtx[1].x = cq->v2[VX];
		vtx[1].y = cq->top;
		vtx[1].z = cq->v2[VY];

		vtx[2].rgba = vtx[1].rgba; 
		vtx[2].s = tcright;
		vtx[2].t = tcbottom;
		vtx[2].x = cq->v2[VX];
		vtx[2].y = cq->u.quad.bottom;
		vtx[2].z = cq->v2[VY];

		vtx[3].rgba = vtx[0].rgba; 
		vtx[3].s = tcleft;
		vtx[3].t = tcbottom;
		vtx[3].x = cq->v1[VX];
		vtx[3].y = cq->u.quad.bottom;
		vtx[3].z = cq->v1[VY];

		IDirect3DDevice3_DrawPrimitive(d3dDevice,
			D3DPT_TRIANGLEFAN, VERTEX_FORMAT,
			vtx, 4, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
	}
	EndScene();
}

void renderSkyMaskLists(rendlist_t *smrl, rendlist_t *skyw)
{
	int			i;
	rendquad_t	*cq;
	vertex_t	vtx[4];

	if(!smrl->numquads && !skyw->numquads) return; // Nothing to do here.

	// Nothing gets written to the color buffer.
	// Instead of setting texture 0 we'll temporarily disable texturing.
	Disable(DGL_TEXTURING);
	// This will effectively disable color buffer writes.
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	for(i=0; i<4; i++) vtx[i].rgba = 0xffffffff;

	BeginScene();

	if(smrl->numquads)
	{
		for(i=0; i<smrl->numquads; i++)
		{
			cq = smrl->quads+i;
			// ONLY the vertices, please.
			vtx[0].x = cq->v1[VX];
			vtx[0].y = cq->top;
			vtx[0].z = cq->v1[VY];
			
			vtx[1].x = cq->v2[VX];
			vtx[1].y = cq->top;
			vtx[1].z = cq->v2[VY];

			vtx[2].x = cq->u.flat.v3[VX];
			vtx[2].y = cq->top;
			vtx[2].z = cq->u.flat.v3[VY];

			IDirect3DDevice3_DrawPrimitive(d3dDevice,
				D3DPT_TRIANGLELIST, VERTEX_FORMAT,
				vtx, 3, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
		}
	}

	// Then the walls.
	if(skyw->numquads)
	{
		for(i=0; i<skyw->numquads; i++)
		{
			cq = skyw->quads + i;
			// Only the verts.
			vtx[0].x = cq->v1[VX];
			vtx[0].y = cq->u.quad.bottom;
			vtx[0].z = cq->v1[VY];
			
			vtx[1].x = cq->v1[VX];
			vtx[1].y = cq->top;
			vtx[1].z = cq->v1[VY];

			vtx[2].x = cq->v2[VX];
			vtx[2].y = cq->top;
			vtx[2].z = cq->v2[VY];

			vtx[3].x = cq->v2[VX];
			vtx[3].y = cq->u.quad.bottom;
			vtx[3].z = cq->v2[VY];

			IDirect3DDevice3_DrawPrimitive(d3dDevice,
				D3DPT_TRIANGLEFAN, VERTEX_FORMAT,
				vtx, 4, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
		}
	}

	EndScene();

	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Enable(DGL_TEXTURING);
}	

void renderDynLightLists(rendlist_t *frl, rendlist_t *wrl)
{
	int			i;
	rendquad_t	*cq;
	vertex_t	vtx[4];

	if(!frl->numquads && !wrl->numquads) return; // Nothing to do.

	// Use a Z bias.
	MatrixMode(DGL_PROJECTION);
	PushMatrix();
	Scalef(.99f, .99f, .99f);

	// Setup the correct rendering state.	
	// Disable fog.
	if(usefog) SetRS(D3DRENDERSTATE_FOGENABLE, FALSE);

	// This'll allow multiple light quads to be rendered on top of each other.
	SetRS(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	SetRS(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);

	// Set up addition blending. The source is added to the destination.
	if(!dlBlend) // The normal setting.
	{
		SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR);
		SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	}
	else if(dlBlend == 1)
	{
		SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	}

	// The light texture.
	Bind(lightTex);

	BeginScene();

	// The flats.
	if(frl->numquads)
	{
		for(i=0; i<frl->numquads; i++)
		{
			cq = frl->quads + i;
			// Set the color.
			vtx[0].rgba = vtx[1].rgba = vtx[2].rgba = MAKERGB_A(cq->color[0].rgb, 0xff);
						
			// The vertices.			
			vtx[0].s = (cq->texoffx - cq->v1[VX]) / cq->texw;
			vtx[0].t = (cq->texoffy - cq->v1[VY]) / cq->texh;
			vtx[0].x = cq->v1[VX];
			vtx[0].y = cq->top;
			vtx[0].z = cq->v1[VY];

			vtx[1].s = (cq->texoffx - cq->v2[VX]) / cq->texw;
			vtx[1].t = (cq->texoffy - cq->v2[VY]) / cq->texh;
			vtx[1].x = cq->v2[VX];
			vtx[1].y = cq->top;
			vtx[1].z = cq->v2[VY];

			vtx[2].s = (cq->texoffx - cq->u.flat.v3[VX]) / cq->texw;
			vtx[2].t = (cq->texoffy - cq->u.flat.v3[VY]) / cq->texh;
			vtx[2].x = cq->u.flat.v3[VX];
			vtx[2].y = cq->top;
			vtx[2].z = cq->u.flat.v3[VY];

			IDirect3DDevice3_DrawPrimitive(d3dDevice,
				D3DPT_TRIANGLELIST, VERTEX_FORMAT,
				vtx, 3, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
		}
	}	

	// The walls.
	if(wrl->numquads)
	{
		float tctl[2], tcbr[2];	// Top left and bottom right.
		for(i=0; i<wrl->numquads; i++)
		{
			cq = wrl->quads + i;
			// Set the color.
			vtx[0].rgba = vtx[1].rgba = vtx[2].rgba = vtx[3].rgba = 
				MAKERGB_A(cq->color[0].rgb, 0xff);

			// Calculate the texture coordinates.
			tcbr[VX] = (tctl[VX]=-cq->texoffx/cq->texw) + cq->u.quad.len/cq->texw;
			tcbr[VY] = (tctl[VY]=cq->texoffy/cq->texh) + (cq->top - cq->u.quad.bottom)/cq->texh;

			// The vertices.
			vtx[0].s = tctl[VX];
			vtx[0].t = tcbr[VY];
			vtx[0].x = cq->v1[VX];
			vtx[0].y = cq->u.quad.bottom;
			vtx[0].z = cq->v1[VY];

			vtx[1].s = tctl[VX];
			vtx[1].t = tctl[VY];
			vtx[1].x = cq->v1[VX];
			vtx[1].y = cq->top;
			vtx[1].z = cq->v1[VY];

			vtx[2].s = tcbr[VX];
			vtx[2].t = tctl[VY];
			vtx[2].x = cq->v2[VX];
			vtx[2].y = cq->top;
			vtx[2].z = cq->v2[VY];

			vtx[3].s = tcbr[VX];
			vtx[3].t = tcbr[VY];
			vtx[3].x = cq->v2[VX];
			vtx[3].y = cq->u.quad.bottom;
			vtx[3].z = cq->v2[VY];

			IDirect3DDevice3_DrawPrimitive(d3dDevice,
				D3DPT_TRIANGLEFAN, VERTEX_FORMAT,
				vtx, 4, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS);
		}
	}

	EndScene();

	// Restore the normal rendering state.
	if(usefog) SetRS(D3DRENDERSTATE_FOGENABLE, TRUE);
	SetRS(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	SetRS(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS);
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

	PopMatrix();
}
*/

#define INLINE				__inline
#define LIST_EMPTY(rl)		(!(rl)->data || !*(rl)->data)

rendlist_t *readlist;

// This is used by drawWall() and drawFlat() when !col.
int globalColor = 0;

// Undone with PopMatrix().
void setupZBias(int level)
{
	float sc = .99f - level*.01f;

	MatrixMode(DGL_PROJECTION);
	PushMatrix();
	Scalef(sc, sc, sc);
}

static void drawWall(rendpoly_t *cq, byte tex, byte col)
{
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
}

static void drawFlat(rendpoly_t *cq, byte tex, byte col)
{
	vertex_t vtx[DGL_MAX_POLY_SIDES];
	rendpoly_vertex_t *rpVtx = cq->vertices;
	int i;

	if(cq->flags & RPF_MISSING_WALL)
	{
/*		float tcleft = 0, tctop = 0;
		float tcright, tcbottom, *v1, *v2;

		// This poly is REALLY a quad that originally had no texture. 
		// Render it as two triangles.
		if(tex)
		{
			tcright = cq->length/cq->texw;
			tcbottom = (cq->top - cq->bottom)/cq->texh;
		}

		v1 = cq->vertices[0].pos;
		v2 = cq->vertices[1].pos;
		
		if(col) glColor3ubv(cq->vertices[0].color.rgb);
		if(tex) glTexCoord2f(tcleft, tctop);
		glVertex3f(v1[VX], cq->top, v1[VY]);
		
		if(col) glColor3ubv(cq->vertices[1].color.rgb);
		if(tex) glTexCoord2f(tcright, tctop);
		glVertex3f(v2[VX], cq->top, v2[VY]);
		
		if(tex) glTexCoord2f(tcright, tcbottom);
		glVertex3f(v2[VX], cq->bottom, v2[VY]);
		
		// The other triangle.
		if(tex) glTexCoord2f(tcright, tcbottom);
		glVertex3f(v2[VX], cq->bottom, v2[VY]);
		
		if(col) glColor3ubv(cq->vertices[0].color.rgb);
		if(tex) glTexCoord2f(tcleft, tcbottom);
		glVertex3f(v1[VX], cq->bottom, v1[VY]);
		
		if(tex) glTexCoord2f(tcleft, tctop);
		glVertex3f(v1[VX], cq->top, v1[VY]);

		//drawWall(cq, tex, col);*/

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
}

void initForReading(rendlist_t *li)
{
	readlist = li;
	readlist->cursor = readlist->data;
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
	int i, c, temp;
	rendpoly_vertex_t *vtx;
	
	poly->type = readByte();
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
			if(poly->flags & RPF_GLOW) 
			{
				/*temp = (int) (temp*1.5);
				if(temp < 32) temp = 32;
				if(temp > 255) temp = 255;*/
				temp = 255;
			}
			vtx->color.rgb[c] = temp;
		}
	}
	return DGL_TRUE;
}

// This is only for solid, non-masked primitives.
void renderList(rendlist_t *rl)
{
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

	/*glBegin(flats? GL_TRIANGLES : GL_QUADS);
	for(i=0, cq=rl->quads; i<rl->numquads; i++, cq++)
	{
		if(!dlBlend && cq->flags & RQF_DLIT) 
		{
			dlight = DGL_TRUE;
			continue;
		}
		if(flats) drawFlat(cq, 1, 1); else drawWall(cq, 1, 1);
	}
	glEnd();*/

	//glBegin(flats? GL_TRIANGLES : GL_QUADS);

	while(readPoly(&cq))
	{
		if(!dlBlend && cq.flags & RPF_DLIT && !usefog)
		{
			dlight = DGL_TRUE;
			continue;
		}
		if(flats) drawFlat(&cq, 1, 1); else drawWall(&cq, 1, 1);
	}
	//glEnd();

	// Need to draw some dlit polys?
	if(dlight)
	{
		initForReading(rl);
		/*GLfloat mat[16];
		// Disable textures during this.
		setupZBias(mat, 0);
		glDepthFunc(GL_LEQUAL);*/
		//glDisable(GL_TEXTURE_2D);
		Disable(DGL_TEXTURING);
		//glBegin(flats? GL_TRIANGLES : GL_QUADS);
		while(readPoly(&cq))
		{
			if(!(cq.flags & RPF_DLIT)) continue;
			if(flats) drawFlat(&cq, 0, 1); else drawWall(&cq, 0, 1);
		}
		//glEnd();
		//glEnable(GL_TEXTURE_2D);
		Enable(DGL_TEXTURING);
		/*glDepthFunc(GL_LESS);
		glLoadMatrixf(mat);*/
	}

	// Enable alpha blending once more.
	//glEnable(GL_BLEND);
	Enable(DGL_BLENDING);
}

// Masked lists only include quads.
void renderMaskedList(rendlist_t *mrl)
{
//	int			i;
//	float		tcleft, tcright, tctop, tcbottom;
	rendpoly_t	cq;
	DGLuint		currentTex;
	
	//if(!mrl->numquads) return;	// No quads to render, I'm afraid.

	if(LIST_EMPTY(mrl)) return;

	initForReading(mrl);

	// Curtex is used to keep track of the current texture.
	// Zero also denotes that no glBegin() has yet been called.
	currentTex = 0;

	// Render quads.
	//for(i=mrl->numquads-1; i>=0; i--)	// Render back to front.
	BeginScene();

	while(readPoly(&cq))
	{
		//cq = mrl->quads+i;

		// Is there a need to change the texture?
		if(currentTex != cq.tex)	
		{
			//if(currentTex) glEnd();	// Finish with the old texture.
			//glBindTexture(GL_TEXTURE_2D, currentTex=cq.tex);
			Bind(currentTex = cq.tex);
			//glBegin(GL_QUADS);	
		}

/*		// Calculate relative texture coordinates.
		tcright = (tcleft=cq->texoffx/cq->texw) + cq->u.quad.len/cq->texw;
		tcbottom = (tctop=cq->texoffy/cq->texh) + (cq->top - cq->u.quad.bottom)/cq->texh;
		
		// The vertices.
		glColor3ubv(cq->color[0].rgb);
		glTexCoord2f(tcleft, tcbottom);
		glVertex3f(cq->v1[VX], cq->u.quad.bottom, cq->v1[VY]);

		glTexCoord2f(tcleft, tctop);
		glVertex3f(cq->v1[VX], cq->top, cq->v1[VY]);

		glColor3ubv(cq->color[1].rgb);
		glTexCoord2f(tcright, tctop);
		glVertex3f(cq->v2[VX], cq->top, cq->v2[VY]);

		glTexCoord2f(tcright, tcbottom);
		glVertex3f(cq->v2[VX], cq->u.quad.bottom, cq->v2[VY]);*/

		drawWall(&cq, DGL_TRUE, DGL_TRUE);
	}
	//if(currentTex) glEnd();	// If something was drawn, finish with it.
	EndScene();
}

void renderSkyMaskLists(rendlist_t *smrl, rendlist_t *skyw)
{
//	int			i;
	rendpoly_t	cq;

	if(LIST_EMPTY(smrl) && LIST_EMPTY(skyw)) return; // Nothing to do here.

	// Nothing gets written to the color buffer.
	//glBindTexture(GL_TEXTURE_2D, 0);
	// Instead of setting texture 0 we'll temporarily disable texturing.
	//glDisable(GL_TEXTURE_2D);
	Disable(DGL_TEXTURING);
	// This will effectively disable color buffer writes.
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	if(!LIST_EMPTY(smrl))
	{
		initForReading(smrl);
		//glBegin(GL_TRIANGLES);

		BeginScene();
		
		//for(i=0, cq=smrl->quads; i<smrl->numquads; i++, cq++)
		while(readPoly(&cq))
		{
			// ONLY the vertices, please.
			/*glVertex3f(cq->v1[VX], cq->top, cq->v1[VY]);
			glVertex3f(cq->v2[VX], cq->top, cq->v2[VY]);
			glVertex3f(cq->u.flat.v3[VX], cq->top, cq->u.flat.v3[VY]);*/

			drawFlat(&cq, DGL_FALSE, DGL_FALSE);
		}
		//glEnd();		
		EndScene();
	}
	
	// Then the walls.
	if(!LIST_EMPTY(skyw))
	{
		initForReading(skyw);
		//glBegin(GL_QUADS);
		BeginScene();
		//for(i=0, cq=skyw->quads; i<skyw->numquads; i++, cq++)
		while(readPoly(&cq))
		{
			// Only the verts.
			/*glVertex3f(cq->v1[VX], cq->u.quad.bottom, cq->v1[VY]);
			glVertex3f(cq->v1[VX], cq->top, cq->v1[VY]);
			glVertex3f(cq->v2[VX], cq->top, cq->v2[VY]);
			glVertex3f(cq->v2[VX], cq->u.quad.bottom, cq->v2[VY]);*/

			drawWall(&cq, DGL_FALSE, DGL_FALSE);
		}
		EndScene();
		//glEnd();
	}
	
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	
	//glEnable(GL_TEXTURE_2D);
	Enable(DGL_TEXTURING);
}

void renderDynLightLists(rendlist_t *frl, rendlist_t *wrl)
{
//	int			i;
	rendpoly_t	cq;
	//GLfloat		projmat[16];

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
		//glBegin(GL_TRIANGLES);
		//for(i=0, cq=frl->quads; i<frl->numquads; i++, cq++)
		while(readPoly(&cq))
		{
			// Set the color.
			//glColor3ubv(cq.vertices[0].color.rgb);
			globalColor = MAKERGB_A(cq.vertices[0].color.rgb, 0xff);

/*			// The vertices.			
			glTexCoord2f((cq.texoffx - cq.vertices[0].pos[VX])/cq.texw,
				(cq.texoffy - cq.vertices[0].pos[VY])/cq.texh);
			glVertex3f(cq.vertices[0].pos[VX], cq.top, cq.vertices[0].pos[VY]);

			glTexCoord2f((cq.texoffx - cq.vertices[1].pos[VX])/cq.texw,
				(cq.texoffy - cq.vertices[1].pos[VY])/cq.texh);
			glVertex3f(cq.vertices[1].pos[VX], cq.top, cq.vertices[1].pos[VY]);

			glTexCoord2f((cq.texoffx - cq.vertices[2].pos[VX])/cq.texw,
				(cq.texoffy - cq.vertices[2].pos[VY])/cq.texh);
			glVertex3f(cq.vertices[2].pos[VX], cq.top, cq.vertices[2].pos[VY]);*/

			drawFlat(&cq, DGL_TRUE, DGL_FALSE);
		}
		//glEnd();
	}	

	// The walls.
	if(!LIST_EMPTY(wrl))
	{
//		float tctl[2], tcbr[2];	// Top left and bottom right.
		initForReading(wrl);
		//glBegin(GL_QUADS);
		//for(i=0, cq=wrl->quads; i<wrl->numquads; i++, cq++)

		while(readPoly(&cq))
		{
			// Set the color.
			globalColor = MAKERGB_A(cq.vertices[0].color.rgb, 0xff);

			// Calculate the texture coordinates.
			/*tcbr[VX] = (tctl[VX]=-cq.texoffx/cq.texw) + cq.length/cq.texw;
			tcbr[VY] = (tctl[VY]=cq.texoffy/cq.texh) + (cq.top - cq.bottom)/cq.texh;

			// The vertices.
			glTexCoord2f(tctl[VX], tcbr[VY]);
			glVertex3f(cq.vertices[0].pos[VX], cq.bottom, cq.vertices[0].pos[VY]);

			glTexCoord2f(tctl[VX], tctl[VY]);
			glVertex3f(cq.vertices[0].pos[VX], cq.top, cq.vertices[0].pos[VY]);

			glTexCoord2f(tcbr[VX], tctl[VY]);
			glVertex3f(cq.vertices[1].pos[VX], cq.top, cq.vertices[1].pos[VY]);

			glTexCoord2f(tcbr[VX], tcbr[VY]);
			glVertex3f(cq.vertices[1].pos[VX], cq.bottom, cq.vertices[1].pos[VY]);*/

			cq.texoffx = -cq.texoffx;

			drawWall(&cq, DGL_TRUE, DGL_FALSE);

			//drawWall(&cq, DGL_TRUE, DGL_FALSE);
		}
		//glEnd();
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
}

void renderDLitPass(rendlist_t *rl, int num)
{
	int			k;//, i;
//	GLfloat		fogColor[4];
	rendpoly_t	cq;
	char		flats, inited;
	//GLfloat		projmat[16];

	// Setup the correct rendering state.	
	if(usefog) return;

	/*
	{
		// Set fog color to black.
		//glGetFloatv(GL_FOG_COLOR, fogColor);
		//glDisable(GL_FOG);
	//	return;
	}*/

	// Adjust the projection matrix for Z bias.
	setupZBias(0);

	// This'll allow multiple light quads to be rendered on top of each other.
	//glDepthMask(GL_FALSE);	// No Z buffer writes.
	//glDepthFunc(GL_LEQUAL);	
	SetRS(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	SetRS(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
	// Set up addition blending. The source is added to the destination.
	//glBlendFunc(GL_DST_COLOR, GL_ZERO);
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

	// All textures are rendered fullbright.
	//glColor3f(1, 1, 1);
	globalColor = 0xffffffff;

	BeginScene();

	for(k=0; k<num; k++, rl++)
	{
		if(LIST_EMPTY(rl)) continue;
		initForReading(rl);
		flats = (*rl->data == rp_flat || rl->data[1] & RPF_MISSING_WALL);
		inited = DGL_FALSE;	// Only after the first dlit poly found.
		//for(i=0, cq=rl->quads; i<rl->numquads; i++, cq++)
		while(readPoly(&cq))
		{
			if(!(cq.flags & RPF_DLIT)) continue;
			// Has the state been initialized for this list?
			if(!inited) 
			{
				inited = DGL_TRUE;
				//glBindTexture(GL_TEXTURE_2D, currentTex=rl->tex);
				Bind(rl->tex);
				//glBegin(flats? GL_TRIANGLES : GL_QUADS);
			}
			if(flats) drawFlat(&cq, 1, 0); else drawWall(&cq, 1, 0);
		}
		// "Just dip it once, and END IT."
		//if(inited) glEnd();
	}

	EndScene();

	// The old projection matrix.
	//glLoadMatrixf(projmat);
	MatrixMode(DGL_PROJECTION);
	PopMatrix();

	// Restore the normal rendering state.
/*	if(usefog) 
	{
		// Restore old fog color.
		//glFogfv(GL_FOG_COLOR, fogColor);

		glEnable(GL_FOG);
	}*/
	/*glDepthMask(GL_TRUE); 
	glDepthFunc(GL_LESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

	SetRS(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	SetRS(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS);
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void RenderList(int format, int num, void *data)
{
	int	i;
	rendlist_t *rl = data;
	rendlist_t **many = data;

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
}




