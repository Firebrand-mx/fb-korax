// drD3D.dll
// Direct3D driver for the Doomsday Graphics Library
//
// dxinit.c : Routines to initialize DirectDraw and Direct3D

#include "drD3D.h"
#include <stdlib.h>
#include <io.h>
#include "resource.h"

#define TXFOUND_RGB			0x1
#define TXFOUND_RGBA		0x2
#define TXFOUND_PAL8		0x4
#define TXFOUND_EVERYTHING	(TXFOUND_RGB|TXFOUND_RGBA|TXFOUND_PAL8)

HWND					hwnd;
LPGUID					devGUID;
LPDIRECTDRAW			ddraw_1 = NULL;
LPDIRECTDRAW4			ddraw = NULL;
LPDIRECTDRAWSURFACE4	sPrimary = NULL, sBack = NULL, sDepth = NULL;
LPDIRECT3D3				d3d = NULL;
LPDIRECT3DDEVICE3		d3dDevice = NULL;
LPDIRECT3DVIEWPORT3		d3dVp = NULL, d3dScissor = NULL;
D3DVIEWPORT2			vpData, scissorData;
HRESULT					hr;				// The result of the last DX call.
DDPIXELFORMAT			pfZBuffer;		// Z-buffer pixel format.
DDPIXELFORMAT			pfTextureRGB;	// RGB texture pixel format.
DDPIXELFORMAT			pfTextureRGBA;	// RGBA texture pixel format.
DDPIXELFORMAT			pfTexturePal8;	// 8 bit palettized texture pixel format.
boolean					gammaSupport = DGL_FALSE;
LPDIRECTDRAWGAMMACONTROL ddGamma = NULL;

int						wantedBits;
char					wantedDriver[40];
unsigned int			wantedTexDepth = 0; //16;
unsigned int			wantedZDepth = 16;
int						wantGoodAlpha = DGL_TRUE;


extern HINSTANCE		hinst;

typedef struct 
{
	char driverName[256];
} driverinfo_t;

driverinfo_t			driverInfo[10];
int						numDrivers;

// -------------------------------------------------------------------------
// Configuration Dialog
// -------------------------------------------------------------------------

BOOL WINAPI ConfigDlgDriverEnum(GUID FAR *lpGUID, LPSTR lpDriverDescription, 
							 LPSTR lpDriverName, void *lpContext, HMONITOR hm)
{
	guard(ConfigDlgDriverEnum);
	strcpy(driverInfo[numDrivers++].driverName, lpDriverName);
	SendMessage( (HWND) lpContext, LB_ADDSTRING, 0, (LPARAM) 
		lpDriverDescription);
	return DDENUMRET_OK;
	unguard;
}

BOOL CALLBACK ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, 
							   LPARAM lParam)
{
	guard(ConfigDialogProc);
	int i;
	HWND it;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		// Init the list boxes.
		numDrivers = 0;
		DirectDrawEnumerateEx(ConfigDlgDriverEnum, 
			it = GetDlgItem(hwndDlg, IDC_DRIVER_LIST),
			DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
		for(i=0; i<numDrivers; i++)
			if(!strcmp(wantedDriver, driverInfo[i].driverName))
			{
				SendMessage(it, LB_SETCURSEL, (WPARAM) i, 0);
				break;
			}

		it = GetDlgItem(hwndDlg, IDC_COLOR_LIST);
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "Auto");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "16-bit");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "24-bit");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "32-bit");
		SendMessage(it, LB_SETCURSEL, (WPARAM) wantedBits==16? 1 
			: wantedBits==24? 2	: wantedBits==32? 3 : 0, 0);

		it = GetDlgItem(hwndDlg, IDC_TEXTURE_LIST);
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "Auto");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "16-bit");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "24-bit");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "32-bit");
		SendMessage(it, LB_SETCURSEL, (WPARAM) wantedTexDepth==16? 1 
			: wantedTexDepth==24? 2	: wantedTexDepth==32? 3 : 0, 0);
		
		it = GetDlgItem(hwndDlg, IDC_ZBUFFER_LIST);
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "Auto");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "16-bit");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "24-bit");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM) "32-bit");
		SendMessage(it, LB_SETCURSEL, (WPARAM) wantedZDepth==16? 1 
			: wantedZDepth==24? 2 : wantedZDepth==32? 3 : 0, 0);

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			// Store the settings.
			i = SendMessage(GetDlgItem(hwndDlg, IDC_DRIVER_LIST), 
				LB_GETCURSEL, 0, 0);
			if(i >= 0) strcpy(wantedDriver, driverInfo[i].driverName);
			
			i = SendMessage(GetDlgItem(hwndDlg, IDC_COLOR_LIST),
				LB_GETCURSEL, 0, 0);
			wantedBits = i==3? 32 : i==1? 16 : i==2? 24 : 0;

			i = SendMessage(GetDlgItem(hwndDlg, IDC_TEXTURE_LIST),
				LB_GETCURSEL, 0, 0);
			wantedTexDepth = i==3? 32 : i==1? 16 : i==2? 24 : 0;

			i = SendMessage(GetDlgItem(hwndDlg, IDC_ZBUFFER_LIST),
				LB_GETCURSEL, 0, 0);
			wantedZDepth = i==3? 32 : i==1? 16 : i==2? 24 : 0;

			EndDialog(hwndDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwndDlg, FALSE);
			return TRUE;
		}
	}
	return FALSE;
	unguard;
}
 
