// drOpenGL.dll
// The Doomsday graphics library driver for OpenGL
//
// main.c : Init and shutdown, API exchange

#include "drOpenGL.h"

//#define RENDER_WIREFRAME

// The APIs.
gl_import_t	gim;
gl_export_t	gex;


// The State.
HWND		hwnd;
HDC			hdc;
HGLRC		hglrc;
int			screenWidth, screenHeight, screenBits, windowed;
DGLuint		currentTex = 0, lightTex = 0;
int			paltexExtAvailable;
int			maxTexSize;
float		nearClip, farClip;
int			usefog;
int			dlBlend=0;
int			inSequence = DGL_FALSE;
int			primType = DGL_FALSE;


// A helpful macro that changes the origin of the screen
// coordinate system.
#define FLIP(y)	(screenHeight - (y+1))


// SANTA'S LITTLE HELPERS --------------------------------------------------

int fullscreenMode(int width, int height, int bpp)
{
	DEVMODE	newMode;
	int		res;

	// Switch to the requested resolution.
	memset(&newMode, 0, sizeof(newMode));	// Clear the structure.
	newMode.dmSize = sizeof(newMode);
	newMode.dmPelsWidth = width;
	newMode.dmPelsHeight = height;
	newMode.dmBitsPerPel = bpp;
	newMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	if(bpp) newMode.dmFields |= DM_BITSPERPEL;
	if((res=ChangeDisplaySettings(&newMode, 0)) != DISP_CHANGE_SUCCESSFUL)
	{
		gim.Message("drOpenGL.setResolution: Error %d.\n", res);
		return 0; // Failed, damn you.
	}

	// Set the correct window style and size.
	SetWindowLong(hwnd, GWL_STYLE, WS_POPUP|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, width, height, 0);

	// Update the screen size variables.
	screenWidth = width;
	screenHeight = height;
	if(bpp) screenBits = bpp;

	// Done!
	return 1;
}

// Only adjusts the window style and size.
void windowedMode(int width, int height)
{
	// We need to have a large enough client area.
	RECT rect;
	int xoff = (GetSystemMetrics(SM_CXSCREEN) - width) / 2, 
		yoff = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	
	if(gim.CheckParm("-nocenter")) xoff = yoff = 0;
	
	rect.left = xoff;
	rect.top = yoff;
	rect.right = xoff + width;
	rect.bottom = yoff + height;

	SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE|WS_CAPTION|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
	AdjustWindowRect(&rect, WS_VISIBLE|WS_CAPTION|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, FALSE);
	SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, rect.right-rect.left,
		rect.bottom-rect.top, 0);

	screenWidth = width;
	screenHeight = height;
}

void initState()
{
	GLfloat fogcol[4] = { .54f, .54f, .54f, 1 };

	nearClip = 5;
	farClip = 8000;	
	currentTex = 0;
	lightTex = 0;

	// Here we configure the OpenGL state and set projection matrix.
	glFrontFace(GL_CW);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_TEXTURE_2D);

	// The projection matrix.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Initialize the modelview matrix.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear also the texture matrix (I'm not using this, though).
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	// Alpha blending is a go!
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);

	// Default state for the white fog is off.
	usefog = 0;
	glDisable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogi(GL_FOG_END, 2100);	// This should be tweaked a bit.
	glFogfv(GL_FOG_COLOR, fogcol);

	//glEnable(GL_POLYGON_SMOOTH);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

#ifdef RENDER_WIREFRAME
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
}

int initOpenGL()
{
	// Create the OpenGL rendering context.
	if(!(hglrc = wglCreateContext(hdc)))
	{
		int res = GetLastError();
		gim.Message("drOpenGL.initOpenGL: Creation of rendering context failed. Error %d.\n",res);
		return 0;
	}

	// Make the context current.
	if(!wglMakeCurrent(hdc, hglrc))
	{
		gim.Message("drOpenGL.initOpenGL: Couldn't make the rendering context current.\n");
		return 0;
	}

	initState();
	return 1;
}


// THE ROUTINES ------------------------------------------------------------

