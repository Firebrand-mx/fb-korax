// drD3D.dll
// Direct3D driver for the Doomsday Graphics Library
//
// matrix.c : Matrix math and stacks

#include "drD3D.h"
#include <math.h>

D3DMATRIX		mvMatrixStack[MODELVIEW_STACK_DEPTH];
D3DMATRIX		projMatrixStack[PROJECTION_STACK_DEPTH];
D3DMATRIX		texMatrixStack[TEXTURE_STACK_DEPTH];

matrixstack_t	modelViewStack = { 0, MODELVIEW_STACK_DEPTH, mvMatrixStack };
matrixstack_t	projStack = { 0, PROJECTION_STACK_DEPTH, projMatrixStack };
matrixstack_t	textureStack = { 0, TEXTURE_STACK_DEPTH, texMatrixStack };

matrixstack_t	*currentStack = &modelViewStack;


// --------------------------------------------------------------------------

// Returns the current matrix on the current stack.
static D3DMATRIX *getCurMatrix()
{
	return currentStack->matrices + currentStack->pos;		
}

D3DMATRIX *getPrevMatrix()
{
	return currentStack->matrices + currentStack->pos-1;
}

D3DMATRIX *getTexMatrix()
{
	return textureStack.matrices + textureStack.pos;
}

// Calculates [out] = [A] * [B]
void matMul(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX *out)
{
	float	*fa = (float*) a;
	float	*fb = (float*) b;
	float	res[16];
	int		i, j, k;

	memset(res, 0, sizeof(D3DMATRIX));

	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			for(k=0; k<4; k++)
				res[4*i + j] += fa[4*k + j] * fb[4*i + k];

	// We're done.
	memcpy(out, res, sizeof(D3DMATRIX));
}

// The vectors must have a dimension of 3.
void vecMatMul(float *in, D3DMATRIX *mat, float *out)
{
	float x = in[VX]*mat->_11 + in[VY]*mat->_21 + in[VZ]*mat->_31 + mat->_41;
	float y = in[VX]*mat->_12 + in[VY]*mat->_22 + in[VZ]*mat->_32 + mat->_42;
	float z = in[VX]*mat->_13 + in[VY]*mat->_23 + in[VZ]*mat->_33 + mat->_43;
	float w = in[VX]*mat->_14 + in[VY]*mat->_24 + in[VZ]*mat->_34 + mat->_44;

	if(w == 0) return;	// uh?
	out[VX] = x/w;
	out[VY] = y/w;
	out[VZ] = z/w;
}

// Updates the appropriate DX transformation matrix.
void uploadMatrix()
{
	D3DTRANSFORMSTATETYPE tsType;

	// DX doesn't know what to do with texture matrices.
	if(currentStack == &textureStack) return;

	if(currentStack == &modelViewStack)
		tsType = D3DTRANSFORMSTATE_VIEW;
	else 
		tsType = D3DTRANSFORMSTATE_PROJECTION;

	if(FAILED(hr = IDirect3DDevice3_SetTransform(d3dDevice, tsType, 
		currentStack->matrices + currentStack->pos)))
		gim.Message( "SetTransform failed.\n");
}


// PUBLIC FUNCTIONS ---------------------------------------------------------

void MatrixMode(int mode)
{
	guard(MatrixMode);
	currentStack = (mode == DGL_PROJECTION)? &projStack
		: (mode == DGL_TEXTURE)? &textureStack
		: &modelViewStack;
	unguard;
}


void PushMatrix(void)
{
	guard(PushMatrix);
	// Is there still room on the stack?
	if(currentStack->pos < currentStack->depth-1)
	{
		// Copy the current matrix to the next position.
		memcpy(currentStack->matrices + currentStack->pos+1,
			currentStack->matrices + currentStack->pos, sizeof(D3DMATRIX));
		currentStack->pos++;
	}
	unguard;
}


void PopMatrix(void)
{
	guard(PopMatrix);
	// Is there something on the stack?
	if(currentStack->pos > 0)
	{
		// Popping is quite trivial.
		currentStack->pos--;
		uploadMatrix();
	}
	unguard;
}


void LoadIdentity(void)
{
	guard(LoadIdentity);
	D3DMATRIX *mat = getCurMatrix();

	memset(mat, 0, sizeof(*mat));
	mat->_11 = mat->_22 = mat->_33 = mat->_44 = 1.0f;
	uploadMatrix();
	unguard;
}


