// drD3D.dll
// Direct3D driver for the Doomsday Graphics Library
//
// state.c : Direct3D rendering state management

#include "drD3D.h"

int			usefog;


// --------------------------------------------------------------------------

void initState()
{
	guard(initState);
	D3DDEVICEDESC	devdesc, dummy;

	currentTexName = 0;
	lightTex = 0;
	useScissor = DGL_FALSE;

	// The projection matrix.
	MatrixMode(DGL_PROJECTION);
	LoadIdentity();

	// Clear also the texture matrix (I'm not using this, though).
	MatrixMode(DGL_TEXTURE);
	LoadIdentity();

	// Initialize the modelview matrix.
	MatrixMode(DGL_MODELVIEW);
	LoadIdentity();

	// Clear the current vertex state.
	memset(&currentVertex, 0, sizeof(vertex_t));

	SetRS(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);	
	SetRS(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
	SetRS(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	SetRS(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRS(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	SetRS(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	SetRS(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
	SetRS(D3DRENDERSTATE_ALPHAREF, 0);

	if(!gim.CheckParm("-nodither"))
		SetRS(D3DRENDERSTATE_DITHERENABLE, TRUE);

	// This enables us to alpha blend RGBA textures with the 
	// current alpha color.
	SetTSS(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	// Fog.
	usefog = DGL_FALSE;
	SetRS(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR);
	SetRS(D3DRENDERSTATE_FOGTABLESTART, 0);
	SetRS(D3DRENDERSTATE_FOGTABLEEND, 2100);
	SetRS(D3DRENDERSTATE_FOGCOLOR, 0x8a8a8a);

	// Find out some info, like the min/max texture size.
	memset(&devdesc, 0, sizeof(devdesc));
	devdesc.dwSize = sizeof(devdesc);
	memset(&dummy, 0, sizeof(dummy));
	dummy.dwSize = sizeof(dummy);
	IDirect3DDevice3_GetCaps(d3dDevice, &devdesc, &dummy);

	maxTexSize = devdesc.dwMaxTextureWidth;
	if(devdesc.dwMaxTextureHeight < (unsigned) maxTexSize)
		maxTexSize = devdesc.dwMaxTextureHeight;

	gim.Message( "Supported texture sizes: min: %ix%i, max: %ix%i\n",
		devdesc.dwMinTextureWidth,
		devdesc.dwMinTextureHeight,
		devdesc.dwMaxTextureWidth,
		devdesc.dwMaxTextureHeight);

	// Check for some special cases.
	if(devdesc.dwMinTextureWidth > 1 || devdesc.dwMinTextureHeight > 1)
		gim.Message( "Warning: No 1x1 textures. You may encounter problems.\n");
	if(devdesc.dwMaxTextureWidth != devdesc.dwMaxTextureHeight)
	{
		gim.Message( "Warning: %i is considered the maximum size for width AND height.\n",
			maxTexSize);
	}

	// How about gamma support?
	if(gammaSupport)
	{
		gim.Message( "Driver supports dynamic gamma ramps for primary surface.\n");
	}
	unguard;
}


// PUBLIC FUNCTIONS ---------------------------------------------------------

int	GetIntegerv(int name, int *v)
{
	guard(GetIntegerv);
	switch(name)
	{
	case DGL_MAX_TEXTURE_SIZE:
		*v = maxTexSize;
		break;

	case DGL_PALETTED_TEXTURES:
		*v = usePalTex;
		break;

	case DGL_SCISSOR_TEST:
		*v = useScissor;
		break;

	case DGL_SCISSOR_BOX:
		v[0] = scissorData.dwX;
		v[1] = scissorData.dwY;
		v[2] = scissorData.dwWidth;
		v[3] = scissorData.dwHeight;
		break;

	case DGL_FOG:
		*v = usefog;
		break;

	case DGL_VERSION:
		*v = DGL_VERSION;
		break;

	case DGL_R:
		*v = (currentVertex.rgba >> 16) & 0xff;
		break;

	case DGL_G:
		*v = (currentVertex.rgba >> 8) & 0xff;
		break;

	case DGL_B:
		*v = currentVertex.rgba & 0xff;
		break;

	case DGL_A:
		*v = (currentVertex.rgba >> 24) & 0xff;
		break;

	default:
		return DGL_ERROR;
	}
	return DGL_OK;
	unguard;
}


int	SetInteger(int name, int value)
{
	guard(SetInteger);
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
	unguard;
}


char* GetString(int name)
{
	guard(GetString);
	switch(name)
	{
	case DGL_VERSION:
		return DRD3D_VERSION_FULL;
	}
	return NULL;
	unguard;
}
	

void Enable(int cap)
{
	guard(Enable);
	switch(cap)
	{
	case DGL_TEXTURING:
		SetTSS(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		break;

	case DGL_BLENDING:
		SetRS(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		break;

	case DGL_FOG:
		SetRS(D3DRENDERSTATE_FOGENABLE, TRUE);
		usefog = DGL_TRUE;
		break;

	case DGL_DEPTH_TEST:
		SetRS(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
		break;

	case DGL_ALPHA_TEST:
		SetRS(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
		break;

	case DGL_CULL_FACE:
		SetRS(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
		break;

	case DGL_SCISSOR_TEST:
		enableScissor(DGL_TRUE);
		break;

	case DGL_COLOR_WRITE:
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		break;

	case DGL_DEPTH_WRITE:
		SetRS(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		break;

	case DGL_PALETTED_TEXTURES:
		enablePalTexExt(DGL_TRUE);
		break;
	}
	unguard;
}

void Disable(int cap)
{
	guard(Disable);
	switch(cap)
	{
	case DGL_TEXTURING:
		SetTSS(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		break;

	case DGL_BLENDING:
		SetRS(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		break;

	case DGL_FOG:
		SetRS(D3DRENDERSTATE_FOGENABLE, FALSE);
		usefog = DGL_FALSE;
		break;

	case DGL_DEPTH_TEST:
		SetRS(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
		break;

	case DGL_ALPHA_TEST:
		SetRS(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
		break;

	case DGL_CULL_FACE:
		SetRS(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
		break;

	case DGL_SCISSOR_TEST:
		enableScissor(DGL_FALSE);
		break;

	case DGL_COLOR_WRITE:
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		break;

	case DGL_DEPTH_WRITE:
		SetRS(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		break;

	case DGL_PALETTED_TEXTURES:
		enablePalTexExt(DGL_FALSE);
		break;
	}
	unguard;
}


void Func(int func, int param1, int param2)
{
	guard(Func);
	D3DBLEND d3dBlendMode[] =
	{
		D3DBLEND_ZERO,
		D3DBLEND_ONE,
		D3DBLEND_DESTCOLOR,
		D3DBLEND_INVDESTCOLOR,
		D3DBLEND_DESTALPHA,
		D3DBLEND_INVDESTALPHA,
		D3DBLEND_SRCCOLOR,
		D3DBLEND_INVSRCCOLOR,
		D3DBLEND_SRCALPHA,
		D3DBLEND_INVSRCALPHA,
		D3DBLEND_SRCALPHASAT
	};
	D3DCMPFUNC d3dCmpFunc[] =
	{
		D3DCMP_ALWAYS,
		D3DCMP_NEVER,
		D3DCMP_LESS,
		D3DCMP_EQUAL,
		D3DCMP_LESSEQUAL,
		D3DCMP_GREATER,
		D3DCMP_GREATEREQUAL,
		D3DCMP_NOTEQUAL
	};

	switch(func)
	{
	case DGL_BLENDING:
		if(param1 >= DGL_ZERO && param1 <= DGL_SRC_ALPHA_SATURATE &&
			param2 >= DGL_ZERO && param2 <= DGL_SRC_ALPHA_SATURATE)
		{
			SetRS(D3DRENDERSTATE_SRCBLEND, d3dBlendMode[param1 - DGL_ZERO]);
			SetRS(D3DRENDERSTATE_DESTBLEND, d3dBlendMode[param2 - DGL_ZERO]);
		}
		break;

	case DGL_DEPTH_TEST:
		if(param1 >= DGL_ALWAYS && param1 <= DGL_NOTEQUAL)
			SetRS(D3DRENDERSTATE_ZFUNC, d3dCmpFunc[param1 - DGL_ALWAYS]);
		break;

	case DGL_ALPHA_TEST:
		if(param1 >= DGL_ALWAYS && param1 <= DGL_NOTEQUAL)
		{
			SetRS(D3DRENDERSTATE_ALPHAFUNC, d3dCmpFunc[param1 - DGL_ALWAYS]);
			SetRS(D3DRENDERSTATE_ALPHAREF, param2);
		}
		break;
	}
	unguard;
}


void Fog(int pname, float param)
{
	guard(Fog);
	int		iparam = (int) param;

	switch(pname)
	{
	case DGL_FOG_MODE:
		SetRS(D3DRENDERSTATE_FOGTABLEMODE, 
			param==DGL_LINEAR? D3DFOG_LINEAR 
			: param==DGL_EXP? D3DFOG_EXP
			: D3DFOG_EXP2);
		break;

	case DGL_FOG_DENSITY:
		SetRS(D3DRENDERSTATE_FOGTABLEDENSITY, *(DWORD*) &param);
		break;

	case DGL_FOG_START:
		SetRS(D3DRENDERSTATE_FOGTABLESTART, *(DWORD*) &param);
		break;

	case DGL_FOG_END:
		SetRS(D3DRENDERSTATE_FOGTABLEEND, *(DWORD*) &param);
		break;

	case DGL_FOG_COLOR:
		if(iparam >= 0 && iparam < 256)
		{
			SetRS(D3DRENDERSTATE_FOGCOLOR, 
				MAKERGB_A(palette[iparam].color, 0xff));
		}
		break;
	}
	unguard;
}


void Fogv(int pname, void *data)
{
	guard(Fogv);
	float	param = *(float*) data;
	byte	*ubvparam = (byte*) data;

	switch(pname)
	{
	case DGL_FOG_COLOR:
		SetRS(D3DRENDERSTATE_FOGCOLOR, 
			MAKE_RGBA(ubvparam[0], ubvparam[1], ubvparam[2], ubvparam[3]));
		break;

	default:
		Fog(pname, param);
		break;
	}
	unguard;
}