int Init(int width, int height, int bpp, int fullscreen)
{
	char	*token, buff[2049];
	int		res, pixForm;
	PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// The size
		1,								// Version
		PFD_DRAW_TO_WINDOW |			// Support flags
			PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,					// Pixel type
		32,								// Bits per pixel
		0,0, 0,0, 0,0, 0,0,
		0,0,0,0,0,
		32,								// Depth bits
		0,0,
		0,								// Layer type (ignored?)
		0,0,0,0
	};
	HWND hDesktop = GetDesktopWindow();
	HDC desktop_hdc = GetDC(hDesktop);
	int deskbpp = GetDeviceCaps(desktop_hdc, PLANES) * GetDeviceCaps(desktop_hdc, BITSPIXEL);

	ReleaseDC(hDesktop, desktop_hdc);

	// By default, use the resolution defined in hexen.cfg.
	screenWidth = width;
	screenHeight = height;
	screenBits = deskbpp;
	windowed = !fullscreen;
	hwnd = FindWindow("DoomMainWClass", NULL);

	if(fullscreen)
	{
		if(!fullscreenMode(screenWidth, screenHeight, bpp))		
		{
			gim.Error("drOpenGL.Init: Resolution change failed (%d x %d).\n",
				screenWidth, screenHeight);
		}
	}
	else
	{
		windowedMode(screenWidth, screenHeight);
	}	

	// Get the device context handle.
	hdc = GetDC(hwnd);

	// Set the pixel format for the device context. This can only be done once.
	// (Windows...).
	pixForm = ChoosePixelFormat(hdc, &pfd);
	if(!pixForm)
	{
		res = GetLastError();
		gim.Error("drOpenGL.Init: Choosing of pixel format failed. Error %d.\n",res);
	}

	// Make sure that the driver is hardware-accelerated.
	DescribePixelFormat(hdc, pixForm, sizeof(pfd), &pfd);
	if(pfd.dwFlags & PFD_GENERIC_FORMAT && !gim.CheckParm("-allowsoftware"))
	{
		gim.Error("drOpenGL.Init: OpenGL driver not accelerated!\nUse the -allowsoftware option to bypass this.\n");
	}

	if(!SetPixelFormat(hdc, pixForm, &pfd))
	{
		res = GetLastError();
		gim.Error("drOpenGL.Init: Setting of pixel format failed. Error %d.\n",res);
	}

	if(!initOpenGL()) gim.Error("drOpenGL.Init: OpenGL init failed.\n");

	// Clear the buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SwapBuffers(hdc);

	// Print some OpenGL information (console must be initialized by now).
	gim.Message("OpenGL information:\n");
	gim.Message("  Vendor: %s\n", glGetString(GL_VENDOR));
	gim.Message("  Renderer: %s\n", glGetString(GL_RENDERER));
	gim.Message("  Version: %s\n", glGetString(GL_VERSION));
	gim.Message("  Extensions:\n");

	paltexExtAvailable = strstr(glGetString(GL_EXTENSIONS), 
		"GL_EXT_shared_texture_palette") != 0;

	memset(buff, 0, sizeof(buff));
	strncpy(buff, glGetString(GL_EXTENSIONS), 2048);
	token = strtok(buff, " ");
	while(token)
	{
		gim.Message( "      %s\n", token);
		token = strtok(NULL, " ");
	}
	gim.Message("  GLU Version: %s\n", gluGetString(GLU_VERSION));
	// Check the maximum texture size.
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	gim.Message("  Maximum texture size: %d\n", maxTexSize);

	if(gim.CheckParm("-dumptextures")) 
	{
		dumpTextures = DGL_TRUE;
		gim.Message("  Dumping textures (mipmap level zero).\n");
	}

	return DGL_OK;
}


void Shutdown(void)
{
	// Delete the rendering context.
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);

	// Go back to normal display settings.
	ChangeDisplaySettings(0, 0);
}


int	ChangeMode(int width, int height, int bpp, int fullscreen)
{
	if(!windowed && !fullscreen)
	{
		// We're currently in a fullscreen mode, but the caller
		// requests we change to windowed.
		ChangeDisplaySettings(0, 0);
		windowedMode(width, height);
		return DGL_OK;
	}
	if(width && height && (width != screenWidth || height != screenHeight))
	{
		if(fullscreen)
		{
			if(!fullscreenMode(width, height, bpp))
				return DGL_ERROR;
		}
		else
		{
			windowedMode(width, height);
		}
	}
	return DGL_OK;
}


void Clear(int bufferbits)
{
	GLbitfield mask = 0;

	if(bufferbits & DGL_COLOR_BUFFER_BIT) mask |= GL_COLOR_BUFFER_BIT;
	if(bufferbits & DGL_DEPTH_BUFFER_BIT) mask |= GL_DEPTH_BUFFER_BIT;
	glClear(mask);
}