void Translatef(float x, float y, float z)
{
	guard(Translatef);
	D3DMATRIX tr, *mat = getCurMatrix();

	memset(&tr, 0, sizeof(tr));
	tr._11 = tr._22 = tr._33 = tr._44 = 1.0f;
	tr._41 = x;
	tr._42 = y;
	tr._43 = z;
	// Multiply the current matrix by the tr matrix.
	matMul(mat, &tr, mat);
	uploadMatrix();
	unguard;
}


void PostTranslatef(float x, float y, float z)
{
	guard(PostTranslatef);
	D3DMATRIX tr, *mat = getCurMatrix();

	memset(&tr, 0, sizeof(tr));
	tr._11 = tr._22 = tr._33 = tr._44 = 1.0f;
	tr._41 = x;
	tr._42 = y;
	tr._43 = z;
	// Multiply the current matrix by the tr matrix.
	matMul(&tr, mat, mat);
	uploadMatrix();
	unguard;
}


void Rotatef(float angle, float x, float y, float z)
{
	guard(Rotatef);
	D3DMATRIX	tr, *mat = getCurMatrix();
	float		angleRad = -angle * (float) PI / 180.0f,
				c = (float) cos(angleRad), 
				s = (float) sin(angleRad), 
				t = 1-c, axisLen;
	
	memset(&tr, 0, sizeof(tr));

	// Normalize the axis, if necessary.
	axisLen = x*x + y*y + z*z;
	if(axisLen == 0) return;	// Can't do that.
	if(axisLen != 1)
	{
		axisLen = (float) sqrt(axisLen);
		x /= axisLen;
		y /= axisLen;
		z /= axisLen;
	}

	tr._11 = t*x*x + c;
	tr._12 = t*x*y - s*z;
	tr._13 = t*x*z + s*y;
	
	tr._21 = t*x*y + s*z;
	tr._22 = t*y*y + c;
	tr._23 = t*y*z - s*x;

	tr._31 = t*x*z - s*y;
	tr._32 = t*y*z + s*x;
	tr._33 = t*z*z + c;

	tr._44 = 1;
	
	// Multiply and upload.
	matMul(mat, &tr, mat);
	uploadMatrix();
	unguard;
}


void Scalef(float x, float y, float z)
{
	guard(Scalef);
	D3DMATRIX tr, *mat = getCurMatrix();

	memset(&tr, 0, sizeof(tr));
	tr._11 = x;
	tr._22 = y;
	tr._33 = z;
	tr._44 = 1.0f;
	// Multiply the current matrix by the tr matrix.
	matMul(mat, &tr, mat);
	uploadMatrix();
	unguard;
}


void Ortho(float left, float top, float right, float bottom, float znear, float zfar)
{
	guard(Ortho);
	D3DMATRIX tr, *mat = getCurMatrix();

	memset(&tr, 0, sizeof(tr));
	tr._11 = 2 / (right-left);
	tr._22 = -2 / (bottom-top);
	tr._33 = -2 / (zfar-znear);
	tr._44 = 1;
	tr._41 = -(right+left) / (right-left);
	tr._42 = (bottom+top) / (bottom-top);
	tr._43 = -(zfar+znear) / (zfar-znear);;

	// Multiply the current matrix by the tr matrix.
	matMul(mat, &tr, mat);
	uploadMatrix();
	unguard;
}


void Perspective(float fovy, float aspect, float zNear, float zFar)
{
	guard(Perspective);
	D3DMATRIX	tr, *mat = getCurMatrix();
	float		left, right, top, bottom;

	memset(&tr, 0, sizeof(tr));
	top = zNear * (float) tan(fovy * PI / 360.0);
	bottom = -top;
	left = bottom * aspect;
	right = top * aspect;
	
	tr._11 = 2*zNear / (right-left);
	tr._22 = 2*zNear / (top-bottom);
	tr._31 = (right+left) / (right-left);
	tr._32 = (top+bottom) / (top-bottom);
	tr._33 = -(zFar+zNear) / (zFar-zNear);
	tr._34 = -1;
	tr._43 = -1*zFar*zNear / (zFar-zNear);
	
	// Multiply and upload.
	matMul(mat, &tr, mat);
	uploadMatrix();
	unguard;
}
