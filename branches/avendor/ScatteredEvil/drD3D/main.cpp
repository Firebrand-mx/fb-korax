// drD3D.dll
// Direct3D driver for the Doomsday Graphics Library

#include "drD3D.h"

// The APIs.
gl_import_t	gim;
gl_export_t	gex;


HINSTANCE			hinst;
//FILE				*debugfile;

// The current state.
int			screenWidth, screenHeight, screenBits, windowed;
int			maxTexSize;
int			palTexAvailable = DGL_FALSE;
int			dlBlend=0, lightTex=0;

//-------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	// Do we really need this?
	hinst = hinstDLL;
	return TRUE;
}

//-------------------------------------------------------------------------

void setupWindow()
{
	// Whatever the mode, let's be the topmost window!
	SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
	if(windowed)
	{
		RECT rect = { 0, 0, screenWidth, screenHeight };
		// We need to have a large enough client area.
		SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE|WS_CAPTION|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
		AdjustWindowRect(&rect, WS_VISIBLE|WS_CAPTION|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, FALSE);
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, rect.right-rect.left,
			rect.bottom-rect.top, 0);
	}
	else
	{
		// In fullscreen, the window is a bit different.
		SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE|WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, screenWidth, screenHeight, 0);
	}
}


int desktopBits()
{
	HWND hDesktop = GetDesktopWindow();
	HDC desktop_hdc = GetDC(hDesktop);
	int deskbpp = GetDeviceCaps(desktop_hdc, PLANES) * GetDeviceCaps(desktop_hdc, BITSPIXEL);

	ReleaseDC(hDesktop, desktop_hdc);
	return deskbpp;
}


// THE ROUTINES ------------------------------------------------------------

int Init(int width, int height, int bpp, int fullscreen)
{
	// Find our window. This may not be the best way to do this...
	hwnd = FindWindow("DoomMainWClass", NULL);

//	debugfile = fopen("d3d.log", "wt");

	screenWidth = width;
	screenHeight = height;
	screenBits = bpp? bpp : desktopBits();
	windowed = !fullscreen;

	// Setup the window.
	setupWindow();

	// Init DirectX.
	if(dxInit(DGL_TRUE) != DGL_OK) 
		gim.Error("drD3D.Init: DirectX init failed. Error: %i.\n", hr&0xffff);

	// Initialize the renderer.
	initState();

	return DGL_OK;
}


void Shutdown(void)
{
	deleteAllTextures(DGL_TRUE);	// Delete and unallocate the list.
	dxShutdown();
}


int	ChangeMode(int width, int height, int bpp, int fullscreen)
{
	if(!windowed && !fullscreen)
	{
		// We're currently in a fullscreen mode, but the caller
		// requests we change to windowed.
		IDirectDraw4_RestoreDisplayMode(ddraw);
		windowed = DGL_TRUE;
		screenWidth = width;
		screenHeight = height;
		screenBits = desktopBits();
		setupWindow();
		return dxRecreateSurfaces();
	}
	if(width && height && (width != screenWidth || height != screenHeight))
	{
		if(fullscreen) return DGL_ERROR;
		windowed = !fullscreen;
		screenWidth = width;
		screenHeight = height;
		screenBits = bpp? bpp : desktopBits();
		setupWindow();
		return dxRecreateSurfaces();
	}
	// Unsupported.
	return DGL_ERROR;
}


void Clear(int bufferbits)
{
	D3DRECT	rect = { 0, 0, screenWidth, screenHeight };
	int		mask = 0;

	if(bufferbits & DGL_COLOR_BUFFER_BIT) mask |= D3DCLEAR_TARGET;
	if(bufferbits & DGL_DEPTH_BUFFER_BIT) mask |= D3DCLEAR_ZBUFFER;

	IDirect3DViewport3_Clear2(d3dVp, 1, &rect, mask, 0, 1.0f, 0);
}


void OnScreen(void)
{
	if(windowed)
	{
		RECT rect;
		GetClientRect(hwnd, &rect);
		ClientToScreen(hwnd, (LPPOINT) &rect.left);
		ClientToScreen(hwnd, (LPPOINT) &rect.right);
		// Blit the back buffer on screen, into the window's client area.
		IDirectDrawSurface4_Blt(sPrimary, &rect, sBack, NULL, 0, NULL);
	}
	else
	{
		// Just do a flip.
		IDirectDrawSurface4_Flip(sPrimary, NULL, 0);
	}
	dxRestoreSurfaces();
}


// The API exchange.
extern "C" __declspec(dllexport) gl_export_t* GetGLAPI(gl_import_t *api)
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