int ConfigDialog(void)
{	
	guard(ConfigDialog);
	int ret;

	ShowCursor(TRUE);
	ret = DialogBox(hinst, MAKEINTRESOURCE(IDD_D3DCONFIG), hwnd, 
		ConfigDialogProc);
	ShowCursor(FALSE);
	return ret;
	unguard;
}

void ReadConfig(void)
{
	guard(ReadConfig);
	static int first_time = DGL_TRUE;
	int showconf = DGL_FALSE;

	if(!first_time) return;
	first_time = DGL_FALSE;

	GetPrivateProfileString("drD3D", "Driver", "", wantedDriver, 40, 
		"drD3D.ini");
	wantedBits = GetPrivateProfileInt("drD3D", "Color", 16, "drD3D.ini");
	wantedTexDepth = GetPrivateProfileInt("drD3D", "Texture", 0, "drD3D.ini");
	wantedZDepth = GetPrivateProfileInt("drD3D", "ZBuffer", 16, "drD3D.ini");
	showconf = GetPrivateProfileInt("drD3D", "CfgNextTime", 1, "drD3D.ini");

	if(showconf 
		|| (GetAsyncKeyState(VK_SHIFT)
			| GetAsyncKeyState(VK_CONTROL)
			| GetAsyncKeyState(VK_MENU)) & 0x8000
		|| gim.CheckParm("-d3dcfg") /*|| Shift/Ctrl/Alt pressed */)
	{
		// Automagically disable config dialog for the next time.
		WritePrivateProfileString("drD3D", "CfgNextTime", "0", "drD3D.ini");

		if(ConfigDialog())
		{
			// Save the settings.
			char buf[80];
			WritePrivateProfileString("drD3D", "Driver", wantedDriver, 
				"drD3D.ini");
			sprintf(buf, "%i", wantedBits);
			WritePrivateProfileString("drD3D", "Color", buf, "drD3D.ini");
			sprintf(buf, "%i", wantedTexDepth);
			WritePrivateProfileString("drD3D", "Texture", buf, "drD3D.ini");
			sprintf(buf, "%i", wantedZDepth);
			WritePrivateProfileString("drD3D", "ZBuffer", buf, "drD3D.ini");
		}		
	}
	unguard;
}

// -------------------------------------------------------------------------

int GetDirectDraw4(GUID *guid)
{
	guard(GetDirectDraw4);
	// Destroy the previously created stuff.
	dxShutdown();

	// Create a new DD object.
	if(FAILED(hr = DirectDrawCreate(guid, &ddraw_1, NULL)))
		return DGL_ERROR;

	// Get the DD4 interface, and return it.
	if(FAILED(hr = IDirectDraw_QueryInterface(ddraw_1, IID_IDirectDraw4, (void **)&ddraw)))
		return DGL_ERROR;

	return DGL_OK;
	unguard;
}


