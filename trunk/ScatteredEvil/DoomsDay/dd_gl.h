//
// The API of the Doomsday Graphics Library.
//
// Resembles OpenGL *a lot*, only much simpler, plus
// some Doomsday-specific routines.

#ifndef __DOOMSDAY_GL_H__
#define __DOOMSDAY_GL_H__

#define DGL_VERSION_NUM		8

#define DGL_MAX_POLY_SIDES	64

// Types.
typedef unsigned char	DGLubyte;
typedef unsigned int	DGLuint;

// Rendpoly flags.
#define RPF_MASKED			0x01	// Use the special list for masked textures.
#define RPF_MISSING_WALL	0x02	// Originally this surface had no texture.
#define RPF_SKY_MASK		0x04	// A sky mask polygon.
#define RPF_LIGHT			0x08	// A dynamic light.
#define RPF_DLIT			0x10	// Normal list: poly is dynamically lit.
#define RPF_GLOW			0x20	// Multiply original vtx colors.

typedef struct
{
	DGLubyte rgb[3];
} gl_rgb_t;

/*typedef struct
{
	short	flags;				// RQF_*.
	gl_rgb_t color[3];			// Color for the vertices (2 for walls, 3 for flats).
	unsigned short texw, texh;	// Size of the texture.
	float	texoffx, texoffy;	// Texture coordinates for left/top (in real texcoords).
	DGLuint	masktex;			// Texture name for masked textures.
	float	v1[2], v2[2];		// Two vertices.
	float	top;				// Top height.
	float	dist[3];			// Distances to the vertices.
	union {			
		struct {
			float bottom;		// Bottom height.
			float len;			// Length of the quad.
		} quad;
		struct {
			float v3[2];		// Third vertex for flats.
		} flat;
	} u;
} rendquad_t; // Or flat triangle.*/

typedef enum
{
	rp_none,
	rp_quad,					// Wall segment.
	rp_flat						// Floor or ceiling.
} rendpolytype_t;

typedef struct
{
	float pos[2];				// X and Y coordinates.
	gl_rgb_t color;				// Color of the vertex.
	float dist;					// Distance to the vertex.
} rendpoly_vertex_t;

// rendpoly_t is only for convenience; the data written in the rendering
// list data buffer is taken from this struct.
typedef struct
{
	rendpolytype_t type;		
	short flags;				// RPF_*.
	int texw, texh;				// Texture width and height.
	float texoffx, texoffy;		// Texture coordinates for left/top (in real texcoords).
	DGLuint tex;				// Used for masked textures.
	float top, bottom;			
	float length;
	int numvertices;			// Number of vertices for the poly.
	rendpoly_vertex_t vertices[DGL_MAX_POLY_SIDES];
} rendpoly_t;

typedef struct
{
	DGLuint	tex;				// The name of the texture for this list.
	//int		numquads;			// Number of quads in the list.
	//int		listsize;			// Absolute size of the list.
	int size;				// Number of bytes allocated for the data.
	unsigned char *cursor;			// A pointer to data, for reading/writing.
	//rendquad_t *quads;		// The list of quads.	
	unsigned char *data;				// Data for a number of polygons.
} rendlist_t;

// A 2-vertex with texture coordinates, using floats
typedef struct
{
	float	pos[2];
	float	tex[2];
} gl_ft2vertex_t;

// A 3-vertex with texture coordinates, using floats
typedef struct
{
	float	pos[3];
	float	tex[2];
} gl_ft3vertex_t;

// A 3-vertex with texture coordinates and a color, using floats
typedef struct
{
	float	pos[3];
	float	tex[2];
	float	color[4];
} gl_fct3vertex_t;

// A colored 3-vertex, using floats
typedef struct
{
	float	pos[3];
	float	color[4];
} gl_fc3vertex_t;


enum
{
	DGL_FALSE						= 0,
	DGL_TRUE						= 1,