void OnScreen(void)
{
	// Swap buffers.
	SwapBuffers(hdc);
#ifdef RENDER_WIREFRAME
	Clear(DGL_COLOR_BUFFER_BIT);
#endif
}


void Viewport(int x, int y, int width, int height)
{
	glViewport(x, FLIP(y+height-1), width, height);
}


void Scissor(int x, int y, int width, int height)
{
	glScissor(x, FLIP(y+height-1), width, height);
}

int	GetIntegerv(int name, int *v)
{
	float rgba[4];

	switch(name)
	{
	case DGL_MAX_TEXTURE_SIZE:
		*v = maxTexSize;
		break;

	case DGL_PALETTED_TEXTURES:
		*v = usePalTex;
		break;

	case DGL_SCISSOR_TEST:
		glGetIntegerv(GL_SCISSOR_TEST, v);
		break;

	case DGL_SCISSOR_BOX:
		glGetIntegerv(GL_SCISSOR_BOX, v);
		v[1] = FLIP(v[1]+v[3]-1);
		break;

	case DGL_FOG:
		*v = usefog;
		break;

	case DGL_VERSION:
		*v = DROGL_VERSION;
		break;

	case DGL_R:
		glGetFloatv(GL_CURRENT_COLOR, rgba);
		*v = (int) (rgba[0] * 255);
		break;

	case DGL_G:
		glGetFloatv(GL_CURRENT_COLOR, rgba);
		*v = (int) (rgba[1] * 255);
		break;

	case DGL_B:
		glGetFloatv(GL_CURRENT_COLOR, rgba);
		*v = (int) (rgba[2] * 255);
		break;

	case DGL_A:
		glGetFloatv(GL_CURRENT_COLOR, rgba);
		*v = (int) (rgba[3] * 255);
		break;

	default:
		return DGL_ERROR;
	}
	return DGL_OK;
}


int	SetInteger(int name, int value)
{
	switch(name)
	{
	case DGL_DL_BLEND_MODE:
		dlBlend = value;
		break;

	case DGL_LIGHT_TEXTURE:
		lightTex = value;
		break;

	default:
		return DGL_ERROR;
	}
	return DGL_OK;
}


char* GetString(int name)
{
	switch(name)
	{
	case DGL_VERSION:
		return DROGL_VERSION_FULL;
	}
	return NULL;
}
	

void Enable(int cap)
{
	switch(cap)
	{
	case DGL_TEXTURING:
		glEnable(GL_TEXTURE_2D);
		break;

	case DGL_BLENDING:
		glEnable(GL_BLEND);
		break;

	case DGL_FOG:
		glEnable(GL_FOG);
		usefog = DGL_TRUE;
		break;

	case DGL_DEPTH_TEST:
		glEnable(GL_DEPTH_TEST);
		break;

	case DGL_ALPHA_TEST:
		glEnable(GL_ALPHA_TEST);
		break;

	case DGL_CULL_FACE:
		glEnable(GL_CULL_FACE);
		break;

	case DGL_SCISSOR_TEST:
		glEnable(GL_SCISSOR_TEST);
		break;

	case DGL_COLOR_WRITE:
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		break;

	case DGL_DEPTH_WRITE:
		glDepthMask(GL_TRUE);
		break;

	case DGL_PALETTED_TEXTURES:
		enablePalTexExt(DGL_TRUE);
		break;
	}
}

void Disable(int cap)
{
	switch(cap)
	{
	case DGL_TEXTURING:
		glDisable(GL_TEXTURE_2D);
		break;

	case DGL_BLENDING:
		glDisable(GL_BLEND);
		break;

	case DGL_FOG:
		glDisable(GL_FOG);
		usefog = DGL_FALSE;
		break;

	case DGL_DEPTH_TEST:
		glDisable(GL_DEPTH_TEST);
		break;

	case DGL_ALPHA_TEST:
		glDisable(GL_ALPHA_TEST);
		break;

	case DGL_CULL_FACE:
		glDisable(GL_CULL_FACE);
		break;

	case DGL_SCISSOR_TEST:
		glDisable(GL_SCISSOR_TEST);
		break;

	case DGL_COLOR_WRITE:
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		break;

	case DGL_DEPTH_WRITE:
		glDepthMask(GL_FALSE);
		break;

	case DGL_PALETTED_TEXTURES:
		enablePalTexExt(DGL_FALSE);
		break;
	}
}


