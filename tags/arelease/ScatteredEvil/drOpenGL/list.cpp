// drOpenGL.dll
// The Doomsday graphics library driver for OpenGL
//
// list.c : List rendering

#include "drOpenGL.h"

#define INLINE				__inline
#define LIST_EMPTY(rl)		(!(rl)->data || !*(rl)->data)

rendlist_t *readlist;

void setupZBias(GLfloat *mat, int level)
{
	float sc = .99f - level*.01f;

	glGetFloatv(GL_PROJECTION_MATRIX, mat);
	glMatrixMode(GL_PROJECTION);
	glScalef(sc, sc, sc);
}

static void drawWall(rendpoly_t *cq, byte tex, byte col)
{
	float tcleft, tcright, tctop, tcbottom;
	
	// Calculate relative texture coordinates.
	if(tex)
	{
		tcright = (tcleft=cq->texoffx/(float)cq->texw) + cq->length/cq->texw;
		tcbottom = (tctop=cq->texoffy/cq->texh) + (cq->top - cq->bottom)/cq->texh;
	}
	
	// The vertices.
	if(col) glColor3ubv(cq->vertices[0].color.rgb);
	if(tex) glTexCoord2f(tcleft, tcbottom);
	glVertex3f(cq->vertices[0].pos[VX], cq->bottom, cq->vertices[0].pos[VY]);
	
	if(tex) glTexCoord2f(tcleft, tctop);
	glVertex3f(cq->vertices[0].pos[VX], cq->top, cq->vertices[0].pos[VY]);
	
	if(col) glColor3ubv(cq->vertices[1].color.rgb);
	if(tex) glTexCoord2f(tcright, tctop);
	glVertex3f(cq->vertices[1].pos[VX], cq->top, cq->vertices[1].pos[VY]);
	
	if(tex) glTexCoord2f(tcright, tcbottom);
	glVertex3f(cq->vertices[1].pos[VX], cq->bottom, cq->vertices[1].pos[VY]);
}

static void drawFlat(rendpoly_t *cq, byte tex, byte col)
{
	if(cq->flags & RPF_MISSING_WALL)
	{
		float tcleft = 0, tctop = 0;
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

		//drawWall(cq, tex, col);
	}
	else if(cq->flags & RPF_LIGHT)
	{
#define FLATVTX_L(vtx)	glTexCoord2f((cq->texoffx - (vtx)->pos[VX])/cq->texw, \
							(cq->texoffy - (vtx)->pos[VY])/cq->texh); \
						glVertex3f((vtx)->pos[VX], cq->top, (vtx)->pos[VY]);

		rendpoly_vertex_t *first = cq->vertices, *vtx = first+1;
		int i = 1;

		// Render the polygon as a fan of triangles.
		for(; i<cq->numvertices-1; i++)
		{
			// All the triangles share the first vertex.
			FLATVTX_L(first);
			FLATVTX_L(vtx);
			vtx++;
			FLATVTX_L(vtx);
		}
	}
	else
	{
#define FLATVTX(vtx)	if(col) glColor3ubv((vtx)->color.rgb); \
						if(tex) glTexCoord2f(((vtx)->pos[VX]+cq->texoffx)/cq->texw, \
							(-(vtx)->pos[VY]-cq->texoffy)/cq->texh); \
						glVertex3f((vtx)->pos[VX], cq->top, (vtx)->pos[VY]);

		rendpoly_vertex_t *first = cq->vertices, *vtx = first+1;
		int i = 1;

		// Render the polygon as a fan of triangles.
		for(; i<cq->numvertices-1; i++)
		{
			// All the triangles share the first vertex.
			FLATVTX(first);
			FLATVTX(vtx);
			vtx++;
			FLATVTX(vtx);
		}

/*		rendpoly_vertex_t *vtx = cq->vertices;
		int i;
		for(i=0; i<cq->numvertices; i++, vtx++)
		{
			FLATVTX(vtx);
		}*/
	}
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
	glDisable(GL_BLEND);	

	// Bind the right texture.
	glBindTexture(GL_TEXTURE_2D, currentTex=rl->tex);

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

	glBegin(flats? GL_TRIANGLES : GL_QUADS);
	while(readPoly(&cq))
	{
		if(!dlBlend && cq.flags & RPF_DLIT && !usefog)
		{
			dlight = DGL_TRUE;
			continue;
		}
		if(flats) drawFlat(&cq, 1, 1); else drawWall(&cq, 1, 1);
	}
	glEnd();

	// Need to draw some dlit polys?
	if(dlight)
	{
		initForReading(rl);
		/*GLfloat mat[16];
		// Disable textures during this.
		setupZBias(mat, 0);
		glDepthFunc(GL_LEQUAL);*/
		glDisable(GL_TEXTURE_2D);
		glBegin(flats? GL_TRIANGLES : GL_QUADS);
		while(readPoly(&cq))
		{
			if(!(cq.flags & RPF_DLIT)) continue;
			if(flats) drawFlat(&cq, 0, 1); else drawWall(&cq, 0, 1);
		}
		glEnd();
		glEnable(GL_TEXTURE_2D);
		/*glDepthFunc(GL_LESS);
		glLoadMatrixf(mat);*/
	}

	// Enable alpha blending once more.
	glEnable(GL_BLEND);
}

