#ifndef __DOOMSDAY_MD2_H__
#define __DOOMSDAY_MD2_H__

#include "dd_def.h"

#define MD2_MAGIC			0x32504449	
#define NUMVERTEXNORMALS	162
#define MAX_MODELS			512

typedef struct 
{ 
	int magic; 
	int version; 
	int skinWidth; 
	int skinHeight; 
	int frameSize; 
	int numSkins; 
	int numVertices; 
	int numTexCoords; 
	int numTriangles; 
	int numGlCommands; 
	int numFrames; 
	int offsetSkins; 
	int offsetTexCoords; 
	int offsetTriangles; 
	int offsetFrames; 
	int offsetGlCommands; 
	int offsetEnd; 
} md2_header_t;

typedef struct
{
	byte vertex[3];
	byte lightNormalIndex;
} md2_triangleVertex_t;

typedef struct
{
	float scale[3];
	float translate[3];
	char name[16];
	md2_triangleVertex_t vertices[1];
} md2_packedFrame_t;

typedef struct
{
	float vertex[3];
	byte lightNormalIndex;
} md2_modelVertex_t;

// Translated frame (vertices in model space).
typedef struct
{
	char name[16];
	md2_modelVertex_t *vertices;
} md2_frame_t;

typedef struct
{
	char name[64];
} md2_skin_t;

typedef struct
{
	short vertexIndices[3];
	short textureIndices[3];
} md2_triangle_t;

typedef struct
{
	short s, t;
} md2_textureCoordinate_t;

typedef struct
{
	float s, t;
	int vertexIndex;
} md2_glCommandVertex_t;

typedef struct
{
	boolean			loaded;
	char			fileName[256];	// Name of the md2 file.
	md2_header_t	header;			// A copy of the header.
	md2_skin_t		*skins;			
	unsigned int	*skintex;		// Texture names for the skins.
	md2_textureCoordinate_t *texCoords;
	md2_triangle_t	*triangles;
	md2_frame_t		*frames;
	int				*glCommands;
} model_t;

extern model_t *modellist[MAX_MODELS];
extern float avertexnormals[NUMVERTEXNORMALS][3];

#endif