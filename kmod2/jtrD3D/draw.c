// drD3D.dll
// Direct3D driver for the Doomsday Graphics Library
//
// draw.c : Drawing operations, vertex handling

#include "drD3D.h"
#include <stdio.h>


int			primType = DGL_FALSE;	// Set in Begin() if the stack is needed.
int			primMode = DGL_FALSE;	// Begin() sets this always.
vertex_t	vertexStack[4];
int			vtxStackPos = 0, vtxCounter = 0;
vertex_t	currentVertex;		
int			inSequence = DGL_FALSE;



// ------------------------------------------------------------------------

static void uploadVertex()
{
	if(primType != DGL_QUADS)
	{
		// The normal Begin/End system is in use.
		if(FAILED(hr = IDirect3DDevice3_Vertex(d3dDevice, &currentVertex)))
			gim.Error("uploadVtx failed.\n");
		vtxCounter++;
	}
	else
	{
		// Quads are rendered using DrawPrimitive.
		// The vertex stack is used to store the previously entered
		// vertices. When the stack is full, the quad is drawn.
		memcpy(vertexStack + vtxStackPos, &currentVertex, sizeof(vertex_t));
		if(vtxStackPos == 3)
		{
			// Now the stack is full.
			if(FAILED(hr = IDirect3DDevice3_DrawPrimitive(d3dDevice,
				D3DPT_TRIANGLEFAN, VERTEX_FORMAT,
				vertexStack, 4, D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS)))
				gim.Error("uploadVtx (DrawPrimitive) failed.\n");
			// Clear the stack.
			vtxStackPos = 0;
		}
		else
		{
			// Move on to the next stack position.
			vtxStackPos++;
		}
	}
}

// API FUNCTIONS ----------------------------------------------------------

void Color3ub(DGLubyte r, DGLubyte g, DGLubyte b)
{
	currentVertex.rgba = MAKE_RGBA(r, g, b, 0xff);
}


void Color3ubv(void *data)
{
	DGLubyte *ubv = data;
	currentVertex.rgba = MAKE_RGBA(ubv[0], ubv[1], ubv[2], 0xff);
}


void Color4ub(DGLubyte r, DGLubyte g, DGLubyte b, DGLubyte a)
{
	currentVertex.rgba = MAKE_RGBA(r, g, b, a);
}


void Color4ubv(void *data)
{
	DGLubyte *ubv = data;
	currentVertex.rgba = MAKE_RGBA(ubv[0], ubv[1], ubv[2], ubv[3]);
}


void Color3f(float r, float g, float b)
{
	DGLubyte col[3];

	CLAMP01(r);	
	CLAMP01(g);
	CLAMP01(b);
	col[0] = (DGLubyte) (0xff * r);
	col[1] = (DGLubyte) (0xff * g);
	col[2] = (DGLubyte) (0xff * b);
	currentVertex.rgba = MAKE_RGBA(col[0], col[1], col[2], 0xff);
}


void Color3fv(float *data)
{
	float		clamped[3] = { data[0], data[1], data[2] };
	DGLubyte	col[3];
	int			i;

	for(i=0; i<3; i++)
	{
		CLAMP01(clamped[i]);
		col[i] = (DGLubyte) (0xff * clamped[i]);
	}
	currentVertex.rgba = MAKE_RGBA(col[0], col[1], col[2], 0xff);
}


void Color4f(float r, float g, float b, float a)
{
	DGLubyte col[4];

	CLAMP01(r);	
	CLAMP01(g);
	CLAMP01(b);
	CLAMP01(a);
	col[0] = (DGLubyte) (0xff * r);
	col[1] = (DGLubyte) (0xff * g);
	col[2] = (DGLubyte) (0xff * b);
	col[3] = (DGLubyte) (0xff * a);
	currentVertex.rgba = MAKE_RGBA(col[0], col[1], col[2], col[3]);
}


void Color4fv(float *data)
{
	float		clamped[4] = { data[0], data[1], data[2], data[3] };
	DGLubyte	col[4];
	int			i;

	for(i=0; i<4; i++)
	{
		CLAMP01(clamped[i]);
		col[i] = (DGLubyte) (0xff * clamped[i]);
	}
	currentVertex.rgba = MAKE_RGBA(col[0], col[1], col[2], col[3]);
}