void Func(int func, int param1, int param2)
{
	switch(func)
	{
	case DGL_BLENDING:
		glBlendFunc(param1==DGL_ZERO? GL_ZERO
			: param1==DGL_ONE? GL_ONE
			: param1==DGL_DST_COLOR? GL_DST_COLOR
			: param1==DGL_ONE_MINUS_DST_COLOR? GL_ONE_MINUS_DST_COLOR
			: param1==DGL_SRC_ALPHA? GL_SRC_ALPHA
			: param1==DGL_ONE_MINUS_SRC_ALPHA? GL_ONE_MINUS_SRC_ALPHA
			: param1==DGL_DST_ALPHA? GL_DST_ALPHA
			: param1==DGL_ONE_MINUS_DST_ALPHA? GL_ONE_MINUS_DST_ALPHA
			: param1==DGL_SRC_ALPHA_SATURATE? GL_SRC_ALPHA_SATURATE
			: GL_ZERO,

			param2==DGL_ZERO? GL_ZERO
			: param2==DGL_ONE? GL_ONE
			: param2==DGL_SRC_COLOR? GL_SRC_COLOR
			: param2==DGL_ONE_MINUS_SRC_COLOR? GL_ONE_MINUS_SRC_COLOR
			: param2==DGL_SRC_ALPHA? GL_SRC_ALPHA
			: param2==DGL_ONE_MINUS_SRC_ALPHA? GL_ONE_MINUS_SRC_ALPHA
			: param2==DGL_DST_ALPHA? GL_DST_ALPHA
			: param2==DGL_ONE_MINUS_DST_ALPHA? GL_ONE_MINUS_DST_ALPHA
			: GL_ZERO);
		break;

	case DGL_DEPTH_TEST:
		glDepthFunc(param1==DGL_NEVER? GL_NEVER
			: param1==DGL_LESS? GL_LESS
			: param1==DGL_EQUAL? GL_EQUAL
			: param1==DGL_LEQUAL? GL_LEQUAL
			: param1==DGL_GREATER? GL_GREATER
			: param1==DGL_NOTEQUAL? GL_NOTEQUAL
			: param1==DGL_GEQUAL? GL_GEQUAL
			: GL_ALWAYS);
		break;

	case DGL_ALPHA_TEST:
		glAlphaFunc(param1==DGL_NEVER? GL_NEVER
			: param1==DGL_LESS? GL_LESS
			: param1==DGL_EQUAL? GL_EQUAL
			: param1==DGL_LEQUAL? GL_LEQUAL
			: param1==DGL_GREATER? GL_GREATER
			: param1==DGL_NOTEQUAL? GL_NOTEQUAL
			: param1==DGL_GEQUAL? GL_GEQUAL
			: GL_ALWAYS,
			param2 / 255.0f);
		break;
	}
}

	
void MatrixMode(int mode)
{
	glMatrixMode(mode==DGL_PROJECTION? GL_PROJECTION
		: mode==DGL_TEXTURE? GL_TEXTURE
		: GL_MODELVIEW);
}


void PushMatrix(void)
{
	glPushMatrix();
}


void PopMatrix(void)
{
	glPopMatrix();
}


void LoadIdentity(void)
{
	glLoadIdentity();
}


void Translatef(float x, float y, float z)
{
	glTranslatef(x, y, z);
}


void Rotatef(float angle, float x, float y, float z)
{
	glRotatef(angle, x, y, z);
}


void Scalef(float x, float y, float z)
{
	glScalef(x, y, z);
}


void Ortho(float left, float top, float right, float bottom, float znear, float zfar)
{
	glOrtho(left, right, bottom, top, znear, zfar);
}


void Perspective(float fovy, float aspect, float zNear, float zFar)
{
	gluPerspective(fovy, aspect, zNear, zFar);
}


void Color3ub(DGLubyte r, DGLubyte g, DGLubyte b)
{
	glColor3ub(r, g, b);
}


void Color3ubv(void *data)
{
	glColor3ubv(data);
}


void Color4ub(DGLubyte r, DGLubyte g, DGLubyte b, DGLubyte a)
{
	glColor4ub(r, g, b, a);
}


void Color4ubv(void *data)
{
	glColor4ubv(data);
}


void Color3f(float r, float g, float b)
{
	glColor3f(r, g, b);
}


void Color3fv(float *data)
{
	glColor3fv(data);
}


void Color4f(float r, float g, float b, float a)
{
	glColor4f(r, g, b, a);
}


void Color4fv(float *data)
{
	glColor4fv(data);
}