	DGL_ERROR						= 0,
	DGL_OK							= 1,
	DGL_UNSUPPORTED					= 2,

	// Formats
	DGL_RGB							= 0x1000,
	DGL_RGBA,
	DGL_COLOR_INDEX_8,
	DGL_COLOR_INDEX_8_PLUS_A8,
	DGL_LUMINANCE,
	DGL_R,
	DGL_G,
	DGL_B,
	DGL_A,
	DGL_DEPTH_COMPONENT,

	DGL_SINGLE_PIXELS,
	DGL_BLOCK,

	DGL_NORMAL_LIST,
	DGL_MASKED_LIST,
	DGL_SKYMASK_LISTS,
	DGL_LIGHT_LISTS,
	DGL_NORMAL_DLIT_LIST,
	
	// Values
	DGL_VERSION						= 0x2000,
	DGL_COLOR_BITS,
	DGL_MAX_TEXTURE_SIZE,
	DGL_SCISSOR_BOX,

	DGL_DL_BLEND_MODE,
	DGL_LIGHT_TEXTURE,

	// Primitives
	DGL_LINES						= 0x3000,
	DGL_TRIANGLES,
	DGL_TRIANGLE_FAN,
	DGL_TRIANGLE_STRIP,
	DGL_QUADS,	
	DGL_QUAD_STRIP,
	DGL_SEQUENCE,

	// Matrices
	DGL_MODELVIEW					= 0x4000,
	DGL_PROJECTION,
	DGL_TEXTURE,

	// Caps
	DGL_TEXTURING					= 0x5000,
	DGL_BLENDING,
	DGL_DEPTH_TEST,
	DGL_ALPHA_TEST,
	DGL_SCISSOR_TEST,
	DGL_CULL_FACE,
	DGL_COLOR_WRITE,
	DGL_DEPTH_WRITE,
	DGL_FOG,
	DGL_PALETTED_TEXTURES,

	// Blending functions
	DGL_ZERO						= 0x6000,
	DGL_ONE,
	DGL_DST_COLOR,
	DGL_ONE_MINUS_DST_COLOR,
	DGL_DST_ALPHA,
	DGL_ONE_MINUS_DST_ALPHA,
	DGL_SRC_COLOR,
	DGL_ONE_MINUS_SRC_COLOR,
	DGL_SRC_ALPHA,
	DGL_ONE_MINUS_SRC_ALPHA,
	DGL_SRC_ALPHA_SATURATE,

	// Comparative functions
	DGL_ALWAYS						= 0x7000,
	DGL_NEVER,
	DGL_LESS,
	DGL_EQUAL,
	DGL_LEQUAL,
	DGL_GREATER,
	DGL_GEQUAL,
	DGL_NOTEQUAL,

	// Miscellaneous
	DGL_MIN_FILTER					= 0xf000,
	DGL_MAG_FILTER,
	DGL_NEAREST,
	DGL_LINEAR,
	DGL_NEAREST_MIPMAP_NEAREST,
	DGL_LINEAR_MIPMAP_NEAREST,
	DGL_NEAREST_MIPMAP_LINEAR,
	DGL_LINEAR_MIPMAP_LINEAR,
	DGL_WRAP_S,
	DGL_WRAP_T,
	DGL_CLAMP,
	DGL_REPEAT,
	DGL_FOG_MODE,
	DGL_FOG_DENSITY,
	DGL_FOG_START,
	DGL_FOG_END,
	DGL_FOG_COLOR,
	DGL_EXP,
	DGL_EXP2,
	DGL_WIDTH,
	DGL_HEIGHT,

	// Various bits
	DGL_COLOR_BUFFER_BIT		= 0x1,
	DGL_DEPTH_BUFFER_BIT		= 0x2,
};


