#ifndef __DROPENGL_H__
#define __DROPENGL_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <string.h>

#include "../DoomsDay/dd_gl.h"


#define DROGL_VERSION		104
#define DROGL_VERSION_TEXT	"v1.04"
#define DROGL_VERSION_FULL	"DGL OpenGL Driver v1.04 ("__DATE__")"


enum { VX, VY, VZ };
enum { CR, CG, CB, CA };

typedef unsigned char byte;

typedef struct
{
	unsigned char color[4];
} rgba_t;


//-------------------------------------------------------------------------
// main.c
//
extern gl_import_t	gim;
extern gl_export_t	gex;
extern int			usefog, dlBlend, maxTexSize;
extern DGLuint		currentTex, lightTex;
extern int			paltexExtAvailable;


//-------------------------------------------------------------------------
// list.c
//
void renderList(rendlist_t *rl);
void renderMaskedList(rendlist_t *mrl);
void renderSkyMaskLists(rendlist_t *smrl, rendlist_t *skyw);
void renderDynLightLists(rendlist_t *frl, rendlist_t *wrl);
void renderDLitPass(rendlist_t *rl, int num);


//-------------------------------------------------------------------------
// texture.c
//
extern rgba_t		palette[256];
extern int			usePalTex, dumpTextures;

int Power2(int num);
int enablePalTexExt(int enable);
DGLuint NewTexture(void);
int LoadTexture(int format, int width, int height, int mipmap, void *data);
void DeleteTextures(int num, DGLuint *names);
void TexParam(int pname, int param);
void GetTexParameterv(int level, int pname, int *v);
void Palette(int format, void *data);
int	Bind(DGLuint texture);


#endif