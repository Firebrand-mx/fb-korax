#ifndef __DRD3D_H__
#define __DRD3D_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include "dd_gl.h"
#include <stdio.h>

namespace Direct3DDriver {

#define DRD3D_VERSION		110
#define DRD3D_VERSION_TEXT	"v1.1"
#define DRD3D_VERSION_FULL	"DGL Direct3D Driver v1.1 ("__DATE__")"

#define MAKE_RGBA(r,g,b,a)	( (b) + ((g)<<8) + ((r)<<16) + ((a)<<24) )
#define MAKERGB_A(rgb, a)	( (rgb[2]) + (rgb[1]<<8) + (rgb[0]<<16) + ((a)<<24) )
#define	CLAMP01(f)			{ if(f<0) f=0; if(f>1) f=1; }
#define SetRS(x, y)			IDirect3DDevice3_SetRenderState(d3dDevice, x, y)
#define SetTSS(s, x, y)		IDirect3DDevice3_SetTextureStageState(d3dDevice, s, x, y)


//------------------------------------------------------------------------
// Types and other useful stuff
//
#define MODELVIEW_STACK_DEPTH	16
#define	PROJECTION_STACK_DEPTH	8
#define	TEXTURE_STACK_DEPTH		8

enum
{
	STACK_MODELVIEW,
	STACK_PROJECTION,
	STACK_TEXTURE
};

enum { VX, VY, VZ };
enum { CR, CG, CB, CA };

#define PI		3.14159265

typedef unsigned char byte;

typedef struct
{
	unsigned char color[4];
} rgba_t;

typedef struct
{
	int			pos;
	int			depth;
	D3DMATRIX	*matrices;
} matrixstack_t;

typedef struct
{
	D3DVALUE	x, y, z;
	DWORD		rgba;
	D3DVALUE	s, t;
} vertex_t;

#define VERTEX_FORMAT	( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )


#define DO_GUARD
#define DO_GUARD_SLOW

//==========================================================================
//
//	Guard macros
//
//==========================================================================

#ifdef DO_GUARD
#define guard(name)		static const char *__FUNC_NAME__ = #name; try {
#define unguard			} catch (...) { DD_CoreDump(__FUNC_NAME__); throw; }
#define unguardf(msg)	} catch (...) { DD_CoreDump(__FUNC_NAME__); DD_CoreDump msg; throw; }
#else
#define guard(name)		static const char *__FUNC_NAME__ = #name; {
#define unguard			}
#define unguardf(msg)	}
#endif

#ifdef DO_GUARD_SLOW
#define guardSlow(name)		guard(name)
#define unguardSlow			unguard
#define unguardfSlow(msg)	unguardf(msg)
#else
#define guardSlow(name)		{
#define unguardSlow			}
#define unguardfSlow(msg)	}
#endif

//------------------------------------------------------------------------
// draw.c
//
extern vertex_t		currentVertex;		// For vertex input.

void Color3ub(DGLubyte r, DGLubyte g, DGLubyte b);
void Color3ubv(void *data);
void Color4ub(DGLubyte r, DGLubyte g, DGLubyte b, DGLubyte a);
void Color4ubv(void *data);
void Color3f(float r, float g, float b);
void Color3fv(float *data);
void Color4f(float r, float g, float b, float a);
void Color4fv(float *data);
int	BeginScene(void);
int	EndScene(void);
void Begin(int mode);
void End(void);
void Vertex2f(float x, float y);
void Vertex2fv(float *data);
void Vertex3f(float x, float y, float z);
void Vertex3fv(float *data);
void TexCoord2f(float s, float t);
void TexCoord2fv(float *data);
void Vertices2ftv(int num, gl_ft2vertex_t *data);
void Vertices3ftv(int num, gl_ft3vertex_t *data);
void Vertices3fctv(int num, gl_fct3vertex_t *data);

//------------------------------------------------------------------------
// dxinit.c
//
extern HWND					hwnd;
extern GUID					ddGuid;
extern LPDIRECTDRAW4		ddraw;
extern LPDIRECTDRAWSURFACE4	sPrimary, sBack, sDepth;
extern LPDIRECT3D3			d3d;
extern LPDIRECT3DDEVICE3	d3dDevice;
extern LPDIRECT3DVIEWPORT3	d3dVp, d3dScissor;
extern LPDIRECTDRAWGAMMACONTROL ddGamma;
extern D3DVIEWPORT2			vpData, scissorData;
extern DDPIXELFORMAT		pfZBuffer;
extern DDPIXELFORMAT		pfTextureRGB;	// RGB texture pixel format.
extern DDPIXELFORMAT		pfTextureRGBA;	// RGBA texture pixel format.
extern DDPIXELFORMAT		pfTexturePal8;	// 8 bit palettized texture pixel format.
extern boolean				gammaSupport;
extern HRESULT				hr;		// The result of the last DX call.

int dxInit(int firstTime);
int dxShutdown();
int dxRestoreSurfaces();
int dxRecreateSurfaces();
void dxDestroySurfaces();

//------------------------------------------------------------------------
// list.c
//
void RenderList(int format, int num, void *data);

//------------------------------------------------------------------------
// main.c
//
extern int		screenWidth, screenHeight, screenBits, windowed;
extern int		maxTexSize;
extern int		palTexAvailable;
extern int		dlBlend, lightTex;

//------------------------------------------------------------------------
// matrix.c
//
void matMul(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX *out);
void vecMatMul(float *in, D3DMATRIX *mat, float *out);
D3DMATRIX *getTexMatrix();

void MatrixMode(int mode);
void PushMatrix(void);
void PopMatrix(void);
void LoadIdentity(void);
void Translatef(float x, float y, float z);
void PostTranslatef(float x, float y, float z);
void Rotatef(float angle, float x, float y, float z);
void Scalef(float x, float y, float z);
void Ortho(float left, float top, float right, float bottom, float znear, float zfar);
void Perspective(float fovy, float aspect, float zNear, float zFar);

//------------------------------------------------------------------------
// misc.c
//
int Grab(int x, int y, int width, int height, int format, void *buffer);
int Project(int num, gl_fc3vertex_t *inVertices, gl_fc3vertex_t *outVertices);
int ReadPixels(int *inData, int format, void *pixels);
int Gamma(int set, DGLubyte *data);

//------------------------------------------------------------------------
// state.c
//
extern int		usefog;

void initState();
int	GetIntegerv(int name, int *v);
int	SetInteger(int name, int value);
char* GetString(int name);
void Enable(int cap);
void Disable(int cap);
void Func(int func, int param1, int param2);
void Fog(int pname, float param);
void Fogv(int pname, void *data);

//------------------------------------------------------------------------
// texture.c
//
extern int		usePalTex;
extern DGLuint	currentTexName;
extern rgba_t	palette[256];

int enablePalTexExt(int enable);
void deleteAllTextures(int unalloc);
void unpackMask(DWORD mask, int *offset, int *bits);
DGLuint NewTexture(void);
int LoadTexture(int format, int width, int height, int mipmap, void *data);
void DeleteTextures(int num, DGLuint *names);
void TexParam(int pname, int param);
void GetTexParameterv(int level, int pname, int *v);
void Palette(int format, void *data);
int	Bind(DGLuint texture);

//------------------------------------------------------------------------
// viewport.c
//
extern int		useScissor;

void enableScissor(int enable);
void Viewport(int x, int y, int width, int height);
void Scissor(int x, int y, int width, int height);

} // namespace Direct3DDriver

#endif