typedef struct
{
	int		apiSize;			// Size of this structure.
	
	// Some general utilities the engine provides.
	int		(*CheckParm)(char *check);
	int		(*Argc)(void);
	char*	(*Argv)(int i);

	void	(*Message)(char *msg, ...);
	void	(*Error)(char *error, ...);
} gl_import_t;


typedef struct
{
	int		version;			// GL API version (driver's).
	
	// Base-level routines.
	int		(*Init)(int width, int height, int bpp, int fullscreen);
	void	(*Shutdown)(void);
	int		(*ChangeMode)(int width, int height, int bpp, int fullscreen);

	// Viewport.
	void	(*Clear)(int bufferbits);
	void	(*OnScreen)(void);
	void	(*Viewport)(int x, int y, int width, int height);
	void	(*Scissor)(int x, int y, int width, int height);

	// State.
	int		(*GetIntegerv)(int name, int *v);
	int		(*SetInteger)(int name, int value);
	char*	(*GetString)(int name);
	void	(*Enable)(int cap);
	void	(*Disable)(int cap);
	void	(*Func)(int func, int param1, int param2);

	// Textures.
	DGLuint	(*NewTexture)(void);
	void	(*DeleteTextures)(int num, DGLuint *names);
	int		(*TexImage)(int format, int width, int height, int mipmap, void *data);
	void	(*TexParameter)(int pname, int param);
	void	(*GetTexParameterv)(int level, int pname, int *v);
	void	(*Palette)(int format, void *data);	
	int		(*Bind)(DGLuint texture);

	// Matrix operations.
	void	(*MatrixMode)(int mode);
	void	(*PushMatrix)(void);
	void	(*PopMatrix)(void);
	void	(*LoadIdentity)(void);
	void	(*Translatef)(float x, float y, float z);
	void	(*Rotatef)(float angle, float x, float y, float z);
	void	(*Scalef)(float x, float y, float z);
	void	(*Ortho)(float left, float top, float right, float bottom, float znear, float zfar);
	void	(*Perspective)(float fovy, float aspect, float zNear, float zFar);

	// Colors.
	void	(*Color3ub)(DGLubyte r, DGLubyte g, DGLubyte b);
	void	(*Color3ubv)(void *data);
	void	(*Color4ub)(DGLubyte r, DGLubyte g, DGLubyte b, DGLubyte a);
	void	(*Color4ubv)(void *data);
	void	(*Color3f)(float r, float g, float b);
	void	(*Color3fv)(float *data);
	void	(*Color4f)(float r, float g, float b, float a);
	void	(*Color4fv)(float *data);

	// Drawing.
	void	(*Begin)(int mode);
	void	(*End)(void);
	void	(*Vertex2f)(float x, float y);
	void	(*Vertex2fv)(float *data);
	void	(*Vertex3f)(float x, float y, float z);
	void	(*Vertex3fv)(float *data);
	void	(*TexCoord2f)(float s, float t);
	void	(*TexCoord2fv)(float *data);
	void	(*Vertices2ftv)(int num, gl_ft2vertex_t *data);
	void	(*Vertices3ftv)(int num, gl_ft3vertex_t *data);
	void	(*Vertices3fctv)(int num, gl_fct3vertex_t *data);
	
	// Rendering.
	void	(*RenderList)(int format, int num, void *data);

	// Miscellaneous.
	int		(*Grab)(int x, int y, int width, int height, int format, void *buffer);
	void	(*Fog)(int pname, float param);
	void	(*Fogv)(int pname, void *data);
	int		(*Project)(int num, gl_fc3vertex_t *inVertices, gl_fc3vertex_t *outVertices);
	int		(*ReadPixels)(int *inData, int format, void *pixels); 
	int		(*Gamma)(int set, DGLubyte *data);
} gl_export_t;


// This is called by the engine to retrieve the GL routines.
//gl_export_t* GetGLAPI(gl_import_t *api);

typedef gl_export_t* (*GETGLAPI)(gl_import_t*);

#endif