/*int	BeginScene(void)
{
	return DGL_OK;
}


int	EndScene(void)
{
	return DGL_OK;
}*/


void Begin(int mode)
{
	if(mode == DGL_SEQUENCE)
	{
		inSequence = DGL_TRUE;
		return;
	}

	// Remember the primitive type.
	primType = mode;

	glBegin(mode==DGL_LINES? GL_LINES 
		: mode==DGL_TRIANGLES? GL_TRIANGLES
		: mode==DGL_TRIANGLE_FAN? GL_TRIANGLE_FAN
		: mode==DGL_TRIANGLE_STRIP? GL_TRIANGLE_STRIP
		: mode==DGL_QUAD_STRIP? GL_QUAD_STRIP
		: GL_QUADS);
}


void End(void)
{
	if(!primType)
	{
		inSequence = DGL_FALSE;
		return;
	}
	glEnd();
	primType = DGL_FALSE;
}


void Vertex2f(float x, float y)
{
	glVertex2f(x, y);
}


void Vertex2fv(float *data)
{
	glVertex2fv(data);
}


void Vertex3f(float x, float y, float z)
{
	glVertex3f(x, y, z);
}


void Vertex3fv(float *data)
{
	glVertex3fv(data);
}


void TexCoord2f(float s, float t)
{
	glTexCoord2f(s, t);
}


void TexCoord2fv(float *data)
{
	glTexCoord2fv(data);
}


void Vertices2ftv(int num, gl_ft2vertex_t *data)
{
	for(; num>0; num--, data++)
	{
		glTexCoord2fv(data->tex);
		glVertex2fv(data->pos);		
	}
}


void Vertices3ftv(int num, gl_ft3vertex_t *data)
{
	for(; num>0; num--, data++)
	{
		glTexCoord2fv(data->tex);
		glVertex3fv(data->pos);		
	}
}


void Vertices3fctv(int num, gl_fct3vertex_t *data)
{
	for(; num>0; num--, data++)
	{
		glColor4fv(data->color);
		glTexCoord2fv(data->tex);
		glVertex3fv(data->pos);		
	}
}
	