BOOL WINAPI DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR lpDriverDescription, 
							 LPSTR lpDriverName, void *lpContext, HMONITOR hm)
{
	guard(DDEnumCallbackEx);
	DDCAPS	caps;

	gim.Message( "%s: %s\n", lpDriverName, lpDriverDescription);

	// Let's find out whether this device has what we need.
	if(GetDirectDraw4(lpGUID) != DGL_OK) return TRUE;

	if(wantedDriver[0])
	{
		if(strstr(lpDriverName, wantedDriver))
		{
			devGUID = lpGUID;
			*(int *)lpContext = DGL_TRUE;
			return FALSE;
		}
		else return TRUE;
	}

	memset(&caps, 0, sizeof(caps));
	caps.dwSize = sizeof(caps);

	IDirectDraw4_GetCaps(ddraw, &caps, NULL);
	gammaSupport = (caps.dwCaps2 & DDCAPS2_PRIMARYGAMMA)? DGL_TRUE : DGL_FALSE;
	gim.Message( "3dAccel:%i 3d:%i wndRend:%i zbuf:%i\n", 
		(caps.dwCaps & DDCAPS_3D) != 0,	
		(caps.ddsCaps.dwCaps & DDSCAPS_3DDEVICE) != 0,
		(caps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED) != 0,
		(caps.ddsCaps.dwCaps & DDSCAPS_ZBUFFER) != 0);

	// The first thing to check for is 3D acceleration.
	if(!(caps.dwCaps & DDCAPS_3D)) return TRUE;
	//if(!(caps.ddsCaps.dwCaps & DDSCAPS_3DDEVICE)) return TRUE;
	if(!(caps.ddsCaps.dwCaps & DDSCAPS_ZBUFFER)) return TRUE;
	if(windowed && !(caps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)) return TRUE;

	gim.Message(" ** Chosen.\n");

	// This is a good device.
	devGUID = lpGUID;
	*(int *)lpContext = DGL_TRUE;
	return FALSE; 
	unguard;
}
 

HRESULT CALLBACK EnumZBuffersCallback(LPDDPIXELFORMAT lpDDPixFmt, void *lpContext)
{
	guard(EnumZBuffersCallback);
	if(lpDDPixFmt->dwFlags & DDPF_ZBUFFER)
	{
		gim.Message( "ZDepth: %i\n", lpDDPixFmt->dwZBufferBitDepth);
		if(lpDDPixFmt->dwZBufferBitDepth == wantedZDepth)
		{
			// This'll do.
			memcpy(&pfZBuffer, lpDDPixFmt, sizeof(pfZBuffer));
			*(int *)lpContext = DGL_TRUE;
			gim.Message( " ** Chosen.\n");
			return FALSE;
		}
	}
	return TRUE;
	unguard
}


void printPixFormat(DDPIXELFORMAT *pf)
{
	guard(printPixFormat);
	gim.Message( "flags: (");
	if(pf->dwFlags & DDPF_RGB) gim.Message( "Rgb");
	if(pf->dwFlags & DDPF_ALPHAPIXELS) gim.Message( "Alpha");
	if(pf->dwFlags & DDPF_PALETTEINDEXED8) gim.Message( "Pal8");
	gim.Message( ") ");
	gim.Message( "bits:%i R:%x G:%x B:%x ",
		pf->dwRGBBitCount, pf->dwRBitMask, pf->dwGBitMask, pf->dwBBitMask);
	if(pf->dwFlags & DDPF_ALPHAPIXELS)
		gim.Message("A:%x", pf->dwRGBAlphaBitMask);
	gim.Message( "\n");
	unguard;
}