void TexCoord2f(float s, float t)
{
	float	vec[3] = { s, t, 0 };

	// We must apply the current texture matrix on the coordinates.
	vecMatMul(vec, getTexMatrix(), vec);

	currentVertex.s = vec[0];
	currentVertex.t = vec[1];
}


void TexCoord2fv(float *data)
{
	float	vec[3] = { data[0], data[1], 0 };

	// We must apply the current texture matrix on the coordinates.
	vecMatMul(vec, getTexMatrix(), vec);

	currentVertex.s = vec[0];
	currentVertex.t = vec[1];
}


int	BeginScene(void)
{
	if(FAILED(hr = IDirect3DDevice3_BeginScene(d3dDevice)))
	{
		gim.Message( "drD3D.BeginScene: Failure! Error: %x\n", hr);
		return DGL_ERROR;
	}
	return DGL_OK;
}


int	EndScene(void)
{
	if(FAILED(hr = IDirect3DDevice3_EndScene(d3dDevice)))
	{
		gim.Message( "drD3D.EndScene: Failure! Error: %x\n", hr);
		return DGL_ERROR;
	}
	return DGL_OK;
}


void Begin(int mode)
{
	D3DPRIMITIVETYPE prim = (mode == DGL_LINES? D3DPT_LINELIST
		: mode == DGL_TRIANGLES? D3DPT_TRIANGLELIST
		: mode == DGL_TRIANGLE_FAN? D3DPT_TRIANGLEFAN
		: D3DPT_TRIANGLESTRIP);

	if(mode == DGL_SEQUENCE)
	{
		inSequence = DGL_TRUE;
		BeginScene();
		return;
	}

	primType = DGL_FALSE;
	primMode = prim;
	vtxCounter = 0;
	vtxStackPos = 0;

	if(!inSequence) BeginScene();

	if(mode == DGL_QUADS || mode == DGL_QUAD_STRIP)
	{
		// Setting primType will tell the vertex loader what to do
		// in these cases.
		primType = mode;
		if(mode == DGL_QUADS) return;	// We'll be using DrawPrimitive.
	}
	if(FAILED(hr = IDirect3DDevice3_Begin(d3dDevice, prim, VERTEX_FORMAT, 
		D3DDP_DONOTLIGHT | D3DDP_DONOTUPDATEEXTENTS)))
		gim.Message( "drD3D.Begin: Failure! Error: %x\n", hr);
}


void End(void)
{
	if(primMode == DGL_FALSE)
	{
		if(inSequence)
		{
			// Sequence ends.
			inSequence = DGL_FALSE;
			EndScene();
		}
		return;
	}

	if(primType != DGL_QUADS)
	{
		if(FAILED(hr = IDirect3DDevice3_End(d3dDevice, 0)))
			gim.Message( "drD3D.End: Failure! Error: %x\n", hr);
	}
	if(!inSequence)	EndScene();

	primMode = primType = DGL_FALSE;
}


void Vertex2f(float x, float y)
{
	currentVertex.x = x;
	currentVertex.y = y;
	currentVertex.z = 0;
	uploadVertex();
}


void Vertex2fv(float *data)
{
	currentVertex.x = data[0];
	currentVertex.y = data[1];
	currentVertex.z = 0;
	uploadVertex();
}


void Vertex3f(float x, float y, float z)
{
	currentVertex.x = x;
	currentVertex.y = y;
	currentVertex.z = z;
	uploadVertex();
}


void Vertex3fv(float *data)
{
	currentVertex.x = data[0];
	currentVertex.y = data[1];
	currentVertex.z = data[2];
	uploadVertex();
}


void Vertices2ftv(int num, gl_ft2vertex_t *data)
{
	for(; num>0; num--, data++)
	{
		TexCoord2fv(data->tex);
		Vertex2fv(data->pos);
	}
}


void Vertices3ftv(int num, gl_ft3vertex_t *data)
{
	for(; num>0; num--, data++)
	{
		TexCoord2fv(data->tex);
		Vertex3fv(data->pos);
	}
}


void Vertices3fctv(int num, gl_fct3vertex_t *data)
{
	for(; num>0; num--, data++)
	{
		Color4fv(data->color);
		TexCoord2fv(data->tex);
		Vertex3fv(data->pos);
	}
}