void RenderList(int format, int num, void *data)
{
	int	i;
	rendlist_t *rl = data;
	rendlist_t **many = data;

	switch(format)
	{
	case DGL_NORMAL_LIST:
		for(i=0; i<num; i++) renderList(rl+i);
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


int Grab(int x, int y, int width, int height, int format, void *buffer)
{
	if(format != DGL_RGB) return DGL_UNSUPPORTED;
	// y+height-1 is the bottom edge of the rectangle. It's
	// flipped to change the origin.
	glReadPixels(x, FLIP(y+height-1), width, height, GL_RGB,
		GL_UNSIGNED_BYTE, buffer);
	return DGL_OK;
}


void Fog(int pname, float param)
{
	int		iparam = (int) param;

	switch(pname)
	{
	case DGL_FOG_MODE:
		glFogi(GL_FOG_MODE, param==DGL_LINEAR? GL_LINEAR
			: param==DGL_EXP? GL_EXP
			: GL_EXP2);
		break;

	case DGL_FOG_DENSITY:
		glFogf(GL_FOG_DENSITY, param);
		break;

	case DGL_FOG_START:
		glFogf(GL_FOG_START, param);
		break;

	case DGL_FOG_END:
		glFogf(GL_FOG_END, param);
		break;

	case DGL_FOG_COLOR:
		if(iparam >= 0 && iparam < 256)
		{
			float col[4];
			int i;
			for(i=0; i<4; i++)
				col[i] = palette[iparam].color[i] / 255.0f;
			glFogfv(GL_FOG_COLOR, col);
		}
		break;
	}
}


void Fogv(int pname, void *data)
{
	float	param = *(float*) data;
	byte	*ubvparam = (byte*) data;
	float	col[4];
	int		i;

	switch(pname)
	{
	case DGL_FOG_COLOR:
		for(i=0; i<4; i++)
			col[i] = ubvparam[i] / 255.0f;
		glFogfv(GL_FOG_COLOR, col);
		break;

	default:
		Fog(pname, param);
		break;
	}
}


// Clipping will be performed.
int Project(int num, gl_fc3vertex_t *inVertices, gl_fc3vertex_t *outVertices)
{
	GLdouble	modelMatrix[16], projMatrix[16];
	GLint		viewport[4];
	GLdouble	x, y, z;
	int			i, numOut;
	gl_fc3vertex_t *in = inVertices, *out = outVertices;

	if(num == 0) return 0;

	// Get the data we'll need in the operation.
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	glGetIntegerv(GL_VIEWPORT, viewport);
	for(i=numOut=0; i<num; i++, in++)
	{
		if(gluProject(in->pos[VX], in->pos[VY], in->pos[VZ],
			modelMatrix, projMatrix, viewport,
			&x, &y, &z) == GL_TRUE)
		{
			// A success: add to the out vertices.
			out->pos[VX] = (float) x;
			out->pos[VY] = (float) FLIP(y);
			out->pos[VZ] = (float) z;
			memcpy(out->color, in->color, sizeof(in->color));
			numOut++;
			out++;
		}
	}
	return numOut;
}


int ReadPixels(int *inData, int format, void *pixels)
{
	int		type = inData[0], num, *coords, i;
	float	*fv = pixels;
	
	if(format != DGL_DEPTH_COMPONENT) return DGL_UNSUPPORTED;

	// Check the type.
	switch(type)
	{
	case DGL_SINGLE_PIXELS:
		num = inData[1];
		coords = inData + 2;
		for(i=0; i<num; i++, coords+=2)
		{
			glReadPixels(coords[0], FLIP(coords[1]), 1, 1,
				GL_DEPTH_COMPONENT, GL_FLOAT, fv+i);
		}
		break;
	
	case DGL_BLOCK:
		coords = inData + 1;
		glReadPixels(coords[0], FLIP(coords[1]+coords[3]-1), coords[2], coords[3],
			GL_DEPTH_COMPONENT, GL_FLOAT, pixels);
		break;

	default:
		return DGL_UNSUPPORTED;
	}
	return DGL_OK;
}


int Gamma(int set, DGLubyte *data)
{
	return DGL_UNSUPPORTED;
}


// The API exchange.
gl_export_t* GetGLAPI(gl_import_t *api)
{
	// Get the imports, with them we can print stuff and
	// generate error messages.
	memcpy(&gim, api, api->apiSize);
			
	memset(&gex, 0, sizeof(gex));
	
	// Fill in the exports. Nothing must be left null!
	gex.version = DGL_VERSION_NUM;

	gex.Init = Init;
	gex.Shutdown = Shutdown;
	gex.ChangeMode = ChangeMode;

	gex.Clear = Clear;
	gex.OnScreen = OnScreen;
	gex.Viewport = Viewport;
	gex.Scissor = Scissor;

	gex.GetIntegerv = GetIntegerv;
	gex.SetInteger = SetInteger;
	gex.GetString = GetString;
	gex.Enable = Enable;
	gex.Disable = Disable;
	gex.Func = Func;

	gex.NewTexture = NewTexture;
	gex.DeleteTextures = DeleteTextures;
	gex.TexImage = LoadTexture;
	gex.TexParameter = TexParam;
	gex.GetTexParameterv = GetTexParameterv;
	gex.Palette = Palette;
	gex.Bind = Bind;

	gex.MatrixMode = MatrixMode;
	gex.PushMatrix = PushMatrix;
	gex.PopMatrix = PopMatrix;
	gex.LoadIdentity = LoadIdentity;
	gex.Translatef = Translatef;
	gex.Rotatef = Rotatef;
	gex.Scalef = Scalef;
	gex.Ortho = Ortho;
	gex.Perspective = Perspective;

	gex.Color3ub = Color3ub;
	gex.Color3ubv = Color3ubv;
	gex.Color4ub = Color4ub;
	gex.Color4ubv = Color4ubv;
	gex.Color3f = Color3f;
	gex.Color3fv = Color3fv;
	gex.Color4f = Color4f;
	gex.Color4fv = Color4fv;

	gex.Begin = Begin;
	gex.End = End;
	gex.Vertex2f = Vertex2f;
	gex.Vertex2fv = Vertex2fv;
	gex.Vertex3f = Vertex3f;
	gex.Vertex3fv = Vertex3fv;
	gex.TexCoord2f = TexCoord2f;
	gex.TexCoord2fv = TexCoord2fv;
	gex.Vertices2ftv = Vertices2ftv;
	gex.Vertices3ftv = Vertices3ftv;
	gex.Vertices3fctv = Vertices3fctv;
	
	gex.RenderList = RenderList;
	gex.Grab = Grab;
	gex.Fog = Fog;
	gex.Fogv = Fogv;
	gex.Project = Project;
	gex.ReadPixels = ReadPixels;
	gex.Gamma = Gamma;

	return &gex;
}