HRESULT CALLBACK EnumTexFormatsCallback(LPDDPIXELFORMAT lpDDPixFmt, void *lpFound)
{
	guard(EnumTexFormatsCallback);
	/*if((*lpFound & TXFOUND_EVERYTHING) == TXFOUND_EVERYTHING)
		return D3DENUMRET_CANCEL;	// We can stop, everything has been found.*/

	gim.Message( "* ");
	printPixFormat(lpDDPixFmt);

	if(lpDDPixFmt->dwFlags & DDPF_PALETTEINDEXED8)
	{
		if(!(*(int *)lpFound & TXFOUND_PAL8))
		{
			// This is good. No need to be picky.
			*(int *)lpFound |= TXFOUND_PAL8;
			memcpy(&pfTexturePal8, lpDDPixFmt, sizeof(DDPIXELFORMAT));
			//gim.Message( " ** Chosen (pal8).\n");
		}
		return D3DENUMRET_OK;
	}
	if(lpDDPixFmt->dwFlags & DDPF_RGB && 
		(lpDDPixFmt->dwRGBBitCount == wantedTexDepth || !wantedTexDepth))
	{
		if(!wantedTexDepth)
		{
			if(lpDDPixFmt->dwRGBBitCount < pfTextureRGB.dwRGBBitCount)
				return D3DENUMRET_OK;
		}

		if(lpDDPixFmt->dwFlags & DDPF_ALPHAPIXELS)
		{
			// Is this a good RGBA format?
			int newOff, newBits, oldOff, oldBits;
			unpackMask(lpDDPixFmt->dwRGBAlphaBitMask, &newOff, &newBits);
			unpackMask(pfTextureRGBA.dwRGBAlphaBitMask, &oldOff, &oldBits);
			if((wantGoodAlpha && newBits >= oldBits) 
				|| (!wantGoodAlpha && (newBits < oldBits || !oldBits)))
			{
				*(int *)lpFound |= TXFOUND_RGBA;
				memcpy(&pfTextureRGBA, lpDDPixFmt, sizeof(DDPIXELFORMAT));
				//gim.Message( " ** Chosen (RGBA).\n");
			}
			return D3DENUMRET_OK;
		}

		// Look for an RGB texture format.
		//if(!(*lpFound & TXFOUND_RGB))
		//{
		*(int *)lpFound |= TXFOUND_RGB;
		memcpy(&pfTextureRGB, lpDDPixFmt, sizeof(DDPIXELFORMAT));
			//gim.Message( " ** Chosen (RGB).\n");
		//}
	}
	return D3DENUMRET_OK;
	unguard;
}
 