// Masked lists only include quads.
void renderMaskedList(rendlist_t *mrl)
{
//	int			i;
//	float		tcleft, tcright, tctop, tcbottom;
	rendpoly_t	cq;
	
	//if(!mrl->numquads) return;	// No quads to render, I'm afraid.

	if(LIST_EMPTY(mrl)) return;

	initForReading(mrl);

	// Curtex is used to keep track of the current texture.
	// Zero also denotes that no glBegin() has yet been called.
	currentTex = 0;

	// Render quads.
	//for(i=mrl->numquads-1; i>=0; i--)	// Render back to front.
	while(readPoly(&cq))
	{
		//cq = mrl->quads+i;

		// Is there a need to change the texture?
		if(currentTex != cq.tex)	
		{
			if(currentTex) glEnd();	// Finish with the old texture.
			glBindTexture(GL_TEXTURE_2D, currentTex=cq.tex);
			glBegin(GL_QUADS);	
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
	if(currentTex) glEnd();	// If something was drawn, finish with it.
}

void renderSkyMaskLists(rendlist_t *smrl, rendlist_t *skyw)
{
//	int			i;
	rendpoly_t	cq;

	if(LIST_EMPTY(smrl) && LIST_EMPTY(skyw)) return; // Nothing to do here.

	// Nothing gets written to the color buffer.
	//glBindTexture(GL_TEXTURE_2D, 0);
	// Instead of setting texture 0 we'll temporarily disable texturing.
	glDisable(GL_TEXTURE_2D);
	// This will effectively disable color buffer writes.
	glBlendFunc(GL_ZERO, GL_ONE);

	if(!LIST_EMPTY(smrl))
	{
		initForReading(smrl);
		glBegin(GL_TRIANGLES);
		//for(i=0, cq=smrl->quads; i<smrl->numquads; i++, cq++)
		while(readPoly(&cq))
		{
			// ONLY the vertices, please.
			/*glVertex3f(cq->v1[VX], cq->top, cq->v1[VY]);
			glVertex3f(cq->v2[VX], cq->top, cq->v2[VY]);
			glVertex3f(cq->u.flat.v3[VX], cq->top, cq->u.flat.v3[VY]);*/

			drawFlat(&cq, DGL_FALSE, DGL_FALSE);
		}
		glEnd();		
	}
	
	// Then the walls.
	if(!LIST_EMPTY(skyw))
	{
		initForReading(skyw);
		glBegin(GL_QUADS);
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
		glEnd();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
}

void renderDynLightLists(rendlist_t *frl, rendlist_t *wrl)
{
//	int			i;
	rendpoly_t	cq;
	GLfloat		projmat[16];

	if(LIST_EMPTY(frl) && LIST_EMPTY(wrl)) return; // Nothing to do.

	// Adjust the projection matrix for Z bias.
	setupZBias(projmat, 0);

	// Setup the correct rendering state.	
	// Disable fog.
	if(usefog) glDisable(GL_FOG);

	// This'll allow multiple light quads to be rendered on top of each other.
	glDepthMask(GL_FALSE); 
	glDepthFunc(GL_LEQUAL);
	// Set up addition blending. The source is added to the destination.
	glBlendFunc(GL_ONE, GL_ONE);

	// The light texture.
	glBindTexture(GL_TEXTURE_2D, currentTex = lightTex);

	// The flats.
	if(!LIST_EMPTY(frl))
	{
		initForReading(frl);
		glBegin(GL_TRIANGLES);
		//for(i=0, cq=frl->quads; i<frl->numquads; i++, cq++)
		while(readPoly(&cq))
		{
			// Set the color.
			glColor3ubv(cq.vertices[0].color.rgb);

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
		glEnd();
	}	

	// The walls.
	if(!LIST_EMPTY(wrl))
	{
		float tctl[2], tcbr[2];	// Top left and bottom right.
		initForReading(wrl);
		glBegin(GL_QUADS);
		//for(i=0, cq=wrl->quads; i<wrl->numquads; i++, cq++)

		while(readPoly(&cq))
		{
			// Set the color.
			glColor3ubv(cq.vertices[0].color.rgb);

			// Calculate the texture coordinates.
			tcbr[VX] = (tctl[VX]=-cq.texoffx/cq.texw) + cq.length/cq.texw;
			tcbr[VY] = (tctl[VY]=cq.texoffy/cq.texh) + (cq.top - cq.bottom)/cq.texh;

			// The vertices.
			glTexCoord2f(tctl[VX], tcbr[VY]);
			glVertex3f(cq.vertices[0].pos[VX], cq.bottom, cq.vertices[0].pos[VY]);

			glTexCoord2f(tctl[VX], tctl[VY]);
			glVertex3f(cq.vertices[0].pos[VX], cq.top, cq.vertices[0].pos[VY]);

			glTexCoord2f(tcbr[VX], tctl[VY]);
			glVertex3f(cq.vertices[1].pos[VX], cq.top, cq.vertices[1].pos[VY]);

			glTexCoord2f(tcbr[VX], tcbr[VY]);
			glVertex3f(cq.vertices[1].pos[VX], cq.bottom, cq.vertices[1].pos[VY]);

			//drawWall(&cq, DGL_TRUE, DGL_FALSE);
		}
		glEnd();
	}

	glLoadMatrixf(projmat);

	// Restore the normal rendering state.
	if(usefog) glEnable(GL_FOG);
	glDepthMask(GL_TRUE); 
	glDepthFunc(GL_LESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void renderDLitPass(rendlist_t *rl, int num)
{
	int			k;//, i;
//	GLfloat		fogColor[4];
	rendpoly_t	cq;
	char		flats, inited;
	GLfloat		projmat[16];

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
	setupZBias(projmat, 0);

	// This'll allow multiple light quads to be rendered on top of each other.
	glDepthMask(GL_FALSE);	// No Z buffer writes.
	glDepthFunc(GL_LEQUAL);	
	// Set up addition blending. The source is added to the destination.
	glBlendFunc(GL_DST_COLOR, GL_ZERO);

	// All textures are rendered fullbright.
	glColor3f(1, 1, 1);

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
				glBindTexture(GL_TEXTURE_2D, currentTex=rl->tex);
				glBegin(flats? GL_TRIANGLES : GL_QUADS);
			}
			if(flats) drawFlat(&cq, 1, 0); else drawWall(&cq, 1, 0);
		}
		// "Just dip it once, and END IT."
		if(inited) glEnd();
	}

	// The old projection matrix.
	glLoadMatrixf(projmat);

	// Restore the normal rendering state.
/*	if(usefog) 
	{
		// Restore old fog color.
		//glFogfv(GL_FOG_COLOR, fogColor);

		glEnable(GL_FOG);
	}*/
	glDepthMask(GL_TRUE); 
	glDepthFunc(GL_LESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}