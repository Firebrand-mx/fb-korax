// drD3D.dll
// Direct3D driver for the Doomsday Graphics Library
//
// texture.c : Texture routines

#include <stdio.h>
#include <stdlib.h>
#include "drD3D.h"
#include "tga.h"

namespace Direct3DDriver {

#define ALLOCBATCH		20		// Allocate this many at one go.
#define NULL_INDEX		-1		// The index of the null texture.

// Texture flags.
#define TXF_IN_USE		0x1		// The texture is current in use.

typedef int	texindex_t;

enum 
{
	BTM_NONE,
	BTM_NEAREST,
	BTM_LINEAR,
};

#define IDX_TO_NAME(x)	( (DGLuint) ((x)+1) )
#define NAME_TO_IDX(x)	( (texindex_t) ((x)-1) )

typedef struct
{
	int						flags;
	LPDIRECTDRAWSURFACE4	surface;	
	LPDIRECT3DTEXTURE2		d3dTex;		
	D3DTEXTUREFILTER		minFilter, magFilter;
	D3DTEXTUREADDRESS		sAddr, tAddr;
} texture_t;

// State information.
LPDIRECTDRAWPALETTE ddPal = NULL;
rgba_t		palette[256];
int			usePalTex = DGL_FALSE;
DGLuint		currentTexName = 0;
int			dumpTextures = DGL_FALSE;
int			maxMipmapLevels = 6;

// The textures.
texture_t	*textures = NULL;
int			numTextures = 0;

//extern FILE *debugfile;


// --------------------------------------------------------------------------

void loadPalette()
{
	guard(loadPalette);
	int		i;
	D3DCOLOR paldata[256];

	if(usePalTex == DGL_FALSE) return;

	memset(paldata, 0, sizeof(paldata));
	// Prepare the color table (RGBA -> RGB).
	for(i=0; i<256; i++)
		paldata[i] = palette[i].color[CR] + (palette[i].color[CG]<<8) + (palette[i].color[CB]<<16);
	IDirectDrawPalette_SetEntries(ddPal, 0, 0, 256, (LPPALETTEENTRY) paldata);
	unguard;
}


int enablePalTexExt(int enable)
{
	guard(enablePalTexExt);
	if(!palTexAvailable) return DGL_FALSE;
	if(enable && usePalTex || !enable && !usePalTex) return DGL_TRUE;

	if(!enable && usePalTex)
	{
		usePalTex = DGL_FALSE;
		IDirectDrawPalette_Release(ddPal);
		ddPal = NULL;
		return DGL_TRUE;
	}	

	usePalTex = DGL_TRUE;
	
	// Create the palette object.
	if(FAILED(hr = IDirectDraw4_CreatePalette(ddraw, DDPCAPS_8BIT | DDPCAPS_ALLOW256, 
		(LPPALETTEENTRY) palette, &ddPal, NULL)))
	{
		I_Error("drD3D.enablePalTexExt: Failed to create the palette. Result:%i.\n", hr&0xffff);
	}
	loadPalette();
	return DGL_TRUE;
	unguard;
}	


void defaultTexParams(texture_t *tex)
{
	guardSlow(defaultTexParams);
	tex->minFilter = D3DFILTER_NEAREST;
	tex->magFilter = D3DFILTER_LINEAR;
	tex->sAddr = D3DTADDRESS_WRAP;
	tex->tAddr = D3DTADDRESS_WRAP;
	unguardSlow;
}

texture_t *getCurTexture()
{
	guardSlow(getCurTexture);
	if(!currentTexName) return NULL;
	return textures + NAME_TO_IDX(currentTexName);
	unguard;
}


// Return a texture index, which is marked as 'in use'.
texindex_t getNewTexture()
{
	guard(getNewTexture);
	texindex_t	i;

	// See if there is a free texture in the texture list.
	for(i=0; i<numTextures; i++)
		if(!(textures[i].flags & TXF_IN_USE))
		{
			// This is perfect for us.
			textures[i].flags |= TXF_IN_USE;
			defaultTexParams(textures+i);
			return i;
		}

	i = numTextures;
	// There are no free texture indices. We must allocate more.
	textures = (texture_t *)realloc(textures, sizeof(texture_t) * (numTextures += ALLOCBATCH));
	memset(textures + i, 0, sizeof(texture_t) * ALLOCBATCH);
	textures[i].flags |= TXF_IN_USE;
	defaultTexParams(textures+i);
	return i;
	unguard;
}


void delTexture(texindex_t idx)
{
	guard(delTexture);
	texture_t	*tex;

	if(idx < 0 || idx > numTextures-1) return;
	tex = textures + idx;
	if(!(tex->flags & TXF_IN_USE)) return; // Not in use, can't delete.

//	fprintf(debugfile, "del tex: %i\n", idx);

	// Clear flags and release the surfaces/interfaces, if necessary.
	if(tex->d3dTex) IDirect3DTexture2_Release(tex->d3dTex);
	if(tex->surface) IDirectDrawSurface4_Release(tex->surface);
	memset(tex, 0, sizeof(*tex));
	unguard;
}


void deleteAllTextures(int unalloc)
{
	guard(deleteAllTextures);
	int		i;

	for(i=0; i<numTextures; i++) delTexture(i);
	if(unalloc)
	{
		free(textures);
		textures = 0;
		numTextures = 0;
	}
	unguard;
}


void createMipmapSurfaces(int mipmap, texture_t *tex, int width, int height, DDPIXELFORMAT *pf)
{
	guard(createMipmapSurfaces);
	DDSURFACEDESC2	sd;

	if(mipmap) return;

	// Release any previously created interfaces.
	if(tex->d3dTex) IDirect3DTexture2_Release(tex->d3dTex);
	if(tex->surface) IDirectDrawSurface4_Release(tex->surface);

	memset(&sd, 0, sizeof(sd));
	sd.dwSize = sizeof(sd);
	sd.dwFlags = DDSD_CAPS /*| DDSD_MIPMAPCOUNT */| DDSD_WIDTH | DDSD_HEIGHT
		| DDSD_TEXTURESTAGE | DDSD_PIXELFORMAT;
	//sd.dwMipMapCount = 2;//maxMipMapLevels;
	sd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	sd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	sd.dwWidth = width;
	sd.dwHeight = height;
	sd.dwTextureStage = 0;
	memcpy(&sd.ddpfPixelFormat, pf, sizeof(DDPIXELFORMAT));
	
	if(FAILED(hr = IDirectDraw4_CreateSurface(ddraw, &sd, &tex->surface, NULL)))
		I_Error("drD3D.createSurfaces: Failed to create texture surfaces (%ix%i). Error:%i.\n", 
		width, height, hr&0xffff);

	// Get the D3DTexture interface.
	if(FAILED(hr = IDirectDrawSurface4_QueryInterface(tex->surface, 
		IID_IDirect3DTexture2, (void **)&tex->d3dTex)))
		I_Error("drD3D.createSurfaces: Failed to get the D3DTexture2 interface.\n");

	IDirect3DDevice3_SetTexture(d3dDevice, 0, tex->d3dTex);
	unguard;
}


void unpackMask(DWORD mask, int *offset, int *bits)
{
	guardSlow(unpackMask);
	int		i;

	*offset = 0;
	*bits = 0;
	// Find the beginning of the mask.
	for(i=0; i<32; i++)
		if(mask & (1<<i)) 
		{
			*offset = i;
			break;
		}
	// How long is it?
	for(; i<32; i++)
		if(mask & (1<<i)) 
			(*bits)++; 
		else break;
	unguardSlow;
}


void copyImage32(DDSURFACEDESC2 *dsd, byte *rgba)
{
	guard(copyImage32);
	int		offsets[4], bits[4], i;
	int		size = dsd->ddpfPixelFormat.dwRGBBitCount/8;
	unsigned x, y, temp;
	byte	*linestart = (byte *)dsd->lpSurface, *out;
	int		comps;
	
	// Determine the pixel format.
	unpackMask(dsd->ddpfPixelFormat.dwRBitMask, offsets+CR, bits+CR);
	unpackMask(dsd->ddpfPixelFormat.dwGBitMask, offsets+CG, bits+CG);
	unpackMask(dsd->ddpfPixelFormat.dwBBitMask, offsets+CB, bits+CB);
	if(dsd->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
	{
		unpackMask(dsd->ddpfPixelFormat.dwRGBAlphaBitMask, offsets+CA, bits+CA);
		comps = 4;
	}
	else
	{
		offsets[CA] = bits[CA] = 0;
		comps = 3;
	}

	for(y=0; y<dsd->dwHeight; y++, linestart += dsd->lPitch)
	{
		out = linestart;
		for(x=0; x<dsd->dwWidth; x++, out += size)
		{
			// rgba is the current input pixel.
			temp = 0;
			for(i=0; i<comps; i++) temp += (rgba[i] >> (8-bits[i])) << offsets[i];
			memcpy(out, &temp, size);
			rgba += 4;
		}
	}
	unguard;
}


void copyImage8(DDSURFACEDESC2 *dsd, byte *data)
{
	guard(copyImage8);
	byte	*line = (byte *)dsd->lpSurface;
	unsigned y;

	// Copy line by line.
	for(y=0; y<dsd->dwHeight; y++, line += dsd->lPitch, data += dsd->dwWidth)
		memcpy(line, data, dsd->dwWidth);
	unguard;
}


void uploadImage(texture_t *tex, int mipmap, int width, int height, 
				   int comps, byte *rgbaBuffer)
{
	guard(uploadImage);
	LPDIRECTDRAWSURFACE4 sLevel, sNext;
	DDSURFACEDESC2 dsd;
	DDSCAPS2 caps;
	int i;

	// We need to find the surface of the correct mipmap level 
	// and blit tempsurf to it.
	sLevel = tex->surface;

	if(mipmap)
	{
		caps.dwCaps = DDSCAPS_MIPMAP | DDSCAPS_TEXTURE;

		IDirectDrawSurface4_AddRef(sLevel);	// We don't want to lose this one.
		// Go browsing through the surfaces.	
		for(i=0; i<mipmap; i++)
		{
			if(FAILED(hr = IDirectDrawSurface4_GetAttachedSurface(sLevel,
				&caps, &sNext)))
			{
				if(hr == DDERR_NOTFOUND)
				{
					IDirectDrawSurface4_Release(sLevel);
					return;
				}
				I_Error("drD3D.uploadImage: Couldn't get next at miplevel %i. W:%i H:%i. Error: %i\n", i, width, height, hr&0xffff);
			}
			IDirectDrawSurface4_Release(sLevel);
			sLevel = sNext;
		}
	}

	memset(&dsd, 0, sizeof(dsd));
	dsd.dwSize = sizeof(dsd);

	IDirectDrawSurface4_GetSurfaceDesc(sLevel, &dsd);
	if(dsd.dwWidth != (unsigned) width || dsd.dwHeight != (unsigned) height)
	{
		I_Error("drD3D.uploadImage (%p): Mipmap surface is of the wrong size!\nThe real %ix%i is NOT %ux%u\n",
			sLevel, width, height, dsd.dwWidth, dsd.dwHeight);
	}

	// Copy the data to the surface.
	if(FAILED(hr = IDirectDrawSurface4_Lock(sLevel, NULL, &dsd, 
		DDLOCK_SURFACEMEMORYPTR, NULL)))
		I_Error("uploadImage: Couldn't lock.\n");
	// Use the correct image copier.
	if(comps == 4)
		copyImage32(&dsd, rgbaBuffer);
	else if(comps == 1)
		copyImage8(&dsd, rgbaBuffer);
	if(FAILED(hr = IDirectDrawSurface4_Unlock(sLevel, NULL)))
		I_Error("drD3D.createTempSurf: Couldn't unlock.\n");

	// Don't forget to attach the palette!
	if(comps == 1)
		IDirectDrawSurface4_SetPalette(sLevel, ddPal);

	if(mipmap) IDirectDrawSurface4_Release(sLevel);
	unguard;
}


int Power2(int num)
{
	int cumul;
	for(cumul=1; num > cumul; cumul <<= 1);
	return cumul;
}


// API ROUTINES -------------------------------------------------------------

DGLuint NewTexture(void)
{
	guard(NewTexture);
	DGLuint texName = IDX_TO_NAME(getNewTexture());
	currentTexName = texName;
	return texName;
	unguard;
}


// Width and height must be powers of two.
int LoadTexture(int format, int width, int height, int mipmap, void *data)
{
	guard(LoadTexture);
	byte		*bdata = (byte *)data;
	texture_t	*tex = getCurTexture();
	int			alphachannel = (format==DGL_RGBA) || (format==DGL_COLOR_INDEX_8_PLUS_A8);

	// Can't operate on the null texture.
	if(!tex || mipmap < 0) return DGL_FALSE;

	// Check that the texture dimensions are valid.
	if(width != Power2(width) || height != Power2(height))
		return DGL_FALSE;
	if(width > maxTexSize || height > maxTexSize)
		return DGL_FALSE;

	if(usePalTex && format == DGL_COLOR_INDEX_8) // Paletted texture?
	{
		if(mipmap == 0 || mipmap == 1)
			createMipmapSurfaces(mipmap, tex, width, height, &pfTexturePal8);
		// The data needs no converting.
		uploadImage(tex, mipmap, width, height, 1, (byte *)data);
	}
	else // Use true color textures.
	{
		int i, colorComps = alphachannel? 4 : 3;
		int numPixels = width * height;
		byte *buffer;
		byte *pixel;
		int needFree = DGL_FALSE;

		// When setting the top level texture, we'll reset the texture surfaces.
		if(mipmap == 0 || mipmap == 1)
		{
			createMipmapSurfaces(mipmap, tex, width, height, 
				alphachannel? &pfTextureRGBA : &pfTextureRGB);
		}

		if(format == DGL_RGBA)
		{
			buffer = (byte *)data;
		}
		else 
		{
			buffer = (byte *)malloc(numPixels * 4);
			needFree = DGL_TRUE;
			for(i=0, pixel=buffer; i<numPixels; i++, pixel+=4)
			{
				if(format == DGL_RGB)
				{
					memcpy(pixel, (byte*) data + i*3, 3);
					pixel[CA] = 0xff;
				}
				else if(format == DGL_COLOR_INDEX_8)
				{
					memcpy(pixel, palette[bdata[i]].color, 3);
					pixel[CA] = 0xff;
				}
				else if(format == DGL_COLOR_INDEX_8_PLUS_A8)
				{	
					memcpy(pixel, palette[bdata[i]].color, 3);
					pixel[CA] = bdata[numPixels+i];
				}
				else if(format == DGL_LUMINANCE)
				{
					pixel[CR] = pixel[CG] = pixel[CB] = bdata[i];
					pixel[CA] = 0xff;
				}
			}
		}
		uploadImage(tex, mipmap, width, height, 4, buffer);

		if(dumpTextures && !mipmap)
		{
			char fn[100];
			sprintf(fn, "%03ic%iw%03ih%03i.tga",
				currentTexName, colorComps, width, height);
			saveTGA24_rgba8888(fn, width, height, buffer);
		}

		if(needFree) free(buffer);
	}
	return DGL_OK;
	unguard;
}


void DeleteTextures(int num, DGLuint *names)
{
	guard(DeleteTextures);
	int		i;

	if(!num) return;
	// Make sure the current texture is set the zero if it's deleted.
	for(i=0; i<num; i++)
		if(currentTexName == names[i]) 
		{
			currentTexName = 0;
			break;		
		}

	// Remove each texture on the list.
	for(i=0; i<num; i++)
		delTexture(NAME_TO_IDX(names[i]));
	unguard;
}

	
void TexParam(int pname, int param)
{
	guard(TexParam);
	texture_t *tex = getCurTexture();
	D3DTEXTUREFILTER d3dFilter[] =
	{
		D3DFILTER_NEAREST,
		D3DFILTER_LINEAR,
		D3DFILTER_MIPNEAREST,
		D3DFILTER_MIPLINEAR,
		D3DFILTER_LINEARMIPNEAREST,
		D3DFILTER_LINEARMIPLINEAR
	};

	if(!tex) return;

	switch(pname)
	{
	case DGL_MIN_FILTER:
		if(param >= DGL_NEAREST && param <= DGL_LINEAR_MIPMAP_LINEAR)
			tex->minFilter = d3dFilter[param - DGL_NEAREST];
		break;

	case DGL_MAG_FILTER:
		if(param >= DGL_NEAREST && param <= DGL_LINEAR_MIPMAP_LINEAR)
			tex->magFilter = d3dFilter[param - DGL_NEAREST];
		break;

	case DGL_WRAP_S:
		tex->sAddr = param==DGL_CLAMP? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
		break;

	case DGL_WRAP_T:
		tex->tAddr = param==DGL_CLAMP? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP;
		break;
	}
	unguard;
}


void GetTexParameterv(int level, int pname, int *v)
{
	// Unimplemented. Isn't needed by the engine.
}


void Palette(int format, void *data)
{
	guard(Palette);
	unsigned char	*ptr = (byte *)data;
	int				i, size = (format==DGL_RGBA? 4 : 3);

	for(i=0; i<256; i++, ptr += size)
	{
		palette[i].color[CR] = ptr[CR];
		palette[i].color[CG] = ptr[CG];
		palette[i].color[CB] = ptr[CB];
		palette[i].color[CA] = format==DGL_RGBA? ptr[CA] : 0xff;
	}
	loadPalette();
	unguard;
}


int	Bind(DGLuint texture)
{
	guard(Bind);
	DGLuint		oldtex = currentTexName;
	texindex_t	idx = NAME_TO_IDX(texture);
	texture_t	*tex;
	int			mipMode = BTM_NONE, pixMode = BTM_NEAREST;	

	if(idx == NULL_INDEX)
	{
		IDirect3DDevice3_SetTexture(d3dDevice, 0, NULL);
		currentTexName = 0;
		return oldtex;
	}
	if(idx < 0 || idx > numTextures-1) return oldtex;
	tex = textures + idx;
	currentTexName = texture;
	IDirect3DDevice3_SetTexture(d3dDevice, 0, tex->d3dTex);
	// Set the appropriate texture rendering state.
	// First choose the appropriate mode to choose the mipmap.
	switch(tex->minFilter)
	{
	case D3DFILTER_NEAREST:
	case D3DFILTER_LINEAR:
		mipMode = BTM_NONE;
		break;

	case D3DFILTER_MIPNEAREST:
	case D3DFILTER_MIPLINEAR:
		mipMode = BTM_NEAREST;
		break;

	case D3DFILTER_LINEARMIPNEAREST:
	case D3DFILTER_LINEARMIPLINEAR:
		mipMode = BTM_LINEAR;
		break;
	}
	// Then choose the min filter.
	switch(tex->minFilter)
	{
	case D3DFILTER_NEAREST:
	case D3DFILTER_MIPNEAREST:
	case D3DFILTER_LINEARMIPNEAREST:
		pixMode = BTM_NEAREST;
		break;
		
	case D3DFILTER_LINEAR:
	case D3DFILTER_MIPLINEAR:
	case D3DFILTER_LINEARMIPLINEAR:
		pixMode = BTM_LINEAR;
		break;
	}
		
	// Set the mipmapping mode.
	SetTSS(0, D3DTSS_MIPFILTER, mipMode==BTM_NONE? D3DTFP_NONE
		: mipMode==BTM_NEAREST? D3DTFP_POINT
		: D3DTFP_LINEAR);
	// Set the min filter mode.				
	SetTSS(0, D3DTSS_MINFILTER, pixMode==BTM_NEAREST? D3DTFN_POINT : D3DTFN_LINEAR);

	// The other modes.
	IDirect3DDevice3_SetTextureStageState(d3dDevice, 0, D3DTSS_MAGFILTER, tex->magFilter);
	IDirect3DDevice3_SetTextureStageState(d3dDevice, 0, D3DTSS_ADDRESSU, tex->sAddr);
	IDirect3DDevice3_SetTextureStageState(d3dDevice, 0, D3DTSS_ADDRESSV, tex->tAddr);
	return oldtex;
	unguard;
}

} // namespace Direct3DDriver