// Creates the system objects, surfaces, devices, etc.
int dxInit(int firstTime)
{
	guard(dxInit);
	DDSURFACEDESC2	sd;
	int				found = DGL_FALSE;
	int				i;

	wantedBits = screenBits;
	strcpy(wantedDriver, "");

	ReadConfig();

	if(!wantedBits) wantedBits = screenBits;

	// Check the command line.
	if((i = gim.CheckParm("-texbits")) && i < gim.Argc()-1)
		wantedTexDepth = atoi(gim.Argv(i+1));
	if((i = gim.CheckParm("-zbits")) && i < gim.Argc()-1)
		wantedZDepth = atoi(gim.Argv(i+1));
	if((i = gim.CheckParm("-driver")) && i < gim.Argc()-1)
		strcpy(wantedDriver, gim.Argv(i+1));
	if(gim.CheckParm("-badtexalpha")) wantGoodAlpha = DGL_FALSE;

	if(firstTime)
	{
		gim.Message( "drD3D.dxInit: Enumerating DirectDraw devices.\n");

		// The first step is to decide which display device we should use.
		DirectDrawEnumerateEx(DDEnumCallbackEx, &found, 
			DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);

		if(!found)
		{
			gim.Message( "drD3D.dxInit: No suitable DirectDraw devices found.\n");
			return DGL_ERROR;
		}
	}
	else
	{
		GetDirectDraw4(devGUID);
	}

	// The device we are going to use is now initialized.
	// Set the cooperative level (exclusive if fullscreen).
	if(FAILED(hr = IDirectDraw4_SetCooperativeLevel(ddraw, hwnd, 
		windowed? DDSCL_NORMAL : DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)))
	{
		gim.Message("drD3D.dxInit: SetCooperativeLevel failed. Result:%i.\n", hr&0xffff);
		return DGL_ERROR;
	}

	memset(&sd, 0, sizeof(sd));
	sd.dwSize = sizeof(sd);

	if(windowed)
	{
		// Check that the bpp is right. 
		IDirectDraw4_GetDisplayMode(ddraw, &sd);
		if(sd.ddpfPixelFormat.dwRGBBitCount != (unsigned) wantedBits)
		{
			gim.Message( "drD3D.dxInit: Desktop color depth must be used in windowed mode.\n");
			return DGL_ERROR;
		}
	}
	else
	{
		// Set the appropriate display mode.
		if(FAILED(hr = IDirectDraw4_SetDisplayMode(ddraw, screenWidth, screenHeight,
			wantedBits, 0, 0)))
		{
			gim.Message( "drD3D.dxInit: Failed to change display mode to %i x %i x %i. Result:%i.\n",
				screenWidth, screenHeight, wantedBits, hr&0xffff);
			return DGL_ERROR;
		}
	}
	screenBits = wantedBits;

	if(windowed)
	{
		LPDIRECTDRAWCLIPPER clipper;

		// Create the primary surface.
		sd.dwFlags = DDSD_CAPS;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		if(FAILED(hr = IDirectDraw4_CreateSurface(ddraw, &sd, &sPrimary, NULL)))
			return DGL_ERROR;

		// Create the back buffer.
		sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
		sd.dwWidth = screenWidth;
		sd.dwHeight = screenHeight;
		if(FAILED(hr = IDirectDraw4_CreateSurface(ddraw, &sd, &sBack, NULL)))
			return DGL_ERROR;

		// Create a clipper.
		if(FAILED(hr = IDirectDraw4_CreateClipper(ddraw, 0, &clipper, NULL)))
			return DGL_ERROR;
		IDirectDrawClipper_SetHWnd(clipper, 0, hwnd);
		IDirectDrawSurface4_SetClipper(sPrimary, clipper);
		IDirectDrawClipper_Release(clipper);
	}
	else // Fullscreen?
	{
		DDSCAPS2 caps;

		// We're running in fullscreen, create a flipping chain.
		sd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE 
			| DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		sd.dwBackBufferCount = 1;
		if(FAILED(hr = IDirectDraw4_CreateSurface(ddraw, &sd, &sPrimary, NULL)))
		{
			gim.Message( "drD3D.dxInit: Failed to create the primary surface. Error: %i.\n", hr & 0xffff);
			return DGL_ERROR;
		}
		// Get a pointer to the back buffer.
		caps.dwCaps = DDSCAPS_BACKBUFFER;
		if(FAILED(hr = IDirectDrawSurface4_GetAttachedSurface(sPrimary,
			&caps, &sBack)))
		{
			gim.Message( "drD3D.dxInit: Failed to get/create the backbuffer. Error: %i.\n", hr & 0xffff);
			return DGL_ERROR;
		}
	}

	// Get the Direct3D interface.
	if(FAILED(hr = IDirectDraw4_QueryInterface(ddraw, IID_IDirect3D3, (void **)&d3d)))
		return DGL_ERROR;

	if(firstTime)
	{
		gim.Message( "drD3D.dxInit: Enumerating z-buffer pixel formats.\n");
		// Enumerate the possible z-buffer formats.
		found = DGL_FALSE;
		if(FAILED(hr = IDirect3D3_EnumZBufferFormats(d3d, IID_IDirect3DHALDevice, 
			EnumZBuffersCallback, (void **)&found)))
			return DGL_ERROR;
		if(!found)
		{
			gim.Message( "drD3D.dxInit: Couldn't find a suitable z-buffer pixel format.\n");
			return DGL_ERROR;
		}
	}
	// Create the z-buffer with the format we just got (pfZBuffer).
	sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
	sd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
	sd.dwWidth = screenWidth;
	sd.dwHeight = screenHeight;
	memcpy(&sd.ddpfPixelFormat, &pfZBuffer, sizeof(pfZBuffer));
	if(FAILED(hr = IDirectDraw4_CreateSurface(ddraw, &sd, &sDepth, NULL)))
	{
		gim.Message( "drD3D.dxInit: Z-buffer creation failed. Result: %i.\n",
			hr & 0xffff);
		return DGL_ERROR;
	}

	// Attach the z-buffer to the back buffer.
	IDirectDrawSurface4_AddAttachedSurface(sBack, sDepth);

	// Now create the rendering device.
	if(FAILED(hr = IDirect3D3_CreateDevice(d3d, IID_IDirect3DHALDevice, sBack, 
		&d3dDevice, NULL)))
	{
		gim.Message( "drD3D.dxInit: D3D device creation failed. Result: %i.\n",
			hr & 0xffff);
		return DGL_ERROR;
	}

	if(firstTime)
	{
		gim.Message( "drD3D.dxInit: Enumerating texture formats.\n");
		// Enumerate the available texture formats.
		found = 0;
		memset(&pfTextureRGB, 0, sizeof(DDPIXELFORMAT));
		memset(&pfTextureRGBA, 0, sizeof(DDPIXELFORMAT));
		if(FAILED(hr = IDirect3DDevice3_EnumTextureFormats(d3dDevice, 
			EnumTexFormatsCallback, &found)))
			return DGL_ERROR;
		if(!(found & TXFOUND_RGB) || !(found & TXFOUND_RGBA))
		{
			gim.Message( "drD3D.dxInit: Couldn't find suitable texture formats!\n");
			return DGL_ERROR;
		}
		if(found & TXFOUND_PAL8) palTexAvailable = DGL_TRUE;

		// Print a report.
		gim.Message( "Chosen texture formats:\n");
		gim.Message( "RGB:  ");
		printPixFormat(&pfTextureRGB);
		gim.Message( "RGBA: ");
		printPixFormat(&pfTextureRGBA);
		
		if(found & TXFOUND_PAL8)
		{
			gim.Message( "Pal8: ");
			printPixFormat(&pfTexturePal8);
		}
	}

	// Setup the default viewport.
	memset(&vpData, 0, sizeof(vpData));
	vpData.dwSize = sizeof(vpData);
	vpData.dwWidth = screenWidth;
	vpData.dwHeight = screenHeight;
	vpData.dvClipX = -1;
	vpData.dvClipY = 1;
	vpData.dvClipWidth = 2;
	vpData.dvClipHeight = 2;
	vpData.dvMaxZ = 1;

	// Create the viewport.
	if(FAILED(hr = IDirect3D3_CreateViewport(d3d, &d3dVp, NULL)))
		return DGL_ERROR;

	IDirect3DDevice3_AddViewport(d3dDevice, d3dVp);
	IDirect3DViewport3_SetViewport2(d3dVp, &vpData);
	IDirect3DDevice3_SetCurrentViewport(d3dDevice, d3dVp);

	// Create the clipper, initialized with the same data as the regular viewport.
	if(FAILED(hr = IDirect3D3_CreateViewport(d3d, &d3dScissor, NULL)))
		return DGL_ERROR;

	memcpy(&scissorData, &vpData, sizeof(vpData));
	IDirect3DDevice3_AddViewport(d3dDevice, d3dScissor);
	IDirect3DViewport3_SetViewport2(d3dScissor, &scissorData);

	// The gamma control?
	if(gammaSupport)
	{
		if(FAILED(hr = IDirectDrawSurface4_QueryInterface(sPrimary, 
			IID_IDirectDrawGammaControl, (void **)&ddGamma)))
		{
			gim.Message( "Query for gamma control failed!\n");
			gammaSupport = DGL_FALSE;
			ddGamma = NULL;
		}
	}

	// All right!
	return DGL_OK;
	unguard;
}

void dxDestroySurfaces()
{
	guard(dxDestroySurfaces);
	if(sDepth) IDirectDrawSurface4_Release(sDepth);
	if(sBack) IDirectDrawSurface4_Release(sBack);
	if(sPrimary) IDirectDrawSurface4_Release(sPrimary);
	sDepth = NULL;
	sBack = NULL;
	sPrimary = NULL;
	unguard;
}

// Destroys all system objects, surfaces, devices, etc.
int dxShutdown()
{
	guard(dxShutdown);
	enablePalTexExt(DGL_FALSE);

	if(d3dScissor) IDirect3DViewport3_Release(d3dScissor);
	if(d3dVp) IDirect3DViewport3_Release(d3dVp);
	if(d3dDevice) IDirect3DDevice3_Release(d3dDevice);
	dxDestroySurfaces();
	if(d3d) IDirect3D3_Release(d3d);
	if(ddraw) IDirectDraw4_Release(ddraw);
	if(ddraw_1) IDirectDraw_Release(ddraw_1);

	// Set everything to null.
	ddraw_1 = NULL;
	ddraw = NULL;
	d3d = NULL;
	d3dDevice = NULL;
	d3dVp = NULL;
	d3dScissor = NULL;
	return DGL_OK;
	unguard;
}

int dxRestoreSurfaces()
{
	guard(dxRestoreSurfaces);
	if(sPrimary) 
		if(FAILED(IDirectDrawSurface4_IsLost(sPrimary)))
		{
			IDirectDrawSurface4_Restore(sPrimary);
		}
	if(sBack) 
		if(FAILED(IDirectDrawSurface4_IsLost(sBack)))
		{
			IDirectDrawSurface4_Restore(sBack);
		}
	if(sDepth)
		if(FAILED(IDirectDrawSurface4_IsLost(sDepth)))
		{
			IDirectDrawSurface4_Restore(sDepth);
		}
	return DGL_OK;
	unguard;
}

int dxRecreateSurfaces()
{
	guard(dxRecreateSurfaces);
	DDSURFACEDESC2	sd;
/*	LPDIRECTDRAWSURFACE4	oldPrimary = sPrimary;
	LPDIRECTDRAWSURFACE4	oldDepth = sDepth, oldBack = sBack;*/

	if(sPrimary)
	{
		IDirectDrawSurface4_Release(sPrimary);
		sPrimary = NULL;
	}

	memset(&sd, 0, sizeof(sd));
	sd.dwSize = sizeof(sd);

	if(windowed)
	{
		LPDIRECTDRAWCLIPPER clipper;

		// Create the primary surface.
		sd.dwFlags = DDSD_CAPS;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		if(FAILED(hr = IDirectDraw4_CreateSurface(ddraw, &sd, &sPrimary, NULL)))
			return DGL_ERROR;

		// Create the back buffer.
		sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
		sd.dwWidth = screenWidth;
		sd.dwHeight = screenHeight;
		if(FAILED(hr = IDirectDraw4_CreateSurface(ddraw, &sd, &sBack, NULL)))
			return DGL_ERROR;

		// Create a clipper.
		if(FAILED(hr = IDirectDraw4_CreateClipper(ddraw, 0, &clipper, NULL)))
			return DGL_ERROR;
		IDirectDrawClipper_SetHWnd(clipper, 0, hwnd);
		IDirectDrawSurface4_SetClipper(sPrimary, clipper);
		IDirectDrawClipper_Release(clipper);
	}
	else // Fullscreen?
	{
		DDSCAPS2 caps;

		if(FAILED(hr = IDirectDraw4_SetDisplayMode(ddraw, screenWidth, screenHeight,
			screenBits, 0, 0)))
		{
			gim.Message( "drD3D.dxInit: Failed to change display mode to %i x %i x %i. Result:%i.\n",
				screenWidth, screenHeight, screenBits, hr&0xffff);
			return DGL_ERROR;
		}

		// We're running in fullscreen, create a flipping chain.
		sd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE 
			| DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		sd.dwBackBufferCount = 1;
		if(FAILED(hr = IDirectDraw4_CreateSurface(ddraw, &sd, &sPrimary, NULL)))
		{
			gim.Message( "drD3D.dxInit: Failed to create the primary surface. Error: %i.\n", hr & 0xffff);
			return DGL_ERROR;
		}
		// Get a pointer to the back buffer.
		caps.dwCaps = DDSCAPS_BACKBUFFER;
		if(FAILED(hr = IDirectDrawSurface4_GetAttachedSurface(sPrimary,
			&caps, &sBack)))
		{
			gim.Message( "drD3D.dxInit: Failed to get/create the backbuffer. Error: %i.\n", hr & 0xffff);
			return DGL_ERROR;
		}
	}
	// Create the z-buffer.
	memset(&sd, 0, sizeof(sd));
	sd.dwSize = sizeof(sd);
	sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
	sd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
	sd.dwWidth = screenWidth;
	sd.dwHeight = screenHeight;
	memcpy(&sd.ddpfPixelFormat, &pfZBuffer, sizeof(pfZBuffer));
	if(FAILED(hr = IDirectDraw4_CreateSurface(ddraw, &sd, &sDepth, NULL)))
	{
		gim.Message( "drD3D.dxInit: Z-buffer creation failed. Result: %i.\n",
			hr & 0xffff);
		return DGL_ERROR;
	}
	// Attach the z-buffer to the back buffer.
	IDirectDrawSurface4_AddAttachedSurface(sBack, sDepth);

	gim.Message("Setting render target to %p.\n", sBack);

	// Update the rendering target.
	//IDirect3DDevice3_SetRenderTarget(d3dDevice, sBack, 0);

	gim.Message("Done.\n");

/*	if(oldDepth) IDirectDrawSurface4_Release(oldDepth);
	if(oldBack) IDirectDrawSurface4_Release(oldBack);
	if(oldPrimary) IDirectDrawSurface4_Release(oldPrimary);*/
	
	return DGL_OK;
	unguard;
}